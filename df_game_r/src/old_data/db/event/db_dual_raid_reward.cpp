#include "old_data/db/event/db_dual_raid_reward.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

namespace dual_raid {
namespace {

const char kClaimTable[] = "dual_raid_reward_claim_0828";
const char kMailTitle[] = "Dual Raid Reward";
const unsigned long long kMailNamespace = 0xD280000000000000ULL;
const unsigned long long kMailPayloadMask = 0x000FFFFFFFFFFFFFULL;

bool IsValidServerGroup(int group) {
    return group >= 0 && group < kMaximumServerGroupCount;
}

bool IsValidRequest(const RewardClaimMessage& request, int user_idx) {
    return request.operation == REWARD_OPERATION_CLAIM && user_idx >= 0 &&
           request.reply_user_idx == user_idx &&
           request.account_id != 0u && request.charac_no != 0u &&
           request.raid_token_hi != 0u && request.raid_token_lo != 0u &&
           request.request_token != 0u && request.reward_item_id != 0u &&
           request.reward_item_id == kOfficialRewardItemId &&
           request.reward_item_count ==
               (request.clear_result ? kOfficialClearRewardCount
                                     : kOfficialFailRewardCount) &&
           request.reward_exp > 0u &&
           request.reward_exp <= kMaximumRewardExp &&
           IsValidServerGroup(request.server_group) &&
           request.team_index < kTeamCount && request.clear_result <= 1u &&
           request.result_code == 0u && request.delivery_key_hi == 0u &&
           request.delivery_key_lo == 0u && request.reserved[0] == 0u &&
           request.reserved[1] == 0u && request.reserved[2] == 0u &&
           request.reserved[3] == 0u;
}

MySQL* GetPartitionDatabase(int handle_index, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) return 0;
    const ENUM_DB_HANDLE_IDX handle =
        static_cast<ENUM_DB_HANDLE_IDX>(handle_index);
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(handle, group)
               : manager->GetDBHandle(handle, group);
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

bool CharacterBelongsToAccount(MySQL* database,
                               const RewardClaimMessage& request) {
    unsigned int charac_no = 0u;
    return database &&
           database->set_query(
               "seLect charac_no from charac_info where charac_no=%u "
               "and m_id=%u and delete_flag=0 limit 2",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.account_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, charac_no) &&
           charac_no == request.charac_no;
}

struct StoredClaim {
    StoredClaim()
        : claim_id(0ULL), account_id(0u), team_index(0u), clear_result(0u),
          reward_item_id(0u), reward_item_count(0u), reward_exp(0u),
          claim_state(0u), delivery_key(0ULL) {}

