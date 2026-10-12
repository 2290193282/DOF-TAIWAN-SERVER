#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/dungeon/mission/dungeon_mission.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "old_data/PVF/dungeon/maze_script.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <cstring>
#include <vector>


class CParty;

// IDA_EA: 0x822CB92
// IDA_NAME: _ZN15CClearCondition16IsClearConditionEv
// DEMANGLED: CClearCondition::IsClearCondition(void)
bool CClearCondition::IsClearCondition() {
    return current_count_ <= target_count_;
}

// IDA_EA: 0x822CBA8
// IDA_NAME: _ZN15CClearCondition5EmptyEv
// DEMANGLED: CClearCondition::Empty(void)
bool CClearCondition::Empty() {
    return cleared_states_.empty();
}

// IDA_EA: 0x82FEFCE
// IDA_NAME: _ZN15CClearCondition14ClearConditionE33ENUM_DUNGEON_CLEAR_CONDITION_TYPEi
// DEMANGLED: CClearCondition::ClearCondition(
//     ENUM_DUNGEON_CLEAR_CONDITION_TYPE,int)
bool CClearCondition::ClearCondition(int condition_type, int value) {
    if (condition_type < 0 || condition_type > 4 || !condition_source_) {
        return false;
    }

    const std::vector<DungeonClearCondition>& conditions = *condition_source_;
    const std::size_t count = std::min(conditions.size(), cleared_states_.size());
    for (std::size_t index = 0; index < count; ++index) {
        const DungeonClearCondition& condition = conditions[index];
        int& completed = cleared_states_[index];
        if (condition.value_00 != condition_type ||
            condition.value_04 != value) {
            continue;
        }
        if (condition.value_08 <= completed) {
            return false;
        }
        ++completed;
        ++target_count_;
    }
    return current_count_ <= target_count_;
}

// IDA_EA: 0x85BF29A
// IDA_NAME: _ZN13CBattle_Field14ClearConditionE33ENUM_DUNGEON_CLEAR_CONDITION_TYPEi
// DEMANGLED: CBattle_Field::ClearCondition(
//     ENUM_DUNGEON_CLEAR_CONDITION_TYPE,int)
bool CBattle_Field::ClearCondition(int condition_type, int value) {
    return !clear_condition_.Empty() &&
           clear_condition_.ClearCondition(condition_type, value);
}

// IDA_EA: 0x85BF2E8
// IDA_NAME: _ZN13CBattle_Field20DestroyPassiveObjectEi
// DEMANGLED: CBattle_Field::DestroyPassiveObject(int)
int CBattle_Field::DestroyPassiveObject(int object_unique_id) {
    if (!current_map_) {
        return 0;
    }

    // CLIENT/SERVER_SOURCE_EVIDENCE: the 0627 native owner at 0x82FF3EC
    // reads CMap+0x0a8, walks passive_object_ids_, and returns membership.
    // MapInfo+0x0a8 is a different map used for monster identity lookup;
    // consulting it here rejects real passive objects and can accept a
    // monster UID that merely has the same numeric value.
    const std::vector<int>& passive_ids = current_map_->passive_object_ids_;
    return std::find(passive_ids.begin(), passive_ids.end(),
                     object_unique_id) != passive_ids.end();
}

// IDA_EA: 0x822D05E
// IDA_NAME: _ZN13CBattle_Field18GetCurrentMapIndexEv
// DEMANGLED: CBattle_Field::GetCurrentMapIndex(void)
int CBattle_Field::GetCurrentMapIndex() {
    if (current_map_) {
        return current_map_->get_index();
    }
    return 0;
}

// IDA_EA: 0x822D7B2
// IDA_NAME: _ZN10BattleData22SetHellPartyValueTotalEi
// DEMANGLED: BattleData::SetHellPartyValueTotal(int)
BattleData* BattleData::SetHellPartyValueTotal(int value) {
    hell_party_value_total_ = static_cast<float>(value);
    return this;
}

