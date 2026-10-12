#include "old_data/db/event/db_facebook_like.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstring>

namespace facebook_like_db {
namespace {

const char kStateTable[] = "facebook_like_charac_state_0828";

struct StoredState {
    StoredState()
        : cycle_key(0u),
          countdown_deadline(0u),
          selected_reward_row(0u),
          selected_reward_item_id(0u),
          claim_token(0u),
          mail_reserved_epoch(0u),
          stage(STAGE_WAIT_FATIGUE),
          mail_state(MAIL_STATE_NONE) {}

    uint32_t cycle_key;
    uint32_t countdown_deadline;
    uint32_t selected_reward_row;
    uint32_t selected_reward_item_id;
    uint32_t claim_token;
    uint32_t mail_reserved_epoch;
    uint8_t stage;
    uint8_t mail_state;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsValidReward(uint32_t row, uint32_t item_id) {
    return row >= 1u && row <= 49u &&
           item_id == 690000944u + row;
}

bool GenerateClaimToken(MySQL* db, uint32_t& claim_token) {
    claim_token = 0u;
    // The token survives process restarts and is echoed through tracked
    // signal128.  Generate it in the authoritative DB transaction so a game
    // process counter cannot collide with an unexpired pre-crash lease.
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (!db ||
            !db->set_query(
                "seLect crc32(concat(uuid(),rand(),unix_timestamp(now())))") ||
            !db->exec(true) || db->get_n_rows() != 1 || !db->fetch() ||
            !db->get_uint(0, claim_token)) {
            return false;
        }
        if (claim_token != 0u) {
            return true;
        }
    }
    return false;
}

bool IsValidStoredState(const StoredState& state) {
    if (state.stage > STAGE_REWARD_MAILED ||
        state.mail_state > MAIL_STATE_PERSISTED) {
        return false;
    }
    if (state.stage <= STAGE_SELECT_REWARD) {
        return state.selected_reward_row == 0u &&
               state.selected_reward_item_id == 0u &&
               state.claim_token == 0u &&
               state.mail_state == MAIL_STATE_NONE &&
               state.mail_reserved_epoch == 0u;
    }
    if (!IsValidReward(state.selected_reward_row,
                       state.selected_reward_item_id)) {
        return false;
    }
    if (state.stage == STAGE_REWARD_SELECTED) {
        if (state.mail_state == MAIL_STATE_NONE) {
            return state.claim_token == 0u &&
                   state.mail_reserved_epoch == 0u;
        }
        return state.mail_state == MAIL_STATE_RESERVED &&
               state.claim_token != 0u &&
               state.mail_reserved_epoch != 0u;
    }
    return state.mail_state == MAIL_STATE_PERSISTED &&
           state.claim_token != 0u && state.mail_reserved_epoch != 0u;
}

void ResetForCycle(StoredState& state, uint32_t cycle_key) {
    state = StoredState();
    state.cycle_key = cycle_key;
}

bool BeginTransaction(MySQL* db) {
    return db && db->set_query("START TRANSACTION") && db->exec(true);
}

void Rollback(MySQL* db) {
    if (db && db->set_query("ROLLBACK")) {
        (void)db->exec(true);
    }
}

bool Commit(MySQL* db) {
    return db && db->set_query("COMMIT") && db->exec(true);
}

bool ReadClock(MySQL* db,
               uint32_t& now_epoch,
               uint32_t& cycle_key,
               uint8_t& weekday) {
    if (!db ||
        !db->set_query(
            "seLect unix_timestamp(now()),"
            "unix_timestamp(date_format(date_sub(now(),interval 6 hour),"
            "'%%Y-%%m-%%d 06:00:00')),"
            "dayofweek(date_sub(now(),interval 6 hour))-1") ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }
    unsigned int raw_weekday = 0u;
    if (!db->get_uint(0, now_epoch) || !db->get_uint(1, cycle_key) ||
        !db->get_uint(2, raw_weekday) || raw_weekday > 6u) {
        return false;
    }
    weekday = static_cast<uint8_t>(raw_weekday);
    return true;
}

bool LoadStateForUpdate(MySQL* db,
                        uint32_t charac_no,
                        StoredState& state) {
    if (!db ||
        !db->set_query(
            "seLect cycle_key,stage,countdown_deadline,"
            "selected_reward_row,selected_reward_item_id,claim_token,"
            "mail_state,ifnull(unix_timestamp(mail_reserved_at),0) "
            "from %s where charac_no=%u for update",
            kStateTable,
            static_cast<unsigned int>(charac_no)) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }

    unsigned int stage = 0u;
    unsigned int mail_state = 0u;
    if (!db->get_uint(0, state.cycle_key) || !db->get_uint(1, stage) ||
        !db->get_uint(2, state.countdown_deadline) ||
        !db->get_uint(3, state.selected_reward_row) ||
        !db->get_uint(4, state.selected_reward_item_id) ||
        !db->get_uint(5, state.claim_token) ||
        !db->get_uint(6, mail_state) ||
        !db->get_uint(7, state.mail_reserved_epoch) ||
        stage > 0xffu || mail_state > 0xffu) {
        return false;
    }
    state.stage = static_cast<uint8_t>(stage);
    state.mail_state = static_cast<uint8_t>(mail_state);
    return IsValidStoredState(state);
}

bool StoreState(MySQL* db, uint32_t charac_no, const StoredState& state) {
    return db && IsValidStoredState(state) &&
           db->set_query(
               "upDate %s set cycle_key=%u,stage=%u,"
               "countdown_deadline=%u,selected_reward_row=%u,"
               "selected_reward_item_id=%u,claim_token=%u,mail_state=%u,"
               "mail_reserved_at=if(%u=0,null,from_unixtime(%u)),"
               "updated_at=now() where charac_no=%u",
               kStateTable,
               static_cast<unsigned int>(state.cycle_key),
               static_cast<unsigned int>(state.stage),
               static_cast<unsigned int>(state.countdown_deadline),
               static_cast<unsigned int>(state.selected_reward_row),
               static_cast<unsigned int>(state.selected_reward_item_id),
               static_cast<unsigned int>(state.claim_token),
               static_cast<unsigned int>(state.mail_state),
               static_cast<unsigned int>(state.mail_reserved_epoch),
               static_cast<unsigned int>(state.mail_reserved_epoch),
               static_cast<unsigned int>(charac_no)) &&
           db->exec(true) && db->getAffectedRowCount() <= 1u;
}

bool SendResult(int user_idx, const StateMessage& source) {
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
    StateMessage* const result = reinterpret_cast<StateMessage*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(StateMessage))));
    if (!result) {
        return false;
    }
    *result = source;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool FinishRequest(int outer_user_idx,
                   const StateMessage& response,
                   bool durable_complete) {
    if (response.operation == OPERATION_MAIL_RESULT) {
        // The outer route is deliberately userless.  A reply UID is merely a
        // post-transaction notification address: failure to find/deliver it
        // must never suppress or roll back the durable mail-result transition.
        return response.reply_user_idx >= 0
                   ? SendResult(response.reply_user_idx, response)
                   : durable_complete;
    }
    return SendResult(outer_user_idx, response);
}

