#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_KILL_MONSTER_INFO_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_KILL_MONSTER_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SaveKillMonsterInfo.
class DB_SaveKillMonsterInfo : public DBDispatcher {
public:
    DB_SaveKillMonsterInfo();
    virtual ~DB_SaveKillMonsterInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(DB_SaveKillMonsterInfo) == 0x04u,
              "DB_SaveKillMonsterInfo IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_KILL_MONSTER_INFO_H_
