#include "old_data/db/event/db_racing_dungeon.h"

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

namespace racing_dungeon {
namespace {

const char kClaimTable[] = "racing_dungeon_claim_0828";
const char kMailTitle[] = "Racing Dungeon Reward";
const unsigned long long kMailNamespace = 0xD824000000000000ULL;
const unsigned long long kMailPayloadMask = 0x0000FFFFFFFFFFFFULL;

enum ReserveResult {
    RESERVE_OK = 0,
    RESERVE_COLLISION = 1,
    RESERVE_DATABASE = 2,
};

struct StoredClaim {
    StoredClaim()
        : claim_id(0ULL), account_id(0u), member_unique_id(0u),
          finish_rank(0u), finish_time_ms(0u), reward_item_id(0u),
          reward_count(0u), claim_state(0u), delivery_key(0ULL) {}

    unsigned long long claim_id;
    uint32_t account_id;
    uint32_t member_unique_id;
    uint32_t finish_rank;
    uint32_t finish_time_ms;
    uint32_t reward_item_id;
    uint32_t reward_count;
    uint32_t claim_state;
    unsigned long long delivery_key;
};

struct StoredEntry {
    StoredEntry()
        : activity_day(0u), entry_ordinal(0u), server_group(0u),
          charac_no(0u), account_id(0u), entry_state(0u) {}

