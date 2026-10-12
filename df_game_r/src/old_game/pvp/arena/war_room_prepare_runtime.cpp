#include "old_game/pvp/battlefield/war_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"

// IDA_EA: 0x86BD5D4
// IDA_NAME: _ZN7WarRoom7PrepareEv
// DEMANGLED: WarRoom::Prepare(void)
void WarRoom::Prepare() {
    const int dungeon_index =
        G_CDataManager()->get_war_area_counter()->dungeon_ids[room_mode_];
    CDungeon* const dungeon = G_CDataManager()->find_dungeon(dungeon_index);
    if (!dungeon) {
        // 0x86BD613 passes a literal zero to the old diagnostic, rather than
        // the selected dungeon index.  Keep that observable failure payload.
        LogManager::logFormat(
            1,
            "WarRoom.cpp",
            "void WarRoom::Prepare()",
            0x66e,
            "WARROOM G_CDataManager()->find_dungeon( %d) fail",
            0);
        return;
    }

    // The original stores the dungeon before trying its map and intentionally
    // leaves it installed when map lookup fails.
    war_field_.dungeon_ = dungeon;
    const int map_index = dungeon->war_room_data_.map_index_;
    CMap* const map = G_CDataManager()->find_map(map_index);
    if (!map) {
        LogManager::logFormat(1,
                              "WarRoom.cpp",
                              "void WarRoom::Prepare()",
                              0x678,
                              "WARROOM Find Map[%d] Error",
                              map_index);
        return;
    }

    war_field_.map_ = map;
}

// IDA_EA: 0x86BE11A
// IDA_NAME: _ZN7WarRoom23SkipLastRankResultTimerEv
// DEMANGLED: WarRoom::SkipLastRankResultTimer(void)
void WarRoom::SkipLastRankResultTimer() {
    // Original body: push ebp; mov ebp, esp; pop ebp; ret.
}
