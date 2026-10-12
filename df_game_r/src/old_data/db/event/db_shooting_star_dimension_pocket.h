#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SHOOTING_STAR_DIMENSION_POCKET_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SHOOTING_STAR_DIMENSION_POCKET_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace shooting_star_dimension_pocket_db {

enum {
    // Dedicated additive outer-846 route.  Shared wiring must first prove that
    // 31 is still unique before publishing the symbolic enum value.
    kTaiwanInnerCode = 31,
    kMaximumServerGroupCount = 18,
    kMaximumShootingRewardCount = 9,
    kMaximumDimensionClearCount = 5,
    kMailTitleBytes = 29,
    kMailContentBytes = 255,
};

enum Operation {
    OPERATION_LOAD = 1,
    OPERATION_REFRESH = 2,
    OPERATION_SHOOTING_RESERVE = 3,
    OPERATION_SHOOTING_COMMIT = 4,
    OPERATION_SHOOTING_CANCEL = 5,
    OPERATION_DIMENSION_CLEAR = 6,
    OPERATION_DIMENSION_SEED = 7,
    OPERATION_DIMENSION_RESERVE_MATERIAL = 8,
    OPERATION_DIMENSION_COMMIT_MATERIAL = 9,
    OPERATION_DIMENSION_CANCEL_MATERIAL = 10,
    OPERATION_DIMENSION_RESERVE_REWARD = 11,
    OPERATION_DIMENSION_COMMIT_REWARD = 12,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_DATABASE_FAILURE = 1,
    RESULT_INVALID_STATE = 2,
    RESULT_EVENT_CLOSED = 3,
    RESULT_ALREADY_CLAIMED = 4,
    RESULT_PENDING = 5,
    RESULT_INVALID_ITEM = 6,
};

enum Phase {
    PHASE_NONE = 0,
    PHASE_RESERVED = 1,
    PHASE_DELIVERED = 2,
};

enum ClaimKind {
    CLAIM_NONE = 0,
    CLAIM_SHOOTING_REWARD = 1,
    CLAIM_DIMENSION_MATERIAL = 2,
    CLAIM_DIMENSION_REWARD = 3,
};

// Owner-local projection of the exact legacy postal scalar fields.  It is
// intentionally independent from packed Inven_Item and safe to copy through
// DB_Q/DISPATCH_Q.
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

// Fixed inner-31 frame.  Every request echoes identity and an unguessable
// session/request pair.  Request-only mail fields are required to be zero for
// operations that cannot deliver an attachment.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t campaign_key;
    // Request: the game-side Unix epoch latched to the authoritative proper-
    // clear counter generation.  Response: the resolved YYYYMMDD activity
    // day.  Reusing this fixed slot keeps the 412-byte Taiwan ABI unchanged.
    uint32_t activity_day;
    uint32_t source_serial;
    uint32_t item_id;
    uint32_t item_count;
    uint32_t payment_count;
    // Exact pre-mutation stack count.  Reconnect recovery accepts only the
    // provable before/after images and leaves every ambiguous image pending.
    uint32_t source_before_count;
    int32_t server_group;
    uint64_t claim_id;
    uint16_t shooting_available_mask;
    uint16_t source_slot;
    uint8_t operation;
    uint8_t result;
    uint8_t phase;
    uint8_t claim_kind;
    uint8_t reward_index;
    uint8_t shooting_active;
    uint8_t dimension_active;
    uint8_t dimension_state;
    uint8_t dimension_clear_count;
    uint8_t daily_reset_hour;
    uint8_t local_mutation_done;
    // Response-only proof that the deterministic DB3 postal row already
    // exists while the DB2 claim is still reserved. Requests must send zero.
    uint8_t physical_delivery_present;
    MailReward mail_reward;
    char mail_title[kMailTitleBytes + 1];
    char mail_content[kMailContentBytes + 1];
};

static_assert(sizeof(MailReward) == 52u,
              "Taiwan 720/721 postal projection must remain 52 bytes");
#if defined(__i386__)
static_assert(sizeof(Message) == 412u,
              "Taiwan inner-31 720/721 frame must remain 412 bytes on i386");
#endif

// The only inner-31 frames allowed to cross the legacy DB_Q lower-bound
// fence. The producer must have queued SIG_SAVE_INVENTORY (35) before
// submitting one of these requests.
bool IsLegacyFencedInventoryCommit(const Message& request);

class DBShootingStarDimensionPocket : public TaiwanInterfaceDBDispatcher {
public:
    DBShootingStarDimensionPocket();
    virtual ~DBShootingStarDimensionPocket();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

}  // namespace tw_event_720_721_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_TW_EVENT_720_721_NATIVE_0828_H_
