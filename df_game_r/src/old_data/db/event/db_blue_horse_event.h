#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_BLUE_HORSE_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_BLUE_HORSE_EVENT_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace blue_horse_event_db {

enum {
    // Feature-owned additive route.  Shared enum/dispatcher wiring is listed
    // in the report and intentionally not changed by this translation unit.
    kTaiwanInnerCode = 29,
    kMaximumServerGroupCount = 18,
    kSlotCount = 3,
    kMaximumRewardCount = 2,
    kMailTitleBytes = 29,
    kMailContentBytes = 255,
};

enum Operation {
    OPERATION_LOAD_BLUE_HORSE = 1,
    OPERATION_ADD_BLUE_HORSE_FATIGUE = 2,
    OPERATION_ROLL_BLUE_HORSE_SLOT = 3,
    OPERATION_RESERVE_BLUE_HORSE_REWARD = 4,
    OPERATION_COMMIT_BLUE_HORSE_REWARD = 5,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_DATABASE_FAILURE = 1,
    RESULT_INVALID_STATE = 2,
    RESULT_NO_TICKET = 3,
    RESULT_SLOT_ALREADY_ROLLED = 4,
    RESULT_PENDING = 5,
    RESULT_EVENT_CLOSED = 6,
};

enum Phase {
    PHASE_NONE = 0,
    PHASE_RESERVED = 1,
    PHASE_DELIVERED = 2,
};

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

// Fixed DB_Q/DISPATCH_Q frame.  Request-only fields are required to remain
// zero unless the operation explicitly owns them; DB validation rejects a
// widened or partially initialized frame.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    // Both the day row and a durable claim are character-scoped.  The reward
    // target must therefore remain the same owned character that reserved it.
    uint32_t reward_charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t activity_day;
    uint32_t fatigue_delta;
    uint32_t combo_item_id;
    uint32_t combo_item_count;
    uint32_t complete_item_id;
    uint32_t complete_item_count;
    int32_t server_group;
    uint64_t claim_id;
    MailReward rewards[kMaximumRewardCount];
    char combo_mail_title[kMailTitleBytes + 1];
    char combo_mail_content[kMailContentBytes + 1];
    char complete_mail_title[kMailTitleBytes + 1];
    char complete_mail_content[kMailContentBytes + 1];
    uint8_t operation;
    uint8_t result;
    uint8_t phase;
    uint8_t event_active;
    uint8_t show_ui;
    uint8_t daily_reset_hour;
    uint8_t slot_index;
    uint8_t roll_result;
    uint8_t ticket_count;
    uint8_t fatigue_remainder;
    uint8_t distance;
    uint8_t slots[kSlotCount];
    uint8_t completed;
    uint8_t combo_symbol;
    uint8_t combo_repeat;
    uint8_t combo_reward_sent;
    uint8_t complete_reward_sent;
    uint8_t reward_count;
};

static_assert(sizeof(MailReward) == 52u,
              "Blue-horse postal projection must remain 52 bytes");
#if defined(__i386__)
static_assert(sizeof(Message) == 752u,
              "Blue-horse DB_Q frame must remain 752 bytes on i386");
#endif

class DBBlueHorseEvent : public TaiwanInterfaceDBDispatcher {
public:
    DBBlueHorseEvent();
    virtual ~DBBlueHorseEvent();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

}  // namespace late_event_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_LATE_EVENT_726_731_0828_H_