bool IsValidRequest(const StateMessage& request, int user_idx) {
    if (request.charac_no == 0u ||
        !IsValidServerGroup(request.server_group) ||
        request.operation < OPERATION_LOAD ||
        request.operation > OPERATION_MAIL_RESULT) {
        return false;
    }
    if (request.operation != OPERATION_MAIL_RESULT &&
        (user_idx == -1 || request.session_token == 0u ||
         request.reply_user_idx != -1 ||
         request.mail_result_reward_item_id != 0u)) {
        return false;
    }
    if (request.operation == OPERATION_MAIL_RESULT &&
        (user_idx != -1 || request.reply_user_idx < -1 ||
         (request.reply_user_idx >= 0 && request.session_token == 0u) ||
         request.claim_token == 0u ||
         !IsValidReward(request.selected_reward_row,
                        request.selected_reward_item_id) ||
         request.mail_result_reward_item_id !=
             request.selected_reward_item_id ||
         request.action != 2u || request.mail_result > 1u)) {
        return false;
    }
    if (request.operation == OPERATION_COUNTDOWN_COMPLETE) {
        return request.action == 0u;
    }
    if (request.operation == OPERATION_SELECT_REWARD) {
        return request.action == 1u && request.selection_index < 7u &&
               IsValidReward(request.selected_reward_row,
                             request.selected_reward_item_id);
    }
    if (request.operation == OPERATION_RESERVE_MAIL) {
        return request.action == 2u &&
               IsValidReward(request.selected_reward_row,
                             request.selected_reward_item_id);
    }
    return true;
}

