#include "old_data/PVF/map/map_script_types.h"

// IDA_EA: 0x89E836C
// IDA_NAME: _ZN17STAssignAnimationC2Ev
// DEMANGLED: STAssignAnimation::STAssignAnimation(void)
STAssignAnimation::STAssignAnimation() : animation_name() {
    Clear();
}

// IDA_EA: 0x83EF55E
// IDA_NAME: _ZN17STAssignAnimationC2ERKS_
// DEMANGLED: STAssignAnimation::STAssignAnimation(STAssignAnimation const&)
STAssignAnimation::STAssignAnimation(const STAssignAnimation& rhs)
    : animation_name(rhs.animation_name),
      layer(rhs.layer),
      position_x(rhs.position_x),
      position_y(rhs.position_y),
      position_z(rhs.position_z) {}

// IDA_EA: 0x836C4E6
// IDA_NAME: _ZN17STAssignAnimationD2Ev
// DEMANGLED: STAssignAnimation::~STAssignAnimation()
STAssignAnimation::~STAssignAnimation() {}

// IDA_EA: 0x89E8328
// IDA_NAME: _ZN17STAssignAnimation5ClearEv
// DEMANGLED: STAssignAnimation::Clear(void)
STAssignAnimation* STAssignAnimation::Clear() {
    animation_name.clear();
    layer = 1;
    position_x = 0;
    position_y = 0;
    position_z = 0;
    return this;
}

// IDA_EA: 0x89E83D2
// IDA_NAME: _ZN16STQuestAnimationC2Ev
// DEMANGLED: STQuestAnimation::STQuestAnimation(void)
STQuestAnimation::STQuestAnimation() : STAssignAnimation() {
    Clear();
}

// IDA_EA: 0x83EF620
// IDA_NAME: _ZN16STQuestAnimationC2ERKS_
// DEMANGLED: STQuestAnimation::STQuestAnimation(STQuestAnimation const&)
STQuestAnimation::STQuestAnimation(const STQuestAnimation& rhs)
    : STAssignAnimation(rhs),
      quest_id(rhs.quest_id),
      enabled(rhs.enabled),
      reserved_19(rhs.reserved_19),
      reserved_1a(rhs.reserved_1a),
      reserved_1b(rhs.reserved_1b) {}

// IDA_EA: 0x83B69E0
// IDA_NAME: _ZN16STQuestAnimationD2Ev
// DEMANGLED: STQuestAnimation::~STQuestAnimation()
STQuestAnimation::~STQuestAnimation() {}

// IDA_EA: 0x89E83AE
// IDA_NAME: _ZN16STQuestAnimation5ClearEv
// DEMANGLED: STQuestAnimation::Clear(void)
STQuestAnimation* STQuestAnimation::Clear() {
    STAssignAnimation::Clear();
    quest_id = 0;
    enabled = 1;
    reserved_19 = 0;
    reserved_1a = 0;
    reserved_1b = 0;
    return this;
}

// IDA_EA: 0x89E8440
// IDA_NAME: _ZN21STAssignPassiveObjectC2Ev
// DEMANGLED: STAssignPassiveObject::STAssignPassiveObject(void)
STAssignPassiveObject::STAssignPassiveObject() {
    Clear();
}

// IDA_EA: 0x89E8414
// IDA_NAME: _ZN21STAssignPassiveObject5ClearEv
// DEMANGLED: STAssignPassiveObject::Clear(void)
STAssignPassiveObject* STAssignPassiveObject::Clear() {
    object_id = -1;
    position_x = 0;
    position_y = 0;
    position_z = 0;
    return this;
}

// IDA_EA: 0x836C5E0
// IDA_NAME: _ZN15STAssignMonsterC2Ev
// DEMANGLED: STAssignMonster::STAssignMonster(void)
STAssignMonster::STAssignMonster()
    : champion_features(), roaming_points(), collision_values() {
    Clear();
}

