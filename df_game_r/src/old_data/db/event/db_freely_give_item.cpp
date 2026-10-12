#include "old_data/db/event/db_freely_give_item.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/db_inventory_mutation_barrier.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace freely_give_item_db {
namespace {

const char kStateTable[] = "freely_give_item_state_0828";
const char kSagaTable[] = "freely_give_item_saga_0828";
const char kMailTitle[] = "Phoenix 1+1 Event";
const unsigned long long kMailNamespace = 0xF800000000000000ULL;

struct Clock {
    Clock() : now(0u), today(0u), yesterday(0u) {}
    uint32_t now;
    uint32_t today;
    uint32_t yesterday;
};

struct StoredState {
    StoredState()
        : owner_server_group(0u), owner_charac_no(0u), owner_name(),
          phase(PHASE_NONE), failed(0u), reset_count(0u),
          current_item_id(0u), item_expiry_day(0u), chicken_count(0u),
          extension_days(0u), online_remainder(0u), fatigue_remainder(0u),
          last_login_day(0u), last_level(0u), last_accounted_epoch(0u),
          last_disconnect_epoch(0u), offline_remainder(0u),
          last_timed_award_key(0u), generation(0u), normal_reward_mask(0u),
          disjoint_reward_mask(0u), total_reward_claimed(0u),
          pending_token(0u), pending_action(ACTION_NONE),
          pending_server_group(-1), pending_charac_no(0u),
          pending_item_id(0u), pending_aux(0u),
          pending_slot(-2), revision(0u) {
        std::memset(owner_name, 0, sizeof(owner_name));
    }

