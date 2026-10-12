#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_TOD_UPDATE_USER_STATE_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_TOD_UPDATE_USER_STATE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_TOD_Update_UserState.
class DB_TOD_Update_UserState : public DBDispatcher {
public:
    // IDA_EA: 0x844F7A2
    // IDA_NAME: _ZN23DB_TOD_Update_UserStateC2Ev
    // DEMANGLED: DB_TOD_Update_UserState::DB_TOD_Update_UserState(void)
    DB_TOD_Update_UserState();

    // IDA_EA: 0x845BC74
    // IDA_NAME: _ZN23DB_TOD_Update_UserStateD2Ev
    // DEMANGLED: DB_TOD_Update_UserState::~DB_TOD_Update_UserState()
    // IDA_EA: 0x845BCA4
    // IDA_NAME: _ZN23DB_TOD_Update_UserStateD0Ev
    // DEMANGLED: DB_TOD_Update_UserState::~DB_TOD_Update_UserState()
    virtual ~DB_TOD_Update_UserState();

    // IDA_EA: 0x84403AA
    // IDA_NAME: _ZN23DB_TOD_Update_UserState8dispatchEiiP6Stream
    // DEMANGLED: DB_TOD_Update_UserState::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_TOD_UPDATE_USER_STATE_H_
