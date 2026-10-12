#ifndef DB_LOAD_BREAK_AWAY_REWARD_DATA_H_
#define DB_LOAD_BREAK_AWAY_REWARD_DATA_H_

#include "old_data/db/core/db_dispatcher.h"

struct STBreakAwayRewards;

// IDA unit: 11_event_runtime/DB_LoadBreakAwayRewardData.
// IDA_EA: makeRequest @0x843915C; dispatch @0x84392DC; ctor/dtor
// @0x844ED3E/@0x845CD84/@0x845CDB4. Request signal 322, reply 323.
class DB_LoadBreakAwayRewardData : public DBDispatcher {
public:
    DB_LoadBreakAwayRewardData();
    virtual ~DB_LoadBreakAwayRewardData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned int charac_no,
                            STBreakAwayRewards& rewards);
};

#endif  // DB_LOAD_BREAK_AWAY_REWARD_DATA_H_
