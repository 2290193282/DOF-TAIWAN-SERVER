#ifndef OLDSRV_DB_DB_LOAD_DUNGEON_CLEAR_H_
#define OLDSRV_DB_DB_LOAD_DUNGEON_CLEAR_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"


// IDA_EA: 0x8419852/0x841986E/0x841989E/0x84198BC/0x841996E/
//         0x8419A76/0x8419DB4/0x8419F0A.
// IDA unit: 13_database_dispatch/DB_LoadDungeonClear.
class DB_LoadDungeonClear : public DBDispatcher {
public:
    DB_LoadDungeonClear();
    virtual ~DB_LoadDungeonClear();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool AradLoadMemberDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload);
    bool AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload);
    bool LoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload);
    bool SendResult(int signal, int user_idx, SIG_LOAD_DUNGEON_CLEAR* payload);
};

#endif  // OLDSRV_DB_DB_LOAD_DUNGEON_CLEAR_H_
