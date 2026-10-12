#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_ACHIEVEMENT_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_ACHIEVEMENT_H_

#include "old_data/db/core/db_dispatcher.h"

class DB_SaveAchievement : public DBDispatcher {
public:
    // IDA_EA: 0x844F84A
    // IDA_NAME: _ZN18DB_SaveAchievementC2Ev
    // DEMANGLED: DB_SaveAchievement::DB_SaveAchievement(void)
    DB_SaveAchievement();

    // IDA_EA: 0x845FA9C
    // IDA_NAME: _ZN18DB_SaveAchievementD2Ev
    // DEMANGLED: DB_SaveAchievement::~DB_SaveAchievement()
    // IDA_EA: 0x845FACC
    // IDA_NAME: _ZN18DB_SaveAchievementD0Ev
    // DEMANGLED: DB_SaveAchievement::~DB_SaveAchievement()
    virtual ~DB_SaveAchievement();

    // IDA_EA: 0x840DEAA
    // IDA_NAME: _ZN18DB_SaveAchievement8dispatchEiiP6Stream
    // DEMANGLED: DB_SaveAchievement::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_SAVE_ACHIEVEMENT_H_
