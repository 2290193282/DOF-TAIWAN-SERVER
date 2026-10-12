#ifndef DNF_DATAMANAGER_MAP_RUNTIME_H_
#define DNF_DATAMANAGER_MAP_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>
#include <ext/hash_map>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/ai_character_script.h"
#include "old_data/PVF/data/datamanager/required_item.h"
#include "old_game/world/region/world_area_types.h"
#include "old_game/world/region/world_value_types.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/hell_party_group_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/map/back_ani_script.h"
#include "old_data/PVF/map/map_script_types.h"
#include "old_data/PVF/dungeon/dungeon_script_value_types.h"
#include "old_data/PVF/dungeon/maze_script.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "old_game/activity/reward/advance_altar_timeline_parameter.h"

namespace advancealtar {
class ProcStage;
}

namespace WongWork {
class CBossTower;
}

HellPartyGroupOrderMapPool* Get_instance_groupOrder_map_pool()
    __asm__("_Z32Get_instance_groupOrder_map_poolv");

namespace map_runtime {

// ABI-safe 140828 Map extensions.  None of these fields is embedded in the
// recovered 0627 STMapScript/CMap layouts; map_runtime.cpp owns the typed
// registry and publishes it only after a complete map load succeeds.
struct ChaosMapConfig0828 {
    bool chaos_zone;
    bool has_phase_max;
    int32_t phase_max;
    bool has_phase_time;
    int32_t phase_time;
    bool has_final_gate_index;
    int32_t final_gate_index;
    bool has_gate_index;
    int32_t gate_index;
    bool has_auto_revive_interval;
    int32_t auto_revive_interval;
    bool has_gate_bonus_fp;
    int32_t gate_bonus_fp;
    bool has_phase_fp;
    int32_t phase_fp;

    ChaosMapConfig0828()
        : chaos_zone(false), has_phase_max(false), phase_max(0),
          has_phase_time(false), phase_time(0),
          has_final_gate_index(false), final_gate_index(0),
          has_gate_index(false), gate_index(0),
          has_auto_revive_interval(false), auto_revive_interval(0),
          has_gate_bonus_fp(false), gate_bonus_fp(0),
          has_phase_fp(false), phase_fp(0) {}
};

struct TwinGauge0828 {
    int32_t first_monster_index;
    int32_t second_monster_index;

    TwinGauge0828() : first_monster_index(-1), second_monster_index(-1) {}
};

// Exact runtime queries.  Chaos values deliberately remain metadata-only:
// neither the 0627 server pseudocode nor the recovered 0828 evidence proves a
// server gameplay policy consumer for them.
bool GetChaosMapConfig0828(int map_index, ChaosMapConfig0828* config);
bool GetTwinGauge0828(int map_index, TwinGauge0828* gauge);

// CDataManager::load_map_files stages the whole map registry and swaps it into
// the visible store only after every map has loaded successfully.
void BeginMapExtensionLoad0828();
void CommitMapExtensionLoad0828();
void AbortMapExtensionLoad0828();
void ClearMapExtensions0828();

}  // namespace map_runtime

void procSpecialPassiveObject(SPOBJ_ACTION_DATAS& action,
                              STAssignSpecialPassiveObj& object,
                              int map_index);

// IDA unit: 12_pvf_datamanager_scripts/STMapScript.
// IDA_EA: 0x836CC94, 0x836C688, 0x89DCF54.
struct STMapScript {
    STMapScript();
    STMapScript(const STMapScript& rhs);
    ~STMapScript();
    STMapScript* Clear();

    int32_t map_index;
    std::list<int> dungeon_indices;
    int32_t map_type;
    std::vector<std::pair<int, int> > greed_cells;
    std::list<STAssignAnimation> animations;
    std::list<STQuestAnimation> quest_animations;
    std::list<STAssignPassiveObject> passive_objects;
    std::vector<STAssignMonster> monsters;
    std::vector<STAssignMonster> additional_monsters;
    int32_t champion;
    std::list<STAssignItem> items;
    std::list<STAssignSpecialPassiveObj> special_passive_objects;
    std::vector<AssignNpc> npcs;
    int32_t elemental_property;
    std::vector<std::string> tiles;
    std::vector<std::string> extended_tiles;
    int32_t far_sight_scroll;
    int32_t middle_sight_scroll;
    int32_t near_sight_scroll;
    int32_t value_094;
    int32_t value_098;
    int32_t value_09c;
    std::vector<BackAniScript> background_animations;
    std::vector<BackGroundEffect> background_effects;
    std::vector<BackGroundEffectBlock> background_effect_blocks;
    std::vector<STDialog> dialogs;
    int32_t absolute_start_path;
    uint8_t block_path_0;
    uint8_t block_path_1;
    uint8_t block_path_2;
    uint8_t block_path_3;
    uint8_t tile_upper;
    uint8_t reserved_0d9;
    uint8_t reserved_0da;
    uint8_t reserved_0db;
    std::vector<TownPath> town_paths;
    std::vector<MapArea> map_areas;
    std::vector<TownPath> town_path_links;
    // [pathgate pos] and [pathgate recognize range].  CMap::set_map copies
    // these two 4x2 integer grids verbatim into its +0x2c/+0x4c members.
    int32_t pathgate_positions[4][2];        // +0x100
    int32_t pathgate_recognize_ranges[4][2]; // +0x120
    MapArea dungeon_start_area;
    MapArea value_15c_area;
    int32_t value_178;
    int32_t value_17c;
    std::vector<std::pair<int, int> > camera_recovery_objects;
    std::string opening_bgm;
    int32_t opening_bgm_index;
    std::vector<std::string> sound_files;
    std::string directory;
    std::string show_dust_left;
    std::string show_dust_right;
    uint8_t sort_bottom;
    uint8_t reserved_1ad;
    uint8_t reserved_1ae;
    uint8_t reserved_1af;
    std::string map_name;
    int32_t player_number_min;
    int32_t player_number_max;
    MapArea pvp_start_area_0;
    MapArea pvp_start_area_1;
    MapArea pvp_start_area_2;
    MapArea pvp_start_area_3;
    MapArea pvp_start_area_4;
    uint8_t pvp_start_area_by_party_slot;
    uint8_t map_move_keep_state_axis_x;
    uint8_t map_move_keep_state_axis_y;
    uint8_t map_move_keep_state_axis_z;
    int32_t shadow_type;
    std::vector<STRivalMapData> rival_positions;
    std::vector<STAICharacterMapArrangeData> apc_arrangements;
    int32_t apc_random_point;
    std::string map_loading_image_path;
    int32_t value_270;
    int32_t value_274;
    std::vector<STRivalMapData> event_monster_positions;
    int32_t heroes_mode_map_index;
    std::vector<STAssignBloodSystem> blood_systems;
    std::vector<STAssignBloodPhaseTime> blood_phase_times;
    std::vector<STAssignDefenseSystem> defense_systems;
    std::vector<STAssignDefensePhaseTime> defense_phase_times;
    std::map<char, std::vector<TournamentMonster> > tournament_monsters;
    std::vector<TournamentStartPos> tournament_start_area_1;
    std::vector<TournamentStartPos> tournament_start_area_2;
    std::vector<StaticStartPos> static_player_start_positions;
    std::vector<StaticStartPos> static_player_start_positions_alt;
    std::map<int, MapArea> limit_player_move_areas;
    std::vector<STAttackedMonster> attacked_monsters;
    int32_t max_monster_drop_exception;
    int32_t max_monster_count_exception;
    int32_t background_pos;
    advancealtar::TimeLine advance_altar_timeline;
    std::string map_dialog;
    int32_t value_350;
    int32_t value_354;
    uint8_t value_358;
    // 0828 [tournament enemies] retains whether each party-size table owns
    // monster definitions (role 0) or APC definitions (role 5).  Reuse the
    // two existing padding bytes so the restored STMapScript ABI stays fixed.
    uint8_t tournament_enemy_role_bases[2];
    uint8_t reserved_35b;
    int32_t customized_screen_edge_left;
    int32_t customized_screen_edge_right;
    int32_t customized_screen_edge_top;
    int32_t customized_screen_edge_bottom;
    uint8_t value_36c;
    uint8_t value_36d;
    uint8_t value_36e;
    uint8_t reserved_36f;
    std::string value_370;
    int32_t value_374;
    int32_t value_378;
    // 0828 [pathgate object] is a repeated, unclosed passive-object ID list.
    // CMap::set_map merges these IDs into its existing passive-object owner.
    std::vector<int> pathgate_object_ids;
};

