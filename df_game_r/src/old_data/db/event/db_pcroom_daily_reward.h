#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_PCROOM_DAILY_REWARD_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_PCROOM_DAILY_REWARD_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

class CUser;
class Stream;

struct DBSelectPcroomDailyReward : public TaiwanInterfaceDBDispatcher {
    struct MSG { unsigned int account_id; };

    // IDA_EA: 0x815E46C
    // IDA_NAME: _ZN25DBSelectPcroomDailyRewardC2Ev
    DBSelectPcroomDailyReward();

    // IDA_EA: 0x815E490 (D2), 0x815E4C0 (D0)
    // IDA_NAME: _ZN25DBSelectPcroomDailyRewardD2Ev / D0Ev
    virtual ~DBSelectPcroomDailyReward();

    static void makeRequest(CUser* user);
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

struct DBUpdatePcroomDailyReward : public TaiwanInterfaceDBDispatcher {
    struct MSG {
        unsigned int account_id;
        char remain_reward_count;
        unsigned char reserved_05[3];
    };

    // IDA_EA: 0x815E4DE
    // IDA_NAME: _ZN25DBUpdatePcroomDailyRewardC2Ev
    DBUpdatePcroomDailyReward();

    // IDA_EA: 0x815E502 (D2), 0x815E532 (D0)
    // IDA_NAME: _ZN25DBUpdatePcroomDailyRewardD2Ev / D0Ev
    virtual ~DBUpdatePcroomDailyReward();

    static void makeRequest(CUser* user);
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(DBSelectPcroomDailyReward) == 0x08u,
              "DBSelectPcroomDailyReward must not extend the Taiwan DB ABI");
static_assert(sizeof(DBUpdatePcroomDailyReward) == 0x08u,
              "DBUpdatePcroomDailyReward must not extend the Taiwan DB ABI");
#endif

#endif
