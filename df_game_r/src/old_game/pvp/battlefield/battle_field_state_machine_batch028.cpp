#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/map_script_types.h"

// IDA_EA: 0x80FDCFC
// IDA_NAME: _ZN13CBattle_Field10GetDungeonEv
// DEMANGLED: CBattle_Field::GetDungeon(void)
CDungeon* CBattle_Field::GetDungeon() {
    return dungeon_;
}

// IDA_EA: 0x822CEE2
// IDA_NAME: _ZN13CBattle_Field19IsAssignNPCByPotionEv
// DEMANGLED: CBattle_Field::IsAssignNPCByPotion(void)
int CBattle_Field::IsAssignNPCByPotion() {
    return GetCurrentMapInfo()->assign_npc_by_potion_;
}

// IDA_EA: 0x822CF18
// IDA_NAME: _ZN13CBattle_Field13getBloodStateEv
// DEMANGLED: CBattle_Field::getBloodState(void)
int CBattle_Field::getBloodState() {
    return blood_state_;
}

// IDA_EA: 0x822CFFC
// IDA_NAME: _ZN13CBattle_Field11get_cur_mapEv
// DEMANGLED: CBattle_Field::get_cur_map(void)
int CBattle_Field::get_cur_map() {
    return GetCurrentMapInfo()->map_index_;
}

// IDA_EA: 0x822D012
// IDA_NAME: _ZN13CBattle_Field21setUltimateDifficultyEc
// DEMANGLED: CBattle_Field::setUltimateDifficulty(char)
CBattle_Field* CBattle_Field::setUltimateDifficulty(char difficulty) {
    blood_spawn_runtime_.ultimate_difficulty =
        static_cast<unsigned char>(difficulty);
    return this;
}

// IDA_EA: 0x822D02E
// IDA_NAME: _ZN13CBattle_Field21getUltimateDifficyltyEv
// DEMANGLED: CBattle_Field::getUltimateDifficylty(void)
int CBattle_Field::getUltimateDifficylty() {
    return blood_spawn_runtime_.ultimate_difficulty;
}

// IDA_EA: 0x822D03E
// IDA_NAME: _ZN13CBattle_Field10GetMapInfoEv
// DEMANGLED: CBattle_Field::GetMapInfo(void)
MapInfo* CBattle_Field::GetMapInfo() {
    return GetCurrentMapInfo();
}

// IDA_EA: 0x82A43BE
// IDA_NAME: _ZN13CBattle_Field8SetPartyEP6CParty
// DEMANGLED: CBattle_Field::SetParty(CParty *)
CBattle_Field* CBattle_Field::SetParty(CParty* party) {
    party_ = party;
    return this;
}

// IDA_EA: 0x82FE69C
// IDA_NAME: _ZN13CBattle_Field21SetTournamentCurRoundEi
// DEMANGLED: CBattle_Field::SetTournamentCurRound(int)
CBattle_Field* CBattle_Field::SetTournamentCurRound(int round) {
    tournament_cur_round_ = round;
    return this;
}

// IDA_EA: 0x82FE6AE
// IDA_NAME: _ZN13CBattle_Field21GetTournamentCurRoundEv
// DEMANGLED: CBattle_Field::GetTournamentCurRound(void)
int CBattle_Field::GetTournamentCurRound() {
    return tournament_cur_round_;
}

