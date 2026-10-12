#include "old_data/db/event/db_levelup_support_3rd.h"

#include "common/manager/db_common_public_abi.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstring>
#include <limits>

namespace levelup_support_3rd_db {
namespace {

const char kStateTable[] = "charac_levelup_support_3rd_0828";
const int kServerGroupCount = 0x12;
const unsigned long long kDeliveryNamespace = 0xD300000000000000ULL;

const uint8_t kSupportLevels[] = {
    5, 10, 15, 20, 25, 28, 30, 35, 40, 45,
    48, 50, 53, 55, 58, 60, 63, 65, 68, 70,
    72, 74, 76, 78, 80, 81, 82, 83, 84, 85,
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kServerGroupCount;
}

bool IsSupportLevel(uint32_t level) {
    for (unsigned int index = 0;
         index < sizeof(kSupportLevels) / sizeof(kSupportLevels[0]);
         ++index) {
        if (level == kSupportLevels[index]) {
            return true;
        }
    }
    return false;
}

uint32_t NextSupportLevel(uint32_t level) {
    if (level == 0u) {
        return kSupportLevels[0];
    }
    for (unsigned int index = 0;
         index + 1u < sizeof(kSupportLevels) / sizeof(kSupportLevels[0]);
         ++index) {
        if (level == kSupportLevels[index]) {
            return kSupportLevels[index + 1u];
        }
    }
    return 0u;
}

unsigned long long DeliveryKey(uint32_t charac_no,
                               uint32_t level,
                               uint8_t reward_index) {
    return kDeliveryNamespace |
           (static_cast<unsigned long long>(charac_no) << 16) |
           (static_cast<unsigned long long>(level) << 8) |
           static_cast<unsigned long long>(reward_index);
}

MySQL* GetDatabase(int handle, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) {
        return 0;
    }
    const ENUM_DB_HANDLE_IDX handle_index =
        static_cast<ENUM_DB_HANDLE_IDX>(handle);
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    // DBMgr::GetDBHandle falls back to DB_HANDLE_LOG when active-global mode
    // has no DB_HANDLE_2/3 mapping for the requested group.  That fallback is
    // appropriate for legacy best-effort callers, but never for a reward
    // transaction.  Require the explicit partition handle and fail closed;
    // single-database deployments retain the canonical primary handle path.
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(handle_index, group)
               : manager->GetDBHandle(handle_index, group);
}

bool Rollback(MySQL* database) {
    return database && database->set_query("rollback") &&
           database->exec(true);
}

bool Commit(MySQL* database) {
    return database && database->set_query("commit") &&
           database->exec(true);
}

bool IsOwnedCharacter(MySQL* game_database, const Message& message) {
    if (!game_database ||
        !game_database->set_query(
            "seLect 1 from charac_info where charac_no=%u and m_id=%u "
            "and delete_flag=0 limit 2",
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(message.account_id)) ||
        !game_database->exec(true)) {
        return false;
    }
    return game_database->get_n_rows() == 1 && game_database->fetch();
}

bool IsValidReward(const MailReward& reward) {
    return reward.item_id != 0u && reward.add_info != 0u &&
           reward.add_info <=
               static_cast<uint32_t>(std::numeric_limits<int>::max()) &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u;
}

bool IsValidClaim(const Message& message) {
    if (!IsSupportLevel(message.requested_level) ||
        message.character_level < message.requested_level ||
        message.reward_count == 0u ||
        message.reward_count > kMaximumRewardCount ||
        message.mail_title[kMailTitleBytes] != 0 ||
        message.mail_content[kMailContentBytes] != 0) {
        return false;
    }
    for (unsigned int index = 0; index < message.reward_count; ++index) {
        if (!IsValidReward(message.rewards[index])) {
            return false;
        }
    }
    return true;
}

bool LoadState(MySQL* game_database, Message& response) {
    if (!IsOwnedCharacter(game_database, response) ||
        !game_database->set_query(
            "seLect last_claimed_level,claim_revision from %s "
            "where charac_no=%u",
            kStateTable,
            static_cast<unsigned int>(response.charac_no)) ||
        !game_database->exec(true)) {
        return false;
    }
    const int rows = game_database->get_n_rows();
    if (rows == 0) {
        response.last_claimed_level = 0u;
        response.claim_revision = 0u;
        response.result = static_cast<uint8_t>(kResultOk);
        return true;
    }
    unsigned int last_level = 0u;
    unsigned int revision = 0u;
    if (rows != 1 || !game_database->fetch() ||
        !game_database->get_uint(0, last_level) ||
        !game_database->get_uint(1, revision) ||
        (last_level != 0u && !IsSupportLevel(last_level)) ||
        (last_level == 0u && revision != 0u) ||
        (last_level != 0u && revision == 0u)) {
        return false;
    }
    response.last_claimed_level = static_cast<uint32_t>(last_level);
    response.claim_revision = static_cast<uint32_t>(revision);
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool FindDelivery(MySQL* mail_database,
                  const Message& message,
                  unsigned int reward_index,
                  bool& found) {
    found = false;
    const MailReward& reward = message.rewards[reward_index];
    const unsigned long long delivery_key = DeliveryKey(
        message.charac_no,
        message.requested_level,
        static_cast<uint8_t>(reward_index));
    if (!mail_database ||
        !mail_database->set_query(
            "seLect item_id,add_info from postal where auction_id=%llu "
            "and receive_charac_no=%u limit 2",
            delivery_key,
            static_cast<unsigned int>(message.charac_no)) ||
        !mail_database->exec(true)) {
        return false;
    }
    const int rows = mail_database->get_n_rows();
    if (rows == 0) {
        return true;
    }
    unsigned int item_id = 0u;
    unsigned int add_info = 0u;
    if (rows != 1 || !mail_database->fetch() ||
        !mail_database->get_uint(0, item_id) ||
        !mail_database->get_uint(1, add_info) ||
        item_id != reward.item_id || add_info != reward.add_info) {
        return false;
    }
    found = true;
    return true;
}

bool ValidateDeliveryCardinality(MySQL* mail_database,
                                 const Message& message) {
    const unsigned long long first = DeliveryKey(
        message.charac_no, message.requested_level, 0u);
    const unsigned long long last = first + 255ULL;
    if (!mail_database ||
        !mail_database->set_query(
            "seLect count(*) from postal where receive_charac_no=%u "
            "and auction_id between %llu and %llu",
            static_cast<unsigned int>(message.charac_no),
            first,
            last) ||
        !mail_database->exec(true) ||
        mail_database->get_n_rows() != 1 || !mail_database->fetch()) {
        return false;
    }
    unsigned int count = 0u;
    return mail_database->get_uint(0, count) &&
           count <= static_cast<unsigned int>(message.reward_count);
}

bool InsertLetter(MySQL* mail_database,
                  const Message& message,
                  int& letter_id) {
    letter_id = 0;
    if (message.mail_content[0] == 0) {
        return true;
    }
    char escaped_title[(kMailTitleBytes + 1) * 2 + 1] = {};
    char escaped_content[(kMailContentBytes + 1) * 2 + 1] = {};
    if (!mail_database ||
        mail_database->escape_string(escaped_title,
                                     message.mail_title) < 0 ||
        mail_database->escape_string(escaped_content,
                                     message.mail_content) < 0 ||
        !mail_database->set_query(
            "inSert into letter(charac_no,send_charac_no,send_charac_name,"
            "letter_text,reg_date,stat) values(%u,0,'%s','%s',now(),0)",
            static_cast<unsigned int>(message.charac_no),
            escaped_title,
            escaped_content) ||
        !mail_database->exec(true)) {
        return false;
    }
    letter_id = static_cast<int>(
        WongWork::DBCommon::GetIdentity(mail_database));
    return letter_id > 0;
}

bool InsertPostal(MySQL* mail_database,
                  const Message& message,
                  unsigned int reward_index,
                  int letter_id) {
    const MailReward& reward = message.rewards[reward_index];
    char escaped_title[(kMailTitleBytes + 1) * 2 + 1] = {};
    if (!mail_database ||
        mail_database->escape_string(escaped_title,
                                     message.mail_title) < 0) {
        return false;
    }
    char* const item_guid = mail_database->blob_to_str(
        0, const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid)));
    char* const random_option = mail_database->blob_to_str(
        1, const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option)));
    const unsigned long long delivery_key = DeliveryKey(
        message.charac_no,
        message.requested_level,
        static_cast<uint8_t>(reward_index));
    if (!item_guid || !random_option ||
        !mail_database->set_query(
            "inSert into postal(occ_time,send_charac_no,receive_charac_no,"
            "seal_flag,item_id,add_info,endurance,upgrade,amplify_option,"
            "amplify_value,gold,send_charac_name,creature_flag,avata_flag,"
            "letter_id,extend_info,item_guid,auction_id,random_option,"
            "chronicle_value,seperate_upgrade,unlimit_flag,type) values("
            "now(),0,%u,%u,%u,%u,%u,%u,%u,%u,0,'%s',%u,%u,%d,%u,"
            "'%s',%llu,'%s',%u,%u,0,0)",
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(reward.seal_flag),
            static_cast<unsigned int>(reward.item_id),
            static_cast<unsigned int>(reward.add_info),
            static_cast<unsigned int>(reward.endurance),
            static_cast<unsigned int>(reward.item_attr),
            static_cast<unsigned int>(reward.amplify_option),
            static_cast<unsigned int>(reward.amplify_value),
            escaped_title,
            static_cast<unsigned int>(reward.creature_flag),
            static_cast<unsigned int>(reward.avatar_flag),
            letter_id,
            static_cast<unsigned int>(reward.extend_info),
            item_guid,
            delivery_key,
            random_option,
            static_cast<unsigned int>(reward.chronicle_value),
            static_cast<unsigned int>(reward.separate_upgrade)) ||
        !mail_database->exec(true)) {
        return false;
    }
    return true;
}