    unsigned long long claim_id;
    uint32_t account_id;
    uint32_t team_index;
    uint32_t clear_result;
    uint32_t reward_item_id;
    uint32_t reward_item_count;
    uint32_t reward_exp;
    uint32_t claim_state;
    unsigned long long delivery_key;
};

bool SelectClaimForUpdate(MySQL* database,
                          const RewardClaimMessage& request,
                          StoredClaim& claim) {
    return database &&
           database->set_query(
               "seLect claim_id,m_id,team_index,clear_result,reward_item_id,"
               "reward_item_count,reward_exp,claim_state,delivery_key from %s "
               "where server_group=%u and raid_token_hi=%u and "
               "raid_token_lo=%u and charac_no=%u for update",
               kClaimTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.raid_token_hi),
               static_cast<unsigned int>(request.raid_token_lo),
               static_cast<unsigned int>(request.charac_no)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_ulonglong(0, claim.claim_id) &&
           database->get_uint(1, claim.account_id) &&
           database->get_uint(2, claim.team_index) &&
           database->get_uint(3, claim.clear_result) &&
           database->get_uint(4, claim.reward_item_id) &&
           database->get_uint(5, claim.reward_item_count) &&
           database->get_uint(6, claim.reward_exp) &&
           database->get_uint(7, claim.claim_state) &&
           database->get_ulonglong(8, claim.delivery_key);
}

bool Matches(const StoredClaim& claim,
             const RewardClaimMessage& request) {
    return claim.claim_id != 0ULL &&
           claim.account_id == request.account_id &&
           claim.team_index == request.team_index &&
           claim.clear_result == request.clear_result &&
           claim.reward_item_id == request.reward_item_id &&
           claim.reward_item_count == request.reward_item_count &&
           claim.reward_exp == request.reward_exp &&
           claim.claim_state <= 1u;
}

bool ReserveClaim(MySQL* database,
                  const RewardClaimMessage& request,
                  StoredClaim& claim) {
    if (!Begin(database)) return false;
    if (!database->set_query(
            "inSert into %s(server_group,raid_token_hi,raid_token_lo,"
            "charac_no,m_id,team_index,clear_result,reward_item_id,"
            "reward_item_count,reward_exp,claim_state,delivery_key,"
            "created_at,updated_at) values(%u,%u,%u,%u,%u,%u,%u,%u,%u,"
            "%u,0,0,now(),now()) on duplicate key update "
            "claim_id=last_insert_id(claim_id)",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.raid_token_hi),
            static_cast<unsigned int>(request.raid_token_lo),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.team_index),
            static_cast<unsigned int>(request.clear_result),
            static_cast<unsigned int>(request.reward_item_id),
            static_cast<unsigned int>(request.reward_item_count),
            static_cast<unsigned int>(request.reward_exp)) ||
        !database->exec(true) ||
        !SelectClaimForUpdate(database, request, claim) ||
        !Matches(claim, request)) {
        Rollback(database);
        return false;
    }

    if (claim.delivery_key == 0ULL) {
        if (claim.claim_id > kMailPayloadMask) {
            Rollback(database);
            return false;
        }
        claim.delivery_key = kMailNamespace | claim.claim_id;
        if (!database->set_query(
                "upDate %s set delivery_key=%llu,updated_at=now() where "
                "claim_id=%llu and delivery_key=0",
                kClaimTable, claim.delivery_key, claim.claim_id) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return false;
        }
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    return true;
}

bool FindMail(MySQL* database,
              unsigned long long delivery_key,
              const RewardClaimMessage& request,
              bool& found) {
    found = false;
    if (!database ||
        !database->set_query(
            "seLect receive_charac_no,item_id,add_info from postal where "
            "auction_id=%llu limit 2", delivery_key) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    unsigned int receiver = 0u;
    unsigned int item_id = 0u;
    unsigned int add_info = 0u;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, receiver) ||
        !database->get_uint(1, item_id) ||
        !database->get_uint(2, add_info) ||
        receiver != request.charac_no ||
        item_id != request.reward_item_id ||
        add_info != request.reward_item_count) {
        return false;
    }
    found = true;
    return true;
}

bool InsertMail(MySQL* database,
                unsigned long long delivery_key,
                const RewardClaimMessage& request) {
    uint8_t item_guid[10] = {};
    uint8_t random_option[14] = {};
    char escaped_title[sizeof(kMailTitle) * 2u + 1u] = {};
    if (!database ||
        database->escape_string(escaped_title, kMailTitle) < 0) {
        return false;
    }
    char* const guid = database->blob_to_str(
        0, item_guid, static_cast<int>(sizeof(item_guid)));
    char* const random = database->blob_to_str(
        1, random_option, static_cast<int>(sizeof(random_option)));
    return guid && random &&
           database->set_query(
               "inSert into postal(occ_time,send_charac_no,"
               "receive_charac_no,seal_flag,item_id,add_info,endurance,"
               "upgrade,amplify_option,amplify_value,gold,send_charac_name,"
               "creature_flag,avata_flag,letter_id,extend_info,item_guid,"
               "auction_id,random_option,chronicle_value,seperate_upgrade,"
               "unlimit_flag,type) values(now(),0,%u,0,%u,%u,0,0,0,0,0,"
               "'%s',0,0,0,0,'%s',%llu,'%s',0,0,0,0)",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.reward_item_id),
               static_cast<unsigned int>(request.reward_item_count),
               escaped_title, guid, delivery_key, random) &&
           database->exec(true);
}