// IDA_EA: 0x83062FE
// IDA_NAME: _ZN13CBattle_Field41get_blood_round_difficylty_of_small_roundEs
// DEMANGLED: CBattle_Field::get_blood_round_difficylty_of_small_round(short)
int CBattle_Field::get_blood_round_difficylty_of_small_round(
    short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    if (phases.empty() ||
        static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    return static_cast<signed char>(phases[small_round].enabled);
}

// IDA_EA: 0x83065F8
// IDA_NAME: _ZN13CBattle_Field30get_blood_round_of_small_roundEs
// DEMANGLED: CBattle_Field::get_blood_round_of_small_round(short)
int CBattle_Field::get_blood_round_of_small_round(short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    if (phases.empty() ||
        static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    return phases[small_round].phase;
}

// IDA_EA: 0x8306670
// IDA_NAME: _ZN13CBattle_Field34get_blood_spawn_cnt_of_small_roundEs
// DEMANGLED: CBattle_Field::get_blood_spawn_cnt_of_small_round(short)
int CBattle_Field::get_blood_spawn_cnt_of_small_round(short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    if (phases.empty() ||
        static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    return phases[small_round].value_14;
}

// IDA_EA: 0x83066EA
// IDA_NAME: _ZN13CBattle_Field36get_blood_spawn_small_round_intervalEs
// DEMANGLED: CBattle_Field::get_blood_spawn_small_round_interval(short)
int CBattle_Field::get_blood_spawn_small_round_interval(short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    if (phases.empty()) {
        return -1;
    }
    if (static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    return phases[small_round].value_08;
}

// IDA_EA: 0x830676A
// IDA_NAME: _ZN13CBattle_Field40get_blood_spawn_time_of_next_small_roundEs
// DEMANGLED: CBattle_Field::get_blood_spawn_time_of_next_small_round(short)
int CBattle_Field::get_blood_spawn_time_of_next_small_round(short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    std::vector<STAssignBloodSystem>& systems = blood_systems_;
    if (phases.empty() || systems.empty()) {
        return -1;
    }
    if (static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    int index = phases[small_round].monster_id;
    if (index == -1) {
        std::vector<short>& indices = blood_rounds_;
        if (indices.empty() ||
            static_cast<short>(indices.size()) <= small_round) {
            return -1;
        }
        index = indices[small_round];
    }
    if (systems.size() <= static_cast<unsigned int>(index)) {
        return -1;
    }
    const STAssignBloodSystem& system = systems[index];
    return system.value_14 - system.value_18 * system.value_24;
}

// IDA_EA: 0x830690A
// IDA_NAME: _ZN13CBattle_Field29get_blood_spawn_time_of_phaseEs
// DEMANGLED: CBattle_Field::get_blood_spawn_time_of_phase(short)
int CBattle_Field::get_blood_spawn_time_of_phase(short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    std::vector<STAssignBloodSystem>& systems = blood_systems_;
    if (phases.empty() || systems.empty()) {
        return -1;
    }
    if (static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    int index = phases[small_round].monster_id;
    if (index == -1) {
        std::vector<short>& indices = blood_rounds_;
        if (indices.empty() ||
            static_cast<short>(indices.size()) <= small_round) {
            return -1;
        }
        index = indices[small_round];
    }
    if (systems.size() <= static_cast<unsigned int>(index)) {
        return -1;
    }
    return systems[index].value_18;
}

// IDA_EA: 0x8306A62
// IDA_NAME: _ZN13CBattle_Field28get_blood_spawn_cnt_of_phaseEss
// DEMANGLED: CBattle_Field::get_blood_spawn_cnt_of_phase(short,short)
int CBattle_Field::get_blood_spawn_cnt_of_phase(short small_round,
                                                short phase) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    std::vector<STAssignBloodSystem>& systems = blood_systems_;
    if (phases.empty() || systems.empty()) {
        return -1;
    }
    if (static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    int index = phases[small_round].monster_id;
    if (index == -1) {
        std::vector<short>& indices = blood_rounds_;
        if (indices.empty() ||
            static_cast<short>(indices.size()) <= small_round) {
            return -1;
        }
        index = indices[small_round];
    }
    if (systems.size() <= static_cast<unsigned int>(index)) {
        return -1;
    }
    int count = 0;
    for (int current = 0; current < phase + 1; ++current) {
        count = systems[index].value_1c +
                current * systems[index].value_20;
    }
    return count;
}

// IDA_EA: 0x8306C0E
// IDA_NAME: _ZN13CBattle_Field46get_blood_spawn_phase_total_cnt_of_small_roundEs
// DEMANGLED: CBattle_Field::get_blood_spawn_phase_total_cnt_of_small_round(short)
int CBattle_Field::get_blood_spawn_phase_total_cnt_of_small_round(
    short small_round) {
    std::vector<STAssignBloodPhaseTime>& phases = blood_phase_times_;
    std::vector<STAssignBloodSystem>& systems = blood_systems_;
    if (phases.empty() || systems.empty()) {
        return -1;
    }
    if (static_cast<short>(phases.size()) <= small_round ||
        small_round < 0) {
        return -1;
    }
    int index = phases[small_round].monster_id;
    if (index == -1) {
        std::vector<short>& indices = blood_rounds_;
        if (indices.empty() ||
            static_cast<short>(indices.size()) <= small_round) {
            return -1;
        }
        index = indices[small_round];
    }
    if (systems.size() <= static_cast<unsigned int>(index)) {
        return -1;
    }
    return systems[index].value_24;
}

// IDA_EA: 0x8306E32
// IDA_NAME: _ZN13CBattle_Field20checkBloodSpawnOrderEs
// DEMANGLED: CBattle_Field::checkBloodSpawnOrder(short)
int CBattle_Field::checkBloodSpawnOrder(short small_round) {
    std::vector<bool>& flags = blood_flags_;
    if (static_cast<short>(flags.size()) < small_round || flags.empty()) {
        return 0;
    }
    for (int index = 0; index < small_round; ++index) {
        if (!flags[index]) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8307656
// IDA_NAME: _ZN13CBattle_Field19IsFirstEnterDungeonEv
// DEMANGLED: CBattle_Field::IsFirstEnterDungeon(void)
int CBattle_Field::IsFirstEnterDungeon() {
    return initial_map_flag_;
}

// IDA_EA: 0x8307662
// IDA_NAME: _ZN13CBattle_Field22ResetFirstEnterDungeonEv
// DEMANGLED: CBattle_Field::ResetFirstEnterDungeon(void)
CBattle_Field* CBattle_Field::ResetFirstEnterDungeon() {
    initial_map_flag_ = 0;
    return this;
}

// IDA_EA: 0x8307C7A
// IDA_NAME: _ZN13CBattle_Field14is_visited_mapEii
// DEMANGLED: CBattle_Field::is_visited_map(int,int)
int CBattle_Field::is_visited_map(int pos_x, int pos_y) {
    return GetMapInfoFromPos(pos_x, pos_y, false)->visited_goto_ != 0;
}

// IDA_EA: 0x8307CBE
// IDA_NAME: _ZN13CBattle_Field19is_visited_map_gotoEii
// DEMANGLED: CBattle_Field::is_visited_map_goto(int,int)
int CBattle_Field::is_visited_map_goto(int pos_x, int pos_y) {
    const MapInfo* const info = GetMapInfoFromPos(pos_x, pos_y, false);
    return info->visited_goto_ || info->visited_transition_;
}

// IDA_EA: 0x8300E44
// IDA_NAME: _ZN13CBattle_Field12getLastPosXYERiS0_
// DEMANGLED: CBattle_Field::getLastPosXY(int &,int &)
int* CBattle_Field::getLastPosXY(int& pos_x, int& pos_y) {
    CDungeon* const dungeon = dungeon_;
    const unsigned int seat = static_cast<unsigned int>(current_seat_no_);
    if (dungeon && reinterpret_cast<const CDungeon*>(dungeon)->IsEnterEachMap() && seat < 4u) {
        pos_x = last_positions_[seat].first;
        pos_y = last_positions_[seat].second;
    } else {
        pos_x = previous_position_.first;
        pos_y = previous_position_.second;
    }
    return &pos_y;
}

// IDA_EA: 0x8307E44
// IDA_NAME: _ZN13CBattle_Field20get_invade_directionEii
// DEMANGLED: CBattle_Field::get_invade_direction(int,int)
int CBattle_Field::get_invade_direction(int pos_x, int pos_y) {
    int current_x = -1;
    int current_y = -1;
    getCurPosXY(current_x, current_y);
    int direction = 0;
    if (current_x == pos_x) {
        if (current_y > pos_y) direction = 8;
        if (current_y < pos_y) direction = 2;
    }
    if (current_y == pos_y) {
        if (current_x < pos_x) direction = 4;
        if (current_x > pos_x) return 1;
    }
    return direction;
}

// IDA_EA: 0x83086D2
// IDA_NAME: _ZN13CBattle_Field15set_visited_mapEiib
// DEMANGLED: CBattle_Field::set_visited_map(int,int,bool)
MapInfo* CBattle_Field::set_visited_map(int pos_x,
                                        int pos_y,
                                        bool visited) {
    int current_x = -1;
    int current_y = -1;
    getCurPosXY(current_x, current_y);
    MapInfo* info;
    if (current_x == pos_x && current_y == pos_y) {
        const int layered_index = getLayeredMapIndex();
        info = layered_index
                   ? getLayeredMapInfoIndex(layered_index - 1)
                   : GetMapInfoFromPos(current_x, current_y, true);
    } else {
        info = GetCurrentMapInfo();
    }
    info->visited_goto_ = visited;
    return info;
}

// IDA_EA: 0x8308A24
// IDA_NAME: _ZN13CBattle_Field20set_visited_map_gotoEii
// DEMANGLED: CBattle_Field::set_visited_map_goto(int,int)
MapInfo* CBattle_Field::set_visited_map_goto(int pos_x, int pos_y) {
    int last_x = -1;
    int last_y = -1;
    getLastPosXY(last_x, last_y);
    MapInfo* info;
    if (last_x == pos_x && last_y == pos_y) {
        const int layered_index = getLayeredMapIndex();
        info = layered_index
                   ? getLayeredMapInfoIndex(layered_index - 1)
                   : GetMapInfoFromPos(pos_x, pos_y, true);
    } else {
        info = GetMapInfoFromPos(pos_x, pos_y, false);
    }
    info->visited_transition_ = 1;
    return info;
}

// IDA_EA: 0x830A812
// IDA_NAME: _ZNK13CBattle_Field34convertAPCRoleType2MonsterRoleTypeEc
// DEMANGLED: CBattle_Field::convertAPCRoleType2MonsterRoleType(char)const
int CBattle_Field::convertAPCRoleType2MonsterRoleType(char role_type) const {
    switch (role_type) {
    case 5: return 0;
    case 6: return 1;
    case 7: return 2;
    case 8: return 3;
    default: return role_type;
    }
}

// IDA_EA: 0x830ED1E
// IDA_NAME: _ZN13CBattle_Field16set_dungeon_diffEc
// DEMANGLED: CBattle_Field::set_dungeon_diff(char)
CBattle_Field* CBattle_Field::set_dungeon_diff(char difficulty) {
    dungeon_difficulty_ = difficulty;
    return this;
}

// IDA_EA: 0x85BF28C
// IDA_NAME: _ZN13CBattle_Field13GetCurrentMapEv
// DEMANGLED: CBattle_Field::GetCurrentMap(void)
CMap* CBattle_Field::GetCurrentMap() {
    return current_map_;
}

// IDA_EA: 0x85BF31C
// IDA_NAME: _ZN13CBattle_Field19isWorldMapTowerAreaEv
// DEMANGLED: CBattle_Field::isWorldMapTowerArea(void)
int CBattle_Field::isWorldMapTowerArea() {
    return world_map_tower_area_;
}

// IDA_EA: 0x85D19D6
// IDA_NAME: _ZN13CBattle_Field20SetAssignNPCByPotionEb
// DEMANGLED: CBattle_Field::SetAssignNPCByPotion(bool)
MapInfo* CBattle_Field::SetAssignNPCByPotion(bool enabled) {
    MapInfo* const info = GetCurrentMapInfo();
    info->assign_npc_by_potion_ = enabled;
    return info;
}