// IDA_EA: 0x82A4E8A
// IDA_NAME: _ZN10BattleData5ResetEv
// DEMANGLED: BattleData::Reset(void)
bool BattleData::Reset() {
    dungeon_playing_time_ = 0;
    map_playing_time_ = 0;
    map_pause_time_ = 0;
    map_start_time_ = 0;
    map_tick_211_ = 0;
    map_tick_212_ = 0;
    std::memset(dungeon_start_statistic_flags_, 0,
                sizeof(dungeon_start_statistic_flags_));
    map_tick_214_ = 0;
    map_tick_215_ = 0;
    map_tick_216_ = 0;
    map_tick_217_ = 0;
    monster_role_other_kill_count_ = 0;
    monster_role_1_kill_count_ = 0;
    monster_role_3_kill_count_ = 0;
    monster_drop_limit_accepted_count_ = 0;
    monster_drop_limit_candidate_count_ = 0;
    std::memset(member_finish_loading_, 0, sizeof(member_finish_loading_));
    std::memset(member_live_, 0, sizeof(member_live_));
    std::memset(member_enter_map_, 0, sizeof(member_enter_map_));
    std::memset(member_state_flag_, 0, sizeof(member_state_flag_));
    std::memset(raw_060_063_, 0, sizeof(raw_060_063_));
    map_move_count_ = 0;

    for (int i = 0; i != 4; ++i) {
        member_map_move_count_[i] = 0;
        normal_rewards_[i].first.reset();
        normal_rewards_[i].second.reset();
        special_rewards_[i].first.reset();
        special_rewards_[i].second.reset();
        event_rewards_[i].first.reset();
        event_rewards_[i].second.reset();
        game_results_[i].Clear();
        skill_use_counts_[i] = 0;
    }

    // IDA leaves the aggregate's +0x7e0 hell-party total intact.  The next
    // dword is reset and every member receives an unlimited paid-revival
    // allowance until the selected dungeon overrides it.
    std::memset(member_start_map_finish_loading_,
                0,
                sizeof(member_start_map_finish_loading_));
    for (int i = 0; i != 4; ++i) {
        member_coin_limit_remaining_[i] = 0x7fffffff;
    }
    return false;
}

// IDA_EA: 0x85BE6BA
// IDA_NAME: _ZN10BattleData27ResetEventDungeonClearPointEv
// DEMANGLED: BattleData::ResetEventDungeonClearPoint(void)
void* BattleData::ResetEventDungeonClearPoint() {
    return std::memset(raw_62c_68b_, 0, sizeof(raw_62c_68b_));
}

// IDA_EA: 0x80F981C
// IDA_NAME: _ZN13CBattle_Field16get_dungeon_diffEv
// DEMANGLED: CBattle_Field::get_dungeon_diff(void)
int CBattle_Field::get_dungeon_diff() {
    return dungeon_difficulty_;
}

// IDA_EA: 0x822D0D8
// IDA_NAME: _ZN13CBattle_Field17get_dungeon_indexEv
// DEMANGLED: CBattle_Field::get_dungeon_index(void)
int CBattle_Field::get_dungeon_index() {
    return dungeon_index_;
}

// IDA_EA: 0x822CF26
// IDA_NAME: _ZN13CBattle_Field20GetEnableHellDungeonEv
// DEMANGLED: CBattle_Field::GetEnableHellDungeon(void)
uint8_t* CBattle_Field::GetEnableHellDungeon() {
    static uint8_t enable_hell_dungeon = 1;
    return &enable_hell_dungeon;
}

// IDA_EA: 0x822CF30
// IDA_NAME: _ZN13CBattle_Field20SetEnableHellDungeonEb
// DEMANGLED: CBattle_Field::SetEnableHellDungeon(bool)
uint8_t* CBattle_Field::SetEnableHellDungeon(bool enabled) {
    uint8_t* const flag = GetEnableHellDungeon();
    *flag = enabled;
    return flag;
}

// IDA_EA: 0x82A43CC
// IDA_NAME: _ZN13CBattle_Field12SetCurSeatNoEi
// DEMANGLED: CBattle_Field::SetCurSeatNo(int)
CBattle_Field* CBattle_Field::SetCurSeatNo(int seat) {
    current_seat_no_ = seat;
    return this;
}

// IDA_EA: 0x822D08C
// IDA_NAME: _ZN13CBattle_Field14IsClearDungeonEv
// DEMANGLED: CBattle_Field::IsClearDungeon(void)
int CBattle_Field::IsClearDungeon() {
    if (static_cast<unsigned char>(clear_condition_.Empty()) == 1u) {
        return check_end_point();
    }
    return static_cast<unsigned char>(clear_condition_.IsClearCondition()) != 0;
}

