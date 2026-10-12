#include "old_game/pvp/battlefield/battle_field.h"

#include <stdint.h>

#include <map>
#include <utility>
#include <vector>

#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"

// IDA_EA: 0x8308AE6
// IDA_NAME: _ZN13CBattle_Field7MoveMapEiiRb
// DEMANGLED: CBattle_Field::MoveMap(int,int,bool &)
int CBattle_Field::MoveMap(int pos_x, int pos_y, bool& layered) {
    // The old function intentionally materializes this pointer before its
    // first gate even though the later branches query it again.
    MapInfo* const entry_map_info = GetCurrentMapInfo();
    (void)entry_map_info;

    // CLIENT_IDA_EVIDENCE: DNF.exe's automatic map-movement state machine
    // (sub_B17B00 case 4, 0xB1835C) sends ordinary CMD45 after the escorted
    // entity reaches the transition point.  CMD541 movement is a separate
    // stream.  The server must therefore accept that existing CMD45 while
    // enemies remain only when the DGN explicitly owns
    // [move map even enemy]; initiating a second server-side move here would
    // race the official client transition.
    if (static_cast<unsigned char>(dungeon_->IsEnterEachMap()) != 1u &&
        !dungeon_runtime::IsMoveMapEvenEnemyDungeon(dungeon_index_) &&
        static_cast<unsigned char>(check_grid_clear()) != 1u) {
        moveMap_1_Log(pos_x, pos_y);
        return 20;
    }

    if (dungeon_ && dungeon_->get_blood_dungeon_type() > 0) {
        reset_blood_values();
        blood_round_.reset_blood_values();
        blood_round_.setBloodRoundOrder();
    }

    if (pos_x < 0 || pos_x > 49 || pos_y < 0 || pos_y > 49) {
        return 1;
    }

    if (GetCurrentMapInfo()->completed_ != 1u) {
        int current_x = -1;
        int current_y = -1;
        getCurPosXY(current_x, current_y);
        if (pos_x == current_x && pos_y == current_y) {
            return 18;
        }
    }

    if (!maze_) {
        moveMap_2_Log(pos_x, pos_y);
        return 21;
    }
    if (maze_->grid_width <= pos_x || maze_->grid_height <= pos_y) {
        return 21;
    }

    int result = check_boss_map(pos_x, pos_y);
    if (result) {
        return result;
    }

    int logical_exit_x = pos_x;
    int logical_exit_y = pos_y;
    const bool time_spiral_override = time_spiral::GetMoveOverride(
        party_, pos_x, pos_y, &logical_exit_x, &logical_exit_y);
    if (!time_spiral_override) {
        result = check_movemap_range(pos_x, pos_y);
        if (result) {
            return result;
        }
    }

    const GridScript* const grid = &maze_->getGridR(pos_x, pos_y);
    if (!grid) {
        moveMap_3_Log(pos_x, pos_y);
        return 21;
    }
    if (time_spiral_override) {
        // The server-selected portal destination is a fixed PVF map, not a
        // door joined to the source map.  Applying the source exit direction
        // to CheckEntrance rejects valid targets such as 27115, 27109 and
        // 27114.  GetMoveOverride above binds this bypass to the exact live
        // owner, source and already-selected target; verify the fixed map
        // against the published coordinate table before touching positions.
        const std::vector<int>& fixed_maps = grid->values_04.empty()
            ? grid->values_10 : grid->values_04;
        int mapped_x = -1;
        int mapped_y = -1;
        const int fixed_map = fixed_maps.size() == 1u ? fixed_maps[0] : -1;
        if (grid->values_04.size() + grid->values_10.size() != 1u ||
            !grid->values_1c.empty() || fixed_map <= 0 ||
            !time_spiral::FindCoordinateForMap(fixed_map, &mapped_x, &mapped_y) ||
            mapped_x != pos_x || mapped_y != pos_y ||
            !G_CDataManager()->find_map(fixed_map)) {
            LogManager::logFormat(
                1, __FILE__, "CBattle_Field::MoveMap", __LINE__,
                "[TIME_SPIRAL][PORTAL_MAP_REJECT] reason=fixed_map target=%d,%d map=%d",
                pos_x, pos_y, fixed_map);
            return 23;
        }

        MapInfo* const source_info = GetCurrentMapInfo();
        MapInfo* const destination_info = GetMapInfoFromPos(pos_x, pos_y, false);
        if (!source_info || !destination_info || source_info == destination_info ||
            (destination_info->visited_goto_ &&
             destination_info->map_index_ != fixed_map)) {
            return 23;
        }
        if (static_cast<unsigned char>(check_visited_map(pos_x, pos_y, layered))) {
            return 0;
        }

        const auto source_position = current_position_;
        const auto source_previous_position = previous_position_;
        CMap* const source_map = current_map_;
        const uint8_t source_visited = source_info->visited_goto_;
        const MapInfo destination_before(*destination_info);
        const auto rollback_portal = [&]() {
            current_position_ = source_position;
            previous_position_ = source_previous_position;
            current_map_ = source_map;
            source_info->visited_goto_ = source_visited;
            *destination_info = destination_before;
            LogManager::logFormat(
                1, __FILE__, "CBattle_Field::MoveMap", __LINE__,
                "[TIME_SPIRAL][PORTAL_MAP_ROLLBACK] source=%d,%d target=%d,%d map=%d",
                source_position.first, source_position.second, pos_x, pos_y, fixed_map);
        };
        set_visited_map(pos_x, pos_y, true);
        swap_curpos_xy(pos_x, pos_y);
        set_visited_map_goto(pos_x, pos_y);
        destination_info->map_layer_ = static_cast<unsigned char>(grid->grid_type);
        int selected_map = fixed_map;
        std::vector<int> unused_candidates;
        result = set_map_id(grid, selected_map, unused_candidates);
        if (result) {
            rollback_portal();
            return result;
        }
        if (ConsistMap() != 1) {
            rollback_portal();
            return 21;
        }
        if (party_) party_->map_int_int_.clear();
        LogManager::logFormat(
            1, __FILE__, "CBattle_Field::MoveMap", __LINE__,
            "[TIME_SPIRAL][PORTAL_MAP_READY] source=%d,%d target=%d,%d map=%d",
            source_position.first, source_position.second, pos_x, pos_y, fixed_map);
        return 0;
    }
    if (static_cast<unsigned char>(
            check_visited_map(pos_x, pos_y, layered))) {
        return 0;
    }

    const int invade_direction = get_invade_direction(
        time_spiral_override ? logical_exit_x : pos_x,
        time_spiral_override ? logical_exit_y : pos_y);
    const int dungeon_index = dungeon_index_;
    const CDungeon* const dungeon =
        G_CDataManager()->find_dungeon(dungeon_index);
    if (!dungeon) {
        moveMap_4_Log(pos_x, pos_y, grid);
        return 1;
    }

    int selected_map = -1;
    bool visited = true;
    choose_random_bossmap_from_dummymaps(
        dungeon,
        grid,
        selected_map,
        visited,
        pos_x,
        pos_y,
        invade_direction);

    std::vector<int> candidates;
    quest_stage_map_selected_ = 0;
    result = get_candidator_map(dungeon,
                                        grid,
                                        selected_map,
                                        candidates,
                                        pos_x,
                                        pos_y,
                                        invade_direction);
    if (result) {
        return result;
    }

    set_visited_map(pos_x, pos_y, visited);
    swap_curpos_xy(pos_x, pos_y);
    set_visited_map_goto(pos_x, pos_y);
    GetCurrentMapInfo()->map_layer_ =
        static_cast<unsigned char>(grid->grid_type);

    result = set_map_id(grid, selected_map, candidates);
    if (result) {
        return result;
    }

    const int consist_result = ConsistMap();
    if (zombie_map_runtime::ResolveGlobalFocusSeat(this) >= 0 &&
        consist_result != 1) {
        return 21;
    }
    if (party_) {
        party_->map_int_int_.clear();
    }
    return 0;
}

