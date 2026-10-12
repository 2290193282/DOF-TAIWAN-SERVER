#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ATTENDANCE_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ATTENDANCE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace event_attendance_db {

enum {
    // RECONSTRUCTED_0828: signal 836 is empty in both the restored DBTask and
    // Inter routing tables.  It is an internal game/DB route, not CMD 689.
    kDbSignal = 836,

    kOperationLoad = 1,
    kOperationCheckpoint = 2,
    kOperationClaim = 3,

    kResultOk = 0,
    kResultInvalidEligibility = 19,
    kResultRewardUnavailable = 22,
    kResultOperationFailed = 255,

    kDailyRewardCount = 4,
    kSumRewardCount = 3,
};

#pragma pack(push, 1)
struct MailReward {
    uint32_t item_id;
    uint32_t add_info;
    uint32_t extend_info;
    uint32_t chronicle_value;
    uint16_t endurance;
    uint16_t amplify_value;
    uint8_t seal_flag;
    uint8_t item_attr;
    uint8_t amplify_option;
    uint8_t separate_upgrade;
    uint8_t avatar_flag;
    uint8_t creature_flag;
    uint8_t item_guid[10];
    uint8_t random_option[14];
};

struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t event_key;
    uint32_t session_token;
    int32_t server_group;
    // Load/checkpoint enter DB_Q with outer user_idx=-1 so disconnect cannot
    // drop the seed or already-observed online time.  This optional UID is
    // only the post-commit response route; claims keep it at -1 and use the
    // outer UID.
    int32_t reply_user_idx;

    // The game thread accounts only server-observed wall time.  Separate
    // deltas prevent seconds before a midnight boundary from leaking into the
    // next day's four-tier progress while still preserving cumulative time.
    uint32_t observed_epoch;
    uint32_t daily_delta_seconds;
    uint32_t total_delta_seconds;

    uint32_t day_stamp;
    uint32_t daily_elapsed_seconds;
    uint32_t total_elapsed_seconds;
    uint32_t revision;

    uint8_t operation;
    uint8_t result;
    uint8_t requested_tier;
    uint8_t client_stage;
    uint8_t claimed_mask;
    uint8_t sum_claimed_mask;
    uint8_t replayed;
    uint8_t new_mail;

    uint32_t process_seconds[kDailyRewardCount];
    uint32_t sum_seconds[kSumRewardCount];
    MailReward daily_rewards[kDailyRewardCount];
    MailReward sum_rewards[kSumRewardCount];
};
#pragma pack(pop)

static_assert(sizeof(MailReward) == 50u,
              "Event-attendance mail record must remain 50 bytes");
static_assert(sizeof(Message) == 438u,
              "Event-attendance DB message wire ABI drifted");

void ClearMessage(Message& message);
bool Queue(int user_idx, const Message& message);

class DB_EventAttendance : public DBDispatcher {
public:
    DB_EventAttendance();
    virtual ~DB_EventAttendance();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

}  // namespace event_attendance_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ATTENDANCE_0828_H_
