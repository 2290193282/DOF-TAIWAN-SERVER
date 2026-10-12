#include "old_game/pvp/battlefield/battle_field.h"

#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"

// IDA_EA: 0x81517E0
// IDA_NAME: _ZN7MapInfo8Add_ItemE8map_item
// DEMANGLED: MapInfo::Add_Item(map_item)
void MapInfo::Add_Item(map_item item) {
    if (items_.find(static_cast<int>(item.unique_id_)) != items_.end()) {
        LogManager::logFormat(1,
                              "localglobal/../battle_field.h",
                              "Add_Item",
                              518,
                              "item.find(%u) fail",
                              item.unique_id_);
        return;
    }
    items_.insert(std::make_pair(static_cast<int>(item.unique_id_), item));
}

// IDA_EA: 0x8151612
// IDA_NAME: _ZN7MapInfo7Add_MobE11map_monster
// DEMANGLED: MapInfo::Add_Mob(map_monster)
void MapInfo::Add_Mob(map_monster monster) {
    const int unique_id = monster.map_monster_unique_id_;
    if (monsters_.find(unique_id) != monsters_.end()) {
        LogManager::logFormat(1,
                              "localglobal/../battle_field.h",
                              "Add_Mob",
                              0x1f2,
                              "monster.find(%u) fail",
                              static_cast<unsigned int>(unique_id));
        return;
    }

    monsters_.insert(std::make_pair(unique_id, monster));
    monster_identity_by_unique_id_.insert(
        std::make_pair(unique_id, monster.monster_id_));
    monster_order_.push_back(unique_id);
    if (monster.counted_by_map_info_) {
        ++named_monster_count_;
    }
}

// IDA_EA: 0x822CC48
// IDA_NAME: _ZN13CBattle_Field17GetMapInfoFromPosEiib
// DEMANGLED: CBattle_Field::GetMapInfoFromPos(int,int,bool)
MapInfo* CBattle_Field::GetMapInfoFromPos(int pos_x, int pos_y,
                                          bool layered) {
    MapInfo* canonical = 0;
    const zombie_map_runtime::HookResult zombie_result =
        zombie_map_runtime::ResolveCanonicalMapInfo(
            this, pos_x, pos_y, layered, &canonical);
    if (zombie_result != zombie_map_runtime::kHookNotManaged) {
        // A rejected registered adapter receives its RoomState-owned poison
        // MapInfo (map_index 0); it must never split back to local map_infos_.
        return canonical;
    }
    const int map_info_key = GetMapInfoKeyValue(pos_x, pos_y);
    std::map<int, MapInfo>::iterator map_info = map_infos_.find(map_info_key);
    if (map_info != map_infos_.end()) {
        if (!layered && maze_) {
            const GridScript& grid = maze_->getGridR(pos_x, pos_y);
            if (!grid.values_1c.empty()) {
                MapInfo* const layered_map_info = getCurrentLayeredMapInfo();
                if (layered_map_info->completed_) {
                    return layered_map_info;
                }
            }
        }
        return &map_info->second;
    }

    MapInfo new_map_info;
    map_infos_.insert(std::make_pair(map_info_key, new_map_info));
    map_info = map_infos_.find(map_info_key);
    return &map_info->second;
}

// IDA_EA: 0x830DF88
// IDA_NAME: _ZN13CBattle_Field22getLayeredMapInfoIndexEi
// DEMANGLED: CBattle_Field::getLayeredMapInfoIndex(int)
MapInfo* CBattle_Field::getLayeredMapInfoIndex(int layered_map_index) {
    int pos_x = -1;
    int pos_y = -1;
    getCurPosXY(pos_x, pos_y);
    MapInfo* canonical = 0;
    const zombie_map_runtime::HookResult zombie_result =
        zombie_map_runtime::ResolveCanonicalMapInfo(
            this, pos_x, pos_y, true, &canonical);
    if (zombie_result != zombie_map_runtime::kHookNotManaged) {
        static_cast<void>(layered_map_index);
        return canonical;
    }
    const int map_info_key = getLayeredMapKeyValue(layered_map_index);
    std::map<int, MapInfo>::iterator map_info = map_infos_.find(map_info_key);
    if (map_info != map_infos_.end()) {
        return &map_info->second;
    }

    MapInfo new_map_info;
    map_infos_.insert(std::make_pair(map_info_key, new_map_info));
    map_info = map_infos_.find(map_info_key);
    return &map_info->second;
}