#if defined(__i386__)
static_assert(sizeof(STMapScript) == 0x388u,
              "STMapScript 0828 extension size drift");
static_assert(offsetof(STMapScript, dungeon_indices) == 0x04u,
              "STMapScript dungeon-index list offset drift");
static_assert(offsetof(STMapScript, greed_cells) == 0x10u,
              "STMapScript greed-cell vector offset drift");
static_assert(offsetof(STMapScript, dungeon_start_area) == 0x140u,
              "STMapScript dungeon-start area offset drift");
static_assert(offsetof(STMapScript, dungeon_start_area) +
                      offsetof(MapArea, width) == 0x148u,
              "STMapScript dungeon-start width offset drift");
#endif

// IDA unit: 05_world_enter_village/CMap.
// IDA_EA: C2 0x834DCDA, D2 0x834DEF6, destroy 0x834E13E,
// set_map 0x834E18A.
class CMap {
public:
    CMap();
    ~CMap();
    void destroy();
    int set_map(STMapScript script);
    int get_index() const;
    int get_map_type() const;
    int getTournamentEnemyRoleBase(int party_member_count) const;
    // RECONSTRUCTED_0828: [combo training] is stored in the first byte of
    // the legacy CMap tail padding.  This preserves sizeof(CMap)==0x130
    // while giving CMD577/578 a map-owned authorization boundary.
    bool isComboTraining() const;
    // IDA_EA: 0x8134796 / 0x81508C6 / 0x81508F0 / 0x81508FC.
    std::list<_mapMonster> get_monster_list() const;
    std::list<_mapItem> get_item_list() const;
    // IDA_EA: 0x8373F08.  The original 0x0b-byte body returns the embedded
    // +0x24 dungeon list by reference; callers retain CMap ownership.
    const std::list<int>& getDungeonLists() const;
    bool HasPassiveObject(int object_id) const;
    int getAPCRandomPoint() const;
    const std::vector<STAICharacterMapArrangeData>& getAICharacters() const;
    int getAssignedNPCCount() const;
    int GetBossCount() const;
    std::list<_mapMonster> get_condition_monster_list() const;
    // IDA_EA: 0x834ECAA
    // IDA_NAME: _ZNK4CMap13CheckEntranceEi
    // DEMANGLED: CMap::CheckEntrance(int)const
    int CheckEntrance(int entrance) const;

    int insert_monster(STAssignMonster monster, int special_passive_index);
    void insert_condition_monster(STAssignMonster monster);
    int insert_item(int item_id,
                    int item_count,
                    int drop_id,
                    int special_passive_index);

    // IDA_EA: 0x834EBDA
    // IDA_NAME: _ZNK4CMap14box_item_indexESt4listISt4pairIiiESaIS2_EE
    // DEMANGLED: CMap::box_item_index(std::list<std::pair<int,int>,std::allocator<std::pair<int,int>>>)const
    int box_item_index(std::list<std::pair<int, int> > item_boxes) const;

    // IDA_EA: 0x834E9A6
    // IDA_NAME: _ZNK4CMap20checkFitInPathObjectEii
    // DEMANGLED: CMap::checkFitInPathObject(int,int)const
    int checkFitInPathObject(int pos_x, int pos_y) const;

    // IDA_EA: 0x834ED1E
    // IDA_NAME: _ZN4CMap7destoryEv
    // The misspelling is part of the original exported ABI.
    void destory();
private:
    friend class WarField;
    // ProcStage's start path retains the CMap-owned advance-altar timeline
    // pointer at +0x0f4.  Keep the original owner relation explicit.
    friend class advancealtar::ProcStage;
    // CBattle_Field's original map assembly reads the concrete script values
    // below directly.  Grant the recovered owner class access to the named
    // fields instead of recreating their layout in gameplay code.
    friend class CBattle_Field;
    // CParty::kill_monster @0x85A27E8 reads the original CMap +0x0f0
    // override while maintaining BattleData's per-map kill/drop counters.
    friend class CParty;
    // Village::set_village records every town unlocked by this dungeon in
    // the original vector at +0x6d4.  Keep the direct owner relation rather
    // than exposing a fabricated dungeon-list accessor.
    friend class Village;

    int32_t boss_count_;                         // +0x000
    int32_t map_index_;                          // +0x004
    int32_t map_type_;                           // +0x008
    std::list<_mapMonster> monsters_;            // +0x00c
    std::list<_mapMonster> condition_monsters_;  // +0x014
    std::list<_mapItem> items_;                  // +0x01c
    std::list<int> dungeon_lists_;               // +0x024
    int pathgate_positions_[4][2];               // +0x02c
    int pathgate_recognize_ranges_[4][2];        // +0x04c
    std::vector<STAICharacterMapArrangeData> ai_characters_;  // +0x06c
    int32_t apc_random_point_;                   // +0x078
    int32_t assigned_npc_count_;                 // +0x07c
    int32_t champion_;                           // +0x080
    std::vector<std::pair<int, int> > greed_cells_;           // +0x084
    std::vector<STRivalMapData> rival_positions_;             // +0x090
    std::vector<STRivalMapData> event_monster_positions_;     // +0x09c
    std::vector<int> passive_object_ids_;                       // +0x0a8
    int32_t heroes_mode_map_index_;             // +0x0b4
    std::vector<STAssignBloodSystem> blood_systems_;           // +0x0b8
    std::vector<STAssignBloodPhaseTime> blood_phase_times_;   // +0x0c4

    // C2/D2 perform no nested-object construction/destruction over this
    // interval.  They are seven physical dwords in the original CMap, kept
    // as scalar members rather than a byte-layout facade.
    int32_t runtime_value_d0_;                   // +0x0d0
    int32_t runtime_value_d4_;                   // +0x0d4
    int32_t runtime_value_d8_;                   // +0x0d8
    int32_t runtime_value_dc_;                   // +0x0dc
    int32_t runtime_value_e0_;                   // +0x0e0
    int32_t runtime_value_e4_;                   // +0x0e4
    int32_t runtime_value_e8_;                   // +0x0e8
    int32_t max_monster_drop_exception_;         // +0x0ec
    int32_t max_monster_count_exception_;        // +0x0f0
    advancealtar::TimeLine advance_altar_timeline_;            // +0x0f4
    int32_t script_value_178_;                   // +0x110
    std::map<char, std::vector<TournamentMonster> >
        tournament_monsters_;                    // +0x114
    uint8_t script_value_36e_;                   // +0x12c
    // +0x12d: [0]=combo training, [1]/[2]=one-/two-member tournament role.
    uint8_t reserved_12d_[3];
};

// IDA_EA: 0x8891C18
// IDA_NAME: _Z12makePathAreaP7MapAreaPA2_KiS3_
// DEMANGLED: makePathArea(MapArea *,int const(*)[2],int const(*)[2])
bool makePathArea(MapArea* areas,
                  const int (*positions)[2],
                  const int (*sizes)[2]);

// IDA_EA: 0x8891F20
// IDA_NAME: _Z12isInPathAreaiiPA2_KiS1_
// DEMANGLED: isInPathArea(int,int,int const(*)[2],int const(*)[2])
int isInPathArea(int pos_x,
                 int pos_y,
                 const int (*positions)[2],
                 const int (*sizes)[2]);

