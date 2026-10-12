#ifndef DNF_DB_PVP_STAT_DISPATCHERS_H_
#define DNF_DB_PVP_STAT_DISPATCHERS_H_

#include "old_data/db/core/db_dispatcher.h"


struct MSG_STAT_DUNGEONCLEAR;
struct MSG_STAT_PVPPLAY;

// IDA unit: 09_dungeon_pvp_party/DB_StatDungeonClear.
class DB_StatDungeonClear : public DBDispatcher {
public:
    DB_StatDungeonClear();
    virtual ~DB_StatDungeonClear();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    int LogWrite(MSG_STAT_DUNGEONCLEAR* message);
};

// IDA unit: 09_dungeon_pvp_party/DB_StatPvpPlay.
class DB_StatPvpPlay : public DBDispatcher {
public:
    DB_StatPvpPlay();
    virtual ~DB_StatPvpPlay();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    int LogWrite(MSG_STAT_PVPPLAY* message);
};

#endif  // DNF_DB_PVP_STAT_DISPATCHERS_H_