void FillResponseState(StateMessage& response,
                       const StoredState& state,
                       uint32_t now_epoch,
                       uint8_t weekday) {
    response.cycle_key = state.cycle_key;
    response.countdown_deadline = state.countdown_deadline;
    response.selected_reward_row = state.selected_reward_row;
    response.selected_reward_item_id = state.selected_reward_item_id;
    response.claim_token = state.claim_token;
    response.now_epoch = now_epoch;
    response.stage = state.stage;
    response.weekday = weekday;
    response.mail_state = state.mail_state;
}

}  // namespace

DB_FacebookLike::DB_FacebookLike() : DBDispatcher() {}

DB_FacebookLike::~DB_FacebookLike() {}

bool DB_FacebookLike::queueRequest(int user_idx,
                                       const StateMessage& source) {
    if (!IsValidRequest(source, user_idx)) {
        return false;
    }
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
    StateMessage* const request = reinterpret_cast<StateMessage*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(StateMessage))));
    if (!request) {
        return false;
    }
    *request = source;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_FacebookLike::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(StateMessage))) {
        return false;
    }
    const StateMessage* const request =
        reinterpret_cast<const StateMessage*>(stream->get_buffer());
    if (!request || !IsValidRequest(*request, user_idx)) {
        return false;
    }

    StateMessage response = *request;
    response.result = static_cast<uint8_t>(RESULT_DATABASE_FAILURE);
    response.show_ui = 0u;
    response.reserved[0] = 0u;

    if (!GlobalData::s_db_mgr) {
        return FinishRequest(user_idx, response, false);
    }

    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(request->server_group);
    MySQL* const db = GlobalData::s_db_mgr->isActiveGlobalDB()
                          ? GlobalData::s_db_mgr->GetDBHandleServerGroup(
                                DB_HANDLE_3, server_group)
                          : GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_3,
                                                              server_group);
    if (!db || !BeginTransaction(db)) {
        return FinishRequest(user_idx, response, false);
    }

    uint32_t now_epoch = 0u;
    uint32_t cycle_key = 0u;
    uint8_t weekday = 0u;
    if (!ReadClock(db, now_epoch, cycle_key, weekday) ||
        !db->set_query(
            "inSert ignore into %s(charac_no,cycle_key,stage) "
            "values(%u,%u,0)",
            kStateTable,
            static_cast<unsigned int>(request->charac_no),
            static_cast<unsigned int>(cycle_key)) ||
        !db->exec(true) || db->getAffectedRowCount() > 1u) {
        Rollback(db);
        return FinishRequest(user_idx, response, false);
    }
    const bool inserted = db->getAffectedRowCount() == 1u;

    StoredState state;
    if (!LoadStateForUpdate(db, request->charac_no, state)) {
        Rollback(db);
        return FinishRequest(user_idx, response, false);
    }

    if (state.cycle_key != cycle_key) {
        ResetForCycle(state, cycle_key);
        response.show_ui = 1u;
    } else if (inserted) {
        response.show_ui = 1u;
    }

    bool operation_valid = true;
    bool pending_recent = false;
    switch (request->operation) {
        case OPERATION_LOAD:
        case OPERATION_FATIGUE_SYNC:
            if (state.stage == STAGE_WAIT_FATIGUE &&
                request->used_fatigue >= kFatigueCondition) {
                state.stage = STAGE_COUNTDOWN;
                state.countdown_deadline = now_epoch + kCountdownSeconds;
            }
            if (state.stage == STAGE_COUNTDOWN &&
                state.countdown_deadline <= now_epoch) {
                state.stage = STAGE_SELECT_REWARD;
                state.countdown_deadline = 0u;
            }
            break;

        case OPERATION_COUNTDOWN_COMPLETE:
            if (state.stage == STAGE_COUNTDOWN &&
                state.countdown_deadline <= now_epoch) {
                state.stage = STAGE_SELECT_REWARD;
                state.countdown_deadline = 0u;
            } else if (state.stage != STAGE_SELECT_REWARD) {
                operation_valid = false;
            }
            break;

        case OPERATION_SELECT_REWARD: {
            const uint32_t expected_row =
                static_cast<uint32_t>(((weekday + 3u) % 7u) * 7u +
                                      request->selection_index + 1u);
            if (request->selected_reward_row != expected_row ||
                !IsValidReward(request->selected_reward_row,
                               request->selected_reward_item_id)) {
                operation_valid = false;
            } else if (state.stage == STAGE_SELECT_REWARD) {
                state.stage = STAGE_REWARD_SELECTED;
                state.selected_reward_row = request->selected_reward_row;
                state.selected_reward_item_id =
                    request->selected_reward_item_id;
            } else if (state.stage != STAGE_REWARD_SELECTED ||
                       state.mail_state != MAIL_STATE_NONE ||
                       state.selected_reward_row !=
                           request->selected_reward_row ||
                       state.selected_reward_item_id !=
                           request->selected_reward_item_id) {
                operation_valid = false;
            }
            break;
        }

        case OPERATION_RESERVE_MAIL:
            if (state.stage == STAGE_REWARD_MAILED &&
                state.mail_state == MAIL_STATE_PERSISTED &&
                state.selected_reward_row == request->selected_reward_row &&
                state.selected_reward_item_id ==
                    request->selected_reward_item_id) {
                // Duplicate action 2 after the durable ACK is idempotent.
                break;
            }
            if (state.stage != STAGE_REWARD_SELECTED ||
                state.selected_reward_row != request->selected_reward_row ||
                state.selected_reward_item_id !=
                    request->selected_reward_item_id) {
                operation_valid = false;
                break;
            }
            if (state.mail_state == MAIL_STATE_NONE) {
                if (!GenerateClaimToken(db, state.claim_token)) {
                    Rollback(db);
                    return FinishRequest(user_idx, response, false);
                }
                state.mail_state = MAIL_STATE_RESERVED;
                state.mail_reserved_epoch = now_epoch;
            } else if (state.mail_state == MAIL_STATE_RESERVED &&
                       state.mail_reserved_epoch +
                               kMailRetryLeaseSeconds <=
                           now_epoch) {
                if (!GenerateClaimToken(db, state.claim_token)) {
                    Rollback(db);
                    return FinishRequest(user_idx, response, false);
                }
                state.mail_reserved_epoch = now_epoch;
            } else {
                pending_recent = true;
            }
            break;

        case OPERATION_MAIL_RESULT:
            if (state.stage == STAGE_REWARD_MAILED &&
                state.mail_state == MAIL_STATE_PERSISTED &&
                request->mail_result != 0u &&
                state.claim_token == request->claim_token &&
                state.selected_reward_item_id ==
                    request->selected_reward_item_id) {
                // Replayed tracked success ACK: idempotent.
                break;
            }
            if (state.stage != STAGE_REWARD_SELECTED ||
                state.mail_state != MAIL_STATE_RESERVED ||
                state.claim_token != request->claim_token ||
                state.selected_reward_row != request->selected_reward_row ||
                state.selected_reward_item_id !=
                    request->selected_reward_item_id) {
                operation_valid = false;
                break;
            }
            if (request->mail_result != 0u) {
                state.stage = STAGE_REWARD_MAILED;
                state.mail_state = MAIL_STATE_PERSISTED;
            } else {
                state.mail_state = MAIL_STATE_NONE;
                state.claim_token = 0u;
                state.mail_reserved_epoch = 0u;
            }
            break;

        default:
            operation_valid = false;
            break;
    }

    response.result = static_cast<uint8_t>(
        pending_recent
            ? RESULT_PENDING_RECENT
            : (operation_valid ? RESULT_SUCCESS : RESULT_INVALID_STATE));
    if (!IsValidStoredState(state) ||
        !StoreState(db, request->charac_no, state) || !Commit(db)) {
        Rollback(db);
        response.result = static_cast<uint8_t>(RESULT_DATABASE_FAILURE);
        return FinishRequest(user_idx, response, false);
    }

    FillResponseState(response, state, now_epoch, weekday);
    if (request->operation == OPERATION_MAIL_RESULT) {
        // A cycle rollover or negative postal ACK can clear the durable token
        // before this return is serialized.  Preserve the consumed request
        // token only as response correlation so the game side always erases
        // the exact pending-mail key; it derives the authoritative session
        // token separately from the returned stage/mail_state.
        response.claim_token = request->claim_token;
    }
    // The durable transition is complete before any optional online result.
    // If the user is gone, the next login loads the authoritative stage.
    return FinishRequest(user_idx, response, true);
}

}  // namespace facebook_like_db_0828