// IDA_EA: 0x889203B
// IDA_NAME: _Z8isInAreaiiP7MapArea
// DEMANGLED: isInArea(int,int,MapArea *)
bool isInArea(int pos_x, int pos_y, MapArea* area);

struct DimensionPartyCount;
class CUser;
namespace advancealtar {
class ProcStage;
}
struct CDungeonLayoutChecks;

// IDA unit: 99_misc_global/STMonsterTypeSpawnProb.
struct STMonsterTypeSpawnProb {
    STMonsterTypeSpawnProb();
    ~STMonsterTypeSpawnProb();
    STMonsterTypeSpawnProb* clear();

    // WarField::ConsistMap @0x86B8A38 consumes all four dwords as the
    // cumulative weights for the four war-room monster groups.  The parser
    // names are not recovered, but this is a physical four-int record, not
    // an opaque byte payload.
    int32_t group_weights_[4];
};

// IDA unit: 09_dungeon_pvp_party/STWarRoomData.
// Its nested vectors are material CDungeon subobjects, not scratch storage.
struct STWarRoomData {
    STWarRoomData();
    ~STWarRoomData();
    STWarRoomData* clear();
    STWarRoomData* copy(STWarRoomData* destination);

    int32_t header_values_[3];                       // +0x000 [point by type]
    int32_t mode_;                                   // +0x00c [phase fatigue]
    int32_t map_index_;                              // +0x010 [warroom map index]
    // [spawn common monster index], [spawn common champion index],
    // [spawn super champion index], [spawn boss index]
    std::vector<int> monster_groups_[4];             // +0x014
    std::vector<int> monster_group_order_;           // +0x044 [spawn step resource pool]
    // [spawn step max], [join cost gold], [gold drop prob]
    int32_t timing_values_[3];                       // +0x050
    // [common monster/common champion/super champion/boss item drop prob]
    int32_t spawn_values_[4];                        // +0x05c
    // [common monster/common champion/super champion/boss item drop list]
    std::vector<int> extra_monster_groups_[4];       // +0x06c
    // [common monster/common champion/super champion/boss exp const]
    int32_t extra_group_values_[4];                  // +0x09c
    // WarRoom::ClearReward @0x86BC5FE consumes this exact slot as an
    // IEEE-754 single-precision multiplier for FinishPoint-derived reward.
    float finish_point_reward_factor_;               // +0x0ac [kill count const]
    std::vector<STMonsterTypeSpawnProb> spawn_probs_; // +0x0b0 [monster type spawn prob]
    // WarField::ConsistMap loads these as IEEE-754 single-precision values
    // at +0x7b0/+0x7b4 of CDungeon.
    // [monster exp bonus per user decrease], [result exp bonus per user decrease]
    float post_spawn_values_[2];                     // +0x0bc
    int32_t condition_values_[4];                    // +0x0c4 [monster type spawn cost]
    int32_t condition_mode_;                         // +0x0d4 [player kc]
    int32_t condition_value_;                        // +0x0d8 [max monster]
    // The same routine multiplies the generated packet value by one of
    // these four IEEE-754 scales (+0x7d0..+0x7dc of CDungeon).
    float event_values_[4];                          // +0x0dc [monster type spawn interval rate]
    // [monster spawn base interval], [monster spawn random interval],
    // [fatigue], [fatigue result], [battle spawn time]
    int32_t tail_values_[5];                         // +0x0ec
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(STWarRoomData, post_spawn_values_) == 0x0bcu,
              "STWarRoomData post-spawn factors must stay at +0x0bc");
static_assert(offsetof(STWarRoomData, finish_point_reward_factor_) == 0x0acu,
              "STWarRoomData FinishPoint factor must stay at +0x0ac");
static_assert(offsetof(STWarRoomData, condition_values_) == 0x0c4u,
              "STWarRoomData group conditions must stay at +0x0c4");
static_assert(offsetof(STWarRoomData, condition_mode_) == 0x0d4u,
              "STWarRoomData mode value must stay at +0x0d4");
static_assert(offsetof(STWarRoomData, condition_value_) == 0x0d8u,
              "STWarRoomData condition value must stay at +0x0d8");
static_assert(offsetof(STWarRoomData, event_values_) == 0x0dcu,
              "STWarRoomData event factors must stay at +0x0dc");
static_assert(sizeof(STWarRoomData) == 0x100u,
              "STWarRoomData IDA ABI size must stay 0x100");
#endif

// IDA unit: 99_misc_global/RandomList.
// The stage selector at 0x814F4FC reads these two physical dwords as the
// map index and its cumulative-selection weight.
struct RandomList {
    int map_index;
    int weight;

    // IDA_EA: 0x88F8AE6
    // IDA_NAME: _ZN10RandomListD2Ev
    // DEMANGLED: RandomList::~RandomList(void)
    ~RandomList();
};

typedef std::map<int, std::vector<RandomList> > RandomTowerMapIndexTable;

// IDA unit: 12_pvf_datamanager_scripts/STDungeonScript.
// IDA_EA: C2 0x836B5E2, D2 0x836BD5C, Clear 0x836ABC6.
// The physical order mirrors the original C2 exactly.  Fundamental fields
// deliberately have no in-class initializers: Clear controls which bytes are
// reset and which values survive a subsequent script import.
struct STDungeonScript {
    STDungeonScript();
    ~STDungeonScript();
    STDungeonScript* Clear();

