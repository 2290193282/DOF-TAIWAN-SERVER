#include "old_data/db/event/db_glass_bottle_event.h"

#include "common/manager/db_common_public_abi.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <cstring>
#include <limits>

namespace glass_bottle_event_db {
namespace {

const char kClaimTable[] = "glass_bottle_letter_claim_0828";
const char kDayTable[] = "glass_bottle_letter_day_0828";
const uint64_t kMailNamespace = 0xD828000000000000ULL;
const uint64_t kMaximumClaimId = 0x0000FFFFFFFFFFFFULL;

const uint32_t kExpectedRewardIds[kMaximumMailRewardCount] = {
    690001534u, 1248u, 690001537u, 3290u, 3311u, 690000835u,
};

struct StoredClaim {
    StoredClaim()
        : claim_id(0ULL), receiver_charac_no(0u), activity_day(0u),
          source_slot(0u), daily_ordinal(0u), phase(PHASE_NONE) {}

    uint64_t claim_id;
    uint32_t receiver_charac_no;
    uint32_t activity_day;
    uint32_t source_slot;
    uint8_t daily_ordinal;
    uint8_t phase;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsValidMailReward(const MailReward& reward,
                       uint32_t expected_item_id) {
    return reward.item_id == expected_item_id && reward.add_info == 1u &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u;
}

bool IsValidRewards(const Message& message) {
    if (message.reward_count != kMaximumMailRewardCount) return false;
    for (unsigned int index = 0u;
         index < kMaximumMailRewardCount;
         ++index) {
        if (!IsValidMailReward(message.rewards[index],
                               kExpectedRewardIds[index])) {
            return false;
        }
    }
    return true;
}

bool IsZeroFilled(const char* text, unsigned int size) {
    if (!text) return false;
    for (unsigned int index = 0u; index < size; ++index) {
        if (text[index] != 0) return false;
    }
    return true;
}

bool IsCanonicalText(const char* text, unsigned int size) {
    if (!text || size < 2u || text[0] == 0 || text[size - 1u] != 0) {
        return false;
    }
    unsigned int terminator = 1u;
    while (terminator < size && text[terminator] != 0) ++terminator;
    if (terminator == size) return false;
    for (unsigned int index = terminator + 1u; index < size; ++index) {
        if (text[index] != 0) return false;
    }
    return true;
}

bool IsValidRequest(const Message& message) {
    if (message.account_id == 0u || message.charac_no == 0u ||
        message.session_token == 0u || message.request_token == 0u ||
        !IsValidServerGroup(message.server_group) ||
        message.event_active > 1u || message.local_mutation_done > 1u ||
        message.daily_reset_hour > 23u ||
        message.operation < OPERATION_LOAD_GLASS_BOTTLE ||
        message.operation > OPERATION_CANCEL_GLASS_BOTTLE) {
        return false;
    }
    if (message.operation == OPERATION_LOAD_GLASS_BOTTLE) {
        return message.claim_id == 0ULL && message.reward_count == 0u &&
               message.local_mutation_done == 0u &&
               IsZeroFilled(message.mail_title,
                            sizeof(message.mail_title)) &&
               IsZeroFilled(message.sender_mail_content,
                            sizeof(message.sender_mail_content)) &&
               IsZeroFilled(message.receiver_mail_content,
                            sizeof(message.receiver_mail_content));
    }
    if (!IsValidRewards(message) || message.event_active != 1u ||
        message.source_slot >= 280u ||
        !IsCanonicalText(message.mail_title,
                         sizeof(message.mail_title)) ||
        !IsCanonicalText(message.sender_mail_content,
                         sizeof(message.sender_mail_content)) ||
        !IsCanonicalText(message.receiver_mail_content,
                         sizeof(message.receiver_mail_content))) {
        return false;
    }
    if (message.operation == OPERATION_RESERVE_GLASS_BOTTLE) {
        return message.claim_id == 0ULL &&
               message.local_mutation_done == 0u;
    }
    if (message.claim_id == 0ULL) return false;
    return message.operation == OPERATION_COMMIT_GLASS_BOTTLE
               ? message.local_mutation_done == 1u
               : message.local_mutation_done == 0u;
}

MySQL* GetPartitionDatabase(int handle, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) return 0;
    const ENUM_DB_HANDLE_IDX handle_index =
        static_cast<ENUM_DB_HANDLE_IDX>(handle);
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(handle_index, group)
               : manager->GetDBHandle(handle_index, group);
}

bool Begin(MySQL* database) {
    return database && database->set_query("start transaction") &&
           database->exec(true);
}

void Rollback(MySQL* database) {
    if (database && database->set_query("rollback")) {
        (void)database->exec(true);
    }
}

bool Commit(MySQL* database) {
    return database && database->set_query("commit") &&
           database->exec(true);
}

bool LoadOwnedCharacter(MySQL* database,
                        const Message& request,
                        uint32_t& level) {
    level = 0u;
    return database &&
           database->set_query(
               "seLect lev from charac_info where charac_no=%u and m_id=%u "
               "and delete_flag=0 limit 2",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.account_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, level) &&
           level >= 50u && level <= 85u;
}

bool ReadActivityDay(MySQL* database,
                     const Message& request,
                     uint32_t& activity_day) {
    activity_day = 0u;
    return database &&
           database->set_query(
               "seLect date_format(date_sub(now(),interval %u hour),"
               "'%%Y%%m%%d')+0",
               static_cast<unsigned int>(request.daily_reset_hour)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, activity_day) &&
           activity_day != 0u;
}

bool LockDayState(MySQL* database,
                  const Message& request,
                  uint32_t activity_day,
                  uint32_t& count) {
    count = 0u;
    if (!database ||
        !database->set_query(
            "inSert into %s(server_group,m_id,charac_no,activity_day,"
            "send_count,updated_at) values(%u,%u,%u,%u,0,now()) "
            "on duplicate key update updated_at=updated_at",
            kDayTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day)) ||
        !database->exec(true)) {
        return false;
    }
    return database->set_query(
               "seLect send_count from %s where server_group=%u and m_id=%u "
               "and charac_no=%u and activity_day=%u for update",
               kDayTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(activity_day)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, count) &&
           count <= kMaximumGlassBottleSendCount;
}

bool ReadClaimActivityDay(MySQL* database,
                          const Message& request,
                          uint64_t claim_id,
                          uint32_t& activity_day) {
    activity_day = 0u;
    return database && claim_id != 0ULL &&
           database->set_query(
               "seLect activity_day from %s where claim_id=%llu and "
               "server_group=%u and m_id=%u and charac_no=%u limit 2",
               kClaimTable,
               static_cast<unsigned long long>(claim_id),
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, activity_day) &&
           activity_day != 0u;
}

bool ReadClaimForUpdate(MySQL* database,
                        const Message& request,
                        uint64_t claim_id,
                        StoredClaim& claim) {
    unsigned int ordinal = 0u;
    unsigned int phase = 0u;
    return database && claim_id != 0ULL &&
        database->set_query(
               "seLect claim_id,receiver_charac_no,activity_day,daily_ordinal,"
               "claim_state,source_slot from %s where claim_id=%llu and "
               "server_group=%u "
               "and m_id=%u and charac_no=%u for update",
               kClaimTable,
               static_cast<unsigned long long>(claim_id),
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_ulonglong(0, claim.claim_id) &&
           database->get_uint(1, claim.receiver_charac_no) &&
           database->get_uint(2, claim.activity_day) &&
           database->get_uint(3, ordinal) && ordinal >= 1u &&
           ordinal <= kMaximumGlassBottleSendCount &&
           database->get_uint(4, phase) && phase <= PHASE_CANCELLED &&
           database->get_uint(5, claim.source_slot) &&
           claim.source_slot < 280u &&
           (claim.daily_ordinal = static_cast<uint8_t>(ordinal), true) &&
           (claim.phase = static_cast<uint8_t>(phase), true);
}

// -1: database/shape failure, 0: no prior request, 1: exact request found.
int FindRequestClaimForUpdate(MySQL* database,
                              const Message& request,
                              StoredClaim& claim) {
    if (!database ||
        !database->set_query(
            "seLect claim_id,receiver_charac_no,activity_day,daily_ordinal,"
            "claim_state,source_slot from %s where server_group=%u and "
            "m_id=%u and charac_no=%u and session_token=%u and "
            "request_token=%u for update",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    unsigned int ordinal = 0u;
    unsigned int phase = 0u;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, claim.claim_id) ||
        !database->get_uint(1, claim.receiver_charac_no) ||
        !database->get_uint(2, claim.activity_day) ||
        !database->get_uint(3, ordinal) ||
        !database->get_uint(4, phase) ||
        !database->get_uint(5, claim.source_slot) ||
        claim.claim_id == 0ULL || claim.claim_id > kMaximumClaimId ||
        claim.receiver_charac_no == 0u || ordinal < 1u ||
        ordinal > kMaximumGlassBottleSendCount ||
        phase < PHASE_RESERVED || phase > PHASE_CANCELLED ||
        claim.source_slot >= 280u) {
        return -1;
    }
    claim.daily_ordinal = static_cast<uint8_t>(ordinal);
    claim.phase = static_cast<uint8_t>(phase);
    return 1;
}

bool SelectReceiver(MySQL* database, uint32_t& receiver_charac_no) {
    receiver_charac_no = 0u;
    return database &&
           database->set_query(
               "seLect charac_no from charac_info where lev between 20 and 49 "
               "and delete_flag=0 order by rand() limit 1") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_uint(0, receiver_charac_no) &&
           receiver_charac_no != 0u;
}

bool FindMail(MySQL* database,
              uint64_t delivery_key,
              uint32_t receiver,
              const MailReward& reward,
              const char* title,
              bool& found,
              uint32_t& letter_id) {
    found = false;
    letter_id = 0u;
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    if (!database || !title ||
        database->escape_string(escaped_title, title) < 0 ||
        !database->set_query(
            "seLect receive_charac_no,item_id,add_info,letter_id,"
            "length(send_charac_name),binary send_charac_name='%s',"
            "seal_flag,endurance,upgrade,amplify_option,amplify_value,"
            "creature_flag,avata_flag,extend_info,chronicle_value,"
            "seperate_upgrade from postal where auction_id=%llu limit 2",
            escaped_title,
            static_cast<unsigned long long>(delivery_key)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    unsigned int values[16] = {};
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, values[0]) ||
        !database->get_uint(1, values[1]) ||
        !database->get_uint(2, values[2]) ||
        !database->get_uint(3, values[3]) ||
        !database->get_uint(4, values[4]) ||
        !database->get_uint(5, values[5]) ||
        !database->get_uint(6, values[6]) ||
        !database->get_uint(7, values[7]) ||
        !database->get_uint(8, values[8]) ||
        !database->get_uint(9, values[9]) ||
        !database->get_uint(10, values[10]) ||
        !database->get_uint(11, values[11]) ||
        !database->get_uint(12, values[12]) ||
        !database->get_uint(13, values[13]) ||
        !database->get_uint(14, values[14]) ||
        !database->get_uint(15, values[15]) ||
        values[0] != receiver || values[1] != reward.item_id ||
        values[2] != reward.add_info || values[3] == 0u ||
        values[3] > static_cast<unsigned int>(
            std::numeric_limits<int>::max()) ||
        values[4] != std::strlen(title) || values[5] != 1u ||
        values[6] != reward.seal_flag ||
        values[7] != reward.endurance ||
        values[8] != reward.item_attr ||
        values[9] != reward.amplify_option ||
        values[10] != reward.amplify_value ||
        values[11] != reward.creature_flag ||
        values[12] != reward.avatar_flag ||
        values[13] != reward.extend_info ||
        values[14] != reward.chronicle_value ||
        values[15] != reward.separate_upgrade) {
        return false;
    }
    letter_id = values[3];
    found = true;
    return true;
}

bool ValidateLetter(MySQL* database,
                    uint32_t letter_id,
                    uint32_t receiver,
                    const char* title,
                    const char* content) {
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    if (!database || letter_id == 0u || !title || !content ||
        database->escape_string(escaped_title, title) < 0 ||
        database->escape_string(escaped_content, content) < 0 ||
        !database->set_query(
            "seLect charac_no,send_charac_no,stat,"
            "length(send_charac_name),length(letter_text),"
            "binary send_charac_name='%s',binary letter_text='%s' "
            "from letter where letter_id=%u limit 2",
            escaped_title, escaped_content,
            static_cast<unsigned int>(letter_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int values[7] = {};
    return database->get_uint(0, values[0]) &&
           database->get_uint(1, values[1]) &&
           database->get_uint(2, values[2]) &&
           database->get_uint(3, values[3]) &&
           database->get_uint(4, values[4]) &&
           database->get_uint(5, values[5]) &&
           database->get_uint(6, values[6]) &&
           values[0] == receiver && values[1] == 0u &&
           values[2] == 0u && values[3] == std::strlen(title) &&
           values[4] == std::strlen(content) &&
           values[5] == 1u && values[6] == 1u;
}

bool InsertLetter(MySQL* database,
                  uint32_t receiver,
                  const char* title,
                  const char* content,
                  uint32_t& letter_id) {
    letter_id = 0u;
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    if (!database || !title || !content ||
        database->escape_string(escaped_title, title) < 0 ||
        database->escape_string(escaped_content, content) < 0 ||
        !database->set_query(
            "inSert into letter(charac_no,send_charac_no,send_charac_name,"
            "letter_text,reg_date,stat) values(%u,0,'%s','%s',now(),0)",
            static_cast<unsigned int>(receiver), escaped_title,
            escaped_content) ||
        !database->exec(true)) {
        return false;
    }
    letter_id = WongWork::DBCommon::GetIdentity(database);
    return letter_id != 0u &&
           letter_id <= static_cast<unsigned int>(
               std::numeric_limits<int>::max());
}

bool InsertMail(MySQL* database,
                uint64_t delivery_key,
                uint32_t receiver,
                const MailReward& reward,
                const char* title,
                uint32_t letter_id) {
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    if (!database || !title || letter_id == 0u ||
        database->escape_string(escaped_title, title) < 0) {
        return false;
    }
    char* const item_guid = database->blob_to_str(
        0, const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid)));
    char* const random_option = database->blob_to_str(
        1, const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option)));
    return item_guid && random_option &&
           database->set_query(
               "inSert into postal(occ_time,send_charac_no,"
               "receive_charac_no,seal_flag,item_id,add_info,endurance,"
               "upgrade,amplify_option,amplify_value,gold,send_charac_name,"
               "creature_flag,avata_flag,letter_id,extend_info,item_guid,"
               "auction_id,random_option,chronicle_value,seperate_upgrade,"
               "unlimit_flag,type) values(now(),0,%u,%u,%u,%u,%u,%u,%u,"
               "%u,0,'%s',%u,%u,%u,%u,'%s',%llu,'%s',%u,%u,0,0)",
               static_cast<unsigned int>(receiver),
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
               static_cast<unsigned int>(letter_id),
               static_cast<unsigned int>(reward.extend_info),
               item_guid,
               static_cast<unsigned long long>(delivery_key),
               random_option,
               static_cast<unsigned int>(reward.chronicle_value),
               static_cast<unsigned int>(reward.separate_upgrade)) &&
           database->exec(true);
}

bool EnsureMail(MySQL* database,
                uint64_t delivery_key,
                uint32_t receiver,
                const MailReward& reward,
                const char* title,
                uint32_t letter_id) {
    bool found = false;
    uint32_t existing_letter_id = 0u;
    if (!FindMail(database, delivery_key, receiver, reward, title,
                  found, existing_letter_id)) {
        return false;
    }
    if (found) return existing_letter_id == letter_id;
    if (InsertMail(database, delivery_key, receiver, reward, title,
                   letter_id)) {
        return true;
    }
    return FindMail(database, delivery_key, receiver, reward, title,
                    found, existing_letter_id) && found &&
           existing_letter_id == letter_id;
}

bool EnsureRoleLetter(MySQL* database,
                      uint64_t delivery_key,
                      uint32_t receiver,
                      const MailReward& reward,
                      const char* title,
                      const char* content,
                      uint32_t& letter_id) {
    bool found = false;
    if (!FindMail(database, delivery_key, receiver, reward, title,
                  found, letter_id)) {
        return false;
    }
    if (found) {
        return ValidateLetter(database, letter_id, receiver, title, content);
    }
    return InsertLetter(database, receiver, title, content, letter_id) &&
           ValidateLetter(database, letter_id, receiver, title, content);
}

uint64_t DeliveryKey(uint64_t claim_id, unsigned int index) {
    return kMailNamespace | (claim_id << 3u) |
           static_cast<uint64_t>(index & 7u);
}

bool HandleLoad(MySQL* database,
                const Message& request,
                Message& response) {
    uint32_t level = 0u;
    uint32_t activity_day = 0u;
    uint32_t delivered = 0u;
    if (!LoadOwnedCharacter(database, request, level)) {
        response.result = RESULT_LEVEL;
        return true;
    }
    if (!Begin(database) ||
        !ReadActivityDay(database, request, activity_day) ||
        !LockDayState(database, request, activity_day, delivered) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.activity_day = activity_day;
    response.send_count = static_cast<uint8_t>(delivered);
    response.phase = PHASE_NONE;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleReserve(MySQL* database,
                   const Message& request,
                   Message& response) {
    uint32_t level = 0u;
    uint32_t activity_day = 0u;
    uint32_t receiver = 0u;
    if (!LoadOwnedCharacter(database, request, level)) {
        response.result = RESULT_LEVEL;
        return true;
    }
    if (!request.event_active) {
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (!Begin(database) ||
        !ReadActivityDay(database, request, activity_day)) {
        Rollback(database);
        return false;
    }

    uint32_t delivered = 0u;
    if (!LockDayState(database, request, activity_day, delivered)) {
        Rollback(database);
        return false;
    }

    StoredClaim replay;
    const int replay_result =
        FindRequestClaimForUpdate(database, request, replay);
    if (replay_result < 0) {
        Rollback(database);
        return false;
    }
    if (replay_result == 1) {
        if (replay.activity_day != activity_day ||
            replay.source_slot != request.source_slot ||
            replay.phase == PHASE_CANCELLED) {
            Rollback(database);
            response.result = RESULT_INVALID_STATE;
            return true;
        }
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.claim_id = replay.claim_id;
        response.receiver_charac_no = replay.receiver_charac_no;
        response.activity_day = replay.activity_day;
        response.send_count = static_cast<uint8_t>(delivered);
        response.phase = replay.phase;
        response.result = RESULT_SUCCESS;
        return true;
    }

    // A reservation cannot be timed out safely: after a process crash the DB
    // cannot prove whether the inventory mutation happened.  Keep it as a
    // durable fail-closed fence until the exact request is committed/cancelled.
    unsigned int pending = 0u;
    if (!database->set_query(
            "seLect count(*) from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and claim_state=1 "
            "for update",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, pending) ||
        pending > 1u) {
        Rollback(database);
        return false;
    }
    if (pending != 0u) {
        Rollback(database);
        response.activity_day = activity_day;
        response.result = RESULT_PENDING;
        return true;
    }

    if (delivered >= kMaximumGlassBottleSendCount) {
        Rollback(database);
        response.activity_day = activity_day;
        response.send_count = static_cast<uint8_t>(
            kMaximumGlassBottleSendCount);
        response.result = RESULT_DAILY_LIMIT;
        return true;
    }
    if (!SelectReceiver(database, receiver)) {
        Rollback(database);
        response.result = RESULT_NO_RECEIVER;
        return true;
    }

    const unsigned int ordinal = delivered + 1u;
    if (!database->set_query(
            "inSert into %s(server_group,m_id,charac_no,activity_day,"
            "daily_ordinal,receiver_charac_no,session_token,request_token,"
            "source_slot,claim_state,created_at,updated_at,delivered_at) "
            "values(%u,%u,%u,%u,%u,%u,%u,%u,%u,1,now(),now(),null) "
            "on duplicate key update claim_id=last_insert_id(claim_id)",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day), ordinal,
            static_cast<unsigned int>(receiver),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token),
            static_cast<unsigned int>(request.source_slot)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }

    if (!database->set_query(
            "seLect claim_id,receiver_charac_no,activity_day,daily_ordinal,"
            "claim_state,source_slot from %s where server_group=%u and "
            "m_id=%u and charac_no=%u and session_token=%u and "
            "request_token=%u for update",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        Rollback(database);
        return false;
    }
    StoredClaim claim;
    unsigned int stored_ordinal = 0u;
    unsigned int stored_phase = 0u;
    unsigned int stored_source_slot = 0u;
    if (!database->get_ulonglong(0, claim.claim_id) ||
        !database->get_uint(1, claim.receiver_charac_no) ||
        !database->get_uint(2, claim.activity_day) ||
        !database->get_uint(3, stored_ordinal) ||
        !database->get_uint(4, stored_phase) ||
        !database->get_uint(5, stored_source_slot) ||
        claim.claim_id == 0ULL || claim.claim_id > kMaximumClaimId ||
        stored_ordinal < 1u ||
        stored_ordinal > kMaximumGlassBottleSendCount ||
        stored_phase < PHASE_RESERVED || stored_phase > PHASE_DELIVERED ||
        stored_source_slot != request.source_slot) {
        Rollback(database);
        return false;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.claim_id = claim.claim_id;
    response.receiver_charac_no = claim.receiver_charac_no;
    response.activity_day = claim.activity_day;
    response.send_count = static_cast<uint8_t>(stored_ordinal - 1u);
    response.phase = static_cast<uint8_t>(stored_phase);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCommit(MySQL* database,
                   const Message& request,
                   Message& response) {
    uint32_t activity_day = 0u;
    if (!ReadClaimActivityDay(database, request, request.claim_id,
                              activity_day) ||
        !Begin(database)) {
        return false;
    }
    uint32_t delivered = 0u;
    if (!LockDayState(database, request, activity_day, delivered)) {
        Rollback(database);
        return false;
    }
    StoredClaim claim;
    if (!ReadClaimForUpdate(database, request, request.claim_id, claim)) {
        Rollback(database);
        return false;
    }
    if (claim.source_slot != request.source_slot) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.activity_day != activity_day) {
        Rollback(database);
        return false;
    }
    if (claim.phase == PHASE_CANCELLED) {
        Rollback(database);
        response.phase = PHASE_CANCELLED;
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.phase == PHASE_DELIVERED) {
        if (delivered < claim.daily_ordinal) {
            Rollback(database);
            return false;
        }
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.receiver_charac_no = claim.receiver_charac_no;
        response.activity_day = claim.activity_day;
        response.send_count = claim.daily_ordinal;
        response.phase = PHASE_DELIVERED;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (claim.phase != PHASE_RESERVED ||
        claim.claim_id > kMaximumClaimId ||
        delivered + 1u != claim.daily_ordinal) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }

    MySQL* const postal = GetPartitionDatabase(3, request.server_group);
    const unsigned int mail_count =
        claim.daily_ordinal == kMaximumGlassBottleSendCount
            ? static_cast<unsigned int>(kMaximumMailRewardCount) : 2u;
    if (!Begin(postal)) {
        Rollback(database);
        return false;
    }
    uint32_t receiver_letter_id = 0u;
    uint32_t sender_letter_id = 0u;
    if (!EnsureRoleLetter(postal,
                          DeliveryKey(claim.claim_id, 0u),
                          claim.receiver_charac_no,
                          request.rewards[0],
                          request.mail_title,
                          request.receiver_mail_content,
                          receiver_letter_id) ||
        !EnsureRoleLetter(postal,
                          DeliveryKey(claim.claim_id, 1u),
                          request.charac_no,
                          request.rewards[1],
                          request.mail_title,
                          request.sender_mail_content,
                          sender_letter_id)) {
        Rollback(postal);
        Rollback(database);
        return false;
    }
    for (unsigned int index = 0u; index < mail_count; ++index) {
        const uint32_t receiver =
            index == 0u ? claim.receiver_charac_no : request.charac_no;
        const uint32_t letter_id =
            index == 0u ? receiver_letter_id : sender_letter_id;
        if (!EnsureMail(postal,
                        DeliveryKey(claim.claim_id, index),
                        receiver,
                        request.rewards[index],
                        request.mail_title,
                        letter_id)) {
            Rollback(postal);
            Rollback(database);
            return false;
        }
    }
    if (!Commit(postal)) {
        Rollback(postal);
        Rollback(database);
        return false;
    }
    if (!database->set_query(
            "upDate %s set claim_state=2,updated_at=now(),delivered_at=now() "
            "where claim_id=%llu and claim_state=1",
            kClaimTable,
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set send_count=%u,updated_at=now() where "
            "server_group=%u and m_id=%u and charac_no=%u and "
            "activity_day=%u and send_count=%u",
            kDayTable,
            static_cast<unsigned int>(claim.daily_ordinal),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(claim.activity_day),
            static_cast<unsigned int>(delivered)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.receiver_charac_no = claim.receiver_charac_no;
    response.activity_day = claim.activity_day;
    response.send_count = claim.daily_ordinal;
    response.phase = PHASE_DELIVERED;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCancel(MySQL* database,
                   const Message& request,
                   Message& response) {
    uint32_t activity_day = 0u;
    if (!ReadClaimActivityDay(database, request, request.claim_id,
                              activity_day) ||
        !Begin(database)) {
        return false;
    }
    uint32_t delivered = 0u;
    if (!LockDayState(database, request, activity_day, delivered)) {
        Rollback(database);
        return false;
    }
    StoredClaim claim;
    if (!ReadClaimForUpdate(database, request, request.claim_id, claim)) {
        Rollback(database);
        return false;
    }
    if (claim.source_slot != request.source_slot) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.activity_day != activity_day ||
        (claim.phase == PHASE_RESERVED &&
         delivered + 1u != claim.daily_ordinal)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.phase == PHASE_DELIVERED) {
        Rollback(database);
        response.phase = PHASE_DELIVERED;
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.phase == PHASE_RESERVED &&
        (!database->set_query(
             "upDate %s set claim_state=3,updated_at=now() where "
             "claim_id=%llu and claim_state=1",
             kClaimTable,
             static_cast<unsigned long long>(claim.claim_id)) ||
         !database->exec(true) || database->getAffectedRowCount() != 1u)) {
        Rollback(database);
        return false;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.activity_day = claim.activity_day;
    response.phase = PHASE_CANCELLED;
    response.result = RESULT_SUCCESS;
    return true;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) return false;
    *destination = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

DBGlassBottleEvent::DBGlassBottleEvent()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBGlassBottleEvent::~DBGlassBottleEvent() {}

bool DBGlassBottleEvent::QueueRequest(int user_idx,
                                          const Message& request) {
    if (user_idx < 0 || !IsValidRequest(request)) return false;
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) return false;
    *destination = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBGlassBottleEvent::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    if (user_idx < 0 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request)) {
        return 0;
    }
    Message response = request;
    response.result = RESULT_DATABASE_FAILURE;
    MySQL* const database = GetPartitionDatabase(2, request.server_group);
    bool handled = false;
    if (database) {
        switch (request.operation) {
            case OPERATION_LOAD_GLASS_BOTTLE:
                handled = HandleLoad(database, request, response);
                break;
            case OPERATION_RESERVE_GLASS_BOTTLE:
                handled = HandleReserve(database, request, response);
                break;
            case OPERATION_COMMIT_GLASS_BOTTLE:
                handled = HandleCommit(database, request, response);
                break;
            case OPERATION_CANCEL_GLASS_BOTTLE:
                handled = HandleCancel(database, request, response);
                break;
            default:
                break;
        }
    }
    if (!handled) response.result = RESULT_DATABASE_FAILURE;
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace tw_event_db_0828
