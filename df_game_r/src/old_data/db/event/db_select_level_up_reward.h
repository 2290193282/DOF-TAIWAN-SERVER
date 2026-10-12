#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_LEVEL_UP_REWARD_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_LEVEL_UP_REWARD_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 11_event_runtime/DB_SelectLevelUpReward.
// IDA_EA: dispatch @0x84423CC; makeRequest @0x844271A; ctor/dtor
// @0x844F8D6/@0x845BA04/@0x845BA34. DBTask slot 654.
class DB_SelectLevelUpReward : public DBDispatcher {
public:
    DB_SelectLevelUpReward();
    virtual ~DB_SelectLevelUpReward();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int charac_no,
                            int charac_level);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_LEVEL_UP_REWARD_H_