    int32_t value_000;                                      // +0x000
    int32_t value_004;                                      // +0x004
    int32_t value_008;                                      // +0x008
    uint8_t value_00c;                                     // +0x00c
    std::string value_010;                                 // +0x010
    int32_t heroes_required_quest;                          // +0x014
    int32_t dungeon_index;                                 // +0x018
    std::string name;                                      // +0x01c
    std::string value_020;                                 // +0x020
    std::string value_024;                                 // +0x024
    std::string value_028;                                 // +0x028
    int32_t value_02c;                                     // +0x02c
    std::string value_030;                                 // +0x030
    int32_t value_034;                                     // +0x034
    std::vector<std::string> value_038;                    // +0x038
    int32_t value_044;                                     // +0x044
    std::vector<std::string> value_048;                    // +0x048
    std::string value_054;                                 // +0x054
    std::string value_058;                                 // +0x058
    int32_t value_05c[6];                                  // +0x05c
    std::string value_074;                                 // +0x074
    int32_t min_required_level;                            // +0x078
    RequiredItem required_item;                            // +0x07c
    // [event dungeon].  This byte is copied to CDungeon+0x85c and gates
    // event-dungeon map routing, give-up penalties, and pickup broadcasts.
    uint8_t event_dungeon;                                 // +0x088
    RequiredItem event_required_item;                      // +0x08c
    std::vector<std::pair<int, int> > clear_reward_items;  // +0x098
    int32_t coin_limit;                                    // +0x0a4
    int32_t value_0a8;                                     // +0x0a8
    int32_t basis_level;                                   // +0x0ac
    float exp_weight;                                      // +0x0b0
    int32_t value_0b4[5];                                  // +0x0b4
    std::vector<std::pair<int, int> > dungeon_drop_items;  // +0x0c8
    std::vector<std::pair<int, int> > dungeon_object_drop_items; // +0x0d4
    std::vector<std::pair<int, int> > value_0e0;           // +0x0e0
    std::vector<stDungeonAssignItem_t> special_passive_object_items; // +0x0ec
    int32_t value_0f8[12];                                 // +0x0f8
    int32_t value_128[5];                                  // +0x128
    int32_t value_13c;                                     // +0x13c
    uint8_t value_140;                                     // +0x140
    uint8_t value_141;                                     // +0x141
    uint8_t value_142;                                     // +0x142
    std::vector<std::pair<int, int> > value_144;           // +0x144
    uint8_t value_150;                                     // +0x150
    int32_t value_154;                                     // +0x154
    uint8_t value_158;                                     // +0x158
    uint8_t value_159;                                     // +0x159
    std::vector<int> value_15c;                            // +0x15c
    int32_t value_168;                                     // +0x168
    int32_t value_16c;                                     // +0x16c
    std::vector<MazeScript> maze_scripts;                  // +0x170
    std::string directory;                                 // +0x17c
    STDeadTowerData dead_tower_data;                        // +0x180
    STWarRoomData war_room_data;                            // +0x18c
    uint8_t ancient_dungeon;                               // +0x28c
    uint8_t value_28d;                                     // +0x28d
    uint8_t value_28e;                                     // +0x28e
    uint8_t value_28f;                                     // +0x28f
    std::map<AbsoluteChangeStat::T, float> absolute_change_stats; // +0x290
    uint8_t value_2a8;                                     // +0x2a8
    // [tower of despair].  CDungeonList::load_list selects the
    // CDungeon_TowerOfDespair vtable when this marker is set.
    uint8_t tower_of_despair;                               // +0x2a9
    uint8_t value_2aa;                                     // +0x2aa
    uint8_t defense_dungeon;                               // +0x2ab
    int32_t value_2ac;                                     // +0x2ac
    int32_t value_2b0[3];                                  // +0x2b0
    int32_t value_2bc[3];                                  // +0x2bc
    std::vector<int> value_2c8;                            // +0x2c8
    advancealtar::ClearRewardData clear_reward_data;       // +0x2d4
    advancealtar::SurvivalRewardData survival_reward_data; // +0x2f8
    uint8_t dimension_possible;                            // +0x304
    int8_t dimension_inout_count;                          // +0x305
    uint8_t dimension_min_party_members;                   // +0x306
    std::vector<DimensionPartyCount> dimension_party_counts; // +0x308
    int8_t revenge_dungeon_min_party_members;              // +0x314
    int8_t revenge_dungeon_max_party_members;              // +0x315
    uint8_t village_attack_revenge_dungeon;                // +0x316
    uint8_t revenge_dungeon_entry_limit;                   // +0x317
    std::vector<_DailySchedule> daily_schedules;           // +0x318
    uint8_t limit_party_count;                             // +0x324
    uint8_t limit_inout_count;                             // +0x325
    stConditionEnterBossRoom_t enter_boss_room_condition;  // +0x328
    int32_t value_338;                                     // +0x338
    int32_t random_tower_max_stage;                        // +0x33c
    RandomTowerMapIndexTable random_tower_map_indexes;     // +0x340
    int32_t dungeon_kind;                                  // +0x358
    std::vector<std::pair<int, int> > value_35c;           // +0x35c
    uint8_t value_368;                                     // +0x368
    int32_t value_36c;                                     // +0x36c
    int32_t value_370;                                     // +0x370
    float value_374;                                       // +0x374
    float value_378;                                       // +0x378
    int32_t value_37c;                                     // +0x37c
    int32_t limit_stackable_item_in_tower;                  // +0x380
    std::vector<RecoveryStairInfo> recovery_stairs;        // +0x384
    STTowerFullDialog tower_dialogs;                        // +0x390
    int32_t tower_item_drop;                               // +0x3b4
    int32_t tower_max_clear_item_num;                       // +0x3b8
    int32_t value_3bc;                                     // +0x3bc
    int32_t value_3c0;                                     // +0x3c0
    int32_t value_3c4;                                     // +0x3c4
    std::vector<STNamedMonsterMapPos> named_monster_positions; // +0x3c8
    int32_t try_gen_event_monster_count;                    // +0x3d4
    int32_t try_gen_event_monster_probability;              // +0x3d8
    int32_t event_monster_total_probability;                // +0x3dc
    std::vector<stEventMonster_t> event_monsters;           // +0x3e0
    int32_t try_gen_second_event_monster_count;             // +0x3ec
    int32_t try_gen_second_event_monster_probability;       // +0x3f0
    int32_t second_event_monster_total_probability;         // +0x3f4
    std::vector<stSecondEventMonster_t> second_event_monsters; // +0x3f8
    std::vector<std::vector<float> > monster_difficulty_bonus; // +0x404
    int32_t value_410[5];                                  // +0x410
    int32_t value_424[5];                                  // +0x424
    float value_438;                                       // +0x438
    float value_43c;                                       // +0x43c
    float value_440;                                       // +0x440
    float value_444;                                       // +0x444
    std::string value_448;                                 // +0x448
    uint8_t value_44c;                                     // +0x44c
    int32_t value_450;                                     // +0x450
    int32_t value_454;                                     // +0x454
    std::vector<int> value_458;                            // +0x458
    std::vector<int> value_464;                            // +0x464
    std::vector<int> value_470;                            // +0x470
    uint8_t value_47c;                                     // +0x47c
    int32_t value_480;                                     // +0x480
    int32_t value_484;                                     // +0x484
    int32_t value_488;                                     // +0x488
    uint8_t value_48c;                                     // +0x48c
    int32_t value_490;                                     // +0x490
    uint8_t value_494;                                     // +0x494
    int32_t value_498;                                     // +0x498
    std::map<int, TournamentRewardRate> tournament_rewards; // +0x49c
    TournamentRewardItemRate tournament_reward_items;      // +0x4b4
    float tournament_reward_weight;                        // +0x4c4
    std::map<uint8_t, uint32_t> tower_reward_rates;         // +0x4c8
    uint8_t value_4e0;                                     // +0x4e0
    std::map<int, UseCoinDungeonDiff> use_coin_difficulty; // +0x4e4
    int32_t value_4fc;                                     // +0x4fc
    int32_t value_500;                                     // +0x500
    uint8_t value_504;                                     // +0x504
    uint8_t value_505;                                     // +0x505
    int32_t value_508;                                     // +0x508
    uint8_t value_50c;                                     // +0x50c
    uint8_t reserved_50d[3];                               // +0x50d
    // CLIENT_EA: ImportDungeonScript_ReadSAODungeonFlag @ 0x15CDA9B.
    // [SAO dungeon] is reduced to a nonzero marker by the client importer.
    uint8_t sao_dungeon;                                   // +0x510
    // CLIENT_EA: ImportDungeonScript_EnableTowerScenarioFlag @ 0x15CFABD.
    // [time spiral] and [tower of despair sao] are valueless tags that share
    // this client-side marker.  Keep the marker separate from sao_dungeon:
    // the client has distinct importer branches and no subtype is exposed.
    uint8_t tower_scenario_enabled;                        // +0x511
    // CLIENT_EA: the 0828 dungeon importer persists [zombie dungeon] in the
    // first byte of the previously unclaimed +0x512 tail pair.
    uint8_t zombie_dungeon;                                // +0x512
    uint8_t reserved_513;                                  // +0x513
    // CLIENT_EA: ImportDungeonScript_ReadEventMonsterRandomMapValues
    // @ 0x15CF33F.  The client reads two numeric tokens in sequence and keeps
    // the second result as zero when the target PVF supplies only one value.
    // These are intentionally raw values; no map-id/weight meaning is proven.
    int32_t event_monster_random_map_value_0;               // +0x514
    int32_t event_monster_random_map_value_1;               // +0x518
    // CLIENT_EA: ImportDungeonScript @0x15C8560 stores these three official
    // boolean policies.  They were previously validated and discarded.
    uint8_t multi_start_point;                              // +0x51c
    uint8_t usable_stackable;                               // +0x51d
    uint8_t use_spectator_mode;                             // +0x51e
    uint8_t reserved_51f;                                  // +0x51f
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(STDungeonScript, dungeon_index) == 0x018,
              "STDungeonScript dungeon index offset mismatch");
static_assert(offsetof(STDungeonScript, name) == 0x01c,
              "STDungeonScript name offset mismatch");
static_assert(offsetof(STDungeonScript, required_item) == 0x07c,
              "STDungeonScript required item offset mismatch");
static_assert(offsetof(STDungeonScript, maze_scripts) == 0x170,
              "STDungeonScript maze vector offset mismatch");
