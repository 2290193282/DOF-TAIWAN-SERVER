#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_BEST_CLEAR_TIME_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_BEST_CLEAR_TIME_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_SaveBestClearTime.
class DB_SaveBestClearTime : public DBDispatcher {
public:
    // IDA_EA: 0x844FCFE
    // IDA_NAME: _ZN20DB_SaveBestClearTimeC2Ev
    // DEMANGLED: DB_SaveBestClearTime::DB_SaveBestClearTime(void)
    DB_SaveBestClearTime();

    // IDA_EA: 0x845B044
    // IDA_NAME: _ZN20DB_SaveBestClearTimeD2Ev
    // DEMANGLED: DB_SaveBestClearTime::~DB_SaveBestClearTime()
    // IDA_EA: 0x845B074
    // IDA_NAME: _ZN20DB_SaveBestClearTimeD0Ev
    // DEMANGLED: DB_SaveBestClearTime::~DB_SaveBestClearTime()
    virtual ~DB_SaveBestClearTime();

    // IDA_EA: 0x8447CC4
    // IDA_NAME: _ZN20DB_SaveBestClearTime8dispatchEiiP6Stream
    // DEMANGLED: DB_SaveBestClearTime::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_BEST_CLEAR_TIME_H_