// IDA_EA: 0x830ED07
// IDA_NAME: _ZN13CBattle_Field19IsEnableHellDungeonEv
// DEMANGLED: CBattle_Field::IsEnableHellDungeon(void)
int CBattle_Field::IsEnableHellDungeon() {
    return *GetEnableHellDungeon();
}

// IDA_EA: 0x85BF250
// IDA_NAME: _ZN13CBattle_Field22IsKilledAllHellGruoupsEv
// DEMANGLED: CBattle_Field::IsKilledAllHellGruoups(void)
int CBattle_Field::IsKilledAllHellGruoups() {
    return all_hell_groups_killed_;
}

// IDA_EA: 0x822CBBE
// IDA_NAME: _ZN13CBattle_Field18GetMapInfoKeyValueEii
// DEMANGLED: CBattle_Field::GetMapInfoKeyValue(int,int)
int CBattle_Field::GetMapInfoKeyValue(int pos_x, int pos_y) {
    return pos_y + 100 * pos_x;
}

// IDA_EA: 0x822CBCC
// IDA_NAME: _ZN13CBattle_Field17GetCurrentMapInfoEv
// DEMANGLED: CBattle_Field::GetCurrentMapInfo(void)
MapInfo* CBattle_Field::GetCurrentMapInfo() {
    int pos_x = -1;
    int pos_y = -1;
    getCurPosXY(pos_x, pos_y);
    return GetMapInfoFromPos(pos_x, pos_y, false);
}

// IDA_EA: 0x830DBDE
// IDA_NAME: _ZN13CBattle_Field9onUseItemEP5CUserRK10Inven_Item
// DEMANGLED: CBattle_Field::onUseItem(CUser *,Inven_Item const&)
MapInfo* CBattle_Field::onUseItem(CUser*, const Inven_Item& item) {
    MapInfo* const current_map = GetCurrentMapInfo();
    if (item.item_id_ == 3037) {
        current_map->normal_kill_check_.setUseSkillMaterial(
            static_cast<uint32_t>(GlobalData::s_systemTime_.getCurTickCount()));
        current_map->boss_kill_check_.setUseSkillMaterial(
            static_cast<uint32_t>(GlobalData::s_systemTime_.getCurTickCount()));
    }

    if (item.item_category_ == 2) {
        ++current_map->play_info_.field_08;
    } else {
        ++current_map->play_info_.field_0c;
    }
    return current_map;
}

// IDA_EA: 0x822D052
// IDA_NAME: _ZN13CBattle_Field7getMazeEv
// DEMANGLED: CBattle_Field::getMaze(void)
MazeScript* CBattle_Field::getMaze() {
    return maze_;
}

// IDA_EA: 0x830DF2E
// IDA_NAME: _ZN13CBattle_Field18getLayeredMapIndexEv
// DEMANGLED: CBattle_Field::getLayeredMapIndex(void)
int CBattle_Field::getLayeredMapIndex() {
    return layered_map_index_;
}

// IDA_EA: 0x830DF3C
// IDA_NAME: _ZN13CBattle_Field18setLayeredMapIndexEv
// DEMANGLED: CBattle_Field::setLayeredMapIndex(void)
CBattle_Field* CBattle_Field::setLayeredMapIndex() {
    ++layered_map_index_;
    return this;
}

// IDA_EA: 0x830DF56
// IDA_NAME: _ZN13CBattle_Field21getLayeredMapKeyValueEi
// DEMANGLED: CBattle_Field::getLayeredMapKeyValue(int)
int CBattle_Field::getLayeredMapKeyValue(int layered_map_index) {
    return 100 * layered_map_index + 10000;
}

// IDA_EA: 0x830DF66
// IDA_NAME: _ZN13CBattle_Field24getCurrentLayeredMapInfoEv
// DEMANGLED: CBattle_Field::getCurrentLayeredMapInfo(void)
MapInfo* CBattle_Field::getCurrentLayeredMapInfo() {
    return getLayeredMapInfoIndex(getLayeredMapIndex());
}