static_assert(offsetof(STDungeonScript, war_room_data) == 0x18c,
              "STDungeonScript war-room offset mismatch");
static_assert(offsetof(STDungeonScript, ancient_dungeon) == 0x28c,
              "STDungeonScript ancient-dungeon offset mismatch");
static_assert(offsetof(STDungeonScript, defense_dungeon) == 0x2ab,
              "STDungeonScript defense-dungeon offset mismatch");
static_assert(offsetof(STDungeonScript,
                       revenge_dungeon_min_party_members) == 0x314,
              "STDungeonScript revenge minimum-party offset mismatch");
static_assert(offsetof(STDungeonScript,
                       revenge_dungeon_max_party_members) == 0x315,
              "STDungeonScript revenge maximum-party offset mismatch");
static_assert(offsetof(STDungeonScript, village_attack_revenge_dungeon) ==
                  0x316,
              "STDungeonScript revenge-dungeon offset mismatch");
static_assert(offsetof(STDungeonScript, revenge_dungeon_entry_limit) == 0x317,
              "STDungeonScript revenge entry-limit offset mismatch");
static_assert(offsetof(STDungeonScript, absolute_change_stats) == 0x290,
              "STDungeonScript absolute-stat map offset mismatch");
static_assert(offsetof(STDungeonScript, dimension_party_counts) == 0x308,
              "STDungeonScript party-count vector offset mismatch");
static_assert(offsetof(STDungeonScript, random_tower_map_indexes) == 0x340,
              "STDungeonScript random-tower map offset mismatch");
static_assert(offsetof(STDungeonScript, recovery_stairs) == 0x384,
              "STDungeonScript recovery vector offset mismatch");
static_assert(offsetof(STDungeonScript, tower_dialogs) == 0x390,
              "STDungeonScript tower dialog offset mismatch");
static_assert(offsetof(STDungeonScript, named_monster_positions) == 0x3c8,
              "STDungeonScript named monster vector offset mismatch");
static_assert(offsetof(STDungeonScript, monster_difficulty_bonus) == 0x404,
              "STDungeonScript monster bonus vector offset mismatch");
static_assert(offsetof(STDungeonScript, value_410) == 0x410,
              "STDungeonScript evil-level table offset mismatch");
static_assert(offsetof(STDungeonScript, value_424) == 0x424,
              "STDungeonScript evil-high-level table offset mismatch");
static_assert(offsetof(STDungeonScript, value_438) == 0x438,
              "STDungeonScript first evil-rate offset mismatch");
static_assert(offsetof(STDungeonScript, value_43c) == 0x43c,
              "STDungeonScript third evil-rate offset mismatch");
static_assert(offsetof(STDungeonScript, value_440) == 0x440,
              "STDungeonScript second evil-rate offset mismatch");
static_assert(offsetof(STDungeonScript, value_444) == 0x444,
              "STDungeonScript fourth evil-rate offset mismatch");
static_assert(offsetof(STDungeonScript, tournament_rewards) == 0x49c,
              "STDungeonScript tournament reward map offset mismatch");
static_assert(offsetof(STDungeonScript, use_coin_difficulty) == 0x4e4,
              "STDungeonScript use-coin map offset mismatch");
static_assert(offsetof(STDungeonScript, sao_dungeon) == 0x510,
              "STDungeonScript SAO-dungeon offset mismatch");
static_assert(offsetof(STDungeonScript, tower_scenario_enabled) == 0x511,
              "STDungeonScript tower-scenario offset mismatch");
static_assert(offsetof(STDungeonScript, zombie_dungeon) == 0x512,
              "STDungeonScript zombie-dungeon offset mismatch");
static_assert(offsetof(STDungeonScript, event_monster_random_map_value_0) ==
                  0x514,
              "STDungeonScript event-random-map first value offset mismatch");
static_assert(offsetof(STDungeonScript, event_monster_random_map_value_1) ==
                  0x518,
              "STDungeonScript event-random-map second value offset mismatch");
static_assert(offsetof(STDungeonScript, multi_start_point) == 0x51c,
              "STDungeonScript multi-start policy offset mismatch");
static_assert(offsetof(STDungeonScript, usable_stackable) == 0x51d,
              "STDungeonScript stackable policy offset mismatch");
static_assert(offsetof(STDungeonScript, use_spectator_mode) == 0x51e,
              "STDungeonScript spectator policy offset mismatch");
static_assert(sizeof(STDungeonScript) == 0x520,
              "STDungeonScript extended size mismatch");
#endif

// The original enum's named type is part of the legacy mangled ABI.  Its
// enumerator table is not recovered; only the two physical quest slots are
// consumed by CDungeon::GetQuestMazeScriptVector.
enum ENUM_MAZE_QUEST_TYPE : int {};

namespace QuickParty {
class CQuickPartySystemManager;
}

// IDA unit: 09_dungeon_pvp_party/CDungeon.
// IDA_EA: C2 0x8373F3C, D2 0x8376FD8, set_dungeon 0x834ACC4.
class CDungeon {
public:
    CDungeon();
    ~CDungeon();

    // The original vtable at 0x8C34A08 has this sole slot.  CDungeon does
    // not have a virtual destructor.
    virtual int _do_after_dungeon_start(CUser* user) const;