// IDA_EA: 0x8377696
// IDA_NAME: _ZN15STAssignMonsterC2ERKS_
// DEMANGLED: STAssignMonster::STAssignMonster(STAssignMonster const&)
STAssignMonster::STAssignMonster(const STAssignMonster& rhs)
    : monster_id(rhs.monster_id),
      enabled(rhs.enabled),
      reserved_05(rhs.reserved_05),
      reserved_06(rhs.reserved_06),
      reserved_07(rhs.reserved_07),
      position_x(rhs.position_x),
      position_y(rhs.position_y),
      position_z(rhs.position_z),
      direction(rhs.direction),
      summon_count(rhs.summon_count),
      respawn_delay(rhs.respawn_delay),
      spawn_type(rhs.spawn_type),
      rank_type(rhs.rank_type),
      champion_features(rhs.champion_features),
      specific_ai_type(rhs.specific_ai_type),
      specific_ai_value_0(rhs.specific_ai_value_0),
      specific_ai_value_1(rhs.specific_ai_value_1),
      roaming_points(rhs.roaming_points),
      team(rhs.team),
      spawn_order(rhs.spawn_order),
      no_hit_animation(rhs.no_hit_animation),
      no_target_change(rhs.no_target_change),
      reserved_56(rhs.reserved_56),
      reserved_57(rhs.reserved_57),
      npc_id(rhs.npc_id),
      collision_values(rhs.collision_values) {}

// IDA_EA: 0x8377622
// IDA_NAME: _ZN15STAssignMonsterD2Ev
// DEMANGLED: STAssignMonster::~STAssignMonster()
STAssignMonster::~STAssignMonster() {}

// IDA_EA: 0x83777F2
// IDA_NAME: _ZN15STAssignMonsteraSERKS_
// DEMANGLED: STAssignMonster::operator=(STAssignMonster const&)
STAssignMonster& STAssignMonster::operator=(const STAssignMonster& rhs) {
    monster_id = rhs.monster_id;
    enabled = rhs.enabled;
    reserved_05 = rhs.reserved_05;
    reserved_06 = rhs.reserved_06;
    reserved_07 = rhs.reserved_07;
    position_x = rhs.position_x;
    position_y = rhs.position_y;
    position_z = rhs.position_z;
    direction = rhs.direction;
    summon_count = rhs.summon_count;
    respawn_delay = rhs.respawn_delay;
    spawn_type = rhs.spawn_type;
    rank_type = rhs.rank_type;
    champion_features = rhs.champion_features;
    specific_ai_type = rhs.specific_ai_type;
    specific_ai_value_0 = rhs.specific_ai_value_0;
    specific_ai_value_1 = rhs.specific_ai_value_1;
    roaming_points = rhs.roaming_points;
    team = rhs.team;
    spawn_order = rhs.spawn_order;
    no_hit_animation = rhs.no_hit_animation;
    no_target_change = rhs.no_target_change;
    reserved_56 = rhs.reserved_56;
    reserved_57 = rhs.reserved_57;
    npc_id = rhs.npc_id;
    collision_values = rhs.collision_values;
    return *this;
}

// IDA_EA: 0x836C4FA
// IDA_NAME: _ZN15STAssignMonster5ClearEv
// DEMANGLED: STAssignMonster::Clear(void)
STAssignMonster* STAssignMonster::Clear() {
    monster_id = -1;
    enabled = 1;
    reserved_05 = 0;
    reserved_06 = 0;
    reserved_07 = 0;
    position_x = 0;
    position_y = 0;
    position_z = 0;
    direction = 0;
    summon_count = 0;
    respawn_delay = 0;
    spawn_type = 1;
    rank_type = 1;
    champion_features.clear();
    specific_ai_type = 0;
    specific_ai_value_0 = 0;
    specific_ai_value_1 = 0;
    roaming_points.clear();
    team = 100;
    spawn_order = 0;
    no_hit_animation = 0;
    no_target_change = 0;
    reserved_56 = 0;
    reserved_57 = 0;
    npc_id = -1;
    collision_values.clear();
    return this;
}