bool DeliverRewards(MySQL* mail_database, const Message& message) {
    if (!ValidateDeliveryCardinality(mail_database, message)) {
        return false;
    }
    int letter_id = -1;
    for (unsigned int index = 0; index < message.reward_count; ++index) {
        bool found = false;
        if (!FindDelivery(mail_database, message, index, found)) {
            return false;
        }
        if (found) {
            continue;
        }
        if (letter_id < 0 &&
            !InsertLetter(mail_database, message, letter_id)) {
            return false;
        }
        if (!InsertPostal(mail_database, message, index, letter_id)) {
            return false;
        }
    }
    return true;
}

bool Claim(MySQL* game_database,
           MySQL* mail_database,
           Message& response) {
    if (!IsValidClaim(response)) {
        response.result = static_cast<uint8_t>(kResultRewardUnavailable);
        return true;
    }
    if (!IsOwnedCharacter(game_database, response)) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        return true;
    }
    if (!game_database->set_query("start transaction") ||
        !game_database->exec(true) ||
        !game_database->set_query(
            "inSert ignore into %s(charac_no,last_claimed_level,"
            "claim_revision,created_at) values(%u,0,0,now())",
            kStateTable,
            static_cast<unsigned int>(response.charac_no)) ||
        !game_database->exec(true) ||
        !game_database->set_query(
            "seLect last_claimed_level,claim_revision from %s "
            "where charac_no=%u for update",
            kStateTable,
            static_cast<unsigned int>(response.charac_no)) ||
        !game_database->exec(true) ||
        game_database->get_n_rows() != 1 || !game_database->fetch()) {
        Rollback(game_database);
        return false;
    }

    unsigned int last_level = 0u;
    unsigned int revision = 0u;
    if (!game_database->get_uint(0, last_level) ||
        !game_database->get_uint(1, revision) ||
        (last_level != 0u && !IsSupportLevel(last_level)) ||
        (last_level == 0u && revision != 0u) ||
        (last_level != 0u && revision == 0u)) {
        Rollback(game_database);
        return false;
    }

    response.last_claimed_level = static_cast<uint32_t>(last_level);
    response.claim_revision = static_cast<uint32_t>(revision);
    if (response.requested_level == last_level) {
        response.replayed = 1u;
        response.result = static_cast<uint8_t>(kResultOk);
        if (!Commit(game_database)) {
            Rollback(game_database);
            return false;
        }
        return true;
    }
    if (response.requested_level != NextSupportLevel(last_level)) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        Rollback(game_database);
        return true;
    }

    if (!DeliverRewards(mail_database, response) ||
        revision == std::numeric_limits<uint32_t>::max() ||
        !game_database->set_query(
            "upDate %s set last_claimed_level=%u,claim_revision=%u "
            "where charac_no=%u and last_claimed_level=%u and "
            "claim_revision=%u",
            kStateTable,
            static_cast<unsigned int>(response.requested_level),
            static_cast<unsigned int>(revision + 1u),
            static_cast<unsigned int>(response.charac_no),
            static_cast<unsigned int>(last_level),
            static_cast<unsigned int>(revision)) ||
        !game_database->exec(true) ||
        game_database->getAffectedRowCount() != 1u ||
        !Commit(game_database)) {
        Rollback(game_database);
        return false;
    }

    response.last_claimed_level = response.requested_level;
    response.claim_revision = static_cast<uint32_t>(revision + 1u);
    response.replayed = 0u;
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