    CDungeon* set_dungeon(STDungeonScript& script);
    CDungeon* InsertMap(int area, int direction, CMap* map);
    int insertStart(int area, CMap* map);
    int get_index() const;
    int get_min_level() const;
    int get_standard_level() const;
    // IDA_EA: 0x81455BC / 0x834D2D4
    // IDA_NAME: _ZNK8CDungeon22getRandomTowerMaxStageEv /
    //           _ZNK8CDungeon21IsOpenScheduleDungeonEv
    int getRandomTowerMaxStage() const;
    int IsOpenScheduleDungeon() const;
    // IDA_EA: 0x81455CA
    // IDA_NAME: _ZNK8CDungeon29getTowerMaxClearRewardItemNumEv
    int getTowerMaxClearRewardItemNum() const;
    // IDA_EA: 0x846993A / 0x846995C
    // IDA_NAME: _ZNK8CDungeon21getDeathTowerMapIndexEj /
    //           _ZNK8CDungeon21getDeathTowerMaxStageEv
    int getDeathTowerMapIndex(unsigned int stage) const;
    int getDeathTowerMaxStage() const;
    // IDA_EA: 0x8150908 / 0x8150914
    const std::vector<std::pair<int, int> >& getObjectDropItems() const;
    const std::vector<stDungeonAssignItem_t>&
    getSpecialPassiveObjectItems() const;
    // IDA_EA: 0x834C95C
    // IDA_NAME: _ZNK8CDungeon14get_clear_itemEi
    // DEMANGLED: CDungeon::get_clear_item(int)const
    // The no-argument original performs its own 0..99 roll before the same
    // cumulative-rate lookup.  Keep both ABIs; callers use both symbols.
    // IDA_EA: 0x834C8C0
    // IDA_NAME: _ZNK8CDungeon14get_clear_itemEv
    int get_clear_item() const;
    int get_clear_item(int random_number) const;
    // IDA_EA: 0x8484854
    // IDA_NAME: _ZNK8CDungeon22getHeroesRequiredQuestEv
    // DEMANGLED: CDungeon::getHeroesRequiredQuest(void)const
    int getHeroesRequiredQuest() const;
    const char* GetDungeonName() const;
    long double get_exp_weight() const;
    int isNoFatigueDungeon() const;
    // CLIENT_EA: CDungeon_IsSAODungeon @ 0x9CA850.
    bool isSAODungeon() const;
    bool isZombieDungeon() const { return zombie_dungeon_ != 0; }
    bool hasMultipleStartPoints() const { return multi_start_point_ != 0; }
    bool allowsStackableItems() const { return usable_stackable_ != 0; }
    bool allowsSpectatorMode() const { return use_spectator_mode_ != 0; }
    int get_blood_dungeon_type() const;
    // IDA_EA: 0x822B4EE
    // IDA_NAME: _ZNK8CDungeon26GetType_DungeonInHeritanceEv
    int GetType_DungeonInHeritance() const;
    // IDA_EA: 0x826B950
    // IDA_NAME: _ZNK8CDungeon18getQuestNpcDungeonEv
    // DEMANGLED: CDungeon::getQuestNpcDungeon(void)const
    // The quick-party pool initializer uses this exact script flag to
    // suppress difficulty-specific pool partitions for quest-NPC dungeons.
    int getQuestNpcDungeon() const;
    int get_dimension_possible() const;
    int get_dimension_min_partymem() const;
    int get_dimension_member_count(DimensionPartyCount& count) const;
    int get_limit_party_count() const;
    // IDA_EA: 0x822B4C0 / 0x822B4CE
    // IDA_NAME: _ZNK8CDungeon14getDungeonKindEv
    //           _ZNK8CDungeon27limitOfStackableItemInTowerEv
    int getDungeonKind() const;
    int limitOfStackableItemInTower() const;
    int get_limit_Inout_count() const;
    // IDA_EA: 0x86B9672.  The WarField/normal monster-drop paths pass the
    // canonical dungeon-owned item list directly to the dungeon-drop
    // generator; keep that ownership boundary typed.
    const std::vector<std::pair<int, int> >& getDropItems() const;
    // Imported from STDungeonScript::event_dungeon at CDungeon+0x85c.
    // The flag is consumed by the original map-routing, party leave and
    // CMD46 item-pickup paths.
    bool isEventDungeon() const { return event_dungeon_ != 0; }
    // 0828 [time spiral] and [required item] are physical dungeon fields.
    // These accessors let the TimeSpiral importer validate the complete PVF
    // contract before the runtime packet closure is published.
    bool isTimeSpiralScript() const { return tower_scenario_enabled_ != 0; }
    const RequiredItem& getRequiredItem() const { return required_item_; }
    // Physical STDungeonScript fields copied by set_dungeon.  These inline
    // predicates name the PVF tags consumed by the original callers; the
    // bytes remain part of CDungeon itself and are not offset-backed views.
    bool isAncientDungeon() const { return ancient_dungeon_ != 0; }
    bool isRevengeDungeon() const {
        return village_attack_revenge_dungeon_ != 0;
    }
    bool isDefenseDungeon() const { return defense_dungeon_ != 0; }
    int getRevengeDungeonMinPartyMembers() const {
        return revenge_dungeon_min_party_members_;
    }
    int getRevengeDungeonMaxPartyMembers() const {
        return revenge_dungeon_max_party_members_;
    }
    int getRevengeDungeonEntryLimit() const {
        return revenge_dungeon_entry_limit_;
    }
    bool isTowerOfDespairDungeon() const;
    int isTournamentDungeon() const;
    int getTournamentRoundFatigue() const;
    int isRiskDungeon() const;
    int GetPartyMemberCoinLimit() const;
    int IsEnterEachMap() const;
    // IDA_EA: 0x8365288
    // IDA_NAME: _ZNK8CDungeon15IsOpenEvenEnemyEv
    int IsOpenEvenEnemy() const;
    // IDA_EA: 0x830E5F6
    // IDA_NAME: _ZNK8CDungeon24getAICharacterAppearRateEv
    // DEMANGLED: CDungeon::getAICharacterAppearRate(void)const
    int getAICharacterAppearRate() const;
    // IDA_EA: 0x830E610 / 0x830E620 / 0x834BF56 / 0x834C75A.
    int IsHaveDummyMap() const;
    int GetDummyAppearCount() const;
    int ChooseRandomMap(int map_type,
                        int direction,
                        int entrance) const;
    int GetStartMap(int direction) const;

    // IDA_EA: 0x830E69E / 0x834D06C.
    const std::vector<MazeScript>*
    GetQuestMazeScriptVector(ENUM_MAZE_QUEST_TYPE quest_type) const;
    // IDA_EA: 0x834BEB8 / 0x834BF0E.  GetRandMaze deliberately retains the
    // original empty-vector precondition: its old body divides by size().
    MazeScript* GetRandMaze(int& index) const;
    MazeScript* GetMaze(int index) const;
    // IDA_EA: 0x834C9F0 / 0x834CB7C.  These validate a parsed maze against
    // the three dungeon-owned map tables and preserve old startup diagnostics.
    int VerifyMaze() const;
    int VerifyMaze(const MazeScript& maze) const;
    MazeScript* GetRandMazebyPartyCount(int member_count) const;

    // IDA_EA: 0x830E62E..0x830E69D.
    int getTryGenEventMonsterCnt() const;
    int getTryGenEventMonsterProb() const;
    int getEventMonsterTotalProb() const;
    const std::vector<stEventMonster_t>* getEventMonsters() const;
    int getTryGenSecondEventMonsterCnt() const;
    int getTryGenSecondEventMonsterProb() const;
    int getSecondEventMonsterTotalProb() const;
    const std::vector<stSecondEventMonster_t>*
    getSecondEventMonsters() const;
    int checkFreeRevivalCondition(int dungeon_diff) const;
    // IDA_EA: 0x8365316
    // IDA_NAME: _ZNK8CDungeon16getLimitCoinDiffEiRi
    int getLimitCoinDiff(int dungeon_diff, int& coin_limit) const;

    // IDA_EA: 0x8150920
    // IDA_NAME: _ZNK8CDungeon24getRandomTowerMapIndexesEv
    // DEMANGLED: CDungeon::getRandomTowerMapIndexes(void)const
    const RandomTowerMapIndexTable& getRandomTowerMapIndexes() const;
    // IDA_EA: 0x815092E
    // IDA_NAME: _ZNK8CDungeon16hasTowerItemDropEv
    // DEMANGLED: CDungeon::hasTowerItemDrop(void)const
    int hasTowerItemDrop() const;
private:
    friend struct CDungeonLayoutChecks;
    // CDataManager::VerifyDungeon @0x8362B44 reads this exact clear-item
    // table before calling VerifyMaze; retain owner-side typed access.
    friend class CDataManager;
    friend class WarField;
    friend class WarRoom;
    // CParty's original reward paths read this physical CDungeon state while
    // selecting circulation-statistic buckets.  Keeping the friendship
    // preserves that direct relationship without an offset helper or a new
    // exported CDungeon facade.
    friend class CParty;
    // Village::set_village @0x86C33B8 appends its town index directly to
    // the dungeon's +0x6d4 vector when the town requires that dungeon to be
    // cleared.  Preserve the original owner-to-owner relationship without a
    // fabricated public mutator or a raw layout projection.
    friend class Village;
    // CUser::exceptedStateMoneyLog reads the same original circulation class
    // byte after following its owning party's active dungeon pointer.
    friend class CUser;
    // CBossTower::onChargeRequireItem @0x814530C reads this original script
    // record directly at +0x7f4/+0x7f8/+0x7fc while expelling unready users.
    friend class WongWork::CBossTower;
    // TowerOfDespairMgr::CheckEnterable consumes the same required-item
    // record used by the original TOD admission path.  This friendship keeps
    // that owner storage internal instead of introducing a fabricated getter.
    friend class TowerOfDespairMgr;
    // The derived entry handler is the only other original owner that reads
    // the admission item record at +0x7f4.
    friend class CDungeon_TowerOfDespair;
    // IDA 0x826AB9E reads the quick-party reward-table row selector directly
    // from +0x89c while deciding the minimum matching population.
    friend class QuickParty::CQuickPartySystemManager;
    // IDA 0x8300C76..0x8304804 is the CBattle_Field map-assembly cluster.
    // It consumes these named CDungeon script members directly.
    friend class CBattle_Field;
    // ProcStage's original accessors read the stage dungeon and its advance
    // altar type directly.  Preserve that proved ownership instead of
    // introducing a second script-value accessor ABI.
    friend class advancealtar::ProcStage;