// IDA_EA: 0x83EF750
// IDA_NAME: _ZN12STAssignItemC2ERKS_
// DEMANGLED: STAssignItem::STAssignItem(STAssignItem const&)
STAssignItem::STAssignItem(const STAssignItem& rhs)
    : item_id(rhs.item_id),
      position_x(rhs.position_x),
      position_y(rhs.position_y),
      position_z(rhs.position_z),
      probability(rhs.probability),
      delay(rhs.delay),
      owner_type(rhs.owner_type),
      owner_id(rhs.owner_id),
      flags(rhs.flags),
      generated_items(rhs.generated_items) {}

// IDA_EA: 0x83B6A8C
// IDA_NAME: _ZN12STAssignItemD2Ev
// DEMANGLED: STAssignItem::~STAssignItem()
STAssignItem::~STAssignItem() {}

// IDA_EA: 0x89E86E4
// IDA_NAME: _ZN18SPOBJ_ACTION_DATASC2Ev
// DEMANGLED: SPOBJ_ACTION_DATAS::SPOBJ_ACTION_DATAS(void)
SPOBJ_ACTION_DATAS::SPOBJ_ACTION_DATAS()
    : action_type(-1), value_04(-1), value_08(-1), value_0c(-1),
      value_10(-1), value_14(-1) {}

// IDA_EA: 0x89F0B92
// IDA_NAME: _ZN18SPOBJ_ACTION_DATASD2Ev
// DEMANGLED: SPOBJ_ACTION_DATAS::~SPOBJ_ACTION_DATAS()
SPOBJ_ACTION_DATAS::~SPOBJ_ACTION_DATAS() {}

// IDA_EA: 0x89E8708
// IDA_NAME: _ZN18SPOBJ_ACTION_DATAS13getActionTypeEv
// DEMANGLED: SPOBJ_ACTION_DATAS::getActionType(void)
int SPOBJ_ACTION_DATAS::getActionType() {
    return action_type;
}

// IDA_EA: 0x89E87A6
// IDA_NAME: _ZN25STAssignSpecialPassiveObjC2Ev
// DEMANGLED: STAssignSpecialPassiveObj::STAssignSpecialPassiveObj(void)
STAssignSpecialPassiveObj::STAssignSpecialPassiveObj() : actions() {
    Clear();
}

// IDA_EA: 0x83EF84C
// IDA_NAME: _ZN25STAssignSpecialPassiveObjC2ERKS_
// DEMANGLED: STAssignSpecialPassiveObj::STAssignSpecialPassiveObj(STAssignSpecialPassiveObj const&)
STAssignSpecialPassiveObj::STAssignSpecialPassiveObj(
    const STAssignSpecialPassiveObj& rhs)
    : object_id(rhs.object_id),
      position_x(rhs.position_x),
      position_y(rhs.position_y),
      position_z(rhs.position_z),
      actions(rhs.actions),
      action_index(rhs.action_index) {}

// IDA_EA: 0x83B6AF6
// IDA_NAME: _ZN25STAssignSpecialPassiveObjD2Ev
// DEMANGLED: STAssignSpecialPassiveObj::~STAssignSpecialPassiveObj()
STAssignSpecialPassiveObj::~STAssignSpecialPassiveObj() {}

// IDA_EA: 0x89E875A
// IDA_NAME: _ZN25STAssignSpecialPassiveObj5ClearEv
// DEMANGLED: STAssignSpecialPassiveObj::Clear(void)
STAssignSpecialPassiveObj* STAssignSpecialPassiveObj::Clear() {
    object_id = -1;
    position_x = 0;
    position_y = 0;
    position_z = 0;
    actions.clear();
    action_index = -1;
    return this;
}

// IDA_EA: 0x89E8936
// IDA_NAME: _ZN16BackGroundEffectC2Ev
// DEMANGLED: BackGroundEffect::BackGroundEffect(void)
BackGroundEffect::BackGroundEffect() : filename() {
    clear();
}

