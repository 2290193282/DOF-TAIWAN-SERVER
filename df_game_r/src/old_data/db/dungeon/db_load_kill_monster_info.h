#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_LOAD_KILL_MONSTER_INFO_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_LOAD_KILL_MONSTER_INFO_H_

#include "old_data/db/core/db_dispatcher.h"

struct SIG_LOAD_KILL_MONSTER_INFO;


// IDA_EA: 0x84414E0/0x844159A/0x84419AC/0x844F812/0x845BB8A/0x845BBBA.
// IDA unit: 13_database_dispatch/DB_LoadKillMonsterInfo.
class DB_LoadKillMonsterInfo : public DBDispatcher {
public:
    DB_LoadKillMonsterInfo();
    virtual ~DB_LoadKillMonsterInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO* payload);
    bool SendResult(int signal, int user_idx, SIG_LOAD_KILL_MONSTER_INFO* payload);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(DB_LoadKillMonsterInfo) == 0x04u,
              "DB_LoadKillMonsterInfo IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_LOAD_KILL_MONSTER_INFO_H_
