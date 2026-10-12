#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_PVP_MISSION_LIST_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_PVP_MISSION_LIST_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843F8FA/0x844F716/0x845BDFA/0x845BE2A.
// IDA unit: 08_quest_skill_growth/DB_SavePvPMissionList.
class DB_SavePvPMissionList : public DBDispatcher {
public:
    DB_SavePvPMissionList();
    virtual ~DB_SavePvPMissionList();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_PVP_MISSION_LIST_H_