// IDA_EA: 0x830E302
// IDA_NAME: _ZN13CBattle_Field12setQuestMazeEb
// DEMANGLED: CBattle_Field::setQuestMaze(bool)
CBattle_Field* CBattle_Field::setQuestMaze(bool quest_maze) {
    quest_maze_ = quest_maze;
    return this;
}

// IDA_EA: 0x830E31E
// IDA_NAME: _ZN13CBattle_Field11isQuestMazeEv
// DEMANGLED: CBattle_Field::isQuestMaze(void)
int CBattle_Field::isQuestMaze() {
    return quest_maze_;
}

// IDA_EA: 0x830E32E
// IDA_NAME: _ZN13CBattle_Field15checkLayeredMapEii
// DEMANGLED: CBattle_Field::checkLayeredMap(int,int)
int CBattle_Field::checkLayeredMap(int pos_x, int pos_y) {
    MazeScript* const maze = getMaze();
    if (!maze) {
        return 0;
    }

    const GridScript& grid = maze->getGridR(pos_x, pos_y);
    if (grid.values_1c.empty()) {
        return 0;
    }

    if (static_cast<int>(grid.values_1c.size()) > getLayeredMapIndex() + 1) {
        setLayeredMapIndex();
    }

    MapInfo* const map_info = getCurrentLayeredMapInfo();
    map_info->completed_ = 1;
    return static_cast<int>(reinterpret_cast<intptr_t>(map_info));
}

// IDA_EA: 0x85BF0B8
// IDA_NAME: _ZN13CBattle_Field13setBloodStateE16ENUM_BLOOD_STATE
// DEMANGLED: CBattle_Field::setBloodState(ENUM_BLOOD_STATE)
bool CBattle_Field::setBloodState(ENUM_BLOOD_STATE state) {
    if (static_cast<int>(state) <= blood_state_) {
        return false;
    }
    blood_state_ = static_cast<int>(state);
    return true;
}

// IDA_EA: 0x85BF260
// IDA_NAME: _ZN13CBattle_Field13get_map_indexEii
// DEMANGLED: CBattle_Field::get_map_index(int,int)
int CBattle_Field::get_map_index(int pos_x, int pos_y) {
    return GetMapInfoFromPos(pos_x, pos_y, false)->map_index_;
}

// IDA_EA: 0x85BF2E8
// IDA_NAME: _ZN13CBattle_Field19IsVisitedCurrentMapEv
// DEMANGLED: CBattle_Field::IsVisitedCurrentMap(void)
int CBattle_Field::IsVisitedCurrentMap() {
    return GetCurrentMapInfo()->visited_goto_;
}

// IDA_EA: 0x830EAC8
// IDA_NAME: _ZN13CBattle_Field14GetLastMapInfoEv
// DEMANGLED: CBattle_Field::GetLastMapInfo(void)
MapInfo* CBattle_Field::GetLastMapInfo() {
    CDungeon* const dungeon = get_dungeon();
    if (dungeon && static_cast<unsigned char>(dungeon->IsEnterEachMap())) {
        const int seat = current_seat_no_;
        return GetMapInfoFromPos(last_positions_[seat].first,
                                 last_positions_[seat].second,
                                 false);
    }
    return GetMapInfoFromPos(previous_position_.first,
                             previous_position_.second,
                             false);
}

// IDA_EA: 0x85BF300
// IDA_NAME: _ZN13CBattle_Field20setWorldMapTowerAreaEb
// DEMANGLED: CBattle_Field::setWorldMapTowerArea(bool)
CBattle_Field* CBattle_Field::setWorldMapTowerArea(bool tower_area) {
    world_map_tower_area_ = tower_area;
    return this;
}

// IDA_EA: 0x85BF32C
// IDA_NAME: _ZN13CBattle_Field10GetCurPosXEv
// DEMANGLED: CBattle_Field::GetCurPosX(void)
int CBattle_Field::GetCurPosX() {
    CDungeon* const dungeon = get_dungeon();
    if (dungeon && static_cast<unsigned char>(dungeon->IsEnterEachMap())) {
        return start_positions_[current_seat_no_].first;
    }
    return current_position_.first;
}

