#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SUGAR_TIME_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SUGAR_TIME_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace sugar_time_db {

enum {
    // Additive Taiwan outer-846 route.  Inner 30 is intentionally kept out
    // of the shared enum until the root integration transaction owns both
    // DB and Inter tables.
    kTaiwanInnerCode = 30,
    kMaximumServerGroupCount = 18,
    kDailyRewardCount = 4,
    kGoalRewardCount = 3,
};

enum Operation {
    OPERATION_LOAD = 1,
    OPERATION_CHECKPOINT = 2,
    OPERATION_CLAIM_DAY = 3,
    OPERATION_CLAIM_FINAL = 4,
    OPERATION_PICK_FINAL = 5,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_DATABASE_FAILURE = 1,
    RESULT_INVALID_ELIGIBILITY = 2,
    RESULT_INVALID_CONFIG = 3,
    RESULT_CAMPAIGN_INACTIVE = 4,
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

// Fixed outer-846/inner-30 DB_Q and DISPATCH_Q frame.  All client-controlled
// values are reduced to the exact command field before entering this frame;
// item records and thresholds are revalidated by the DB worker.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t event_key;
    uint32_t session_token;
    uint32_t observed_epoch;
    int32_t server_group;

    uint32_t day_stamp;
    uint32_t daily_elapsed_seconds;
    uint32_t total_elapsed_seconds;
    uint32_t revision;
    uint32_t selected_goal_seconds;

    uint32_t process_seconds[kDailyRewardCount];
    uint32_t goal_seconds[kGoalRewardCount];
    MailReward daily_rewards[kDailyRewardCount];
    MailReward goal_rewards[kGoalRewardCount];

    uint8_t operation;
    uint8_t result;
    uint8_t requested_stage;
    uint8_t daily_claimed_count;
    uint8_t final_claimed;
    uint8_t replayed;
    uint8_t new_mail;
    // Copied from CEnvironment::get_fatigue_time().  The DB campaign row
    // must carry the same reset hour before any load or mutation is accepted.
    uint8_t daily_reset_hour;
};
#pragma pack(pop)

static_assert(sizeof(MailReward) == 50u,
              "SugarTime postal projection must remain 50 bytes");
static_assert(sizeof(Message) == 430u,
              "SugarTime inner-30 message wire ABI drifted");

void ClearMessage(Message& message);

class DBSugarTime : public TaiwanInterfaceDBDispatcher {
public:
    DBSugarTime();
    virtual ~DBSugarTime();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

}  // namespace sugar_time_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SUGAR_TIME_0828_H_
