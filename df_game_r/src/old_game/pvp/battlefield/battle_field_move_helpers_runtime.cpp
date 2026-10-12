#include "old_game/pvp/battlefield/battle_field.h"

#include <cstdlib>
#include <map>
#include <utility>
#include <vector>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

namespace {

const char* MapKind(CBattle_Field* field, int pos_x, int pos_y) {
    const MapInfo* const info =
        field->GetMapInfoFromPos(pos_x, pos_y, false);
    return info->completed_ ? "layeredMap" : "generalMap";
}

}  // namespace

// IDA_EA: 0x830766E
// IDA_NAME: _ZN13CBattle_Field13moveMap_1_LogEii
void CBattle_Field::moveMap_1_Log(int pos_x, int pos_y) {
    const MapInfo* const info = GetMapInfoFromPos(pos_x, pos_y, false);
    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "void CBattle_Field::moveMap_1_Log(int, int)",
        0x1126,
        "TRY MoveMap [1][Dungeon Type : %d][Map Index : %d](%d,%d) -> (%d,%d) [Map Kind : %s]",
        dungeon_index_,
        info->map_index_,
        current_position_.first,
        current_position_.second,
        pos_x,
        pos_y,
        MapKind(this, pos_x, pos_y));
}

// IDA_EA: 0x8307740
// IDA_NAME: _ZN13CBattle_Field13moveMap_2_LogEii
void CBattle_Field::moveMap_2_Log(int pos_x, int pos_y) {
    const MapInfo* const info = GetMapInfoFromPos(pos_x, pos_y, false);
    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "void CBattle_Field::moveMap_2_Log(int, int)",
        0x1138,
        "TRY MoveMap [2][Dungeon Type : %d][Map Index : %d](%d,%d) -> (%d,%d) [Map Kind : %s]",
        dungeon_index_,
        info->map_index_,
        current_position_.first,
        current_position_.second,
        pos_x,
        pos_y,
        MapKind(this, pos_x, pos_y));
}

// IDA_EA: 0x8307812
// IDA_NAME: _ZN13CBattle_Field13moveMap_3_LogEii
void CBattle_Field::moveMap_3_Log(int pos_x, int pos_y) {
    const MapInfo* const info = GetMapInfoFromPos(pos_x, pos_y, false);
    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "void CBattle_Field::moveMap_3_Log(int, int)",
        0x114a,
        "TRY MoveMap [3][Dungeon Type : %d][Map Index : %d](%d,%d) -> (%d,%d) [Map Kind : %s]",
        dungeon_index_,
        info->map_index_,
        current_position_.first,
        current_position_.second,
        pos_x,
        pos_y,
        MapKind(this, pos_x, pos_y));
}

// IDA_EA: 0x83078E4
// IDA_NAME: _ZN13CBattle_Field13moveMap_4_LogEiiPK10GridScript
void CBattle_Field::moveMap_4_Log(int /*pos_x*/,
                                 int /*pos_y*/,
                                 const GridScript* grid) {
    if (!grid) {
        return;
    }
    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "void CBattle_Field::moveMap_4_Log(int, int, const GridScript*)",
        0x115b,
        "TRY MoveMap [4] Error(G_CDataManager()->find_dungeon( m_Dungeon )) Dungeon[%d] Pos(%d,%d), gs->mapIndex(%d) [Map Kind : %s], gs->layeredMapIndex(%d)\n",
        dungeon_index_,
        current_position_.first,
        current_position_.second,
        static_cast<int>(grid->values_04.size()),
        MapKind(this, current_position_.first, current_position_.second),
        static_cast<int>(grid->values_1c.size()));
}

