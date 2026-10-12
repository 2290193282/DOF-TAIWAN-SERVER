#ifndef DNF_DATAMANAGER_MAP_SCRIPT_TYPES_H_
#define DNF_DATAMANAGER_MAP_SCRIPT_TYPES_H_

#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

// IDA unit: 12_pvf_datamanager_scripts/STAssignAnimation.
struct STAssignAnimation {
    STAssignAnimation();
    STAssignAnimation(const STAssignAnimation& rhs);
    ~STAssignAnimation();
    STAssignAnimation* Clear();

    std::string animation_name;
    int32_t layer;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
};

// IDA unit: 12_pvf_datamanager_scripts/STQuestAnimation.
struct STQuestAnimation : public STAssignAnimation {
    STQuestAnimation();
    STQuestAnimation(const STQuestAnimation& rhs);
    ~STQuestAnimation();
    STQuestAnimation* Clear();

    int32_t quest_id;
    uint8_t enabled;
    uint8_t reserved_19;
    uint8_t reserved_1a;
    uint8_t reserved_1b;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignPassiveObject.
struct STAssignPassiveObject {
    STAssignPassiveObject();
    STAssignPassiveObject* Clear();

    int32_t object_id;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignMonster.
struct STAssignMonster {
    STAssignMonster();
    STAssignMonster(const STAssignMonster& rhs);
    ~STAssignMonster();
    STAssignMonster& operator=(const STAssignMonster& rhs);
    STAssignMonster* Clear();

    int32_t monster_id;
    uint8_t enabled;
    uint8_t reserved_05;
    uint8_t reserved_06;
    uint8_t reserved_07;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
    int32_t direction;
    int32_t summon_count;
    int32_t respawn_delay;
    int32_t spawn_type;
    int32_t rank_type;
    std::vector<int> champion_features;
    int32_t specific_ai_type;
    int32_t specific_ai_value_0;
    int32_t specific_ai_value_1;
    std::vector<std::pair<int, int> > roaming_points;
    int32_t team;
    int32_t spawn_order;
    uint8_t no_hit_animation;
    uint8_t no_target_change;
    uint8_t reserved_56;
    uint8_t reserved_57;
    int32_t npc_id;
    std::vector<uint8_t> collision_values;
};

// IDA evidence: ExportMapScript serializes exactly the two dwords at +0/+4.
struct STItemGenerateInfo {
    int32_t item_id;
    int32_t count;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignItem.
struct STAssignItem {
    STAssignItem(const STAssignItem& rhs);
    ~STAssignItem();

    int32_t item_id;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
    int32_t probability;
    int32_t delay;
    int32_t owner_type;
    int32_t owner_id;
    int32_t flags;
    std::vector<STItemGenerateInfo> generated_items;
};

// IDA unit: 12_pvf_datamanager_scripts/SPOBJ_ACTION_DATAS.
struct SPOBJ_ACTION_DATAS {
    SPOBJ_ACTION_DATAS();
    ~SPOBJ_ACTION_DATAS();
    int getActionType();

    int32_t action_type;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
    int32_t value_10;
    int32_t value_14;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignSpecialPassiveObj.
struct STAssignSpecialPassiveObj {
    STAssignSpecialPassiveObj();
    STAssignSpecialPassiveObj(const STAssignSpecialPassiveObj& rhs);
    ~STAssignSpecialPassiveObj();
    STAssignSpecialPassiveObj* Clear();

    int32_t object_id;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
    std::vector<SPOBJ_ACTION_DATAS> actions;
    int32_t action_index;
};

// IDA unit: 12_pvf_datamanager_scripts/BackGroundEffect.
struct BackGroundEffect {
    BackGroundEffect();
    BackGroundEffect(const BackGroundEffect& rhs);
    ~BackGroundEffect();
    BackGroundEffect& operator=(const BackGroundEffect& rhs);
    BackGroundEffect* clear();

    int32_t effect_type;
    int32_t swing_type;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
    int32_t layer;
    int32_t value_18;
    std::string filename;
};

// IDA unit: 12_pvf_datamanager_scripts/BackGroundEffectBlock.
struct BackGroundEffectBlock {
    BackGroundEffectBlock();
    BackGroundEffectBlock(const BackGroundEffectBlock& rhs);
    ~BackGroundEffectBlock();
    BackGroundEffectBlock& operator=(const BackGroundEffectBlock& rhs);
    BackGroundEffectBlock* clear();

    int32_t block_type;
    int32_t position_x;
    int32_t position_y;
    std::string filename;
};

// IDA unit: 12_pvf_datamanager_scripts/STDialog.
struct STDialog {
    STDialog();
    STDialog(const STDialog& rhs);
    ~STDialog();
    STDialog& operator=(const STDialog& rhs);
    bool clear();

    std::string dialog;
    uint8_t on_start;
    uint8_t on_complete;
    uint8_t reserved_06;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignBloodSystem.
struct STAssignBloodSystem {
    STAssignBloodSystem();
    ~STAssignBloodSystem();
    STAssignBloodSystem* Clear();

