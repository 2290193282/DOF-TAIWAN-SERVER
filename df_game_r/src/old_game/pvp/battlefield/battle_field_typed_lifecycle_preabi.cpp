// Canonical ABI provider for the CParty battle-field graph.

#include "old_game/pvp/battlefield/battle_field.h"

#include <cstring>

// IDA_EA: 0x8151182
// IDA_NAME: _ZN11map_monster14isNamedMonsterEv
// DEMANGLED: map_monster::isNamedMonster(void)
bool map_monster::isNamedMonster() {
    return (monster_id_ > 49999 && monster_id_ <= 51000) ||
           (monster_id_ > 55999 && monster_id_ <= 60000);
}

// IDA_EA: 0x81511C8
// IDA_NAME: _ZN11map_monsterC2Ev
// DEMANGLED: map_monster::map_monster(void)
map_monster::map_monster()
    : active_static_info_(),
      related_monster_ids_() {
    // The old C2 first constructs the two ownership-bearing children, then
    // writes only these proven scalar locations.  In particular, monster_id_
    // and the unrecovered byte spans are intentionally left untouched.
    hell_party_group_order_ = 0;
    hell_party_group_index_ = 0;
    blood_small_round_order_ = 0;
    blood_spawn_rate_per_mille_ = 0;
    blood_system_value_08_ = 0;
    blood_system_value_0c_ = 0;
    blood_system_value_10_ = 0;
    blood_runtime_flag_ = 0;
    blood_variant_ = 0;
    blood_round_ = 0;
    blood_round_difficulty_ = 0;
    drop_limit_excluded_ = 0;
    counted_by_map_info_ = 0;
    spawn_weight_ = 100;
    monster_kind_ = 0;
    runtime_value_18_ = 0;
    completed_ = 0;
}

// IDA_EA: 0x8151AEA
// IDA_NAME: _ZN11map_monsterC2ERKS_
// DEMANGLED: map_monster::map_monster(map_monster const&)
map_monster::map_monster(const map_monster& rhs)
    : spawn_sequence_(rhs.spawn_sequence_),
      map_monster_unique_id_(rhs.map_monster_unique_id_),
      monster_kind_(rhs.monster_kind_),
      monster_id_(rhs.monster_id_),
      monster_level_(rhs.monster_level_),
      specific_drop_count_(rhs.specific_drop_count_),
      random_drop_count_(rhs.random_drop_count_),
      packet_value_13_(rhs.packet_value_13_),
      stage_monster_type_(rhs.stage_monster_type_),
      runtime_value_18_(rhs.runtime_value_18_),
      runtime_value_1c_(rhs.runtime_value_1c_),
      runtime_value_20_(rhs.runtime_value_20_),
      sdc_damaged_packet_count_(rhs.sdc_damaged_packet_count_),
      sdc_damage_value_(rhs.sdc_damage_value_),
      assigned_team_(rhs.assigned_team_),
      spawn_weight_(rhs.spawn_weight_),
      active_static_info_(rhs.active_static_info_),
      related_monster_ids_(rhs.related_monster_ids_),
      hell_party_group_order_(rhs.hell_party_group_order_),
      hell_party_group_index_(rhs.hell_party_group_index_),
      blood_small_round_order_(rhs.blood_small_round_order_),
      blood_spawn_rate_per_mille_(rhs.blood_spawn_rate_per_mille_),
      blood_system_value_08_(rhs.blood_system_value_08_),
      blood_system_value_0c_(rhs.blood_system_value_0c_),
      blood_system_value_10_(rhs.blood_system_value_10_),
      blood_runtime_flag_(rhs.blood_runtime_flag_),
      blood_variant_(rhs.blood_variant_),
      blood_round_(rhs.blood_round_),
      blood_round_difficulty_(rhs.blood_round_difficulty_),
      drop_limit_excluded_(rhs.drop_limit_excluded_),
      counted_by_map_info_(rhs.counted_by_map_info_),
      completed_(rhs.completed_) {
    std::memcpy(raw_09_0b_, rhs.raw_09_0b_, sizeof(raw_09_0b_));
    runtime_value_1c_ = rhs.runtime_value_1c_;
    runtime_value_20_ = rhs.runtime_value_20_;
    sdc_damaged_packet_count_ = rhs.sdc_damaged_packet_count_;
    no_hit_animation_ = rhs.no_hit_animation_;
    std::memcpy(raw_35_37_, rhs.raw_35_37_, sizeof(raw_35_37_));
    hell_party_difficulty_ = rhs.hell_party_difficulty_;
    reserved_339_ = rhs.reserved_339_;
}