// IDA_EA: 0x85BF382
// IDA_NAME: _ZN13CBattle_Field10GetCurPosYEv
// DEMANGLED: CBattle_Field::GetCurPosY(void)
int CBattle_Field::GetCurPosY() {
    CDungeon* const dungeon = get_dungeon();
    if (dungeon && static_cast<unsigned char>(dungeon->IsEnterEachMap())) {
        return start_positions_[current_seat_no_].second;
    }
    return current_position_.second;
}

// IDA_EA: 0x830A57C
// IDA_NAME: _ZN13CBattle_Field22reset_hell_party_valueEv
// DEMANGLED: CBattle_Field::reset_hell_party_value(void)
bool CBattle_Field::reset_hell_party_value() {
    stHellParty_t* values = G_CDataManager()->get_hellparty_script_values();
    if (!values) {
        return false;
    }
    hell_party_value_max_ = values->max_base_value;
    hell_party_min_value_ = values->min_base_value;
    hell_party_clear_base_value_ = values->base_rising_value;
    hell_party_sub_rate_ = values->sub_base_rates / 10000.0f;
    hell_party_pass_sub_rate_ = values->pass_sub_base_rates / 10000.0f;
    hell_party_extra_clear_rate_ =
        values->ancient_dungeon_add_rate / 10000.0f;
    for (int i = 0; i <= 3; ++i) {
        hell_party_level_rates_[i] =
            values->level_difference_rates[i] / 10000.0f;
    }
    for (int i = 0; i <= 4; ++i) {
        hell_party_difficulty_rates_[i] =
            values->difficulty_rates[i] / 10000.0f;
    }
    for (int i = 0; i <= 3; ++i) {
        hell_party_member_rates_[i] =
            values->party_rates[i] / 10000.0f;
    }
    return false;
}

// IDA_EA: 0x8306D66
// IDA_NAME: _ZN13CBattle_Field11CBloodRound18reset_blood_valuesEv
// DEMANGLED: CBattle_Field::CBloodRound::reset_blood_values(void)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::reset_blood_values() {
    blood_last_small_round_order = 0;
    blood_round_order = 0;
    blood_small_round_order = 0;
    return this;
}

// IDA_EA: 0x8306D86
// IDA_NAME: _ZN13CBattle_Field18reset_blood_valuesEv
// DEMANGLED: CBattle_Field::reset_blood_values(void)
CBattle_Field* CBattle_Field::reset_blood_values() {
    blood_systems_.clear();
    blood_phase_times_.clear();
    blood_rounds_.clear();
    blood_flags_.clear();
    std::memset(blood_spawn_runtime_.registered_small_rounds,
                0,
                sizeof(blood_spawn_runtime_.registered_small_rounds));
    std::memset(blood_spawn_runtime_.phase_finished,
                0,
                sizeof(blood_spawn_runtime_.phase_finished));
    blood_spawn_runtime_.selected_small_round = 0xffffu;
    blood_spawn_runtime_.selected_system_monster_id = 0;
    blood_spawn_runtime_.selected_variant = 0;
    return this;
}

