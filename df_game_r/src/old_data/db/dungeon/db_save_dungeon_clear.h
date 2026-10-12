#ifndef OLDSRV_DB_DB_SAVE_DUNGEON_CLEAR_H_
#define OLDSRV_DB_DB_SAVE_DUNGEON_CLEAR_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"


// IDA_EA: 0x8419FEC/0x841A008/0x841A038/0x841A056/0x841A0A6/0x841A1A2.
// IDA unit: 13_database_dispatch/DB_SaveDungeonClear.
class DB_SaveDungeonClear : public DBDispatcher {
public:
    DB_SaveDungeonClear();
    virtual ~DB_SaveDungeonClear();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool SaveDungeonClear(SIG_SAVE_DUNGEON_CLEAR* payload);
    bool SendResult(int signal, int user_idx, SIG_SAVE_DUNGEON_CLEAR* payload);
};

#endif  // OLDSRV_DB_DB_SAVE_DUNGEON_CLEAR_H_