// IDA_EA: 0x8151D3A
// IDA_NAME: _ZN11map_monsteraSERKS_
// DEMANGLED: map_monster::operator=(map_monster const&)
map_monster& map_monster::operator=(const map_monster& rhs) {
    spawn_sequence_ = rhs.spawn_sequence_;
    map_monster_unique_id_ = rhs.map_monster_unique_id_;
    monster_kind_ = rhs.monster_kind_;
    std::memcpy(raw_09_0b_, rhs.raw_09_0b_, sizeof(raw_09_0b_));
    monster_id_ = rhs.monster_id_;
    monster_level_ = rhs.monster_level_;
    specific_drop_count_ = rhs.specific_drop_count_;
    random_drop_count_ = rhs.random_drop_count_;
    packet_value_13_ = rhs.packet_value_13_;
    stage_monster_type_ = rhs.stage_monster_type_;
    std::memcpy(raw_15_17_, rhs.raw_15_17_, sizeof(raw_15_17_));
    runtime_value_18_ = rhs.runtime_value_18_;
    runtime_value_1c_ = rhs.runtime_value_1c_;
    runtime_value_20_ = rhs.runtime_value_20_;
    sdc_damaged_packet_count_ = rhs.sdc_damaged_packet_count_;
    sdc_damage_value_ = rhs.sdc_damage_value_;
    assigned_team_ = rhs.assigned_team_;
    spawn_weight_ = rhs.spawn_weight_;
    no_hit_animation_ = rhs.no_hit_animation_;
    std::memcpy(raw_35_37_, rhs.raw_35_37_, sizeof(raw_35_37_));
    active_static_info_ = rhs.active_static_info_;
    related_monster_ids_ = rhs.related_monster_ids_;
    hell_party_group_order_ = rhs.hell_party_group_order_;
    hell_party_group_index_ = rhs.hell_party_group_index_;
    hell_party_difficulty_ = rhs.hell_party_difficulty_;
    reserved_339_ = rhs.reserved_339_;
    blood_small_round_order_ = rhs.blood_small_round_order_;
    blood_spawn_rate_per_mille_ = rhs.blood_spawn_rate_per_mille_;
    blood_system_value_08_ = rhs.blood_system_value_08_;
    blood_system_value_0c_ = rhs.blood_system_value_0c_;
    blood_system_value_10_ = rhs.blood_system_value_10_;
    blood_runtime_flag_ = rhs.blood_runtime_flag_;
    blood_variant_ = rhs.blood_variant_;
    blood_round_ = rhs.blood_round_;
    blood_round_difficulty_ = rhs.blood_round_difficulty_;
    drop_limit_excluded_ = rhs.drop_limit_excluded_;
    counted_by_map_info_ = rhs.counted_by_map_info_;
    completed_ = rhs.completed_;
    return *this;
}

// IDA_EA: 0x81515AE
// IDA_NAME: _ZN11map_monsterD2Ev
// DEMANGLED: map_monster::~map_monster()
map_monster::~map_monster() {}

// IDA_EA: 0x8151394
// IDA_NAME: _ZN7MapInfoC2Ev
// DEMANGLED: MapInfo::MapInfo(void)
MapInfo::MapInfo()
    : monsters_(),
      items_(),
      monster_kill_counts_(),
      monster_identity_by_unique_id_(),
      monster_order_(),
      repeated_monsters_() {
    Clear();
}

// IDA_EA: 0x81514AE
// IDA_NAME: _ZN7MapInfo5ClearEv
// DEMANGLED: MapInfo::Clear(void)
MapInfo* MapInfo::Clear() {
    visited_ = 0;
    visited_goto_ = 0;
    visited_transition_ = 0;
    map_index_ = 0;
    map_layer_ = 0;
    monsters_.clear();
    items_.clear();
    next_monster_unique_id_ = 0;
    next_drop_item_unique_id_ = 0;
    map_clearable_ = true;
    dummy_boss_map_pending_ = 0;
    random_map_seed_ = 0;
    normal_kill_check_.init();
    boss_kill_check_.init();
    play_info_.init();
    monster_kill_counts_.clear();
    assign_npc_by_potion_ = 0;
    monster_identity_by_unique_id_.clear();
    monster_order_.clear();
    named_monster_count_ = 0;
    repeated_monsters_.clear();
    completed_ = 0;
    return this;
}