    int32_t monster_id;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
    int32_t value_10;
    int32_t value_14;
    int32_t value_18;
    int32_t value_1c;
    int32_t value_20;
    int32_t value_24;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignBloodPhaseTime.
struct STAssignBloodPhaseTime {
    STAssignBloodPhaseTime();
    ~STAssignBloodPhaseTime();
    STAssignBloodPhaseTime* Clear();

    int32_t phase;
    int32_t monster_id;
    int32_t value_08;
    float rate;
    int32_t value_10;
    int32_t value_14;
    uint8_t enabled;
    uint8_t reserved_19;
    uint8_t reserved_1a;
    uint8_t reserved_1b;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignDefenseSystem.
struct STAssignDefenseSystem {
    STAssignDefenseSystem();
    ~STAssignDefenseSystem();
    STAssignDefenseSystem* Clear();

    int32_t monster_id;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
    int32_t value_10;
    int32_t value_14;
    int32_t value_18;
    int32_t value_1c;
    int32_t value_20;
    int32_t value_24;
};

// IDA unit: 12_pvf_datamanager_scripts/STAssignDefensePhaseTime.
struct STAssignDefensePhaseTime {
    STAssignDefensePhaseTime();
    ~STAssignDefensePhaseTime();
    STAssignDefensePhaseTime* Clear();

    int32_t phase;
    int32_t monster_id;
    int32_t value_08;
    float rate;
    int32_t value_10;
    int32_t value_14;
};

// IDA unit: 12_pvf_datamanager_scripts/TournamentMonster.
struct TournamentMonster {
    TournamentMonster();
    ~TournamentMonster();
    TournamentMonster* Clear();

    int32_t monster_id;
    int32_t count;
};

// IDA unit: 12_pvf_datamanager_scripts/TournamentStartPos.
struct TournamentStartPos {
    TournamentStartPos();
    ~TournamentStartPos();

    int32_t position_x;
    int32_t position_y;
    int32_t direction;
};

// IDA unit: 12_pvf_datamanager_scripts/StaticStartPos.
struct StaticStartPos {
    StaticStartPos();
    ~StaticStartPos();

    int32_t player_slot;
    int32_t position_x;
    int32_t position_y;
    int32_t direction;
};

// IDA unit: 12_pvf_datamanager_scripts/STAttackedMonster.
struct STAttackedMonster {
    ~STAttackedMonster();

    int32_t monster_id;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
    int32_t cumulative_count;
    int32_t value_14;
    int32_t value_18;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STAssignAnimation) == 0x14, "STAssignAnimation IDA size mismatch");
static_assert(sizeof(STQuestAnimation) == 0x1c, "STQuestAnimation IDA size mismatch");
static_assert(sizeof(STAssignPassiveObject) == 0x10, "STAssignPassiveObject IDA size mismatch");
static_assert(sizeof(STAssignMonster) == 0x68, "STAssignMonster IDA size mismatch");
static_assert(sizeof(STItemGenerateInfo) == 0x08, "STItemGenerateInfo IDA size mismatch");
static_assert(sizeof(STAssignItem) == 0x30, "STAssignItem IDA size mismatch");
static_assert(sizeof(SPOBJ_ACTION_DATAS) == 0x18, "SPOBJ_ACTION_DATAS IDA size mismatch");
static_assert(sizeof(STAssignSpecialPassiveObj) == 0x20, "STAssignSpecialPassiveObj IDA size mismatch");
static_assert(sizeof(BackGroundEffect) == 0x20, "BackGroundEffect IDA size mismatch");
static_assert(sizeof(BackGroundEffectBlock) == 0x10, "BackGroundEffectBlock IDA size mismatch");
static_assert(sizeof(STDialog) == 0x08, "STDialog IDA size mismatch");
static_assert(sizeof(STAssignBloodSystem) == 0x28, "STAssignBloodSystem IDA size mismatch");
static_assert(sizeof(STAssignBloodPhaseTime) == 0x1c, "STAssignBloodPhaseTime IDA size mismatch");
static_assert(sizeof(STAssignDefenseSystem) == 0x28, "STAssignDefenseSystem IDA size mismatch");
static_assert(sizeof(STAssignDefensePhaseTime) == 0x18, "STAssignDefensePhaseTime IDA size mismatch");
static_assert(sizeof(TournamentMonster) == 0x08, "TournamentMonster IDA size mismatch");
static_assert(sizeof(TournamentStartPos) == 0x0c, "TournamentStartPos IDA size mismatch");
static_assert(sizeof(StaticStartPos) == 0x10, "StaticStartPos IDA size mismatch");
static_assert(sizeof(STAttackedMonster) == 0x1c, "STAttackedMonster IDA size mismatch");
#endif

#endif  // DNF_DATAMANAGER_MAP_SCRIPT_TYPES_H_
