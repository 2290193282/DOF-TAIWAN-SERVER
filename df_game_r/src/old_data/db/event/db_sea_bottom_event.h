#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SEA_BOTTOM_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SEA_BOTTOM_EVENT_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace sea_bottom_event_db {

enum {
    // Feature-owned additive route.  Shared wiring must publish inner 32 only
    // after its final uniqueness check.
    kTaiwanInnerCode = 32,
    kMaximumServerGroupCount = 18,
    kMaximumDepth = 2000,
    kSpecifiedRewardCount = 20,
    kClaimedMask = 0x000fffff,
    kMailTitleBytes = 29,
    kMailContentBytes = 255,
};

enum Operation {
    OPERATION_LOAD = 1,
    OPERATION_REFRESH = 2,
    OPERATION_RESERVE = 3,
    OPERATION_COMMIT = 4,
    OPERATION_CANCEL = 5,
    // Persist the immutable mail projection only after DB_HANDLE_3 proves
    // the exact inventory -1 mutation.  COMMIT may replay this projection
    // indefinitely without regenerating time-dependent item fields.
    OPERATION_PREPARE_MAIL = 6,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_DATABASE_FAILURE = 1,
    RESULT_INVALID_STATE = 2,
    RESULT_EVENT_CLOSED = 3,
    RESULT_PENDING = 4,
    RESULT_INVALID_ITEM = 5,
    RESULT_INVALID_DEPTH = 6,
    RESULT_RECOVERY_AMBIGUOUS = 7,
};

enum Phase {
    PHASE_NONE = 0,
    PHASE_RESERVED = 1,
    PHASE_DELIVERED = 2,
    PHASE_CANCELLED = 3,
    PHASE_MAIL_PREPARED = 4,
};

// Owner-local projection of the exact legacy postal scalar fields.  Packed
// Inven_Item never crosses DB_Q/DISPATCH_Q.
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

// Fixed inner-32 frame.  All client-derived input is revalidated before the
// reserve, while commit/cancel can only address the immutable reserved row.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t campaign_key;
    uint32_t revision;
    uint32_t current_depth;
    uint32_t target_depth;
    uint32_t claimed_mask;
    uint32_t claimed_mask_after;
    uint32_t box_item_id;
    uint32_t box_count_before;
    uint32_t box_total_before;
    uint32_t reward_item_id;
    uint32_t reward_count;
    int32_t server_group;
    uint64_t move_id;
    uint16_t source_slot;
    int16_t box_weight;
    uint8_t operation;
    uint8_t result;
    uint8_t phase;
    uint8_t active;
    uint8_t local_mutation_done;
    uint8_t rollover;
    uint8_t client_reply_expected;
    uint8_t recovery_action;
    uint8_t inventory_pre_sha256[32];
    uint8_t inventory_post_sha256[32];
    MailReward mail_reward;
    char mail_title[kMailTitleBytes + 1];
    char mail_content[kMailContentBytes + 1];
};

static_assert(sizeof(MailReward) == 52u,
              "Sea Bottom postal projection must remain 52 bytes");
#if defined(__i386__)
static_assert(sizeof(Message) == 488u,
              "Taiwan inner-32 Sea Bottom frame must remain 488 bytes");
#endif

// The only inner-32 frame allowed to cross the legacy DB_Q lower-bound
// fence. It freezes the mail projection only after SIG_SAVE_INVENTORY (35)
// has made the exact post-mutation inventory image durable.
bool IsLegacyFencedInventoryCommit(const Message& request);

class DBSeaBottomEvent : public TaiwanInterfaceDBDispatcher {
public:
    DBSeaBottomEvent();
    virtual ~DBSeaBottomEvent();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

}  // namespace tw_seabottom_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_TW_SEABOTTOM_NATIVE_0828_H_