// IDA_EA: 0x830EEA2
// IDA_NAME: _ZN7MapInfoaSERKS_
// DEMANGLED: MapInfo::operator=(MapInfo const&)
MapInfo& MapInfo::operator=(const MapInfo& source) {
    // Preserve the original fieldwise copy boundary: the ABI padding bytes
    // remain padding and are not promoted into gameplay state.
    visited_ = source.visited_;
    visited_goto_ = source.visited_goto_;
    visited_transition_ = source.visited_transition_;
    map_index_ = source.map_index_;
    map_layer_ = source.map_layer_;
    monsters_ = source.monsters_;
    items_ = source.items_;
    next_monster_unique_id_ = source.next_monster_unique_id_;
    next_drop_item_unique_id_ = source.next_drop_item_unique_id_;
    map_clearable_ = source.map_clearable_;
    dummy_boss_map_pending_ = source.dummy_boss_map_pending_;
    random_map_seed_ = source.random_map_seed_;
    normal_kill_check_ = source.normal_kill_check_;
    boss_kill_check_ = source.boss_kill_check_;
    play_info_ = source.play_info_;
    monster_kill_counts_ = source.monster_kill_counts_;
    assign_npc_by_potion_ = source.assign_npc_by_potion_;
    monster_identity_by_unique_id_ = source.monster_identity_by_unique_id_;
    monster_order_ = source.monster_order_;
    named_monster_count_ = source.named_monster_count_;
    repeated_monsters_ = source.repeated_monsters_;
    completed_ = source.completed_;
    return *this;
}

// IDA_EA: 0x81518BC
// IDA_NAME: _ZN7MapInfoD2Ev
// DEMANGLED: MapInfo::~MapInfo()
MapInfo::~MapInfo() {}

// IDA_EA: 0x82A4406
// IDA_NAME: _ZN8CRidableC2Ev
// DEMANGLED: CRidable::CRidable(void)
CRidable::CRidable()
    : std::map<int, DungeonRidableObject>() {}

// IDA_EA: 0x82A441A
// IDA_NAME: _ZN8CRidableD2Ev
// DEMANGLED: CRidable::~CRidable()
CRidable::~CRidable() {}

// IDA_EA: 0x82A4342
// IDA_NAME: _ZN15CClearConditionC2Ev
// DEMANGLED: CClearCondition::CClearCondition(void)
CClearCondition::CClearCondition()
    : cleared_states_() {
    Clear();
}

// IDA_EA: 0x82A438A
// IDA_NAME: _ZN15CClearCondition5ClearEv
// DEMANGLED: CClearCondition::Clear(void)
CClearCondition* CClearCondition::Clear() {
    current_count_ = 0;
    target_count_ = 0;
    cleared_states_.clear();
    condition_source_ = 0;
    return this;
}

// IDA_EA: 0x82A442E
// IDA_NAME: _ZN15CClearConditionD2Ev
// DEMANGLED: CClearCondition::~CClearCondition()
CClearCondition::~CClearCondition() {}

// IDA_EA: 0x830EA64
// IDA_NAME: _ZN13CBattle_Field14TournamentInfoC2Ev
// DEMANGLED: CBattle_Field::TournamentInfo::TournamentInfo(void)
CBattle_Field::TournamentInfo::TournamentInfo()
    : monsters_() {
    Clear();
}

// IDA_EA: 0x830EA1E
// IDA_NAME: _ZN13CBattle_Field14TournamentInfo5ClearEv
// DEMANGLED: CBattle_Field::TournamentInfo::Clear(void)
CBattle_Field::TournamentInfo* CBattle_Field::TournamentInfo::Clear() {
    round_ = -1;
    monsters_[0].Clear();
    monsters_[1].Clear();
    key_ = 0;
    return this;
}

// IDA_EA: 0x830EAA6
// IDA_NAME: _ZNK13CBattle_Field14TournamentInfoltERKS0_
// DEMANGLED: CBattle_Field::TournamentInfo::operator<(CBattle_Field::TournamentInfo const&)const
bool CBattle_Field::TournamentInfo::operator<(
    const TournamentInfo& rhs) const {
    return key_ < rhs.key_;
}

// IDA_EA: 0x8314BB6
// IDA_NAME: _ZN13CBattle_Field14TournamentInfoD2Ev
// DEMANGLED: CBattle_Field::TournamentInfo::~TournamentInfo()
CBattle_Field::TournamentInfo::~TournamentInfo() {}

// IDA_EA: 0x82A4458
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData14stRewardItem_tC2Ev
// DEMANGLED: CBattle_Field::CBloodClearRewardData::stRewardItem_t::stRewardItem_t(void)
CBattle_Field::CBloodClearRewardData::stRewardItem_t::stRewardItem_t()
    : items_() {}

// IDA_EA: 0x82A448E
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardDataC2Ev
// DEMANGLED: CBattle_Field::CBloodClearRewardData::CBloodClearRewardData(void)
CBattle_Field::CBloodClearRewardData::CBloodClearRewardData()
    : reward_items_() {}

// IDA_EA: 0x8284770
// IDA_NAME: _ZN24CTournamentDungeonRewardC2Ev
// DEMANGLED: CTournamentDungeonReward::CTournamentDungeonReward(void)
CTournamentDungeonReward::CTournamentDungeonReward()
    : rewards_() {
    ResetReward();
}