    uint16_t owner_server_group;
    uint32_t owner_charac_no;
    char owner_name[kCharacterNameBytes];
    uint8_t phase;
    uint8_t failed;
    uint8_t reset_count;
    uint32_t current_item_id;
    uint16_t item_expiry_day;
    uint8_t chicken_count;
    uint8_t extension_days;
    uint32_t online_remainder;
    uint32_t fatigue_remainder;
    uint32_t last_login_day;
    uint8_t last_level;
    uint32_t last_accounted_epoch;
    uint32_t last_disconnect_epoch;
    uint32_t offline_remainder;
    uint32_t last_timed_award_key;
    uint32_t generation;
    uint8_t normal_reward_mask;
    uint8_t disjoint_reward_mask;
    uint8_t total_reward_claimed;
    uint32_t pending_token;
    uint8_t pending_action;
    int16_t pending_server_group;
    uint32_t pending_charac_no;
    uint32_t pending_item_id;
    uint32_t pending_aux;
    int32_t pending_slot;
    uint32_t revision;
};

struct StoredSaga {
    StoredSaga()
        : account_id(0u), action_token(0u), server_group(-1),
          charac_no(0u), action(ACTION_NONE), proof_kind(0u),
          saga_state(SAGA_STATE_NONE), reserve_session_token(0u),
          reserve_request_token(0u), stage_session_token(0u),
          stage_request_token(0u), replay_session_token(0u),
          replay_request_token(0u), pre_money(0u), post_money(0u) {
        std::memset(pre_inventory_sha256, 0,
                    sizeof(pre_inventory_sha256));
        std::memset(post_inventory_sha256, 0,
                    sizeof(post_inventory_sha256));
    }
    uint32_t account_id;
    uint32_t action_token;
    int server_group;
    uint32_t charac_no;
    uint8_t action;
    uint8_t proof_kind;
    uint8_t saga_state;
    uint32_t reserve_session_token;
    uint32_t reserve_request_token;
    uint32_t stage_session_token;
    uint32_t stage_request_token;
    uint32_t replay_session_token;
    uint32_t replay_request_token;
    uint32_t pre_money;
    uint32_t post_money;
    uint8_t pre_inventory_sha256[32];
    uint8_t post_inventory_sha256[32];
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool HasTerminator(const char* value, unsigned int capacity) {
    return value && std::memchr(value, 0, capacity) != 0;
}

bool HasNonZeroDigest(const uint8_t digest[32]) {
    if (!digest) return false;
    uint8_t value = 0u;
    for (unsigned int index = 0u; index < 32u; ++index) {
        value = static_cast<uint8_t>(value | digest[index]);
    }
    return value != 0u;
}

bool IsLocalMutationAction(uint8_t action) {
    return action == ACTION_CLAIM_BOX ||
           action == ACTION_CLAIM_COUPON_LOCAL ||
           action == ACTION_USE_COUPON ||
           action == ACTION_CONVERT_UNLIMITED ||
           action == ACTION_RESET || action == ACTION_SELECT_WEAPON;
}

bool IsMailOnlyAction(uint8_t action) {
    return action == ACTION_CLAIM_COUPON_MAIL ||
           action == ACTION_UPGRADE_REWARD;
}

bool IsValidConfig(const Message& message) {
    return message.chicken_max == 9u &&
           message.base_extension_days == 30u &&
           message.unlimited_threshold_days == 60u &&
           message.total_extension_days == 120u &&
           message.each_time_unit == 1u &&
           message.connect_seconds == 1200u &&
           message.offline_seconds == 3600u &&
           message.fatigue_unit == 40u &&
           message.reset_money == 100000u;
}

bool IsValidMailReward(const MailReward& reward) {
    return reward.item_id != 0u && reward.add_info > 0u &&
           reward.add_info <= 999u && reward.avatar_flag <= 1u &&
           reward.creature_flag <= 1u;
}

bool IsValidRequest(const Message& message) {
    if (message.account_id == 0u || message.charac_no == 0u ||
        message.session_token == 0u || message.request_token == 0u ||
        !IsValidServerGroup(message.server_group) ||
        !HasTerminator(message.current_name, sizeof(message.current_name)) ||
        !IsValidConfig(message) || message.event_active > 1u ||
        message.reply_required > 1u ||
        message.operation < OPERATION_LOAD ||
        message.operation > OPERATION_STAGE_ACTION ||
        message.proof_kind > PERSISTENCE_PROOF_INVENTORY_MONEY ||
        message.saga_state > SAGA_STATE_CANCELLED ||
        message.reserved_tail[0] != 0u) {
        return false;
    }
    // The former direct select operation updated the event ledger without a
    // persisted inventory proof.  Selection now uses durable action 8.
    if (message.operation == OPERATION_SELECT_WEAPON) return false;
    if (message.operation == OPERATION_RESERVE_ACTION) {
        return message.action >= ACTION_CLAIM_BOX &&
               message.action <= ACTION_SELECT_WEAPON &&
               message.action_token == 0u &&
               message.saga_state == SAGA_STATE_NONE &&
               ((IsLocalMutationAction(message.action) &&
                 message.proof_kind ==
                     PERSISTENCE_PROOF_INVENTORY_MONEY &&
                 HasNonZeroDigest(message.pre_inventory_sha256) &&
                 !HasNonZeroDigest(message.post_inventory_sha256)) ||
                (IsMailOnlyAction(message.action) &&
                 message.proof_kind == PERSISTENCE_PROOF_NONE &&
                 !HasNonZeroDigest(message.pre_inventory_sha256) &&
                 !HasNonZeroDigest(message.post_inventory_sha256) &&
                 message.pre_money == 0u && message.post_money == 0u));
    }
    if (message.operation == OPERATION_COMMIT_ACTION ||
        message.operation == OPERATION_CANCEL_ACTION) {
        return message.action >= ACTION_CLAIM_BOX &&
               message.action <= ACTION_SELECT_WEAPON &&
               message.action_token != 0u &&
               message.pending_token == message.action_token &&
               message.pending_action == message.action &&
               message.pending_server_group == message.server_group &&
               message.pending_charac_no == message.charac_no &&
               ((IsLocalMutationAction(message.action) &&
                 message.proof_kind ==
                     PERSISTENCE_PROOF_INVENTORY_MONEY &&
                 HasNonZeroDigest(message.pre_inventory_sha256) &&
                 HasNonZeroDigest(message.post_inventory_sha256) &&
                 ((message.operation == OPERATION_COMMIT_ACTION &&
                   message.saga_state == SAGA_STATE_STAGED) ||
                  (message.operation == OPERATION_CANCEL_ACTION &&
                   (message.saga_state == SAGA_STATE_RESERVED ||
                    message.saga_state == SAGA_STATE_STAGED)))) ||
                (IsMailOnlyAction(message.action) &&
                 message.proof_kind == PERSISTENCE_PROOF_NONE &&
                 message.saga_state == SAGA_STATE_RESERVED &&
                 !HasNonZeroDigest(message.pre_inventory_sha256) &&
                 !HasNonZeroDigest(message.post_inventory_sha256) &&
                 message.pre_money == 0u && message.post_money == 0u));
    }
    if (message.operation == OPERATION_STAGE_ACTION) {
        return IsLocalMutationAction(message.action) &&
               message.action_token != 0u &&
               message.pending_token == message.action_token &&
               message.pending_action == message.action &&
               message.pending_server_group == message.server_group &&
               message.pending_charac_no == message.charac_no &&
               message.proof_kind ==
                   PERSISTENCE_PROOF_INVENTORY_MONEY &&
               (message.saga_state == SAGA_STATE_RESERVED ||
                message.saga_state == SAGA_STATE_STAGED) &&
               HasNonZeroDigest(message.pre_inventory_sha256) &&
               HasNonZeroDigest(message.post_inventory_sha256) &&
               (message.action == ACTION_SELECT_WEAPON ||
                message.pre_money != message.post_money ||
                std::memcmp(message.pre_inventory_sha256,
                            message.post_inventory_sha256, 32u) != 0);
    }
    return message.action == ACTION_NONE && message.action_token == 0u &&
           message.proof_kind == PERSISTENCE_PROOF_NONE &&
           message.saga_state == SAGA_STATE_NONE &&
           !HasNonZeroDigest(message.pre_inventory_sha256) &&
           !HasNonZeroDigest(message.post_inventory_sha256) &&
           message.pre_money == 0u && message.post_money == 0u;
}

MySQL* GetEventDatabase() {
    return GlobalData::s_db_mgr
               ? GlobalData::s_db_mgr->GetDBHandleExact(
                     DB_HANDLE_FEATURE_DB9,
                     static_cast<ENUM_SERVER_GROUP>(0))
               : 0;
}

MySQL* GetPartitionDatabase(int handle, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) {
        return 0;
    }
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    ENUM_DB_HANDLE_IDX exact = DB_HANDLE_COUNT;
    if (handle == 2) exact = DB_HANDLE_FEATURE_DB2;
    else if (handle == 3) exact = DB_HANDLE_FEATURE_DB3;
    else if (handle == 9) exact = DB_HANDLE_FEATURE_DB9;
    return exact == DB_HANDLE_COUNT ? 0
                                    : manager->GetDBHandleExact(exact, group);
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

bool ReadClock(MySQL* database, Clock& clock) {
    return database &&
           database->set_query(
               "seLect unix_timestamp(now()),"
               "date_format(curdate(),'%%Y%%m%%d')+0,"
               "date_format(date_sub(curdate(),interval 1 day),"
               "'%%Y%%m%%d')+0") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, clock.now) &&
           database->get_uint(1, clock.today) &&
           database->get_uint(2, clock.yesterday) && clock.now != 0u &&
           clock.today != 0u && clock.yesterday != 0u;
}

bool LoadOwnedCharacter(const Message& message,
                        char* database_name,
                        uint32_t& database_level) {
    MySQL* const database = GetPartitionDatabase(2, message.server_group);
    if (!database ||
        !database->set_query(
            "seLect charac_name,lev from charac_info where charac_no=%u "
            "and m_id=%u and delete_flag=0 limit 2",
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(message.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() ||
        !database->get_str(0, database_name, kCharacterNameBytes) ||
        !database->get_uint(1, database_level) || database_level == 0u ||
        database_level > 85u) {
        return false;
    }
    return std::strncmp(database_name,
                        message.current_name,
                        kCharacterNameBytes) == 0;
}

bool InsertState(MySQL* database, uint32_t account_id) {
    return database &&
           database->set_query(
               "inSert ignore into %s(m_id,created_at) values(%u,now())",
               kStateTable,
               static_cast<unsigned int>(account_id)) &&
           database->exec(true);
}

bool LoadState(MySQL* database,
               uint32_t account_id,
               bool for_update,
               StoredState& state) {
    if (!database ||
        !database->set_query(
            "seLect owner_server_group,owner_charac_no,owner_charac_name,"
            "phase,failed,reset_count,current_item_id,item_expiry_day,"
            "chicken_count,extension_days,online_remainder_seconds,"
            "fatigue_remainder,last_login_day,last_level,"
            "last_accounted_epoch,last_disconnect_epoch,"
            "offline_remainder_seconds,last_timed_award_key,generation,"
            "normal_reward_mask,disjoint_reward_mask,total_reward_claimed,"
            "pending_token,pending_action,pending_server_group,"
            "pending_charac_no,"
            "pending_item_id,pending_aux,pending_slot,revision from %s "
            "where m_id=%u%s",
            kStateTable,
            static_cast<unsigned int>(account_id),
            for_update ? " for update" : "") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }

    unsigned int owner_server_group = 0u;
    unsigned int phase = 0u;
    unsigned int failed = 0u;
    unsigned int reset_count = 0u;
    unsigned int item_expiry_day = 0u;
    unsigned int chicken_count = 0u;
    unsigned int extension_days = 0u;
    unsigned int last_level = 0u;
    unsigned int normal_mask = 0u;
    unsigned int separate_mask = 0u;
    unsigned int total_reward = 0u;
    unsigned int pending_action = 0u;
    int pending_server_group = -1;
    int pending_slot = -2;
    if (!database->get_uint(0, owner_server_group) ||
        !database->get_uint(1, state.owner_charac_no) ||
        !database->get_str(2, state.owner_name, sizeof(state.owner_name)) ||
        !database->get_uint(3, phase) ||
        !database->get_uint(4, failed) ||
        !database->get_uint(5, reset_count) ||
        !database->get_uint(6, state.current_item_id) ||
        !database->get_uint(7, item_expiry_day) ||
        !database->get_uint(8, chicken_count) ||
        !database->get_uint(9, extension_days) ||
        !database->get_uint(10, state.online_remainder) ||
        !database->get_uint(11, state.fatigue_remainder) ||
        !database->get_uint(12, state.last_login_day) ||
        !database->get_uint(13, last_level) ||
        !database->get_uint(14, state.last_accounted_epoch) ||
        !database->get_uint(15, state.last_disconnect_epoch) ||
        !database->get_uint(16, state.offline_remainder) ||
        !database->get_uint(17, state.last_timed_award_key) ||
        !database->get_uint(18, state.generation) ||
        !database->get_uint(19, normal_mask) ||
        !database->get_uint(20, separate_mask) ||
        !database->get_uint(21, total_reward) ||
        !database->get_uint(22, state.pending_token) ||
        !database->get_uint(23, pending_action) ||
        !database->get_int(24, pending_server_group) ||
        !database->get_uint(25, state.pending_charac_no) ||
        !database->get_uint(26, state.pending_item_id) ||
        !database->get_uint(27, state.pending_aux) ||
        !database->get_int(28, pending_slot) ||
        !database->get_uint(29, state.revision) ||
        owner_server_group >= kMaximumServerGroupCount || phase > 3u ||
        failed > 1u || reset_count > 1u || item_expiry_day > 65535u ||
        chicken_count > 9u || extension_days > 120u || last_level > 85u ||
        state.offline_remainder >= 3600u || normal_mask > 3u ||
        separate_mask > 3u || total_reward > 1u ||
        pending_action > ACTION_SELECT_WEAPON ||
        pending_server_group < -1 ||
        pending_server_group >= kMaximumServerGroupCount) {
        return false;
    }
    state.owner_server_group = static_cast<uint16_t>(owner_server_group);
    state.phase = static_cast<uint8_t>(phase);
    state.failed = static_cast<uint8_t>(failed);
    state.reset_count = static_cast<uint8_t>(reset_count);
    state.item_expiry_day = static_cast<uint16_t>(item_expiry_day);
    state.chicken_count = static_cast<uint8_t>(chicken_count);
    state.extension_days = static_cast<uint8_t>(extension_days);
    state.last_level = static_cast<uint8_t>(last_level);
    state.normal_reward_mask = static_cast<uint8_t>(normal_mask);
    state.disjoint_reward_mask = static_cast<uint8_t>(separate_mask);
    state.total_reward_claimed = static_cast<uint8_t>(total_reward);
    state.pending_action = static_cast<uint8_t>(pending_action);
    state.pending_server_group =
        static_cast<int16_t>(pending_server_group);
    state.pending_slot = static_cast<int32_t>(pending_slot);
    return (state.pending_action == ACTION_NONE) ==
               (state.pending_token == 0u) &&
           (state.pending_action == ACTION_NONE
                ? state.pending_server_group == -1 &&
                      state.pending_charac_no == 0u
                : IsValidServerGroup(state.pending_server_group) &&
                      state.pending_charac_no != 0u) &&
           (state.owner_charac_no == 0u || state.owner_name[0] != 0);
}

bool ReadDigest(MySQL* database, int column, uint8_t digest[32]) {
    return database && digest && database->get_binary_length(column) == 32 &&
           database->get_binary(column,
                                reinterpret_cast<char*>(digest), 32);
}

void DigestHex(const uint8_t digest[32], char output[65]) {
    static const char kHex[] = "0123456789abcdef";
    for (unsigned int index = 0u; index < 32u; ++index) {
        output[index * 2u] = kHex[(digest[index] >> 4u) & 0x0fu];
        output[index * 2u + 1u] = kHex[digest[index] & 0x0fu];
    }
    output[64] = 0;
}

bool LoadSaga(MySQL* database,
              uint32_t account_id,
              uint32_t action_token,
              bool for_update,
              StoredSaga& saga) {
    if (!database || account_id == 0u || action_token == 0u ||
        !database->set_query(
            "seLect m_id,action_token,server_group,charac_no,action,"
            "proof_kind,saga_state,reserve_session_token,"
            "reserve_request_token,stage_session_token,stage_request_token,"
            "replay_session_token,replay_request_token,pre_money,post_money,"
            "pre_image_sha256,post_image_sha256 from %s where m_id=%u and "
            "action_token=%u%s",
            kSagaTable,
            static_cast<unsigned int>(account_id),
            static_cast<unsigned int>(action_token),
            for_update ? " for update" : "") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int action = 0u;
    unsigned int proof_kind = 0u;
    unsigned int saga_state = 0u;
    if (!database->get_uint(0, saga.account_id) ||
        !database->get_uint(1, saga.action_token) ||
        !database->get_int(2, saga.server_group) ||
        !database->get_uint(3, saga.charac_no) ||
        !database->get_uint(4, action) ||
        !database->get_uint(5, proof_kind) ||
        !database->get_uint(6, saga_state) ||
        !database->get_uint(7, saga.reserve_session_token) ||
        !database->get_uint(8, saga.reserve_request_token) ||
        !database->get_uint(9, saga.stage_session_token) ||
        !database->get_uint(10, saga.stage_request_token) ||
        !database->get_uint(11, saga.replay_session_token) ||
        !database->get_uint(12, saga.replay_request_token) ||
        !database->get_uint(13, saga.pre_money) ||
        !database->get_uint(14, saga.post_money) ||
        !ReadDigest(database, 15, saga.pre_inventory_sha256) ||
        !ReadDigest(database, 16, saga.post_inventory_sha256)) {
        return false;
    }
    saga.action = static_cast<uint8_t>(action);
    saga.proof_kind = static_cast<uint8_t>(proof_kind);
    saga.saga_state = static_cast<uint8_t>(saga_state);
    return saga.account_id == account_id &&
           saga.action_token == action_token &&
           saga.server_group >= 0 &&
           saga.server_group < kMaximumServerGroupCount &&
           saga.charac_no != 0u && action >= ACTION_CLAIM_BOX &&
           action <= ACTION_SELECT_WEAPON &&
           proof_kind <= PERSISTENCE_PROOF_INVENTORY_MONEY &&
           saga_state >= SAGA_STATE_RESERVED &&
           saga_state <= SAGA_STATE_CANCELLED &&
           saga.reserve_session_token != 0u &&
           saga.reserve_request_token != 0u &&
           ((saga.stage_session_token == 0u) ==
            (saga.stage_request_token == 0u)) &&
           ((saga.replay_session_token == 0u) ==
            (saga.replay_request_token == 0u)) &&
           ((IsLocalMutationAction(saga.action) &&
             saga.proof_kind == PERSISTENCE_PROOF_INVENTORY_MONEY &&
             HasNonZeroDigest(saga.pre_inventory_sha256)) ||
            (IsMailOnlyAction(saga.action) &&
             saga.proof_kind == PERSISTENCE_PROOF_NONE));
}

bool InsertSaga(MySQL* database,
                const Message& request,
                uint32_t action_token) {
    char pre_hex[65] = {};
    char post_hex[65] = {};
    DigestHex(request.pre_inventory_sha256, pre_hex);
    DigestHex(request.post_inventory_sha256, post_hex);
    return database && action_token != 0u &&
           database->set_query(
               "inSert into %s(m_id,action_token,server_group,charac_no,"
               "action,proof_kind,saga_state,reserve_session_token,"
               "reserve_request_token,pre_money,post_money,"
               "pre_image_sha256,post_image_sha256,created_at,updated_at) "
               "values(%u,%u,%d,%u,%u,%u,%u,%u,%u,%u,0,unhex('%s'),"
               "unhex('%s'),now(),now())",
               kSagaTable,
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(action_token),
               request.server_group,
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.action),
               static_cast<unsigned int>(request.proof_kind),
               static_cast<unsigned int>(SAGA_STATE_RESERVED),
               static_cast<unsigned int>(request.session_token),
               static_cast<unsigned int>(request.request_token),
               static_cast<unsigned int>(request.pre_money),
               pre_hex,
               post_hex) &&
           database->exec(true);
}

bool SagaMatches(const StoredSaga& saga,
                 const StoredState& state,
                 const Message& request) {
    return saga.account_id == request.account_id &&
           saga.action_token == request.action_token &&
           saga.server_group == request.server_group &&
           saga.charac_no == request.charac_no &&
           saga.action == request.action &&
           saga.action == state.pending_action &&
           saga.action_token == state.pending_token &&
           saga.server_group == state.pending_server_group &&
           saga.charac_no == state.pending_charac_no &&
           saga.proof_kind == request.proof_kind &&
           saga.saga_state == request.saga_state &&
           saga.pre_money == request.pre_money &&
           std::memcmp(saga.pre_inventory_sha256,
                       request.pre_inventory_sha256, 32u) == 0;
}

uint32_t NextCorrelationToken(uint32_t token) {
    ++token;
    return token == 0u ? 1u : token;
}

bool IsNextCorrelation(const Message& request,
                       uint32_t session_token,
                       uint32_t request_token) {
    return session_token != 0u && request_token != 0u &&
           request.session_token == session_token &&
           request.request_token == NextCorrelationToken(request_token);
}

bool IsReserveOrReplayCorrelation(const StoredSaga& saga,
                                  const Message& request) {
    return IsNextCorrelation(request, saga.reserve_session_token,
                             saga.reserve_request_token) ||
           IsNextCorrelation(request, saga.replay_session_token,
                             saga.replay_request_token);
}

bool IsStageCorrelation(const StoredSaga& saga, const Message& request) {
    return (saga.stage_session_token == request.session_token &&
            saga.stage_request_token == request.request_token) ||
           IsReserveOrReplayCorrelation(saga, request);
}

bool IsFinalCorrelation(const StoredSaga& saga, const Message& request) {
    if (saga.saga_state == SAGA_STATE_STAGED &&
        IsNextCorrelation(request, saga.stage_session_token,
                          saga.stage_request_token)) {
        return true;
    }
    return IsReserveOrReplayCorrelation(saga, request);
}

bool UpdateSagaReplayLease(MySQL* database,
                           const StoredSaga& saga,
                           const Message& request) {
    return database && request.session_token != 0u &&
           request.request_token != 0u &&
           database->set_query(
               "upDate %s set replay_session_token=%u,"
               "replay_request_token=%u,updated_at=now() where m_id=%u and "
               "action_token=%u and saga_state in (%u,%u)",
               kSagaTable,
               static_cast<unsigned int>(request.session_token),
               static_cast<unsigned int>(request.request_token),
               static_cast<unsigned int>(saga.account_id),
               static_cast<unsigned int>(saga.action_token),
               static_cast<unsigned int>(SAGA_STATE_RESERVED),
               static_cast<unsigned int>(SAGA_STATE_STAGED)) &&
           database->exec(true);
}

void CopySaga(const StoredSaga& saga, Message& response) {
    response.action_token = saga.action_token;
    response.proof_kind = saga.proof_kind;
    response.saga_state = saga.saga_state;
    response.pre_money = saga.pre_money;
    response.post_money = saga.post_money;
    std::memcpy(response.pre_inventory_sha256,
                saga.pre_inventory_sha256, 32u);
    std::memcpy(response.post_inventory_sha256,
                saga.post_inventory_sha256, 32u);
}

bool UpdateSagaStage(MySQL* database,
                     const StoredSaga& saga,
                     const Message& request) {
    char post_hex[65] = {};
    DigestHex(request.post_inventory_sha256, post_hex);
    return database &&
           database->set_query(
               "upDate %s set saga_state=%u,post_money=%u,"
               "post_image_sha256=unhex('%s'),stage_session_token=%u,"
               "stage_request_token=%u,staged_at=now(),updated_at=now() "
               "where m_id=%u and action_token=%u and saga_state in (%u,%u)",
               kSagaTable,
               static_cast<unsigned int>(SAGA_STATE_STAGED),
               static_cast<unsigned int>(request.post_money),
               post_hex,
               static_cast<unsigned int>(request.session_token),
               static_cast<unsigned int>(request.request_token),
               static_cast<unsigned int>(saga.account_id),
               static_cast<unsigned int>(saga.action_token),
               static_cast<unsigned int>(SAGA_STATE_RESERVED),
               static_cast<unsigned int>(SAGA_STATE_STAGED)) &&
           database->exec(true);
}

bool FinalizeSaga(MySQL* database,
                  const StoredSaga& saga,
                  uint8_t terminal_state,
                  const Message& request) {
    if (!database || (terminal_state != SAGA_STATE_COMMITTED &&
                      terminal_state != SAGA_STATE_CANCELLED)) {
        return false;
    }
    char post_hex[65] = {};
    DigestHex(request.post_inventory_sha256, post_hex);
    return database->set_query(
               "upDate %s set saga_state=%u,post_money=%u,"
               "post_image_sha256=unhex('%s'),final_session_token=%u,"
               "final_request_token=%u,finalized_at=now(),updated_at=now() "
               "where m_id=%u and action_token=%u and saga_state in (%u,%u)",
               kSagaTable,
               static_cast<unsigned int>(terminal_state),
               static_cast<unsigned int>(request.post_money),
               post_hex,
               static_cast<unsigned int>(request.session_token),
               static_cast<unsigned int>(request.request_token),
               static_cast<unsigned int>(saga.account_id),
               static_cast<unsigned int>(saga.action_token),
               static_cast<unsigned int>(SAGA_STATE_RESERVED),
               static_cast<unsigned int>(SAGA_STATE_STAGED)) &&
           database->exec(true);
}

bool SaveState(MySQL* database,
               uint32_t account_id,
               StoredState& state) {
    char escaped_name[kCharacterNameBytes * 2 + 1] = {};
    if (!database ||
        database->escape_string(escaped_name, state.owner_name) < 0) {
        return false;
    }
    const bool saved = database->set_query(
               "upDate %s set owner_server_group=%u,owner_charac_no=%u,"
               "owner_charac_name='%s',phase=%u,failed=%u,reset_count=%u,"
               "current_item_id=%u,item_expiry_day=%u,chicken_count=%u,"
               "extension_days=%u,online_remainder_seconds=%u,"
               "fatigue_remainder=%u,last_login_day=%u,last_level=%u,"
               "last_accounted_epoch=%u,last_disconnect_epoch=%u,"
               "offline_remainder_seconds=%u,last_timed_award_key=%u,"
               "generation=%u,normal_reward_mask=%u,"
               "disjoint_reward_mask=%u,total_reward_claimed=%u,"
               "pending_token=%u,pending_action=%u,pending_server_group=%d,"
               "pending_charac_no=%u,"
               "pending_item_id=%u,pending_aux=%u,pending_slot=%d,"
               "pending_created_at=%s,revision=revision+1 where m_id=%u "
               "and revision=%u",
               kStateTable,
               static_cast<unsigned int>(state.owner_server_group),
               static_cast<unsigned int>(state.owner_charac_no),
               escaped_name,
               static_cast<unsigned int>(state.phase),
               static_cast<unsigned int>(state.failed),
               static_cast<unsigned int>(state.reset_count),
               static_cast<unsigned int>(state.current_item_id),
               static_cast<unsigned int>(state.item_expiry_day),
               static_cast<unsigned int>(state.chicken_count),
               static_cast<unsigned int>(state.extension_days),
               static_cast<unsigned int>(state.online_remainder),
               static_cast<unsigned int>(state.fatigue_remainder),
               static_cast<unsigned int>(state.last_login_day),
               static_cast<unsigned int>(state.last_level),
               static_cast<unsigned int>(state.last_accounted_epoch),
               static_cast<unsigned int>(state.last_disconnect_epoch),
               static_cast<unsigned int>(state.offline_remainder),
               static_cast<unsigned int>(state.last_timed_award_key),
               static_cast<unsigned int>(state.generation),
               static_cast<unsigned int>(state.normal_reward_mask),
               static_cast<unsigned int>(state.disjoint_reward_mask),
               static_cast<unsigned int>(state.total_reward_claimed),
               static_cast<unsigned int>(state.pending_token),
               static_cast<unsigned int>(state.pending_action),
               static_cast<int>(state.pending_server_group),
               static_cast<unsigned int>(state.pending_charac_no),
               static_cast<unsigned int>(state.pending_item_id),
               static_cast<unsigned int>(state.pending_aux),
               static_cast<int>(state.pending_slot),
               state.pending_action == ACTION_NONE ? "null" : "now()",
               static_cast<unsigned int>(account_id),
               static_cast<unsigned int>(state.revision)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
    if (saved) {
        ++state.revision;
    }
    return saved;
}

void CopyState(const StoredState& state, Message& response) {
    response.revision = state.revision;
    response.owner_charac_no = state.owner_charac_no;
    response.current_item_id = state.current_item_id;
    response.online_remainder = state.online_remainder;
    response.fatigue_remainder = state.fatigue_remainder;
    response.last_login_day = state.last_login_day;
    response.last_disconnect_epoch = state.last_disconnect_epoch;
    response.generation = state.generation;
    response.pending_token = state.pending_token;
    response.pending_charac_no = state.pending_charac_no;
    response.pending_server_group = state.pending_server_group;
    response.pending_item_id = state.pending_item_id;
    response.pending_aux = state.pending_aux;
    response.action_slot = state.pending_slot;
    std::memset(response.owner_name, 0, sizeof(response.owner_name));
    std::strncpy(response.owner_name,
                 state.owner_name,
                 sizeof(response.owner_name) - 1u);
    response.phase = state.phase;
    response.failed = state.failed;
    response.reset_count = state.reset_count;
    response.chicken_count = state.chicken_count;
    response.extension_days = state.extension_days;
    response.normal_reward_mask = state.normal_reward_mask;
    response.disjoint_reward_mask = state.disjoint_reward_mask;
    response.total_reward_claimed = state.total_reward_claimed;
    response.pending_action = state.pending_action;
    response.owner_match =
        state.owner_charac_no == response.charac_no &&
        state.owner_server_group ==
            static_cast<uint16_t>(response.server_group);
    if (response.connect_seconds != 0u) {
        const uint32_t remainder =
            state.online_remainder % response.connect_seconds;
        response.countdown_seconds = static_cast<uint16_t>(
            remainder == 0u ? response.connect_seconds
                            : response.connect_seconds - remainder);
    }
}

void AddNotice(Message& response, uint8_t reason, uint32_t award_count) {
    if (award_count == 0u ||
        response.notice_count >= kMaximumNoticeCount) {
        return;
    }
    Notice& notice = response.notices[response.notice_count++];
    notice.reason = reason;
    notice.award_count = static_cast<uint8_t>(
        std::min<uint32_t>(award_count, 255u));
}

uint32_t AwardChicken(StoredState& state,
                      const Message& message,
                      uint32_t requested) {
    const uint32_t room =
        static_cast<uint32_t>(message.chicken_max - state.chicken_count);
    const uint32_t awarded = std::min<uint32_t>(room, requested);
    state.chicken_count = static_cast<uint8_t>(
        static_cast<uint32_t>(state.chicken_count) + awarded);
    return awarded;
}

bool CanAccrue(const StoredState& state, const Message& message) {
    return message.event_active != 0u &&
           state.phase == PHASE_PERIODIC_WEAPON && !state.failed;
}

void AccountOnline(StoredState& state,
                   Message& response,
                   const Clock& clock,
                   bool emit_notice) {
    if (!CanAccrue(state, response) ||
        state.last_accounted_epoch == 0u ||
        clock.now <= state.last_accounted_epoch) {
        state.last_accounted_epoch = clock.now;
        return;
    }
    uint32_t elapsed = clock.now - state.last_accounted_epoch;
    if (elapsed > 86400u) {
        elapsed = 86400u;
    }
    const uint64_t total =
        static_cast<uint64_t>(state.online_remainder) + elapsed;
    const uint32_t units = static_cast<uint32_t>(
        total / response.connect_seconds);
    state.online_remainder = static_cast<uint32_t>(
        total % response.connect_seconds);
    state.last_accounted_epoch = clock.now;
    const uint32_t awarded = AwardChicken(
        state,
        response,
        units * static_cast<uint32_t>(response.each_time_unit));
    if (emit_notice) {
        AddNotice(response, REASON_ONLINE, awarded);
    }
}

bool GenerateToken(MySQL* database, uint32_t& token) {
    token = 0u;
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (!database ||
            !database->set_query(
                "seLect crc32(concat(uuid(),rand(),unix_timestamp(now())))") ||
            !database->exec(true) || database->get_n_rows() != 1 ||
            !database->fetch() || !database->get_uint(0, token)) {
            return false;
        }
        if (token != 0u) {
            return true;
        }
    }
    return false;
}

void ClearPending(StoredState& state) {
    state.pending_token = 0u;
    state.pending_action = ACTION_NONE;
    state.pending_server_group = -1;
    state.pending_charac_no = 0u;
    state.pending_item_id = 0u;
    state.pending_aux = 0u;
    state.pending_slot = -2;
}

bool IsOwner(const StoredState& state, const Message& message) {
    return state.owner_charac_no == message.charac_no &&
           state.owner_server_group ==
               static_cast<uint16_t>(message.server_group);
}

uint8_t ValidateReserve(const StoredState& state, const Message& request) {
    if (!request.event_active) {
        return RESULT_EVENT_CLOSED;
    }
    if (state.pending_action != ACTION_NONE) {
        return RESULT_PENDING;
    }
    switch (request.action) {
        case ACTION_CLAIM_BOX:
            if (state.phase != PHASE_NONE || state.owner_charac_no != 0u) {
                return RESULT_ALREADY_CLAIMED;
            }
            return request.action_item_id ? RESULT_SUCCESS : RESULT_SCRIPT;
        case ACTION_CLAIM_COUPON_LOCAL:
        case ACTION_CLAIM_COUPON_MAIL:
            if (state.failed) return RESULT_FAILED;
            if (state.phase != PHASE_PERIODIC_WEAPON ||
                state.current_item_id == 0u) return RESULT_NO_WEAPON;
            if (state.owner_server_group != request.server_group)
                return RESULT_DIFFERENT_SERVER;
            if (state.chicken_count < request.chicken_max)
                return RESULT_NOT_ENOUGH_CHICKEN;
            if ((request.action == ACTION_CLAIM_COUPON_LOCAL) !=
                IsOwner(state, request)) return RESULT_STATE;
            return request.action_item_id ? RESULT_SUCCESS : RESULT_SCRIPT;
        case ACTION_USE_COUPON:
            if (state.failed) return RESULT_FAILED;
            if (state.phase != PHASE_PERIODIC_WEAPON ||
                state.current_item_id == 0u) return RESULT_NO_WEAPON;
            if (!IsOwner(state, request)) return RESULT_NOT_OWNER;
            if (state.extension_days >= request.total_extension_days)
                return RESULT_STATE;
            if (request.action_slot < 0 || request.action_aux > 65535u)
                return RESULT_STATE;
            return request.action_aux > state.item_expiry_day
                       ? RESULT_SUCCESS : RESULT_STATE;
        case ACTION_CONVERT_UNLIMITED:
            if (state.failed) return RESULT_FAILED;
            if (state.phase != PHASE_PERIODIC_WEAPON ||
                state.current_item_id == 0u) return RESULT_NO_WEAPON;
            if (!IsOwner(state, request)) return RESULT_NOT_OWNER;
            if (state.extension_days < request.unlimited_threshold_days)
                return RESULT_BELOW_THRESHOLD;
            return request.action_item_id ? RESULT_SUCCESS : RESULT_SCRIPT;
        case ACTION_RESET:
            if (state.failed) return RESULT_FAILED;
            if (state.reset_count != 0u) return RESULT_ALREADY_RESET;
            if (state.phase != PHASE_PERIODIC_WEAPON ||
                state.current_item_id == 0u) return RESULT_NO_WEAPON;
            if (!IsOwner(state, request)) return RESULT_NOT_OWNER;
            return RESULT_SUCCESS;
        case ACTION_UPGRADE_REWARD: {
            if (state.failed || state.phase != PHASE_PERIODIC_WEAPON ||
                !IsOwner(state, request) || request.action_item_id == 0u)
                return RESULT_STATE;
            const uint32_t mode = (request.action_aux >> 8u) & 0xffu;
            const uint32_t bit_index = request.action_aux & 0xffu;
            if ((mode != 1u && mode != 2u) || bit_index >= 2u)
                return RESULT_SCRIPT;
            const uint8_t bit = static_cast<uint8_t>(1u << bit_index);
            const uint8_t mask = mode == 1u ? state.normal_reward_mask
                                            : state.disjoint_reward_mask;
            return (mask & bit) ? RESULT_ALREADY_CLAIMED : RESULT_SUCCESS;
        }
        case ACTION_SELECT_WEAPON:
            if (state.failed || state.phase != PHASE_BOX_CLAIMED ||
                !IsOwner(state, request) || request.action_item_id == 0u ||
                request.action_aux == 0u || request.action_aux > 65535u ||
                request.action_slot < -1 ||
                request.action_slot >=
                    oldsrv::config::kMainInventoryItemCount) {
                return RESULT_STATE;
            }
            return RESULT_SUCCESS;
        default:
            return RESULT_STATE;
    }
}

void Mix32(unsigned long long& hash, uint32_t value) {
    for (unsigned int index = 0; index < 4u; ++index) {
        hash ^= static_cast<unsigned long long>(value & 0xffu);
        hash *= 1099511628211ULL;
        value >>= 8u;
    }
}

unsigned long long DeliveryKey(const Message& request,
                               const StoredState& state,
                               uint32_t discriminator) {
    unsigned long long hash = 1469598103934665603ULL;
    Mix32(hash, request.account_id);
    Mix32(hash, state.generation);
    Mix32(hash, state.pending_token);
    Mix32(hash, discriminator);
    return kMailNamespace | (hash & 0x00FFFFFFFFFFFFFFULL);
}

bool FindMail(MySQL* database,
              unsigned long long key,
              uint32_t receiver,
              const MailReward& reward,
              bool& found) {
    found = false;
    if (!database ||
        !database->set_query(
            "seLect receive_charac_no,item_id,add_info from postal where "
            "auction_id=%llu limit 2",
            key) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    unsigned int actual_receiver = 0u;
    unsigned int item_id = 0u;
    unsigned int add_info = 0u;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, actual_receiver) ||
        !database->get_uint(1, item_id) ||
        !database->get_uint(2, add_info) ||
        actual_receiver != receiver || item_id != reward.item_id ||
        add_info != reward.add_info) {
        return false;
    }
    found = true;
    return true;
}

bool InsertMail(MySQL* database,
                unsigned long long key,
                uint32_t receiver,
                const MailReward& reward) {
    char escaped_title[sizeof(kMailTitle) * 2 + 1] = {};
    if (!database ||
        database->escape_string(escaped_title, kMailTitle) < 0) {
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
               "%u,0,'%s',%u,%u,0,%u,'%s',%llu,'%s',%u,%u,0,0)",
               static_cast<unsigned int>(receiver),
               static_cast<unsigned int>(reward.seal_flag),
               static_cast<unsigned int>(reward.item_id),
               static_cast<unsigned int>(reward.add_info),
               static_cast<unsigned int>(reward.endurance),
               static_cast<unsigned int>(reward.upgrade),
               static_cast<unsigned int>(reward.amplify_option),
               static_cast<unsigned int>(reward.amplify_value),
               escaped_title,
               static_cast<unsigned int>(reward.creature_flag),
               static_cast<unsigned int>(reward.avatar_flag),
               static_cast<unsigned int>(reward.extend_info),
               item_guid,
               key,
               random_option,
               static_cast<unsigned int>(reward.chronicle_value),
               static_cast<unsigned int>(reward.separate_upgrade)) &&
           database->exec(true);
}

bool EnsureMail(const Message& request,
                const StoredState& state,
                uint32_t discriminator) {
    if (!IsValidMailReward(request.mail_reward) ||
        state.owner_charac_no == 0u ||
        state.owner_server_group >= kMaximumServerGroupCount) {
        return false;
    }
    MySQL* const database =
        GetPartitionDatabase(3, state.owner_server_group);
    const unsigned long long key =
        DeliveryKey(request, state, discriminator);
    bool found = false;
    if (!FindMail(database,
                  key,
                  state.owner_charac_no,
                  request.mail_reward,
                  found)) {
        return false;
    }
    if (found) return true;
    if (InsertMail(database,
                   key,
                   state.owner_charac_no,
                   request.mail_reward)) {
        return true;
    }
    return FindMail(database,
                    key,
                    state.owner_charac_no,
                    request.mail_reward,
                    found) && found;
}

bool HandleLoad(MySQL* database,
                const Message& request,
                Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    Clock clock;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database) || !ReadClock(database, clock) ||
        !InsertState(database, request.account_id)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    if (!LoadState(database, request.account_id, true, state)) {
        Rollback(database);
        return false;
    }

    const uint32_t offline_start = state.last_disconnect_epoch != 0u
                                       ? state.last_disconnect_epoch
                                       : state.last_accounted_epoch;
    if (CanAccrue(state, request) && offline_start != 0u &&
        clock.now > offline_start) {
        uint32_t offline = clock.now - offline_start;
        if (offline > 2592000u) offline = 2592000u;
        const uint64_t total =
            static_cast<uint64_t>(state.offline_remainder) + offline;
        const uint32_t units = static_cast<uint32_t>(
            total / request.offline_seconds);
        state.offline_remainder = static_cast<uint32_t>(
            total % request.offline_seconds);
        const uint32_t awarded = AwardChicken(
            state,
            request,
            units * static_cast<uint32_t>(request.each_time_unit));
        AddNotice(response, REASON_OFFLINE, awarded);
    }
    state.last_disconnect_epoch = 0u;

    if (CanAccrue(state, request) &&
        state.last_login_day != clock.today) {
        const uint8_t reason = state.last_login_day == clock.yesterday
                                   ? REASON_CONSECUTIVE_LOGIN
                                   : REASON_FIRST_LOGIN;
        const uint32_t awarded = AwardChicken(
            state,
            request,
            static_cast<uint32_t>(request.each_time_unit));
        AddNotice(response, reason, awarded);
        state.last_login_day = clock.today;
    }
    state.last_level = static_cast<uint8_t>(database_level);
    state.last_accounted_epoch = clock.now;
    StoredSaga saga;
    if (state.pending_action != ACTION_NONE &&
        (!LoadSaga(database, request.account_id, state.pending_token, true,
                   saga) ||
         saga.action != state.pending_action ||
         saga.server_group != state.pending_server_group ||
         saga.charac_no != state.pending_charac_no ||
         saga.saga_state >= SAGA_STATE_COMMITTED ||
         !UpdateSagaReplayLease(database, saga, request))) {
        Rollback(database);
        return false;
    }
    if (state.pending_action != ACTION_NONE) {
        saga.replay_session_token = request.session_token;
        saga.replay_request_token = request.request_token;
    }
    if (!SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
    if (state.pending_action != ACTION_NONE) CopySaga(saga, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleProgress(MySQL* database,
                    const Message& request,
                    Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    Clock clock;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database) || !ReadClock(database, clock)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    if (!LoadState(database, request.account_id, true, state)) {
        Rollback(database);
        return false;
    }

    if (request.operation == OPERATION_ONLINE_CHECKPOINT ||
        request.operation == OPERATION_DISCONNECT) {
        AccountOnline(state, response, clock, request.reply_required != 0u);
        if (request.operation == OPERATION_DISCONNECT) {
            state.last_disconnect_epoch = clock.now;
        }
    } else if (CanAccrue(state, request) &&
               request.operation == OPERATION_FATIGUE) {
        const uint64_t total =
            static_cast<uint64_t>(state.fatigue_remainder) +
            request.source_value;
        const uint32_t units = static_cast<uint32_t>(
            total / request.fatigue_unit);
        state.fatigue_remainder = static_cast<uint32_t>(
            total % request.fatigue_unit);
        const uint32_t awarded = AwardChicken(
            state,
            request,
            units * static_cast<uint32_t>(request.each_time_unit));
        AddNotice(response, REASON_FATIGUE, awarded);
    } else if (CanAccrue(state, request) &&
               request.operation == OPERATION_LEVEL_UP) {
        const uint32_t new_level =
            std::min<uint32_t>(database_level, request.current_level);
        if (new_level > state.last_level) {
            const uint32_t awarded = AwardChicken(
                state,
                request,
                (new_level - state.last_level) *
                    static_cast<uint32_t>(request.each_time_unit));
            AddNotice(response, REASON_LEVEL_UP, awarded);
            state.last_level = static_cast<uint8_t>(new_level);
        }
    } else if (CanAccrue(state, request) &&
               request.operation == OPERATION_TIMED &&
               request.source_value != 0u &&
               request.source_value > state.last_timed_award_key) {
        const uint32_t awarded = AwardChicken(
            state,
            request,
            static_cast<uint32_t>(request.each_time_unit));
        AddNotice(response, REASON_TIMED_LOGIN, awarded);
        state.last_timed_award_key = request.source_value;
    }

    if (!SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleReserve(MySQL* database,
                   const Message& request,
                   Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    if (!LoadState(database, request.account_id, true, state)) {
        Rollback(database);
        return false;
    }
    const uint8_t validation = ValidateReserve(state, request);
    if (validation != RESULT_SUCCESS) {
        CopyState(state, response);
        response.result = validation;
        return Commit(database);
    }
    uint32_t token = 0u;
    if (!GenerateToken(database, token)) {
        Rollback(database);
        return false;
    }
    state.pending_token = token;
    state.pending_action = request.action;
    state.pending_server_group =
        static_cast<int16_t>(request.server_group);
    state.pending_charac_no = request.charac_no;
    state.pending_item_id = request.action_item_id;
    state.pending_aux = request.action_aux;
    state.pending_slot = request.action_slot;
    if (!InsertSaga(database, request, token) ||
        !SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
    response.action_token = token;
    StoredSaga saga;
    saga.account_id = request.account_id;
    saga.action_token = token;
    saga.server_group = request.server_group;
    saga.charac_no = request.charac_no;
    saga.action = request.action;
    saga.proof_kind = request.proof_kind;
    saga.saga_state = SAGA_STATE_RESERVED;
    saga.reserve_session_token = request.session_token;
    saga.reserve_request_token = request.request_token;
    saga.pre_money = request.pre_money;
    std::memcpy(saga.pre_inventory_sha256,
                request.pre_inventory_sha256, 32u);
    CopySaga(saga, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleStage(MySQL* database,
                 const Message& request,
                 Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    StoredSaga saga;
    if (!LoadState(database, request.account_id, true, state) ||
        !LoadSaga(database, request.account_id, request.action_token, true,
                   saga) ||
        !SagaMatches(saga, state, request) ||
        !IsStageCorrelation(saga, request) ||
        !IsLocalMutationAction(saga.action) ||
        (saga.saga_state != SAGA_STATE_RESERVED &&
         saga.saga_state != SAGA_STATE_STAGED)) {
        Rollback(database);
        return false;
    }
    const bool persisted_pre = inventory_mutation_barrier::
        HasExactPersistedFreelyGiveImageAndMoney(
            request.server_group, request.charac_no,
            request.pre_inventory_sha256, request.pre_money);
    const bool same_staged_post = saga.saga_state == SAGA_STATE_STAGED &&
        saga.post_money == request.post_money &&
        std::memcmp(saga.post_inventory_sha256,
                    request.post_inventory_sha256, 32u) == 0;
    const bool persisted_staged_post = !persisted_pre && same_staged_post &&
        inventory_mutation_barrier::
            HasExactPersistedFreelyGiveImageAndMoney(
                request.server_group, request.charac_no,
                request.post_inventory_sha256, request.post_money);
    if ((!persisted_pre && !persisted_staged_post) ||
        !UpdateSagaStage(database, saga, request) || !Commit(database)) {
        Rollback(database);
        return false;
    }
    saga.saga_state = SAGA_STATE_STAGED;
    saga.stage_session_token = request.session_token;
    saga.stage_request_token = request.request_token;
    saga.post_money = request.post_money;
    std::memcpy(saga.post_inventory_sha256,
                request.post_inventory_sha256, 32u);
    CopyState(state, response);
    CopySaga(saga, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCommit(MySQL* database,
                  const Message& request,
                  Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    StoredSaga saga;
    if (!LoadState(database, request.account_id, true, state) ||
        !LoadSaga(database, request.account_id, request.action_token, true,
                  saga)) {
        Rollback(database);
        return false;
    }
    if (state.pending_token != request.action_token ||
        state.pending_action != request.action ||
        state.pending_server_group != request.server_group ||
        state.pending_charac_no != request.charac_no) {
        CopyState(state, response);
        response.result = state.pending_action == ACTION_NONE
                              ? RESULT_STATE : RESULT_PENDING;
        return Commit(database);
    }
    if (!SagaMatches(saga, state, request) ||
        !IsFinalCorrelation(saga, request) ||
        saga.saga_state >= SAGA_STATE_COMMITTED) {
        Rollback(database);
        return false;
    }
    if (IsLocalMutationAction(request.action)) {
        if (saga.saga_state != SAGA_STATE_STAGED ||
            saga.post_money != request.post_money ||
            std::memcmp(saga.post_inventory_sha256,
                        request.post_inventory_sha256, 32u) != 0 ||
            !inventory_mutation_barrier::
                 HasExactPersistedFreelyGiveImageAndMoney(
                     request.server_group, request.charac_no,
                     request.post_inventory_sha256,
                     request.post_money)) {
            Rollback(database);
            return false;
        }
    } else if (!IsMailOnlyAction(request.action) ||
               saga.saga_state != SAGA_STATE_RESERVED) {
        Rollback(database);
        return false;
    }

    if (request.action == ACTION_CLAIM_COUPON_MAIL &&
        !EnsureMail(request, state, 1u)) {
        Rollback(database);
        return false;
    }
    if (request.action == ACTION_USE_COUPON &&
        state.extension_days + 1u >= request.total_extension_days &&
        !state.total_reward_claimed &&
        !EnsureMail(request, state, 2u)) {
        Rollback(database);
        return false;
    }
    if (request.action == ACTION_UPGRADE_REWARD) {
        const uint32_t mode = (state.pending_aux >> 8u) & 0xffu;
        const uint32_t bit_index = state.pending_aux & 0xffu;
        if (!EnsureMail(request, state,
                        0x100u + mode * 0x10u + bit_index)) {
            Rollback(database);
            return false;
        }
    }

    switch (request.action) {
        case ACTION_CLAIM_BOX:
            state.owner_server_group =
                static_cast<uint16_t>(request.server_group);
            state.owner_charac_no = request.charac_no;
            std::memset(state.owner_name, 0, sizeof(state.owner_name));
            std::strncpy(state.owner_name,
                         request.current_name,
                         sizeof(state.owner_name) - 1u);
            state.phase = PHASE_BOX_CLAIMED;
            state.current_item_id = 0u;
            state.failed = 0u;
            ++state.generation;
            if (state.generation == 0u) ++state.generation;
            break;
        case ACTION_CLAIM_COUPON_LOCAL:
        case ACTION_CLAIM_COUPON_MAIL:
            if (state.chicken_count < request.chicken_max) {
                CopyState(state, response);
                response.result = RESULT_NOT_ENOUGH_CHICKEN;
                return Commit(database);
            }
            state.chicken_count = static_cast<uint8_t>(
                state.chicken_count - request.chicken_max);
            break;
        case ACTION_USE_COUPON:
            if (state.extension_days >= request.total_extension_days ||
                state.pending_aux <= state.item_expiry_day) {
                CopyState(state, response);
                response.result = RESULT_STATE;
                return Commit(database);
            }
            ++state.extension_days;
            state.item_expiry_day = static_cast<uint16_t>(state.pending_aux);
            if (state.extension_days >= request.total_extension_days) {
                state.total_reward_claimed = 1u;
            }
            break;
        case ACTION_CONVERT_UNLIMITED:
            state.phase = PHASE_COMPLETED;
            state.current_item_id = state.pending_item_id;
            state.item_expiry_day = 0u;
            break;
        case ACTION_RESET:
            state.owner_server_group = 0u;
            state.owner_charac_no = 0u;
            std::memset(state.owner_name, 0, sizeof(state.owner_name));
            state.phase = PHASE_NONE;
            state.failed = 0u;
            state.reset_count = 1u;
            state.current_item_id = 0u;
            state.item_expiry_day = 0u;
            state.chicken_count = 0u;
            state.extension_days = 0u;
            state.online_remainder = 0u;
            state.offline_remainder = 0u;
            state.fatigue_remainder = 0u;
            state.last_login_day = 0u;
            state.last_level = 0u;
            state.last_accounted_epoch = 0u;
            state.last_disconnect_epoch = 0u;
            state.last_timed_award_key = 0u;
            state.normal_reward_mask = 0u;
            state.disjoint_reward_mask = 0u;
            state.total_reward_claimed = 0u;
            ++state.generation;
            if (state.generation == 0u) ++state.generation;
            break;
        case ACTION_UPGRADE_REWARD: {
            const uint32_t mode = (state.pending_aux >> 8u) & 0xffu;
            const uint32_t bit_index = state.pending_aux & 0xffu;
            if ((mode != 1u && mode != 2u) || bit_index >= 2u) {
                CopyState(state, response);
                response.result = RESULT_SCRIPT;
                return Commit(database);
            }
            const uint8_t bit = static_cast<uint8_t>(1u << bit_index);
            if (mode == 1u) state.normal_reward_mask |= bit;
            else state.disjoint_reward_mask |= bit;
            break;
        }
        case ACTION_SELECT_WEAPON:
            state.phase = PHASE_PERIODIC_WEAPON;
            state.current_item_id = state.pending_item_id;
            state.item_expiry_day = static_cast<uint16_t>(state.pending_aux);
            state.failed = 0u;
            break;
        default:
            CopyState(state, response);
            response.result = RESULT_STATE;
            return Commit(database);
    }
    ClearPending(state);
    if (!FinalizeSaga(database, saga, SAGA_STATE_COMMITTED, request) ||
        !SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCancel(MySQL* database,
                  const Message& request,
                  Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    StoredSaga saga;
    if (!LoadState(database, request.account_id, true, state) ||
        !LoadSaga(database, request.account_id, request.action_token, true,
                  saga)) {
        Rollback(database);
        return false;
    }
    if (state.pending_token != request.action_token ||
        state.pending_action != request.action ||
        state.pending_server_group != request.server_group ||
        state.pending_charac_no != request.charac_no) {
        CopyState(state, response);
        response.result = RESULT_STATE;
        return Commit(database);
    }
    if (!SagaMatches(saga, state, request) ||
        !IsFinalCorrelation(saga, request) ||
        saga.saga_state >= SAGA_STATE_COMMITTED) {
        Rollback(database);
        return false;
    }
    if (IsLocalMutationAction(request.action)) {
        if (request.post_money != saga.pre_money ||
            std::memcmp(request.post_inventory_sha256,
                        saga.pre_inventory_sha256, 32u) != 0 ||
            !inventory_mutation_barrier::
                 HasExactPersistedFreelyGiveImageAndMoney(
                     request.server_group, request.charac_no,
                     request.post_inventory_sha256,
                     request.post_money)) {
            Rollback(database);
            return false;
        }
    } else if (!IsMailOnlyAction(request.action)) {
        Rollback(database);
        return false;
    }
    ClearPending(state);
    if (!FinalizeSaga(database, saga, SAGA_STATE_CANCELLED, request) ||
        !SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleSelectWeapon(MySQL* database,
                        const Message& request,
                        Message& response) {
    char character_name[kCharacterNameBytes] = {};
    uint32_t database_level = 0u;
    Clock clock;
    if (!LoadOwnedCharacter(request, character_name, database_level) ||
        !Begin(database) || !ReadClock(database, clock)) {
        Rollback(database);
        return false;
    }
    StoredState state;
    if (!LoadState(database, request.account_id, true, state)) {
        Rollback(database);
        return false;
    }
    if (!request.event_active || state.phase != PHASE_BOX_CLAIMED ||
        !IsOwner(state, request) || request.source_value == 0u ||
        request.action_aux == 0u || request.action_aux > 65535u) {
        CopyState(state, response);
        response.result = RESULT_STATE;
        return Commit(database);
    }
    state.phase = PHASE_PERIODIC_WEAPON;
    state.current_item_id = request.source_value;
    state.item_expiry_day = static_cast<uint16_t>(request.action_aux);
    state.extension_days = request.base_extension_days;
    state.failed = 0u;
    state.online_remainder = 0u;
    state.offline_remainder = 0u;
    state.last_accounted_epoch = clock.now;
    state.last_disconnect_epoch = 0u;
    state.last_level = static_cast<uint8_t>(database_level);
    if (state.last_login_day != clock.today) {
        const uint8_t reason = state.last_login_day == clock.yesterday
                                   ? REASON_CONSECUTIVE_LOGIN
                                   : REASON_FIRST_LOGIN;
        const uint32_t awarded = AwardChicken(
            state, request, static_cast<uint32_t>(request.each_time_unit));
        AddNotice(response, reason, awarded);
        state.last_login_day = clock.today;
    }
    if (!SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleMarkFailed(MySQL* database,
                      const Message& request,
                      Message& response) {
    if (!Begin(database)) return false;
    StoredState state;
    if (!LoadState(database, request.account_id, true, state)) {
        Rollback(database);
        return false;
    }
    if (state.phase == PHASE_PERIODIC_WEAPON &&
        IsOwner(state, request) &&
        state.current_item_id == request.source_value) {
        state.failed = 1u;
    }
    if (!SaveState(database, request.account_id, state) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(state, response);
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

DBFreelyGiveItem::DBFreelyGiveItem()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBFreelyGiveItem::~DBFreelyGiveItem() {}

bool DBFreelyGiveItem::QueueRequest(int user_idx,
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
    if (request.operation == OPERATION_LEVEL_UP) {
        return legacy_feature_fence::QueueAfterLegacy(
            legacy_feature_fence::kFenceFreelyGiveLevelUp, guard);
    }
    if ((request.operation == OPERATION_COMMIT_ACTION ||
         request.operation == OPERATION_CANCEL_ACTION) &&
        IsLocalMutationAction(request.action)) {
        return legacy_feature_fence::QueueAfterLegacy(
            legacy_feature_fence::
                kFenceFreelyGiveInventoryMoneyCommit,
            guard);
    }
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBFreelyGiveItem::dispatch(int signal,
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
    response.notice_count = 0u;
    std::memset(response.notices, 0, sizeof(response.notices));

    MySQL* const database = GetEventDatabase();
    bool handled = false;
    if (database) {
        switch (request.operation) {
            case OPERATION_LOAD:
                handled = HandleLoad(database, request, response);
                break;
            case OPERATION_ONLINE_CHECKPOINT:
            case OPERATION_FATIGUE:
            case OPERATION_LEVEL_UP:
            case OPERATION_TIMED:
            case OPERATION_DISCONNECT:
                handled = HandleProgress(database, request, response);
                break;
            case OPERATION_RESERVE_ACTION:
                handled = HandleReserve(database, request, response);
                break;
            case OPERATION_COMMIT_ACTION:
                handled = HandleCommit(database, request, response);
                break;
            case OPERATION_CANCEL_ACTION:
                handled = HandleCancel(database, request, response);
                break;
            case OPERATION_STAGE_ACTION:
                handled = HandleStage(database, request, response);
                break;
            case OPERATION_SELECT_WEAPON:
                handled = HandleSelectWeapon(database, request, response);
                break;
            case OPERATION_MARK_FAILED:
                handled = HandleMarkFailed(database, request, response);
                break;
            default:
                break;
        }
    }
    if (!handled) response.result = RESULT_DATABASE_FAILURE;
    if (!request.reply_required) return handled ? 1 : 0;
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace freely_give_item_db_0828