    uint32_t activity_day;
    uint32_t entry_ordinal;
    uint32_t server_group;
    uint32_t charac_no;
    uint32_t account_id;
    uint32_t entry_state;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 &&
           server_group < kMaximumServerGroupCount;
}

uint8_t RewardCountForRank(uint8_t rank) {
    static const uint8_t counts[kMaximumRacers] = {3u, 2u, 1u, 0u};
    return rank >= 1u && rank <= kMaximumRacers
               ? counts[rank - 1u] : 0xffu;
}

bool IsZeroTail(const RewardClaimMessage& message) {
    return message.reserved[0] == 0u && message.reserved[1] == 0u;
}

bool IsValidRequest(const RewardClaimMessage& request, int user_idx) {
    if (user_idx < 0 || request.reply_user_idx != user_idx ||
        request.account_id == 0u || request.charac_no == 0u ||
        request.entry_token_hi == 0u || request.entry_token_lo == 0u ||
        request.request_token == 0u ||
        !IsValidServerGroup(request.server_group) ||
        request.member_unique_id == 0xffffu ||
        request.result_code != 0u ||
        request.claim_state != REWARD_CLAIM_PENDING ||
        request.replayed != 0u || request.delivery_key_hi != 0u ||
        request.delivery_key_lo != 0u || !IsZeroTail(request)) {
        return false;
    }
    if (request.operation == REWARD_OPERATION_RESERVE_ENTRY) {
        return request.session_token_hi == 0u &&
               request.session_token_lo == 0u &&
               request.finish_time_ms == 0u &&
               request.reward_item_id == 0u &&
               request.activity_day == 0u && request.entry_ordinal == 0u &&
               request.finish_rank == 0u && request.reward_count == 0u;
    }
    if (request.operation == REWARD_OPERATION_CANCEL_ENTRY) {
        return request.session_token_hi == 0u &&
               request.session_token_lo == 0u &&
               request.finish_time_ms == 0u &&
               request.reward_item_id == 0u &&
               request.activity_day != 0u &&
               request.entry_ordinal >= 1u &&
               request.entry_ordinal <= kOfficialEntryConditionCount &&
               request.finish_rank == 0u && request.reward_count == 0u;
    }
    return request.operation == REWARD_OPERATION_CLAIM &&
           request.session_token_hi != 0u &&
           request.session_token_lo != 0u &&
           request.activity_day != 0u &&
           request.entry_ordinal >= 1u &&
           request.entry_ordinal <= kOfficialEntryConditionCount &&
           request.finish_time_ms > 0u &&
           request.finish_time_ms <= kMaximumFinishTimeMs &&
           request.reward_item_id == kOfficialRewardItemId &&
           request.finish_rank >= 1u &&
           request.finish_rank <= kMaximumRacers &&
           request.reward_count == RewardCountForRank(request.finish_rank);
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
    unsigned int ignored = 0u;
    return database &&
           database->set_query(
               "seLect charac_no from charac_info where charac_no=%u "
               "and m_id=%u and delete_flag=0 limit 2",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.account_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, ignored) &&
           ignored == request.charac_no;
}

bool ReadActivityDay(MySQL* database, uint32_t& activity_day) {
    activity_day = 0u;
    return database &&
           database->set_query(
               "seLect cast(date_format(curdate(),'%%Y%%m%%d') as unsigned)") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, activity_day) &&
           activity_day != 0u;
}

bool SelectEntryForUpdate(MySQL* database,
                          const RewardClaimMessage& request,
                          StoredEntry& entry) {
    return database &&
           database->set_query(
               "seLect activity_day,entry_ordinal,server_group,charac_no,"
               "m_id,entry_state from racing_dungeon_entry_0828 where "
               "entry_token_hi=%u and entry_token_lo=%u for update",
               static_cast<unsigned int>(request.entry_token_hi),
               static_cast<unsigned int>(request.entry_token_lo)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_uint(0, entry.activity_day) &&
           database->get_uint(1, entry.entry_ordinal) &&
           database->get_uint(2, entry.server_group) &&
           database->get_uint(3, entry.charac_no) &&
           database->get_uint(4, entry.account_id) &&
           database->get_uint(5, entry.entry_state);
}

bool EntryIdentityMatches(const StoredEntry& entry,
                          const RewardClaimMessage& request) {
    return entry.server_group ==
               static_cast<uint32_t>(request.server_group) &&
           entry.charac_no == request.charac_no &&
           entry.account_id == request.account_id && entry.entry_state <= 1u;
}

ReserveResult ReserveEntry(MySQL* database,
                           RewardClaimMessage& response) {
    uint32_t today = 0u;
    if (!ReadActivityDay(database, today) || !Begin(database)) {
        return RESERVE_DATABASE;
    }

    StoredEntry prior;
    if (SelectEntryForUpdate(database, response, prior)) {
        if (!EntryIdentityMatches(prior, response) ||
            prior.activity_day != today || prior.entry_state != 0u) {
            Rollback(database);
            return RESERVE_COLLISION;
        }
        response.activity_day = prior.activity_day;
        response.entry_ordinal =
            static_cast<uint16_t>(prior.entry_ordinal);
        if (!Commit(database)) {
            Rollback(database);
            return RESERVE_DATABASE;
        }
        return RESERVE_OK;
    }

    if (!database->set_query(
            "inSert ignore into racing_dungeon_account_day_0828("
            "activity_day,m_id,entry_count,updated_at) values(%u,%u,0,now())",
            static_cast<unsigned int>(today),
            static_cast<unsigned int>(response.account_id)) ||
        !database->exec(true) ||
        !database->set_query(
            "seLect entry_count from racing_dungeon_account_day_0828 where "
            "activity_day=%u and m_id=%u for update",
            static_cast<unsigned int>(today),
            static_cast<unsigned int>(response.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    unsigned int count = 0u;
    if (!database->get_uint(0, count) ||
        count >= kOfficialEntryConditionCount) {
        Rollback(database);
        return RESERVE_COLLISION;
    }
    const unsigned int ordinal = count + 1u;
    if (!database->set_query(
            "inSert into racing_dungeon_entry_0828(entry_token_hi,"
            "entry_token_lo,activity_day,entry_ordinal,server_group,"
            "charac_no,m_id,entry_state,created_at,updated_at) values("
            "%u,%u,%u,%u,%u,%u,%u,0,now(),now())",
            static_cast<unsigned int>(response.entry_token_hi),
            static_cast<unsigned int>(response.entry_token_lo),
            static_cast<unsigned int>(today), ordinal,
            static_cast<unsigned int>(response.server_group),
            static_cast<unsigned int>(response.charac_no),
            static_cast<unsigned int>(response.account_id)) ||
        !database->exec(true) ||
        !database->set_query(
            "upDate racing_dungeon_account_day_0828 set entry_count=%u,"
            "updated_at=now() where activity_day=%u and m_id=%u and "
            "entry_count=%u",
            ordinal, static_cast<unsigned int>(today),
            static_cast<unsigned int>(response.account_id), count) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    response.activity_day = today;
    response.entry_ordinal = static_cast<uint16_t>(ordinal);
    return RESERVE_OK;
}

ReserveResult CancelEntry(MySQL* database,
                          const RewardClaimMessage& request) {
    if (!Begin(database)) return RESERVE_DATABASE;
    StoredEntry entry;
    if (!SelectEntryForUpdate(database, request, entry)) {
        // A retry after the prior DELETE is a successful idempotent cancel.
        if (!Commit(database)) {
            Rollback(database);
            return RESERVE_DATABASE;
        }
        return RESERVE_OK;
    }
    if (!EntryIdentityMatches(entry, request) || entry.entry_state != 0u ||
        entry.activity_day != request.activity_day ||
        entry.entry_ordinal != request.entry_ordinal) {
        Rollback(database);
        return RESERVE_COLLISION;
    }
    if (!database->set_query(
            "seLect entry_count from racing_dungeon_account_day_0828 where "
            "activity_day=%u and m_id=%u for update",
            static_cast<unsigned int>(entry.activity_day),
            static_cast<unsigned int>(entry.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    unsigned int count = 0u;
    if (!database->get_uint(0, count) || count != entry.entry_ordinal ||
        !database->set_query(
            "deLete from racing_dungeon_entry_0828 where entry_token_hi=%u "
            "and entry_token_lo=%u and entry_state=0",
            static_cast<unsigned int>(request.entry_token_hi),
            static_cast<unsigned int>(request.entry_token_lo)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate racing_dungeon_account_day_0828 set entry_count=%u,"
            "updated_at=now() where activity_day=%u and m_id=%u and "
            "entry_count=%u",
            count - 1u, static_cast<unsigned int>(entry.activity_day),
            static_cast<unsigned int>(entry.account_id), count) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    return RESERVE_OK;
}

bool SelectClaimForUpdate(MySQL* database,
                          const RewardClaimMessage& request,
                          StoredClaim& claim) {
    return database &&
           database->set_query(
               "seLect claim_id,m_id,member_unique_id,finish_rank,"
               "finish_time_ms,reward_item_id,reward_count,claim_state,"
               "delivery_key from %s where server_group=%u and "
               "session_token_hi=%u and session_token_lo=%u and "
               "charac_no=%u for update",
               kClaimTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.session_token_hi),
               static_cast<unsigned int>(request.session_token_lo),
               static_cast<unsigned int>(request.charac_no)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_ulonglong(0, claim.claim_id) &&
           database->get_uint(1, claim.account_id) &&
           database->get_uint(2, claim.member_unique_id) &&
           database->get_uint(3, claim.finish_rank) &&
           database->get_uint(4, claim.finish_time_ms) &&
           database->get_uint(5, claim.reward_item_id) &&
           database->get_uint(6, claim.reward_count) &&
           database->get_uint(7, claim.claim_state) &&
           database->get_ulonglong(8, claim.delivery_key);
}

bool Matches(const StoredClaim& claim,
             const RewardClaimMessage& request) {
    return claim.claim_id != 0ULL &&
           claim.account_id == request.account_id &&
           claim.member_unique_id == request.member_unique_id &&
           claim.finish_rank == request.finish_rank &&
           claim.finish_time_ms == request.finish_time_ms &&
           claim.reward_item_id == request.reward_item_id &&
           claim.reward_count == request.reward_count &&
           claim.claim_state <= REWARD_CLAIM_DELIVERED;
}

ReserveResult ReserveClaim(MySQL* database,
                           const RewardClaimMessage& request,
                           StoredClaim& claim,
                           bool& replayed) {
    replayed = false;
    if (!Begin(database)) return RESERVE_DATABASE;
    StoredEntry entry;
    if (!SelectEntryForUpdate(database, request, entry) ||
        !EntryIdentityMatches(entry, request) ||
        entry.activity_day != request.activity_day ||
        entry.entry_ordinal != request.entry_ordinal) {
        Rollback(database);
        return RESERVE_COLLISION;
    }
    if (!database->set_query(
            "inSert into %s(server_group,session_token_hi,"
            "session_token_lo,entry_token_hi,entry_token_lo,activity_day,"
            "entry_ordinal,charac_no,m_id,member_unique_id,finish_rank,"
            "finish_time_ms,reward_item_id,reward_count,claim_state,"
            "delivery_key,created_at,updated_at) values(%u,%u,%u,%u,%u,"
            "%u,%u,%u,%u,%u,%u,%u,%u,%u,0,0,now(),now()) "
            "on duplicate key update "
            "claim_id=last_insert_id(claim_id)",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.session_token_hi),
            static_cast<unsigned int>(request.session_token_lo),
            static_cast<unsigned int>(request.entry_token_hi),
            static_cast<unsigned int>(request.entry_token_lo),
            static_cast<unsigned int>(request.activity_day),
            static_cast<unsigned int>(request.entry_ordinal),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.member_unique_id),
            static_cast<unsigned int>(request.finish_rank),
            static_cast<unsigned int>(request.finish_time_ms),
            static_cast<unsigned int>(request.reward_item_id),
            static_cast<unsigned int>(request.reward_count)) ||
        !database->exec(true) ||
        !SelectClaimForUpdate(database, request, claim)) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    if (!Matches(claim, request)) {
        Rollback(database);
        return RESERVE_COLLISION;
    }
    replayed = claim.claim_state == REWARD_CLAIM_DELIVERED;

    if (claim.delivery_key == 0ULL) {
        if (claim.claim_id > kMailPayloadMask) {
            Rollback(database);
            return RESERVE_DATABASE;
        }
        claim.delivery_key = kMailNamespace | claim.claim_id;
        if (!database->set_query(
                "upDate %s set delivery_key=%llu,updated_at=now() where "
                "claim_id=%llu and delivery_key=0",
                kClaimTable, claim.delivery_key, claim.claim_id) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return RESERVE_DATABASE;
        }
    }

    if (request.reward_count == 0u &&
        claim.claim_state == REWARD_CLAIM_PENDING) {
        if (!database->set_query(
                "upDate %s set claim_state=1,delivered_at=now(),"
                "updated_at=now() where claim_id=%llu and claim_state=0",
                kClaimTable, claim.claim_id) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return RESERVE_DATABASE;
        }
        claim.claim_state = REWARD_CLAIM_DELIVERED;
    }
    if (entry.entry_state == 0u &&
        (!database->set_query(
             "upDate racing_dungeon_entry_0828 set entry_state=1,"
             "completed_at=now(),updated_at=now() where entry_token_hi=%u "
             "and entry_token_lo=%u and entry_state=0",
             static_cast<unsigned int>(request.entry_token_hi),
             static_cast<unsigned int>(request.entry_token_lo)) ||
         !database->exec(true) ||
         database->getAffectedRowCount() != 1u)) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    if (!Commit(database)) {
        Rollback(database);
        return RESERVE_DATABASE;
    }
    return RESERVE_OK;
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
        add_info != request.reward_count) {
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
               static_cast<unsigned int>(request.reward_count),
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
                  bool& replayed) {
    if (request.reward_count == 0u) return true;
    if (!Begin(claim_database)) return false;
    StoredClaim locked;
    if (!SelectClaimForUpdate(claim_database, request, locked) ||
        !Matches(locked, request) || locked.delivery_key == 0ULL) {
        Rollback(claim_database);
        return false;
    }
    claim = locked;
    if (locked.claim_state == REWARD_CLAIM_DELIVERED) {
        replayed = true;
        if (!Commit(claim_database)) {
            Rollback(claim_database);
            return false;
        }
        return true;
    }

    // Keep the InnoDB outbox row locked while the MyISAM postal table is
    // reconciled.  A crash after INSERT is replayed by the same auction_id.
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
    claim.claim_state = REWARD_CLAIM_DELIVERED;
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

DBRacingDungeon::DBRacingDungeon()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBRacingDungeon::~DBRacingDungeon() {}

bool DBRacingDungeon::QueueRequest(
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

int DBRacingDungeon::dispatch(int signal,
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
    MySQL* const claim_database =
        GetPartitionDatabase(2, request.server_group);
    if (!claim_database) {
        (void)QueueResponse(user_idx, response);
        return 1;
    }
    if (!CharacterBelongsToAccount(claim_database, request)) {
        response.result_code = REWARD_RESULT_IDENTITY;
        (void)QueueResponse(user_idx, response);
        return 1;
    }

    if (request.operation == REWARD_OPERATION_RESERVE_ENTRY) {
        const ReserveResult result = ReserveEntry(claim_database, response);
        response.result_code = result == RESERVE_OK
                                   ? REWARD_RESULT_SUCCESS
                                   : (result == RESERVE_COLLISION
                                          ? REWARD_RESULT_POLICY
                                          : REWARD_RESULT_DATABASE);
        (void)QueueResponse(user_idx, response);
        return 1;
    }
    if (request.operation == REWARD_OPERATION_CANCEL_ENTRY) {
        const ReserveResult result = CancelEntry(claim_database, request);
        response.result_code = result == RESERVE_OK
                                   ? REWARD_RESULT_SUCCESS
                                   : (result == RESERVE_COLLISION
                                          ? REWARD_RESULT_COLLISION
                                          : REWARD_RESULT_DATABASE);
        (void)QueueResponse(user_idx, response);
        return 1;
    }

    StoredClaim claim;
    bool replayed = false;
    MySQL* const mail_database =
        GetPartitionDatabase(3, request.server_group);
    if (!mail_database) {
        (void)QueueResponse(user_idx, response);
        return 1;
    }
    const ReserveResult reserve =
        ReserveClaim(claim_database, request, claim, replayed);
    if (reserve == RESERVE_COLLISION) {
        response.result_code = REWARD_RESULT_COLLISION;
    } else if (reserve == RESERVE_OK &&
               DeliverClaim(claim_database, mail_database,
                            request, claim, replayed)) {
        response.result_code = replayed
            ? REWARD_RESULT_ALREADY_DELIVERED : REWARD_RESULT_SUCCESS;
        response.claim_state = REWARD_CLAIM_DELIVERED;
        response.replayed = replayed ? 1u : 0u;
        response.delivery_key_hi =
            static_cast<uint32_t>(claim.delivery_key >> 32u);
        response.delivery_key_lo =
            static_cast<uint32_t>(claim.delivery_key & 0xffffffffULL);
    }
    (void)QueueResponse(user_idx, response);
    return 1;
}

}  // namespace racing_dungeon_0828