// IDA_EA: 0x82FF486
// IDA_NAME: _ZN13CBattle_Field13SelectDungeonEiPK8CDungeoni17ENUM_DUNGEON_TYPEiP9CWorldMapbi
// DEMANGLED: CBattle_Field::SelectDungeon(int,CDungeon const*,int,
//             ENUM_DUNGEON_TYPE,int,CWorldMap *,bool,int)
bool CBattle_Field::SelectDungeon(int dungeon_index,
                                  const CDungeon* dungeon,
                                  int dungeon_diff,
                                  ENUM_DUNGEON_TYPE dungeon_type,
                                  int member_count,
                                  CWorldMap* world_map,
                                  bool hell_party,
                                  int parameter) {
    time_spiral::ResetParty(party_);
    dungeon_index_ = dungeon_index;
    set_dungeon_diff(static_cast<char>(dungeon_diff));
    party_member_count_ = member_count;
    dungeon_select_parameter_ = parameter;
    hell_party_enabled_ = 0;
    dungeon_type_value_ = static_cast<int>(dungeon_type);

    if (static_cast<unsigned char>(IsEnableHellDungeon()) &&
        world_map &&
        static_cast<unsigned char>(world_map->IsInHellDungeon())) {
        if (static_cast<int>(dungeon_type) == 1) {
            party_->sub_hellparty_value();
            hell_party_difficulty_ = static_cast<signed char>(
                GetHellPartyDifficulty(static_cast<ENUM_DUNGEON_TYPE>(1)));
            if (!hell_party_difficulty_) {
                LogManager::logFormat(
                    1,
                    "battle_field.cpp",
                    "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
                    0x1fc,
                    "HELLPARTY_LOG : HELLPARTY DIFFICULTY ERROR!!\n");
                return false;
            }
            hell_party_event_type_ = static_cast<unsigned char>(
                GetHellPartyEventType(hell_party_difficulty_));
            hell_party_enabled_ = 1;
        } else if (hell_party) {
            if (party_->get_member_count() != 1 &&
                CheckHellPartyCondition()) {
                party_->sub_hellparty_value();
                hell_party_enabled_ = 1;
                hell_party_requested_ = hell_party;
                hell_party_difficulty_ = static_cast<signed char>(
                    GetHellPartyDifficulty(dungeon_type));
                if (!hell_party_difficulty_) {
                    LogManager::logFormat(
                        1,
                        "battle_field.cpp",
                        "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
                        0x22b,
                        "HELLPARTY_LOG : HELLPARTY DIFFICULTY ERROR!!\n");
                    return false;
                }
                hell_party_event_type_ = static_cast<unsigned char>(
                    GetHellPartyEventType(hell_party_difficulty_));
            }
        }
    }

    if (dungeon && dungeon->hell_dungeon_ != 1u) {
        hell_party_enabled_ = 0;
    }

    dungeon_ = const_cast<CDungeon*>(dungeon);
    champion_spawn_rate_ = dungeon->champion_spawn_rates_[dungeon_diff];
    runtime_value_1a4_ = 0;

    MazeScript* const appropriate_maze = GetAppropriateMaze(
        static_cast<ENUM_DUNGEON_DIFFICULTY>(dungeon_diff));
    if (!appropriate_maze) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
            0x25f,
            "[CMD16_SELECT_DUNGEON_FAIL] stage=maze dungeon=%d difficulty=%d mode=%d members=%d parameter=%d\n",
            dungeon_index,
            dungeon_diff,
            static_cast<int>(dungeon_type),
            member_count,
            parameter);
        return false;
    }
    selected_maze_index_ = appropriate_maze->maze_index;
    setQuestMazeInfo();
    clear_condition_.SetDungeonClearConditionVector(
        appropriate_maze->clear_conditions);
    SetGridPath(appropriate_maze, dungeon->GetDummyAppearCount());

    int pos_x = -1;
    int pos_y = -1;
    getCurPosXY(pos_x, pos_y);
    if (!maze_ || pos_x < 0 || pos_y < 0 ||
        pos_x >= maze_->grid_width || pos_y >= maze_->grid_height ||
        static_cast<unsigned int>(pos_x + pos_y * maze_->grid_width) >=
            maze_->grids.size()) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
            0x274,
            "[CMD16_SELECT_DUNGEON_FAIL] stage=grid dungeon=%d difficulty=%d mode=%d members=%d maze=%d pos=(%d,%d) size=(%d,%d) grids=%u\n",
            dungeon_index,
            dungeon_diff,
            static_cast<int>(dungeon_type),
            member_count,
            appropriate_maze->maze_index,
            pos_x,
            pos_y,
            maze_ ? maze_->grid_width : -1,
            maze_ ? maze_->grid_height : -1,
            maze_ ? static_cast<unsigned int>(maze_->grids.size()) : 0u);
        return false;
    }
    const GridScript* const grid = &maze_->getGridR(pos_x, pos_y);
    if (!grid) {
        return false;
    }

    int start_map = 0;
    int canonical_zombie_map = 0;
    const zombie_map_runtime::HookResult zombie_map_contract =
        zombie_map_runtime::ResolveCanonicalStartMapIndex(
            this, pos_x, pos_y, &canonical_zombie_map);
    if (zombie_map_contract == zombie_map_runtime::kHookHandled) {
        // 0828 zombi2.dgn gives every playable coordinate exactly one
        // [map specification].  The 0627 GetStartMap(grid_type) path cannot
        // represent that contract: J and G each occur at two coordinates and
        // would randomly select the other coordinate's map half the time.
        if (grid->values_04.size() != 1u ||
            grid->values_04[0] != canonical_zombie_map ||
            !G_CDataManager()->find_map(canonical_zombie_map)) {
            LogManager::logFormat(
                1,
                "battle_field.cpp",
                "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
                0x27b,
                "[CMD16_SELECT_DUNGEON_FAIL] stage=zombie-map-spec dungeon=%d maze=%d pos=(%d,%d) grid_type=%d canonical=%d specifications=%u first=%d\n",
                dungeon_index,
                appropriate_maze->maze_index,
                pos_x,
                pos_y,
                grid->grid_type,
                canonical_zombie_map,
                static_cast<unsigned int>(grid->values_04.size()),
                grid->values_04.empty() ? -1 : grid->values_04[0]);
            return false;
        }
        start_map = canonical_zombie_map;
    } else if (zombie_map_contract == zombie_map_runtime::kHookRejected) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
            0x27b,
            "[CMD16_SELECT_DUNGEON_FAIL] stage=zombie-map-contract dungeon=%d maze=%d pos=(%d,%d) grid_type=%d\n",
            dungeon_index,
            appropriate_maze->maze_index,
            pos_x,
            pos_y,
            grid->grid_type);
        return false;
    } else {
        start_map = dungeon->GetStartMap(grid->grid_type);
    }
    if (start_map <= 0) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
            0x27c,
            "[CMD16_SELECT_DUNGEON_FAIL] stage=start-map dungeon=%d difficulty=%d mode=%d members=%d maze=%d pos=(%d,%d) grid_type=%d start_map=%d; [CBattle_Field::SelectDungeon] Error Map\n",
            dungeon_index,
            dungeon_diff,
            static_cast<int>(dungeon_type),
            member_count,
            appropriate_maze->maze_index,
            current_position_.first,
            current_position_.second,
            grid->grid_type,
            start_map);
        return false;
    }

    const zombie_map_runtime::HookResult zombie_start =
        zombie_map_runtime::InitializeCanonicalStartCell(
            this,
            pos_x,
            pos_y,
            start_map,
            static_cast<uint8_t>(grid->grid_type));
    if (zombie_start == zombie_map_runtime::kHookRejected) {
        return false;
    }
    if (zombie_start == zombie_map_runtime::kHookNotManaged) {
        MapInfo map_info;
        map_info.map_index_ = start_map;
        map_info.map_layer_ = static_cast<unsigned char>(grid->grid_type);
        map_infos_.insert(
            std::make_pair(GetMapInfoKeyValue(pos_x, pos_y), map_info));
    }
    const int consist_result = ConsistMap();
    if (zombie_start == zombie_map_runtime::kHookHandled &&
        consist_result != 1) {
        return false;
    }
    if (time_spiral::IsDungeon(dungeon_index) &&
        !time_spiral::BeginRun(party_, dungeon_index)) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "bool CBattle_Field::SelectDungeon(int, const CDungeon*, int, ENUM_DUNGEON_TYPE, int, CWorldMap*, bool, int)",
            0x28e,
            "[CMD16_SELECT_DUNGEON_FAIL] stage=time-spiral dungeon=%d difficulty=%d mode=%d members=%d\n",
            dungeon_index,
            dungeon_diff,
            static_cast<int>(dungeon_type),
            member_count);
        reset_field();
        return false;
    }
    return true;
}