// IDA_EA: 0x82847DC
// IDA_NAME: _ZN24CTournamentDungeonRewardD2Ev
// DEMANGLED: CTournamentDungeonReward::~CTournamentDungeonReward()
CTournamentDungeonReward::~CTournamentDungeonReward() {}

// IDA_EA: 0x8284820
// IDA_NAME: _ZN24CTournamentDungeonReward11ResetRewardEv
// DEMANGLED: CTournamentDungeonReward::ResetReward(void)
CTournamentDungeonReward* CTournamentDungeonReward::ResetReward() {
    for (int row = 0; row != 2; ++row) {
        for (int column = 0; column != 2; ++column) {
            rewards_[row][column].Clear();
        }
    }
    reward_round_ = 0;
    return this;
}

// IDA_EA: 0x82A44FE
// IDA_NAME: _ZN13CBattle_FieldC2Ev
// DEMANGLED: CBattle_Field::CBattle_Field(void)
CBattle_Field::CBattle_Field()
    : start_positions_(),
      last_positions_(),
      map_infos_(),
      map_info_mutex_(),
      state_mutex_(),
      map_path_(),
      ridable_(),
      clear_condition_(),
      map_order_(),
      tournament_infos_(),
      boss_room_conditions_(),
      event_monsters_(),
      blood_systems_(),
      blood_phase_times_(),
      blood_rounds_(),
      blood_flags_(),
      blood_clear_reward_(),
      tower_reward_(),
      tournament_reward_() {
    // The archive performs no scalar stores until every nested owner above
    // has been constructed.  Keep this body in its original write/call
    // order; bytes not named below are intentionally preserved.
    party_ = 0;
    current_position_ = std::make_pair(-1, -1);
    previous_position_ = std::make_pair(-1, -1);
    maze_start_position_ = std::make_pair(-1, -1);
    maze_boss_position_ = std::make_pair(-1, -1);
    maze_boss_neighbor_position_ = std::make_pair(-1, -1);
    for (int index = 0; index != 4; ++index) {
        start_positions_[index] = std::make_pair(0, 0);
        last_positions_[index] = std::make_pair(0, 0);
    }
    initial_map_flag_ = 1;
    map_height_ = 0;
    map_width_ = map_height_;
    blood_spawn_runtime_.ultimate_difficulty = 0;
    village_monster_uid_ = 0;
    dimension_level_ = 0;
    instance_dungeon_mode_ = 0;
    world_map_tower_area_ = 0;
    tournament_infos_.clear();
    tournament_reward_.ResetReward();
    tournament_cur_round_ = 0;
    layered_map_index_ = -1;
    quest_maze_ = 0;
    SetCurSeatNo(255);
}

// IDA_EA: 0x82A499C
// IDA_NAME: _ZN13CBattle_FieldD2Ev
// DEMANGLED: CBattle_Field::~CBattle_Field()
CBattle_Field::~CBattle_Field() {}

// IDA_EA: 0x82A4CEA
// IDA_NAME: _ZN10BattleDataC2Ev
// DEMANGLED: BattleData::BattleData(void)
BattleData::BattleData()
    : normal_rewards_(),
      special_rewards_(),
      event_rewards_(),
      game_results_() {
    Reset();
}

// IDA_EA: 0x82A54D6
// IDA_NAME: _ZN10BattleDataD2Ev
// DEMANGLED: BattleData::~BattleData()
BattleData::~BattleData() {}

// IDA_EA: 0x84B75D2
// IDA_NAME: _ZN13GameResultSetC2Ev
// DEMANGLED: GameResultSet::GameResultSet(void)
GameResultSet::GameResultSet()
    : GameResultType() {}

// IDA_EA: 0x84B75E6
// IDA_NAME: _ZN13GameResultSetD2Ev
// DEMANGLED: GameResultSet::~GameResultSet()
GameResultSet::~GameResultSet() {}

// IDA_EA: 0x84B75FA
// IDA_NAME: _ZN13GameResultSet5ClearEv
// DEMANGLED: GameResultSet::Clear(void)
GameResultSet* GameResultSet::Clear() {
    GameResultType::Clear();
    return this;
}

// IDA_EA: 0x82A517C
// IDA_NAME: _ZN16CTraceMobDieHackC2Ev
// DEMANGLED: CTraceMobDieHack::CTraceMobDieHack(void)
CTraceMobDieHack::CTraceMobDieHack()
    : monster_damage_log(),
      secondary_damage_log(),
      mob_killers(),
      hack_logs() {
    reset();
}

// IDA_EA: 0x82A5236
// IDA_NAME: _ZN16CTraceMobDieHackD2Ev
// DEMANGLED: CTraceMobDieHack::~CTraceMobDieHack()
CTraceMobDieHack::~CTraceMobDieHack() {}
