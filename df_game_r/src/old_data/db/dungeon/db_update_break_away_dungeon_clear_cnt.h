#ifndef DB_UPDATE_BREAK_AWAY_DUNGEON_CLEAR_CNT_H_
#define DB_UPDATE_BREAK_AWAY_DUNGEON_CLEAR_CNT_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 09_dungeon_pvp_party/DB_UpdateBreakAwayDungeonClearCnt.
class DB_UpdateBreakAwayDungeonClearCnt : public DBDispatcher {
public:
    DB_UpdateBreakAwayDungeonClearCnt();
    virtual ~DB_UpdateBreakAwayDungeonClearCnt();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // DB_UPDATE_BREAK_AWAY_DUNGEON_CLEAR_CNT_H_