// IDA_EA: 0x830A1F6
// IDA_NAME: _ZN13CBattle_Field11reset_fieldEv
// DEMANGLED: CBattle_Field::reset_field(void)
CBattle_Field* CBattle_Field::reset_field() {
    time_spiral::ResetParty(party_);
    dungeon_mission::ResetParty(party_);
    dungeon_index_ = 0;
    map_height_ = 0;
    map_width_ = 0;
    maze_start_position_ = std::make_pair(0, 0);
    maze_boss_position_ = std::make_pair(0, 0);
    maze_boss_neighbor_position_ = std::make_pair(0, 0);
    current_position_ = std::make_pair(0, 0);
    for (int i = 0; i <= 3; ++i) {
        start_positions_[i] = std::make_pair(0, 0);
        last_positions_[i] = std::make_pair(0, 0);
    }
    initial_map_flag_ = 1;
    dungeon_difficulty_ = 0;
    dungeon_select_parameter_ = 0;
    party_member_count_ = 0;
    champion_spawn_rate_ = 0;
    key_count_ = 0;
    dungeon_ = 0;
    dungeon_type_value_ = 0;
    instance_dungeon_mode_ = 0;
    hell_party_enabled_ = 0;
    quest_stage_map_index_ = -1;
    quest_stage_pos_x_ = -1;
    quest_stage_pos_y_ = -1;
    quest_stage_map_selected_ = 0;
    ridable_.Clear();
    hell_party_runtime_pending_ = 0;
    hell_party_requested_ = 0;
    hell_party_runtime_flag_ = 0;
    std::memset(hell_party_runtime_values_,
                0,
                sizeof(hell_party_runtime_values_));
    small_maze_hell_party_suppressed_ = 0;
    hell_party_difficulty_ = 0;
    all_hell_groups_killed_ = 0;
    map_infos_.clear();
    boss_room_conditions_.clear();
    map_order_.clear();
    boss_die_check_started_at_sec_ = 0;
    event_monsters_.clear();
    reset_blood_values();
    blood_round_.reset_blood_values();
    blood_state_ = 0;
    blood_clear_reward_.reset();
    tower_reward_.reset_tod();
    unsigned short& village_monster = village_monster_uid_;
    if (village_monster) {
        GlobalData::s_villageMonsterMgr->OnKillVillageMonster(
                village_monster_village_,
                village_monster_area_,
                village_monster,
                false);
    }
    village_monster = 0;
    village_monster_village_ = 0;
    village_monster_area_ = 0;
    village_monster_route_ = 0;
    elevator_timer_state_ = 2;
    elevator_timer_count_ = 0;
    elevator_timer_armed_ = 1;
    hell_party_event_type_ = 0;
    world_map_tower_area_ = 0;
    layered_map_index_ = -1;
    quest_maze_ = 0;
    return SetCurSeatNo(255);
}

// IDA_EA: 0x82A50B0
// IDA_NAME: _ZN16CTraceMobDieHack5resetEv
// DEMANGLED: CTraceMobDieHack::reset(void)
bool CTraceMobDieHack::reset() {
    monster_damage_log.clear();
    trace_begin_second = 0;
    mob_killers.clear();
    hack_logs.clear();
    trace_active = false;
    dungeon_diff = 5;
    for (int i = 0; i <= 3; ++i) {
        trace_users[i].user = 0;
        trace_users[i].elapsed_seconds = 0;
        trace_users[i].sample_count = 0;
    }
    // The original epilogue returns the final `i <= 3` condition (false).
    return false;
}

// IDA_EA: 0x8599CB2
// IDA_NAME: _ZN16CTraceMobDieHack14reportHackInfoEv
// DEMANGLED: CTraceMobDieHack::reportHackInfo(void)
bool CTraceMobDieHack::reportHackInfo() {
    if (!GlobalData::s_villageMonsterMgr->OnEvent()) {
        for (int i = 0; i <= 3; ++i) {
            CUser* const user = trace_users[i].user;
            const int count = trace_users[i].sample_count;
            if (user && count > 1) {
                const int average_time =
                    trace_users[i].elapsed_seconds / count;
                if (dungeon_diff != 5 && !special_dungeon) {
                    int threshold = 10;
                    if (dungeon_diff == 1) {
                        threshold = 6;
                    } else if (dungeon_diff == 2) {
                        threshold = 7;
                    } else if (dungeon_diff == 3) {
                        threshold = 8;
                    } else if (dungeon_diff == 0) {
                        threshold = 5;
                    }
                    if (average_time <= threshold) {
                        user->getHackAnalyzer()->addServerHackCnt(
                            user,
                            static_cast<WongWork::ENUM_HACKTYPE>(0x19eu),
                            static_cast<unsigned int>(
                                threshold - average_time + 1),
                            0u,
                            0u);
                    }
                }
                if (special_dungeon) {
                    if (average_time <= 9) {
                        user->getHackAnalyzer()->addServerHackCnt(
                            user,
                            static_cast<WongWork::ENUM_HACKTYPE>(0x194u),
                            1u,
                            0u,
                            0u);
                    }
                } else if (average_time <= 19) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(0x193u),
                        1u,
                        0u,
                        0u);
                }
            }
        }
    }
    return reset();
}

// IDA_EA: 0x85BF4CC
// IDA_NAME: _ZN16CTraceMobDieHack10traceBeginElb
// DEMANGLED: CTraceMobDieHack::traceBegin(long,bool)
CTraceMobDieHack* CTraceMobDieHack::traceBegin(long begin_second,
                                                bool special) {
    trace_begin_second = begin_second;
    trace_active = true;
    special_dungeon = special;
    return this;
}