// IDA_EA: 0x83079BA
// IDA_NAME: _ZN13CBattle_Field14check_boss_mapEii
int CBattle_Field::check_boss_map(int pos_x, int pos_y) {
    if (maze_boss_position_.first != pos_x ||
        maze_boss_position_.second != pos_y ||
        !dungeon_ ||
        dungeon_->enter_boss_room_condition_.enabled != 1) {
        return 0;
    }

    for (std::vector<
             CBattle_Field::stConditionEnterBossRoomStatus_t>::const_iterator
             it = boss_room_conditions_.begin();
         it != boss_room_conditions_.end();
         ++it) {
        if (it->boss_room_id < it->status) {
            LogManager::logFormat(
                1,
                "battle_field.cpp",
                "int CBattle_Field::check_boss_map(int, int)",
                0x1181,
                "TRY MoveMap error condition [%d][%d]/[%d]",
                it->condition_type,
                it->boss_room_id,
                it->status);
            return 7;
        }
    }
    return 0;
}

// IDA_EA: 0x8307B2A
// IDA_NAME: _ZN13CBattle_Field19check_movemap_rangeEii
int CBattle_Field::check_movemap_range(int pos_x, int pos_y) {
    int current_x = -1;
    int current_y = -1;
    getCurPosXY(current_x, current_y);
    const int delta_x = std::abs(pos_x - current_x);
    const int delta_y = std::abs(pos_y - current_y);
    if (delta_x <= 1 && delta_y <= 1 &&
        (delta_x != 1 || delta_y != 1)) {
        return 0;
    }

    const MapInfo* const info =
        GetMapInfoFromPos(pos_x, pos_y, false);
    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "int CBattle_Field::check_movemap_range(int, int)",
        0x119f,
        "TRY MoveMap [3][Dungeon Type : %d][Map Index : %d](%d,%d) -> (%d,%d) [Map Kind : %s]",
        dungeon_index_,
        info->map_index_,
        current_position_.first,
        current_position_.second,
        pos_x,
        pos_y,
        MapKind(this, pos_x, pos_y));
    return 21;
}

// IDA_EA: 0x8307D0C
// IDA_NAME: _ZN13CBattle_Field18on_not_visited_mapEiiRb
CBattle_Field* CBattle_Field::on_not_visited_map(int pos_x,
                                                 int pos_y,
                                                 bool& layered) {
    layered = true;
    GetCurrentMapInfo()->visited_goto_ = 1;
    swap_curpos_xy(pos_x, pos_y);
    const int map_index = GetCurrentMapInfo()->map_index_;
    CMap* const map = G_CDataManager()->find_map(map_index);
    if (map) {
        current_map_ = map;
    }
    return this;
}

// IDA_EA: 0x8307D7C
// IDA_NAME: _ZN13CBattle_Field17check_visited_mapEiiRb
int CBattle_Field::check_visited_map(int pos_x,
                                     int pos_y,
                                     bool& layered) {
    if (dungeon_ &&
        static_cast<unsigned char>(dungeon_->IsEnterEachMap())) {
        if (static_cast<unsigned char>(
                is_visited_map_goto(pos_x, pos_y))) {
            on_not_visited_map(pos_x, pos_y, layered);
            return 1;
        }
    } else if (static_cast<unsigned char>(
                   is_visited_map(pos_x, pos_y))) {
        on_not_visited_map(pos_x, pos_y, layered);
        return 1;
    }
    return 0;
}