// IDA_EA: 0x83D9748
// IDA_NAME: _ZN16BackGroundEffectC2ERKS_
// DEMANGLED: BackGroundEffect::BackGroundEffect(BackGroundEffect const&)
BackGroundEffect::BackGroundEffect(const BackGroundEffect& rhs)
    : effect_type(rhs.effect_type), swing_type(rhs.swing_type),
      position_x(rhs.position_x), position_y(rhs.position_y),
      position_z(rhs.position_z), layer(rhs.layer), value_18(rhs.value_18),
      filename(rhs.filename) {}

// IDA_EA: 0x83DC8AC
// IDA_NAME: _ZN16BackGroundEffectD2Ev
// DEMANGLED: BackGroundEffect::~BackGroundEffect()
BackGroundEffect::~BackGroundEffect() {}

// IDA_EA: 0x89ED354
// IDA_NAME: _ZN16BackGroundEffectaSERKS_
// DEMANGLED: BackGroundEffect::operator=(BackGroundEffect const&)
BackGroundEffect& BackGroundEffect::operator=(const BackGroundEffect& rhs) {
    effect_type = rhs.effect_type;
    swing_type = rhs.swing_type;
    position_x = rhs.position_x;
    position_y = rhs.position_y;
    position_z = rhs.position_z;
    layer = rhs.layer;
    value_18 = rhs.value_18;
    filename = rhs.filename;
    return *this;
}

// IDA_EA: 0x89E88DA
// IDA_NAME: _ZN16BackGroundEffect5clearEv
// DEMANGLED: BackGroundEffect::clear(void)
BackGroundEffect* BackGroundEffect::clear() {
    effect_type = 2;
    swing_type = 7;
    position_x = -1;
    position_y = -1;
    position_z = -1;
    layer = -1;
    value_18 = -1;
    filename.clear();
    return this;
}

// IDA_EA: 0x89E897E
// IDA_NAME: _ZN21BackGroundEffectBlockC2Ev
// DEMANGLED: BackGroundEffectBlock::BackGroundEffectBlock(void)
BackGroundEffectBlock::BackGroundEffectBlock() : filename() {
    clear();
}

// IDA_EA: 0x83D98D2
// IDA_NAME: _ZN21BackGroundEffectBlockC2ERKS_
// DEMANGLED: BackGroundEffectBlock::BackGroundEffectBlock(BackGroundEffectBlock const&)
BackGroundEffectBlock::BackGroundEffectBlock(const BackGroundEffectBlock& rhs)
    : block_type(rhs.block_type),
      position_x(rhs.position_x),
      position_y(rhs.position_y),
      filename(rhs.filename) {}

// IDA_EA: 0x83DC8DC
// IDA_NAME: _ZN21BackGroundEffectBlockD2Ev
// DEMANGLED: BackGroundEffectBlock::~BackGroundEffectBlock()
BackGroundEffectBlock::~BackGroundEffectBlock() {}

// IDA_EA: 0x89ED740
// IDA_NAME: _ZN21BackGroundEffectBlockaSERKS_
// DEMANGLED: BackGroundEffectBlock::operator=(BackGroundEffectBlock const&)
BackGroundEffectBlock& BackGroundEffectBlock::operator=(
    const BackGroundEffectBlock& rhs) {
    block_type = rhs.block_type;
    position_x = rhs.position_x;
    position_y = rhs.position_y;
    filename = rhs.filename;
    return *this;
}

// IDA_EA: 0x89E89C6
// IDA_NAME: _ZN21BackGroundEffectBlock5clearEv
// DEMANGLED: BackGroundEffectBlock::clear(void)
BackGroundEffectBlock* BackGroundEffectBlock::clear() {
    block_type = 2;
    position_x = 2;
    position_y = 0;
    filename.clear();
    return this;
}

// IDA_EA: 0x89E8A1C
// IDA_NAME: _ZN8STDialogC2Ev
// DEMANGLED: STDialog::STDialog(void)
STDialog::STDialog() : dialog() {
    clear();
}

