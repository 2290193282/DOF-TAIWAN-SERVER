#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_FREELY_GIVE_ITEM_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_FREELY_GIVE_ITEM_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace freely_give_item_db {

enum {
    kTaiwanInnerCode = TaiwanInternalPack::FREELY_GIVE_ITEM,
    kEventId = 118,
    kMaximumServerGroupCount = 18,
    kCharacterNameBytes = 128,
    kMaximumNoticeCount = 3,
};

enum Operation {
    OPERATION_LOAD = 1,
    OPERATION_ONLINE_CHECKPOINT = 2,
    OPERATION_FATIGUE = 3,
    OPERATION_LEVEL_UP = 4,
    OPERATION_TIMED = 5,
    OPERATION_DISCONNECT = 6,
    OPERATION_RESERVE_ACTION = 7,
    OPERATION_COMMIT_ACTION = 8,
    OPERATION_CANCEL_ACTION = 9,
    OPERATION_SELECT_WEAPON = 10,
    OPERATION_MARK_FAILED = 11,
    // Persists the exact local post-image in the event saga before the
    // legacy signal-35 inventory snapshot is allowed to run.
    OPERATION_STAGE_ACTION = 12,
};

enum Action {
    ACTION_NONE = 0,
    ACTION_CLAIM_BOX = 1,
    ACTION_CLAIM_COUPON_LOCAL = 2,
    ACTION_CLAIM_COUPON_MAIL = 3,
    ACTION_USE_COUPON = 4,
    ACTION_CONVERT_UNLIMITED = 5,
    ACTION_RESET = 6,
    ACTION_UPGRADE_REWARD = 7,
    ACTION_SELECT_WEAPON = 8,
};

enum PersistenceProofKind {
    PERSISTENCE_PROOF_NONE = 0,
    PERSISTENCE_PROOF_INVENTORY_MONEY = 1,
};

enum SagaState {
    SAGA_STATE_NONE = 0,
    SAGA_STATE_RESERVED = 1,
    SAGA_STATE_STAGED = 2,
    SAGA_STATE_COMMITTED = 3,
    SAGA_STATE_CANCELLED = 4,
};

enum Phase {
    PHASE_NONE = 0,
    PHASE_BOX_CLAIMED = 1,
    PHASE_PERIODIC_WEAPON = 2,
    PHASE_COMPLETED = 3,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_EVENT_CLOSED = 1,
    RESULT_PENDING = 2,
    RESULT_STATE = 3,
    RESULT_ALREADY_CLAIMED = 4,
    RESULT_DIFFERENT_SERVER = 5,
    RESULT_NOT_ENOUGH_CHICKEN = 6,
    RESULT_NO_WEAPON = 7,
    RESULT_COUPON_SHORTAGE = 8,
    RESULT_BELOW_THRESHOLD = 9,
    RESULT_FAILED = 10,
    RESULT_ALREADY_RESET = 11,
    RESULT_NOT_OWNER = 12,
    RESULT_INVENTORY = 13,
    RESULT_SCRIPT = 14,
    RESULT_DATABASE_FAILURE = 255,
};

enum IncreaseReason {
    REASON_SILENT_START = 1,
    REASON_SILENT_RESET = 2,
    REASON_SILENT_RESUME = 3,
    REASON_FIRST_LOGIN = 4,
    REASON_CONSECUTIVE_LOGIN = 5,
    REASON_LEVEL_UP = 6,
    REASON_TIMED_LOGIN = 7,
    REASON_FATIGUE = 8,
    REASON_ONLINE = 9,
    REASON_OFFLINE = 10,
};

struct Notice {
    uint8_t reason;
    uint8_t award_count;
    uint8_t reserved[2];
};

// Exact postal projection used by the deterministic cross-character coupon
// and 120-day reward deliveries.  It is deliberately independent of the
// packed gameplay Inven_Item ABI.
struct MailReward {
    uint32_t item_id;
    uint32_t add_info;
    uint32_t extend_info;
    uint32_t chronicle_value;
    uint16_t endurance;
    uint16_t amplify_value;
    uint8_t seal_flag;
    uint8_t item_attr;
    uint8_t upgrade;
    uint8_t amplify_option;
    uint8_t item_guid[10];
    uint8_t random_option[14];
    uint8_t separate_upgrade;
    uint8_t avatar_flag;
    uint8_t creature_flag;
    uint8_t reserved;
};

// Fixed cross-thread record for Taiwan outer signal 846 / inner code 20.
// All temporal decisions are made with DB NOW(); observed_epoch is correlation
// evidence only and never grants progress by itself.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t observed_epoch;
    uint32_t current_level;
    uint32_t source_value;
    uint32_t action_token;
    uint32_t action_item_id;
    uint32_t action_aux;
    uint32_t revision;
    uint32_t owner_charac_no;
    uint32_t current_item_id;
    uint32_t online_remainder;
    uint32_t fatigue_remainder;
    uint32_t last_login_day;
    uint32_t last_disconnect_epoch;
    uint32_t generation;
    uint32_t pending_token;
    uint32_t pending_charac_no;
    uint32_t pending_item_id;
    uint32_t pending_aux;
    uint32_t connect_seconds;
    uint32_t offline_seconds;
    uint32_t fatigue_unit;
    uint32_t reset_money;
    int32_t server_group;
    int32_t pending_server_group;
    int32_t action_slot;
    char current_name[kCharacterNameBytes];
    char owner_name[kCharacterNameBytes];
    MailReward mail_reward;
    Notice notices[kMaximumNoticeCount];
    uint16_t countdown_seconds;
    uint8_t operation;
    uint8_t result;
    uint8_t action;
    uint8_t phase;
    uint8_t failed;
    uint8_t reset_count;
    uint8_t chicken_count;
    uint8_t extension_days;
    uint8_t normal_reward_mask;
    uint8_t disjoint_reward_mask;
    uint8_t total_reward_claimed;
    uint8_t owner_match;
    uint8_t notice_count;
    uint8_t pending_action;
    uint8_t event_active;
    uint8_t reply_required;
    uint8_t chicken_max;
    uint8_t base_extension_days;
    uint8_t unlimited_threshold_days;
    uint8_t total_extension_days;
    uint8_t each_time_unit;
    uint8_t proof_kind;
    uint8_t saga_state;
    uint8_t reserved_tail[1];
    uint32_t pre_money;
    uint32_t post_money;
    uint8_t pre_inventory_sha256[32];
    uint8_t post_inventory_sha256[32];
};

static_assert(sizeof(Notice) == 4u,
              "Phoenix notice record must remain four bytes");
static_assert(sizeof(MailReward) == 52u,
              "Phoenix postal projection must remain 52 bytes");
static_assert(sizeof(Message) == 536u,
              "Phoenix Taiwan inner payload must remain 536 bytes");

class DBFreelyGiveItem : public TaiwanInterfaceDBDispatcher {
public:
    DBFreelyGiveItem();
    virtual ~DBFreelyGiveItem();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

#if defined(__i386__)
static_assert(sizeof(DBFreelyGiveItem) == 0x08u,
              "Phoenix Taiwan DB owner must preserve the base-only ABI");
#endif

}  // namespace freely_give_item_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_FREELY_GIVE_ITEM_0828_H_