// IDA_EA: 0x8599A88
// IDA_NAME: _ZN16CTraceMobDieHack8traceEndEP5CUserPS1_
// DEMANGLED: CTraceMobDieHack::traceEnd(CUser *,CUser **)
bool CTraceMobDieHack::traceEnd(CUser* manager, CUser* const users[4]) {
    if (!trace_active) {
        return false;
    }

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    trace_active = false;
    CParty* const manager_party = manager ? manager->GetParty() : 0;
    CDungeon* const dungeon = manager_party
                                   ? manager_party->battle_field().GetDungeon()
                                   : 0;
    if (!dungeon) {
        return false;
    }

    const int dungeon_index = dungeon->get_index();
    if (dungeon_index == 1 || dungeon_index == 2 || dungeon_index == 41 ||
        dungeon_index == 33) {
        return true;
    }

    for (int index = 0; index <= 3; ++index) {
        CUser* const user = users[index];
        CParty* const user_party = user ? user->GetParty() : 0;
        CDungeon* const user_dungeon =
            user_party ? user_party->battle_field().GetDungeon() : 0;
        if (!user || !user->getCurCharacR() || !user_dungeon) {
            continue;
        }
        if (user->get_charac_level() <= user_dungeon->get_standard_level() +
                                             10) {
            trace_users[index].user = user;
            trace_users[index].elapsed_seconds +=
                current_second - trace_begin_second;
            ++trace_users[index].sample_count;
        }
    }
    return false;
}

// IDA_EA: 0x85BF4F6
// IDA_NAME: _ZN16CTraceMobDieHack16addMonsterDamageEiti
// DEMANGLED: CTraceMobDieHack::addMonsterDamage(int,unsigned short,int)
int CTraceMobDieHack::addMonsterDamage(int damage_value,
                                       unsigned short monster_type,
                                       int monster_id) {
    std::map<unsigned short, stMonsterDamageLog_t>::iterator found =
        monster_damage_log.find(monster_type);
    if (found == monster_damage_log.end()) {
        stMonsterDamageLog_t entry;
        entry.monster_id = monster_type;
        entry.source_id = static_cast<uint16_t>(monster_id);
        entry.occurrence_count = 1;
        entry.damage_value = damage_value;
        found = monster_damage_log.insert(std::make_pair(monster_type, entry)).first;
    } else if (found->second.source_id == static_cast<uint16_t>(monster_id)) {
        ++found->second.occurrence_count;
    } else {
        found->second.monster_id = monster_type;
        found->second.source_id = static_cast<uint16_t>(monster_id);
        found->second.occurrence_count = 1;
        found->second.damage_value = damage_value;
    }
    return found->second.occurrence_count;
}

// IDA_EA: 0x85BF628
// IDA_NAME: _ZN16CTraceMobDieHack12setMobKillerEi
// DEMANGLED: CTraceMobDieHack::setMobKiller(int)
void CTraceMobDieHack::setMobKiller(int monster_id) {
    mob_killers.push_back(monster_id);
}

// IDA_EA: 0x85BF646
// IDA_NAME: _ZN16CTraceMobDieHack14setDungeonDiffEi
// DEMANGLED: CTraceMobDieHack::setDungeonDiff(int)
CTraceMobDieHack* CTraceMobDieHack::setDungeonDiff(int value) {
    dungeon_diff = value;
    return this;
}

// IDA_EA: 0x830DE1C
// IDA_NAME: _ZN13CBattle_Field21GetTotalSkillMaterialEv
// DEMANGLED: CBattle_Field::GetTotalSkillMaterial(void)
int CBattle_Field::GetTotalSkillMaterial() {
    MapInfo current;
    int total = 0;
    for (std::map<int, MapInfo>::iterator it = map_infos_.begin();
         it != map_infos_.end();
         ++it) {
        current = it->second;
        if (current.dummy_boss_map_pending_ != 1 && current.visited_goto_) {
            total +=
                static_cast<int>(
                    current.normal_kill_check_.use_skill_material_time);
        }
    }
    return total;
}