// IDA_EA: 0x83D9A2C
// IDA_NAME: _ZN8STDialogC2ERKS_
// DEMANGLED: STDialog::STDialog(STDialog const&)
STDialog::STDialog(const STDialog& rhs)
    : dialog(rhs.dialog),
      on_start(rhs.on_start),
      on_complete(rhs.on_complete),
      reserved_06(rhs.reserved_06) {}

// IDA_EA: 0x836C674
// IDA_NAME: _ZN8STDialogD2Ev
// DEMANGLED: STDialog::~STDialog()
STDialog::~STDialog() {}

// IDA_EA: 0x89EDAFC
// IDA_NAME: _ZN8STDialogaSERKS_
// DEMANGLED: STDialog::operator=(STDialog const&)
STDialog& STDialog::operator=(const STDialog& rhs) {
    dialog = rhs.dialog;
    on_start = rhs.on_start;
    on_complete = rhs.on_complete;
    reserved_06 = rhs.reserved_06;
    return *this;
}

// IDA_EA: 0x89E8A5E
// IDA_NAME: _ZN8STDialog5clearEv
// DEMANGLED: STDialog::clear(void)
bool STDialog::clear() {
    dialog.clear();
    on_start = 0;
    on_complete = 0;
    reserved_06 = 0;
    return true;
}

// IDA_EA: 0x89E84BC
// IDA_NAME: _ZN19STAssignBloodSystemC2Ev
// DEMANGLED: STAssignBloodSystem::STAssignBloodSystem(void)
STAssignBloodSystem::STAssignBloodSystem() {
    Clear();
}

// IDA_EA: 0x89F02F8
// IDA_NAME: _ZN19STAssignBloodSystemD2Ev
// DEMANGLED: STAssignBloodSystem::~STAssignBloodSystem()
STAssignBloodSystem::~STAssignBloodSystem() {}

// IDA_EA: 0x89E8454
// IDA_NAME: _ZN19STAssignBloodSystem5ClearEv
// DEMANGLED: STAssignBloodSystem::Clear(void)
STAssignBloodSystem* STAssignBloodSystem::Clear() {
    monster_id = -1;
    value_04 = 0;
    value_08 = 0;
    value_0c = 0;
    value_10 = 0;
    value_14 = 0;
    value_18 = 0;
    value_1c = 0;
    value_20 = 0;
    value_24 = 0;
    return this;
}

// IDA_EA: 0x89E8518
// IDA_NAME: _ZN22STAssignBloodPhaseTimeC2Ev
// DEMANGLED: STAssignBloodPhaseTime::STAssignBloodPhaseTime(void)
STAssignBloodPhaseTime::STAssignBloodPhaseTime() {
    Clear();
}

// IDA_EA: 0x89F04F0
// IDA_NAME: _ZN22STAssignBloodPhaseTimeD2Ev
// DEMANGLED: STAssignBloodPhaseTime::~STAssignBloodPhaseTime()
STAssignBloodPhaseTime::~STAssignBloodPhaseTime() {}

// IDA_EA: 0x89E84D0
// IDA_NAME: _ZN22STAssignBloodPhaseTime5ClearEv
// DEMANGLED: STAssignBloodPhaseTime::Clear(void)
STAssignBloodPhaseTime* STAssignBloodPhaseTime::Clear() {
    phase = 0;
    monster_id = -1;
    value_08 = 0;
    rate = 1.0f;
    value_10 = 0;
    value_14 = 0;
    enabled = 0;
    reserved_19 = 0;
    reserved_1a = 0;
    reserved_1b = 0;
    return this;
}

// IDA_EA: 0x89E8594
// IDA_NAME: _ZN21STAssignDefenseSystemC2Ev
// DEMANGLED: STAssignDefenseSystem::STAssignDefenseSystem(void)
STAssignDefenseSystem::STAssignDefenseSystem() {
    Clear();
}