// IDA_EA: 0x8307ECA
// IDA_NAME: _ZN13CBattle_Field36choose_random_bossmap_from_dummymapsEPK8CDungeonPK10GridScriptRiRbiii
void CBattle_Field::choose_random_bossmap_from_dummymaps(
    const CDungeon* dungeon,
    const GridScript* grid,
    int& selected_map,
    bool& visited,
    int pos_x,
    int pos_y,
    int invade_direction) {
    if (!dungeon || !grid || !maze_) {
        return;
    }

    if (static_cast<unsigned char>(dungeon->IsHaveDummyMap())) {
        if (maze_boss_position_.first != pos_x ||
            maze_boss_position_.second != pos_y) {
            return;
        }

        if (map_path_index_ >= map_path_.size()) {
            MapInfo* const info =
                GetMapInfoFromPos(pos_x, pos_y, false);
            if (info->dummy_boss_map_pending_) {
                info->dummy_boss_map_pending_ = 0;
            }
            return;
        }

        maze_boss_position_ = map_path_[map_path_index_];
        MapInfo* const boss_info =
            GetMapInfoFromPos(maze_boss_position_.first,
                              maze_boss_position_.second,
                              false);
        if (boss_info->visited_goto_) {
            boss_info->visited_goto_ = 0;
            boss_info->map_clearable_ = 2;
        }

        int current_x = -1;
        int current_y = -1;
        getCurPosXY(current_x, current_y);
        if (maze_boss_position_.first == current_x &&
            maze_boss_position_.second == current_y) {
            visited = false;
            boss_info->map_clearable_ = 2;
        }

        selected_map = dungeon->ChooseRandomMap(
            2, grid->grid_type, invade_direction);
        GetMapInfoFromPos(pos_x, pos_y, false)->dummy_boss_map_pending_ = 1;
        ++map_path_index_;
        return;
    }

    if (dungeon->isEventDungeon()) {
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 maze_->values_20.begin();
             it != maze_->values_20.end();
             ++it) {
            if (it->first == pos_x && it->second == pos_y) {
                maze_boss_position_ = std::make_pair(pos_x, pos_y);
                (void)maze_->getGridR(pos_x, pos_y);
                return;
            }
        }
    }
}

// IDA_EA: 0x8308214
// IDA_NAME: _ZN13CBattle_Field18get_candidator_mapEPK8CDungeonPK10GridScriptRiRSt6vectorIiSaIiEEiii
int CBattle_Field::get_candidator_map(const CDungeon* dungeon,
                                      const GridScript* grid,
                                      int& selected_map,
                                      std::vector<int>& candidates,
                                      int pos_x,
                                      int pos_y,
                                      int invade_direction) {
    if (!dungeon || !grid) {
        return 23;
    }
    if (selected_map != -1) {
        return 0;
    }

    if (maze_boss_position_.first == pos_x &&
        maze_boss_position_.second == pos_y) {
        if (!grid->values_10.empty()) {
            for (std::vector<int>::const_iterator it =
                     grid->values_10.begin();
                 it != grid->values_10.end();
                 ++it) {
                CMap* const map = G_CDataManager()->find_map(*it);
                if (!map) {
                    return 23;
                }
                if (static_cast<unsigned char>(
                        map->CheckEntrance(invade_direction))) {
                    candidates.push_back(*it);
                }
            }
        } else {
            selected_map = dungeon->ChooseRandomMap(
                1, grid->grid_type, invade_direction);
        }
    } else if (maze_boss_neighbor_position_.first == pos_x &&
               maze_boss_neighbor_position_.second == pos_y &&
               dungeon_select_parameter_) {
        selected_map = dungeon_select_parameter_;
        if (selected_map <= 0) {
            LogManager::logFormat(
                1,
                "battle_field.cpp",
                "int CBattle_Field::get_candidator_map(const CDungeon*, const GridScript*, int&, INT_VECTOR&, int, int, int)",
                0x12bb,
                "[CBattle_Field::MoveMap] Error(quest_stage_map_==-1) Dungeon[%d] Pos(%d,%d), candidator(%d), gs->mapIndex(%d), [Map Kind : %s], gs->layeredMapIndex(%d)\n",
                dungeon_index_,
                current_position_.first,
                current_position_.second,
                static_cast<int>(candidates.size()),
                static_cast<int>(grid->values_04.size()),
                MapKind(this,
                        current_position_.first,
                        current_position_.second),
                static_cast<int>(grid->values_1c.size()));
        }
    } else if (quest_stage_pos_x_ == pos_x &&
               quest_stage_pos_y_ == pos_y &&
               hell_party_enabled_) {
        selected_map = quest_stage_map_index_;
        quest_stage_map_selected_ = 1;
    } else if (!grid->values_04.empty()) {
        for (std::vector<int>::const_iterator it =
                 grid->values_04.begin();
             it != grid->values_04.end();
             ++it) {
            CMap* const map = G_CDataManager()->find_map(*it);
            if (!map) {
                return 23;
            }
            if (static_cast<unsigned char>(
                    map->CheckEntrance(invade_direction))) {
                candidates.push_back(*it);
            }
        }
        if (GetCurrentMapInfo()->completed_ != 1u &&
            candidates.empty()) {
            for (std::vector<int>::const_iterator it =
                     grid->values_04.begin();
                 it != grid->values_04.end();
                 ++it) {
                (void)G_CDataManager()->find_map(*it);
                LogManager::logFormat(
                    1,
                    "battle_field.cpp",
                    "int CBattle_Field::get_candidator_map(const CDungeon*, const GridScript*, int&, INT_VECTOR&, int, int, int)",
                    0x12e4,
                    "[CBattle_Field::MoveMap] Error(< map_candidator.size()==0) Dungeon[%d] Pos(%d,%d), (Direction: %d) \n",
                    dungeon_index_,
                    current_position_.first,
                    current_position_.second,
                    invade_direction);
                LogManager::logFormat(
                    1,
                    "battle_field.cpp",
                    "int CBattle_Field::get_candidator_map(const CDungeon*, const GridScript*, int&, INT_VECTOR&, int, int, int)",
                    0x12e5,
                    "[CBattle_Field::MoveMap] Error(< map_candidator.size()==0) gs->mapIndexes_[%d]=%d \n",
                    static_cast<int>(it - grid->values_04.begin()),
                    *it);
            }
        }
    } else {
        selected_map = dungeon->ChooseRandomMap(
            0, grid->grid_type, invade_direction);
    }
    return 0;
}