void ClearMessage(Message& message) {
    std::memset(&message, 0, sizeof(message));
    message.server_group = -1;
    message.result = static_cast<uint8_t>(kResultOperationFailed);
}

bool Queue(int user_idx, const Message& message) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || message.account_id == 0u ||
        message.charac_no == 0u || message.session_token == 0u ||
        !IsValidServerGroup(message.server_group) || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = message;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

DB_LevelupSupport3rd::DB_LevelupSupport3rd()
    : DBDispatcher() {}

DB_LevelupSupport3rd::~DB_LevelupSupport3rd() {}

bool DB_LevelupSupport3rd::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || request->account_id == 0u ||
        request->charac_no == 0u || request->session_token == 0u ||
        !IsValidServerGroup(request->server_group)) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultOperationFailed);
    response.replayed = 0u;
    MySQL* const game_database = GetDatabase(DB_HANDLE_2,
                                             request->server_group);
    bool handled = false;
    if (request->operation == kOperationLoad) {
        handled = LoadState(game_database, response);
    } else if (request->operation == kOperationClaim) {
        MySQL* const mail_database = GetDatabase(DB_HANDLE_3,
                                                 request->server_group);
        handled = game_database && mail_database &&
                  Claim(game_database, mail_database, response);
    }
    if (!handled) {
        response.result = static_cast<uint8_t>(kResultOperationFailed);
    }
    return SendResult(user_idx, response);
}

}  // namespace levelup_support_3rd_db_0828