// IDA_EA: 0x89F0736
// IDA_NAME: _ZN21STAssignDefenseSystemD2Ev
// DEMANGLED: STAssignDefenseSystem::~STAssignDefenseSystem()
STAssignDefenseSystem::~STAssignDefenseSystem() {}

// IDA_EA: 0x89E852C
// IDA_NAME: _ZN21STAssignDefenseSystem5ClearEv
// DEMANGLED: STAssignDefenseSystem::Clear(void)
STAssignDefenseSystem* STAssignDefenseSystem::Clear() {
    monster_id = -1;
    value_04 = 0;
    value_08 = 0;
    value_0c = 0;
    value_10 = 0;
    value_14 = 0;
    value_18 = 0;
    value_1c = 0;
    value_20 = 0;
    value_24 = 0;
    return this;
}

// IDA_EA: 0x89E85EA
// IDA_NAME: _ZN24STAssignDefensePhaseTimeC2Ev
// DEMANGLED: STAssignDefensePhaseTime::STAssignDefensePhaseTime(void)
STAssignDefensePhaseTime::STAssignDefensePhaseTime() {
    Clear();
}

// IDA_EA: 0x89F0964
// IDA_NAME: _ZN24STAssignDefensePhaseTimeD2Ev
// DEMANGLED: STAssignDefensePhaseTime::~STAssignDefensePhaseTime()
STAssignDefensePhaseTime::~STAssignDefensePhaseTime() {}

// IDA_EA: 0x89E85A8
// IDA_NAME: _ZN24STAssignDefensePhaseTime5ClearEv
// DEMANGLED: STAssignDefensePhaseTime::Clear(void)
STAssignDefensePhaseTime* STAssignDefensePhaseTime::Clear() {
    phase = 0;
    monster_id = -1;
    value_08 = 0;
    rate = 1.0f;
    value_10 = 0;
    value_14 = 0;
    return this;
}

// IDA_EA: 0x830E4D4
// IDA_NAME: _ZN17TournamentMonsterC2Ev
// DEMANGLED: TournamentMonster::TournamentMonster(void)
TournamentMonster::TournamentMonster() {
    Clear();
}

// IDA_EA: 0x89F1BC0
// IDA_NAME: _ZN17TournamentMonsterD2Ev
// DEMANGLED: TournamentMonster::~TournamentMonster()
TournamentMonster::~TournamentMonster() {}

// IDA_EA: 0x830E4BC
// IDA_NAME: _ZN17TournamentMonster5ClearEv
// DEMANGLED: TournamentMonster::Clear(void)
TournamentMonster* TournamentMonster::Clear() {
    monster_id = 0;
    count = 0;
    return this;
}

// IDA_EA: 0x89E8A94
// IDA_NAME: _ZN18TournamentStartPosC2Ev
// DEMANGLED: TournamentStartPos::TournamentStartPos(void)
TournamentStartPos::TournamentStartPos()
    : position_x(0), position_y(0), direction(0) {}

// IDA_EA: 0x89F1FD0
// IDA_NAME: _ZN18TournamentStartPosD2Ev
// DEMANGLED: TournamentStartPos::~TournamentStartPos()
TournamentStartPos::~TournamentStartPos() {}

// IDA_EA: 0x89E8AB6
// IDA_NAME: _ZN14StaticStartPosC2Ev
// DEMANGLED: StaticStartPos::StaticStartPos(void)
StaticStartPos::StaticStartPos()
    : player_slot(0), position_x(0), position_y(0), direction(0) {}

// IDA_EA: 0x89F21EC
// IDA_NAME: _ZN14StaticStartPosD2Ev
// DEMANGLED: StaticStartPos::~StaticStartPos()
StaticStartPos::~StaticStartPos() {}

// IDA_EA: 0x89F1392
// IDA_NAME: _ZN17STAttackedMonsterD2Ev
// DEMANGLED: STAttackedMonster::~STAttackedMonster()
STAttackedMonster::~STAttackedMonster() {}
