#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_GLASS_BOTTLE_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_GLASS_BOTTLE_EVENT_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace glass_bottle_event_db {

enum {
    // Additive Taiwan outer-846 inner route.  The shared enum owner adds the
    // symbolic value only when this feature is wired; 28 is unused in the
    // restored table at the feature freeze point.
    kTaiwanInnerCode = 28,
    kMaximumServerGroupCount = 18,
    kMaximumMailRewardCount = 6,
    kMaximumGlassBottleSendCount = 10,
    kMailTitleBytes = 29,
    kMailContentBytes = 255,
};

enum Operation {
    OPERATION_LOAD_GLASS_BOTTLE = 1,
    OPERATION_RESERVE_GLASS_BOTTLE = 2,
    OPERATION_COMMIT_GLASS_BOTTLE = 3,
    OPERATION_CANCEL_GLASS_BOTTLE = 4,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_DATABASE_FAILURE = 1,
    RESULT_INVALID_STATE = 2,
    RESULT_LEVEL = 3,
    RESULT_DAILY_LIMIT = 4,
    RESULT_NO_RECEIVER = 5,
    RESULT_PENDING = 6,
    RESULT_EVENT_CLOSED = 7,
};

enum Phase {
    PHASE_NONE = 0,
    PHASE_RESERVED = 1,
    PHASE_DELIVERED = 2,
    PHASE_CANCELLED = 3,
};

// Exact owner-local projection of the fields consumed by the legacy postal
// table.  It intentionally does not alias packed Inven_Item.
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

// Fixed DB_Q / DISPATCH_Q frame for outer signal 846, inner code 28.
// Reward order is authoritative and validated by the DB owner:
//   0 receiver 690001534; 1 sender 1248;
//   2..5 tenth-send extras 690001537,3290,3311,690000835.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t receiver_charac_no;
    uint32_t activity_day;
    uint32_t source_slot;
    int32_t server_group;
    uint64_t claim_id;
    MailReward rewards[kMaximumMailRewardCount];
    uint8_t operation;
    uint8_t result;
    uint8_t phase;
    uint8_t send_count;
    uint8_t reward_count;
    uint8_t event_active;
    uint8_t local_mutation_done;
    uint8_t daily_reset_hour;
    char mail_title[kMailTitleBytes + 1];
    char sender_mail_content[kMailContentBytes + 1];
    char receiver_mail_content[kMailContentBytes + 1];
};

static_assert(sizeof(MailReward) == 52u,
              "Glass-bottle postal projection must remain 52 bytes");
#if defined(__i386__)
static_assert(sizeof(Message) == 904u,
              "Taiwan inner-28 event frame must remain 904 bytes on i386");
#endif

class DBGlassBottleEvent : public TaiwanInterfaceDBDispatcher {
public:
    DBGlassBottleEvent();
    virtual ~DBGlassBottleEvent();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

}  // namespace tw_event_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_TW_EVENT_720_721_724_0828_H_