// IDA_EA: 0x830878E
// IDA_NAME: _ZN13CBattle_Field14swap_curpos_xyEii
CBattle_Field* CBattle_Field::swap_curpos_xy(int pos_x, int pos_y) {
    const unsigned int seat = static_cast<unsigned int>(current_seat_no_);
    if (dungeon_ &&
        static_cast<unsigned char>(dungeon_->IsEnterEachMap()) &&
        seat < 4u) {
        last_positions_[seat] = start_positions_[seat];
        start_positions_[seat] = std::make_pair(pos_x, pos_y);
    } else {
        previous_position_ = current_position_;
        current_position_ = std::make_pair(pos_x, pos_y);
    }
    return this;
}

// IDA_EA: 0x8308882
// IDA_NAME: _ZN13CBattle_Field10set_map_idEPK10GridScriptRiRSt6vectorIiSaIiEE
int CBattle_Field::set_map_id(const GridScript* grid,
                              int& selected_map,
                              std::vector<int>& candidates) {
    if (!grid) {
        return 21;
    }
    if (selected_map < 0) {
        if (!candidates.empty()) {
            selected_map =
                candidates[get_rand_int(static_cast<int>(candidates.size()))];
        }
        if (GetCurrentMapInfo()->completed_) {
            selected_map =
                grid->values_1c[getLayeredMapIndex()];
        }
    }

    GetCurrentMapInfo()->map_index_ = selected_map;
    if (selected_map > 0) {
        return 0;
    }

    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "int CBattle_Field::set_map_id(const GridScript*, int&, INT_VECTOR&)",
        0x1363,
        "[CBattle_Field::MoveMap] Error Dungeon[%d] Pos(%d,%d), candidator(%d), gs->mapIndex(%d) [Map Kind : %s], gs->layeredMapIndex(%d)\n",
        dungeon_index_,
        current_position_.first,
        current_position_.second,
        static_cast<int>(candidates.size()),
        static_cast<int>(grid->values_04.size()),
        MapKind(this, current_position_.first, current_position_.second),
        static_cast<int>(grid->values_1c.size()));
    return 21;
}
