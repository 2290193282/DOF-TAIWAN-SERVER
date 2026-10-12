#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_FACEBOOK_LIKE_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_FACEBOOK_LIKE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace facebook_like_db {

enum {
    // Rebuilt-only DB/Inter route.  Slot 835 is empty in both restored
    // 852-entry tables and is not a client packet identifier.
    kDbSignal = 835,
    kMaximumServerGroupCount = 18,
    kFatigueCondition = 7,
    kCountdownSeconds = 420,
    kMailRetryLeaseSeconds = 60,
};

enum Operation {
    OPERATION_LOAD = 1,
    OPERATION_FATIGUE_SYNC = 2,
    OPERATION_COUNTDOWN_COMPLETE = 3,
    OPERATION_SELECT_REWARD = 4,
    OPERATION_RESERVE_MAIL = 5,
    OPERATION_MAIL_RESULT = 6,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_DATABASE_FAILURE = 1,
    RESULT_INVALID_STATE = 2,
    RESULT_PENDING_RECENT = 3,
};

enum Stage {
    STAGE_WAIT_FATIGUE = 0,
    STAGE_COUNTDOWN = 1,
    STAGE_SELECT_REWARD = 2,
    STAGE_REWARD_SELECTED = 3,
    STAGE_REWARD_MAILED = 4,
};

enum MailState {
    MAIL_STATE_NONE = 0,
    MAIL_STATE_RESERVED = 1,
    MAIL_STATE_PERSISTED = 2,
};

// Fixed DB_Q / DISPATCH_Q payload.  The game-owned session token prevents a
// delayed result from being applied after the pooled CUser changes character.
struct StateMessage {
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t cycle_key;
    uint32_t countdown_deadline;
    uint32_t selected_reward_row;
    uint32_t selected_reward_item_id;
    uint32_t claim_token;
    // Immutable tracked-ACK correlation. selected_reward_item_id is the
    // authoritative response state and can legitimately become zero at 06:00.
    uint32_t mail_result_reward_item_id;
    uint32_t now_epoch;
    int32_t server_group;
    // OPERATION_MAIL_RESULT always enters DB_Q with outer user_idx=-1 so the
    // durable SQL018 transition cannot be rejected by DBDispatcher when the
    // CUser disconnects.  This field is only a best-effort post-commit reply
    // route; the game side still validates the full session/claim identity.
    int32_t reply_user_idx;
    uint16_t used_fatigue;
    uint8_t operation;
    uint8_t result;
    uint8_t stage;
    uint8_t weekday;
    uint8_t action;
    uint8_t selection_index;
    uint8_t mail_state;
    uint8_t mail_result;
    uint8_t show_ui;
    uint8_t reserved[1];
};

static_assert(sizeof(StateMessage) == 56u,
              "CMD725 internal DB state payload must remain 56 bytes");

class DB_FacebookLike : public DBDispatcher {
public:
    DB_FacebookLike();
    virtual ~DB_FacebookLike();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // user_idx must be -1 for OPERATION_MAIL_RESULT.  The live UID, when one
    // is still strictly correlated, travels only in StateMessage as a
    // best-effort result route and never gates the InnoDB transaction.
    static bool queueRequest(int user_idx, const StateMessage& request);
};

}  // namespace facebook_like_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_FACEBOOK_LIKE_0828_H_