    typedef __gnu_cxx::hash_map<int, CMap*> StartMapHash;
    typedef std::multimap<int, CMap*> DirectionMapList;

    int32_t dungeon_inheritance_type_;          // +0x004
    int32_t dungeon_index_;                     // +0x008
    int32_t min_level_;                         // +0x00c
    int32_t standard_level_;                    // +0x010
    float exp_weight_;                          // +0x014
    std::string dungeon_name_;                  // +0x018
    std::map<int, int> clear_item_rates_;       // +0x01c
    int32_t heroes_required_quest_;             // +0x034
    uint8_t dimension_possible_;                // +0x038
    int8_t dimension_inout_count_;              // +0x039
    uint8_t dimension_min_party_members_;       // +0x03a
    uint8_t limit_party_count_;                 // +0x03b
    uint8_t limit_inout_count_;                 // +0x03c

    std::vector<std::pair<int, int> > drop_items_;          // +0x040
    std::vector<std::pair<int, int> > object_drop_items_;   // +0x04c
    std::vector<stDungeonAssignItem_t>
        special_passive_object_items_;                       // +0x058
    StartMapHash start_maps_[16];                             // +0x064
    DirectionMapList normal_maps_[16];                       // +0x1a4
    DirectionMapList boss_maps_[16];                         // +0x324
    DirectionMapList dummy_maps_[16];                        // +0x4a4

    uint8_t has_dummy_maps_;                                 // +0x624
    int32_t dummy_appear_count_;                             // +0x628
    std::vector<MazeScript> maze_scripts_;                   // +0x62c
    std::vector<MazeScript> quest_maze_scripts_[2];          // +0x638
    int32_t ai_character_appear_rate_;                       // +0x650
    int32_t random_tower_max_stage_;                         // +0x654
    RandomTowerMapIndexTable random_tower_map_indexes_;      // +0x658
    int32_t dungeon_kind_;                                   // +0x670
    std::vector<int> map_index_weights_;                     // +0x674
    int32_t map_index_weight_count_;                         // +0x680
    int32_t try_gen_event_monster_count_;                    // +0x684
    int32_t try_gen_event_monster_probability_;              // +0x688
    int32_t event_monster_total_probability_;                // +0x68c
    std::vector<stEventMonster_t> event_monsters_;           // +0x690
    int32_t try_gen_second_event_monster_count_;             // +0x69c
    int32_t try_gen_second_event_monster_probability_;       // +0x6a0
    int32_t second_event_monster_total_probability_;         // +0x6a4
    std::vector<stSecondEventMonster_t> second_event_monsters_; // +0x6a8
    uint8_t limit_stackable_item_in_tower_;                  // +0x6b4
    uint8_t tower_item_drop_;                                // +0x6b5
    int32_t runtime_value_6b8_;                              // +0x6b8
    std::vector<DimensionPartyCount> dimension_party_counts_; // +0x6bc
    uint8_t no_fatigue_dungeon_;                             // +0x6c8
    uint8_t dimension_party_count_enabled_;                  // +0x6c9
    int32_t runtime_value_6cc_;                              // +0x6cc
    uint8_t enter_each_map_;                                 // +0x6d0
    uint8_t open_even_enemy_;                                // +0x6d1
    std::vector<int> village_indices_;                       // +0x6d4
    int32_t champion_spawn_rates_[5];                        // +0x6e0
    STWarRoomData war_room_data_;                            // +0x6f4
    RequiredItem required_item_;                             // +0x7f4
    uint8_t defense_dungeon_;                                // +0x800
    // Imported from STDungeonScript::value_2ac by the [advance altar type]
    // parser.  ProcStage::getStageType reads this exact word.
    int32_t advance_altar_stage_type_;                        // +0x804
    int32_t script_values_808_[3];                           // +0x808
    std::vector<int> dungeon_runtime_values_;                // +0x814
    int32_t script_values_820_[3];                           // +0x820
    advancealtar::ClearRewardData clear_reward_data_;        // +0x82c
    advancealtar::SurvivalRewardData survival_reward_data_;  // +0x850
    uint8_t event_dungeon_;                                  // +0x85c
    RequiredItem additional_required_item_;                  // +0x860
    std::vector<std::pair<int, int> > recovery_stair_items_; // +0x86c
    int8_t revenge_dungeon_min_party_members_;               // +0x878
    int8_t revenge_dungeon_max_party_members_;               // +0x879
    uint8_t village_attack_revenge_dungeon_;                 // +0x87a
    uint8_t revenge_dungeon_entry_limit_;                    // +0x87b
    int32_t coin_limit_;                                    // +0x87c
    int32_t recovery_stair_values_[5];                       // +0x880
    int32_t script_value_894_;                               // +0x894
    int32_t script_value_898_;                               // +0x898
    uint8_t ancient_dungeon_;                                // +0x89c
    uint8_t hell_dungeon_;                                   // +0x89d
    // Read by the CParty reward and in/out flows (for example 0x85AAE02).
    // A nonzero value selects the alternate circulation-statistic bucket.
    uint8_t circulation_statistic_class_;                    // +0x89e
    uint8_t blood_dungeon_type_;                             // +0x89f
    int32_t blood_max_round_;                                // +0x8a0
    int32_t script_value_8a4_;                               // +0x8a4
    int8_t tournament_dungeon_state_;                        // +0x8a8
    std::map<int, TournamentRewardRate> tournament_rewards_; // +0x8ac
    TournamentRewardItemRate tournament_reward_items_;       // +0x8c4
    float tournament_reward_weight_;                         // +0x8d4
    std::map<uint8_t, uint32_t> tower_reward_rates_;         // +0x8d8
    std::vector<STNamedMonsterMapPos> named_monster_positions_; // +0x8f0
    uint8_t quest_npc_dungeon_;                              // +0x8fc
    std::vector<std::vector<float> > monster_difficulty_bonus_; // +0x900
    stConditionEnterBossRoom_t enter_boss_room_condition_;   // +0x90c
    std::map<int, UseCoinDungeonDiff> use_coin_difficulty_;  // +0x91c
    uint8_t sao_dungeon_;                                   // +0x934
    uint8_t tower_scenario_enabled_;                        // +0x935
    uint8_t zombie_dungeon_;                                // +0x936
    uint8_t reserved_937_;                                  // +0x937
    int32_t event_monster_random_map_value_0_;               // +0x938
    int32_t event_monster_random_map_value_1_;               // +0x93c
    uint8_t multi_start_point_;                              // +0x940
    uint8_t usable_stackable_;                               // +0x941
    uint8_t use_spectator_mode_;                             // +0x942
    uint8_t reserved_943_;                                  // +0x943
};