bool EnsureMail(MySQL* database,
                unsigned long long delivery_key,
                const RewardClaimMessage& request) {
    bool found = false;
    if (!FindMail(database, delivery_key, request, found)) return false;
    if (found) return true;
    if (InsertMail(database, delivery_key, request)) return true;
    return FindMail(database, delivery_key, request, found) && found;
}

bool DeliverClaim(MySQL* claim_database,
                  MySQL* mail_database,
                  const RewardClaimMessage& request,
                  StoredClaim& claim,
                  bool& already_delivered) {
    already_delivered = false;
    if (!Begin(claim_database)) return false;
    StoredClaim locked;
    if (!SelectClaimForUpdate(claim_database, request, locked) ||
        !Matches(locked, request) || locked.delivery_key == 0ULL) {
        Rollback(claim_database);
        return false;
    }
    claim = locked;
    if (locked.claim_state == 1u) {
        already_delivered = true;
        if (!Commit(claim_database)) {
            Rollback(claim_database);
            return false;
        }
        return true;
    }

    // The reservation was committed before this second locking transaction.
    // Holding its row lock across the MyISAM postal reconcile serializes
    // concurrent claims.  If the process dies after postal INSERT but before
    // this COMMIT, a retry sees the same deterministic auction_id and only
    // advances the still-pending InnoDB row.
    if (!EnsureMail(mail_database, locked.delivery_key, request) ||
        !claim_database->set_query(
            "upDate %s set claim_state=1,delivered_at=now(),updated_at=now() "
            "where claim_id=%llu and claim_state=0 and delivery_key=%llu",
            kClaimTable, locked.claim_id, locked.delivery_key) ||
        !claim_database->exec(true) ||
        claim_database->getAffectedRowCount() != 1u ||
        !Commit(claim_database)) {
        Rollback(claim_database);
        return false;
    }
    return true;
}

bool QueueResponse(int user_idx, const RewardClaimMessage& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    RewardClaimMessage* const destination =
        reinterpret_cast<RewardClaimMessage*>(guard.GetInBufferUntyped(
            static_cast<int>(sizeof(RewardClaimMessage))));
    if (!destination) return false;
    *destination = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

DBDualRaidReward::DBDualRaidReward()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBDualRaidReward::~DBDualRaidReward() {}

bool DBDualRaidReward::QueueRequest(
    int user_idx, const RewardClaimMessage& request) {
    if (!IsValidRequest(request, user_idx)) return false;
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    RewardClaimMessage* const destination =
        reinterpret_cast<RewardClaimMessage*>(guard.GetInBufferUntyped(
            static_cast<int>(sizeof(RewardClaimMessage))));
    if (!destination) return false;
    *destination = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBDualRaidReward::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    if (!stream ||
        stream->length() != static_cast<int>(sizeof(RewardClaimMessage))) {
        return 0;
    }
    RewardClaimMessage request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request, user_idx)) {
        return 0;
    }

    RewardClaimMessage response = request;
    response.result_code = REWARD_RESULT_DATABASE;
    StoredClaim claim;
    MySQL* const claim_database =
        GetPartitionDatabase(2, request.server_group);
    MySQL* const mail_database =
        GetPartitionDatabase(3, request.server_group);
    bool already_delivered = false;
    if (claim_database && mail_database &&
        CharacterBelongsToAccount(claim_database, request) &&
        ReserveClaim(claim_database, request, claim) &&
        DeliverClaim(claim_database, mail_database, request, claim,
                     already_delivered)) {
        response.result_code = already_delivered
            ? REWARD_RESULT_ALREADY_DELIVERED : REWARD_RESULT_SUCCESS;
        response.delivery_key_hi =
            static_cast<uint32_t>(claim.delivery_key >> 32u);
        response.delivery_key_lo =
            static_cast<uint32_t>(claim.delivery_key & 0xffffffffULL);
    }
    (void)QueueResponse(user_idx, response);
    return 1;
}

}  // namespace dual_raid_0828