// The class is polymorphic, so these checks are intentionally kept in its
// friend instead of making layout fields public.  Each offset is observed in
// CDungeon C2/D2 (0x8373F3C/0x8376FD8) or set_dungeon (0x834ACC4).
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CDungeonLayoutChecks {
    static_assert(__builtin_offsetof(CDungeon, dungeon_inheritance_type_) == 0x004,
                  "CDungeon inheritance offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, dungeon_name_) == 0x018,
                  "CDungeon name offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, clear_item_rates_) == 0x01c,
                  "CDungeon clear-rate map offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, drop_items_) == 0x040,
                  "CDungeon drop-items offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, special_passive_object_items_) == 0x058,
                  "CDungeon passive-items offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, start_maps_) == 0x064,
                  "CDungeon start-map table offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, normal_maps_) == 0x1a4,
                  "CDungeon normal-map table offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, boss_maps_) == 0x324,
                  "CDungeon boss-map table offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, dummy_maps_) == 0x4a4,
                  "CDungeon dummy-map table offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, maze_scripts_) == 0x62c,
                  "CDungeon maze offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, random_tower_map_indexes_) == 0x658,
                  "CDungeon random-tower map offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, dungeon_kind_) == 0x670,
                  "CDungeon kind offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, event_monsters_) == 0x690,
                  "CDungeon event-monster offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, second_event_monsters_) == 0x6a8,
                  "CDungeon second-event-monster offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, dimension_party_counts_) == 0x6bc,
                  "CDungeon dimension-count offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, village_indices_) == 0x6d4,
                  "CDungeon village-index vector offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, war_room_data_) == 0x6f4,
                  "CDungeon war-room offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, required_item_) == 0x7f4,
                  "CDungeon required-item offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, defense_dungeon_) == 0x800,
                  "CDungeon defense-dungeon flag offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, dungeon_runtime_values_) == 0x814,
                  "CDungeon runtime-vector offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, clear_reward_data_) == 0x82c,
                  "CDungeon clear-reward offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, survival_reward_data_) == 0x850,
                  "CDungeon survival-reward offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, event_dungeon_) == 0x85c,
                  "CDungeon event-dungeon flag offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, additional_required_item_) == 0x860,
                  "CDungeon additional-required-item offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, recovery_stair_items_) == 0x86c,
                  "CDungeon recovery-stair offset mismatch");
    static_assert(__builtin_offsetof(
                      CDungeon, revenge_dungeon_min_party_members_) == 0x878,
                  "CDungeon revenge minimum-party offset mismatch");
    static_assert(__builtin_offsetof(
                      CDungeon, revenge_dungeon_max_party_members_) == 0x879,
                  "CDungeon revenge maximum-party offset mismatch");
    static_assert(__builtin_offsetof(CDungeon,
                                     village_attack_revenge_dungeon_) ==
                      0x87a,
                  "CDungeon revenge-dungeon offset mismatch");
    static_assert(__builtin_offsetof(CDungeon,
                                     revenge_dungeon_entry_limit_) == 0x87b,
                  "CDungeon revenge entry-limit offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, ancient_dungeon_) == 0x89c,
                  "CDungeon ancient-dungeon offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, hell_dungeon_) == 0x89d,
                  "CDungeon hell-dungeon flag offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, circulation_statistic_class_) == 0x89e,
                  "CDungeon circulation-statistic class offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, coin_limit_) == 0x87c,
                  "CDungeon coin-limit offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, tournament_rewards_) == 0x8ac,
                  "CDungeon tournament-reward map offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, tournament_reward_items_) == 0x8c4,
                  "CDungeon tournament-item-rate offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, tower_reward_rates_) == 0x8d8,
                  "CDungeon tower-reward map offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, named_monster_positions_) == 0x8f0,
                  "CDungeon named-monster offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, monster_difficulty_bonus_) == 0x900,
                  "CDungeon difficulty-bonus offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, enter_boss_room_condition_) == 0x90c,
                  "CDungeon boss-room condition offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, blood_max_round_) == 0x8a0,
                  "CDungeon blood max-round offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, use_coin_difficulty_) == 0x91c,
                  "CDungeon use-coin map offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, sao_dungeon_) == 0x934,
                  "CDungeon SAO-dungeon offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, tower_scenario_enabled_) ==
                      0x935,
                  "CDungeon tower-scenario offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, zombie_dungeon_) == 0x936,
                  "CDungeon zombie-dungeon offset mismatch");
    static_assert(__builtin_offsetof(CDungeon,
                                     event_monster_random_map_value_0_) ==
                      0x938,
                  "CDungeon event-random-map first value offset mismatch");
    static_assert(__builtin_offsetof(CDungeon,
                                     event_monster_random_map_value_1_) ==
                      0x93c,
                  "CDungeon event-random-map second value offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, multi_start_point_) == 0x940,
                  "CDungeon multi-start policy offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, usable_stackable_) == 0x941,
                  "CDungeon stackable policy offset mismatch");
    static_assert(__builtin_offsetof(CDungeon, use_spectator_mode_) == 0x942,
                  "CDungeon spectator policy offset mismatch");
};
#endif

// IDA unit: 05_world_enter_village/CDungeonList.
// IDA_EA: 0x834D320, 0x834D376.
class CDungeonList {
public:
    // The original storage itself is this hash_map.  Expose its const
    // iterator only to owner-side diagnostic consumers that must retain the
    // binary's native traversal order without rebuilding a parallel index.
    typedef __gnu_cxx::hash_map<int, CDungeon*> DungeonHash;
    typedef DungeonHash::const_iterator const_iterator;

    CDungeonList();
    ~CDungeonList();
    int load_list();
    int destroy();
    int insert_dungeon(CDungeon* dungeon);
    CDungeon* find_dungeon(int dungeon_index) const;
    CDungeon* get_dungeon_at_index(int ordinal) const;
    CDungeonList* getDungeonIndexList();
    int size();

    const_iterator begin() const { return dungeons_.begin(); }
    const_iterator end() const { return dungeons_.end(); }

private:
    DungeonHash dungeons_;
};

// IDA unit: 05_world_enter_village/CMapList.
// IDA_EA: 0x834EDA0, 0x834F18E, 0x834F4BA.
class CMapList {
public:
    CMapList();
    ~CMapList();
    bool destroy();
    int insert_map(int area,
                   int direction,
                   CMap* map,
                   std::vector<PVPMAP_DATA>& pvp_map_data);
    CMap* find_map(int map_index);
    static void LoadPvPSinglePlayableMap(char* filename);
    CMapList* GetMapList();
    int GetPVPMapCount() const;

    // Declaration-only direct C consumer edge.
    // IDA_EA: 0x834F5C8
    // IDA_NAME: _ZN8CMapList13GetPvpMapInfoEiR10PvPMapInfo
    // DEMANGLED: CMapList::GetPvpMapInfo(int,PvPMapInfo &)
    static int GetPvpMapInfo(int map_no, PvPMapInfo& info);
private:
    // CDataManager::Verify @0x8360D94 preserves the direct old map-index
    // traversal and its last-entry result semantics.
    friend class CDataManager;
    typedef __gnu_cxx::hash_map<int, CMap*> MapHash;
    MapHash maps_;
    MapHash normal_maps_[16];
    MapHash boss_maps_[16];
    MapHash dummy_maps_[16];
    int pvp_map_count_;
    static std::map<int, PvPMapInfo> mapPvpMapInfo_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STMapScript) == 0x388,
              "STMapScript 0828 extension size mismatch");
static_assert(sizeof(STDungeonScript) == 0x520,
              "STDungeonScript extended size mismatch");
static_assert(sizeof(CMap) == 0x130, "CMap IDA size mismatch");
static_assert(sizeof(RequiredItem) == 0x0c,
              "RequiredItem IDA size mismatch");
static_assert(sizeof(STMonsterTypeSpawnProb) == 0x10,
              "STMonsterTypeSpawnProb IDA size mismatch");
static_assert(sizeof(STWarRoomData) == 0x100,
              "STWarRoomData IDA size mismatch");
static_assert(sizeof(RandomList) == 0x8,
              "RandomList IDA ABI size mismatch");
static_assert(sizeof(CDungeon) == 0x944,
              "CDungeon extended size mismatch");
static_assert(sizeof(CDungeonList) == 0x14,
              "CDungeonList IDA size mismatch");
static_assert(sizeof(CMapList) == 0x3d8, "CMapList IDA size mismatch");
#endif

int ImportMapScript(STMapScript* script, const char* filename);
int ImportDungeonScript(STDungeonScript* script, const char* filename);

void ScanChampionFeatures(int feature_count, STAssignMonster& monster)
    __asm__("_Z20ScanChampionFeaturesiR15STAssignMonster");
void ScanChampionFeatures(int feature_count,
                          STAICharacterMapArrangeData& character)
    __asm__("_Z20ScanChampionFeaturesiR27STAICharacterMapArrangeData");

#endif  // DNF_DATAMANAGER_MAP_RUNTIME_H_
