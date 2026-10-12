#include "common/manager/active_static_info_abi.h"

// IDA_NAME: g_ActiveStaticInfoCount
int g_ActiveStaticInfoCount = 0;

// IDA_NAME: g_ActiveStaticInfoSize
int g_ActiveStaticInfoSize = 0;

// IDA_EA: 0x81509AA
// IDA_NAME: _ZN16ActiveStaticInfo5clearEv
// DEMANGLED: ActiveStaticInfo::clear(void)
ActiveStaticInfo* ActiveStaticInfo::clear() {
    initial_state = 0;
    hp_max_base = 0;
    hp_max_rate = 1.0f;
    mp_max_base = 0;
    mp_max_rate = 1.0f;
    primary_state_enabled = 1;
    primary_state_auxiliary_a = 0;
    primary_state_auxiliary_b = 0;
    primary_state_auxiliary_c = 0;

    physical_attack.value = 0;
    physical_attack.rate = 1.0f;
    physical_defense.value = 0;
    physical_defense.rate = 1.0f;
    magical_attack.value = 0;
    magical_attack.rate = 1.0f;
    magical_defense.value = 0;
    magical_defense.rate = 1.0f;

    for (int index = 0; index <= 3; ++index) {
        elements.tolerance[index] = 0;
        elements.tolerance_rate[index] = 1.0f;
        elements.attack[index] = 0;
        elements.attack_rate[index] = 1.0f;
    }
    for (int index = 0; index <= 16; ++index) {
        active_status.resistance[index] = 0;
        active_status.resistance_rate[index] = 1.0f;
    }

    motion.general_recovery.value = 0;
    motion.general_recovery.rate = 1.0f;
    motion.hp_regeneration.value = 0;
    motion.hp_regeneration.rate = 1.0f;
    motion.mp_regeneration.value = 0;
    motion.mp_regeneration.rate = 1.0f;
    motion.move_speed.value = 1000;
    motion.move_speed.rate = 1.0f;
    motion.attack_speed.value = 1000;
    motion.attack_speed.rate = 1.0f;
    motion.cast_speed.value = 1000;
    motion.cast_speed.rate = 1.0f;
    motion.hit_recovery.value = 0;
    motion.hit_recovery.rate = 1.0f;
    motion.rigidity.value = 0;
    motion.rigidity.rate = 1.0f;
    motion.jump_power.value = 0;
    motion.jump_power.rate = 1.0f;
    motion.jump_speed.value = 100;
    motion.jump_speed.rate = 1.0f;
    motion.weight.value = 0;
    motion.weight.rate = 1.0f;
    motion.equipment_physical_attack.value = 0;
    motion.equipment_physical_attack.rate = 1.0f;
    motion.equipment_physical_defense.value = 0;
    motion.equipment_physical_defense.rate = 1.0f;
    motion.equipment_magical_attack.value = 0;
    motion.equipment_magical_attack.rate = 1.0f;
    motion.equipment_magical_defense.value = 0;
    motion.equipment_magical_defense.rate = 1.0f;
    motion.auxiliary_combat.value = 0;
    motion.auxiliary_combat.rate = 0.0f;

    for (int index = 0; index != 10; ++index) {
        combat_adjustments[index] = 0;
    }
    base_table_fractional_bonus = 0.0f;
    for (int index = 0; index != 3; ++index) {
        combat_modes[index] = 0;
    }
    combat_adjustment_rate = 1.0f;
    for (int index = 0; index != 4; ++index) {
        behavior_adjustments[index] = 0;
    }
    behavior_adjustment_rate = 1.0f;
    attack_cancel_value = 0;
    attack_cancel_state = 0;
    attack_cancel_rate = 1.0f;
    attack_cancel_count = 0;
    for (int index = 0; index != 18; ++index) {
        status_effects.level[index] = 0;
        status_effects.duration[index] = 0;
        status_effects.probability[index] = 0;
    }
    for (int index = 0; index != 9; ++index) {
        terminal_adjustments[index] = 0;
    }
    terminal_primary_rate = 1.0f;
    terminal_secondary_rate = 1.0f;
    terminal_primary_flag = 0;
    terminal_secondary_flag = 0;
    terminal_final_rate = 1.0f;
    return this;
}

// IDA_EA: 0x8150EC6
// IDA_NAME: _ZN16ActiveStaticInfoC2Ev
// DEMANGLED: ActiveStaticInfo::ActiveStaticInfo(void)
ActiveStaticInfo::ActiveStaticInfo() {
    ++g_ActiveStaticInfoCount;
    g_ActiveStaticInfoSize = 752 * g_ActiveStaticInfoCount;
    clear();
}

// The old map_monster copy path copies the complete 0x2f0 value.  Making the
// generated value copy explicit keeps the allocation counters balanced when
// a copied map_monster later destroys its ActiveStaticInfo subobject.
ActiveStaticInfo::ActiveStaticInfo(const ActiveStaticInfo& other) {
    ++g_ActiveStaticInfoCount;
    g_ActiveStaticInfoSize = 752 * g_ActiveStaticInfoCount;
    *this = other;
}

ActiveStaticInfo& ActiveStaticInfo::operator=(const ActiveStaticInfo& other) {
    if (this == &other) {
        return *this;
    }

    initial_state = other.initial_state;
    hp_max_base = other.hp_max_base;
    hp_max_rate = other.hp_max_rate;
    mp_max_base = other.mp_max_base;
    mp_max_rate = other.mp_max_rate;
    primary_state_enabled = other.primary_state_enabled;
    primary_state_auxiliary_a = other.primary_state_auxiliary_a;
    primary_state_auxiliary_b = other.primary_state_auxiliary_b;
    primary_state_auxiliary_c = other.primary_state_auxiliary_c;
    physical_attack = other.physical_attack;
    physical_defense = other.physical_defense;
    magical_attack = other.magical_attack;
    magical_defense = other.magical_defense;
    elements = other.elements;
    active_status = other.active_status;
    motion = other.motion;
    for (int index = 0; index != 10; ++index) {
        combat_adjustments[index] = other.combat_adjustments[index];
    }
    base_table_fractional_bonus = other.base_table_fractional_bonus;
    for (int index = 0; index != 3; ++index) {
        combat_modes[index] = other.combat_modes[index];
    }
    combat_adjustment_rate = other.combat_adjustment_rate;
    for (int index = 0; index != 4; ++index) {
        behavior_adjustments[index] = other.behavior_adjustments[index];
    }
    behavior_adjustment_rate = other.behavior_adjustment_rate;
    attack_cancel_value = other.attack_cancel_value;
    attack_cancel_state = other.attack_cancel_state;
    attack_cancel_rate = other.attack_cancel_rate;
    attack_cancel_count = other.attack_cancel_count;
    status_effects = other.status_effects;
    for (int index = 0; index != 9; ++index) {
        terminal_adjustments[index] = other.terminal_adjustments[index];
    }
    terminal_primary_rate = other.terminal_primary_rate;
    terminal_secondary_rate = other.terminal_secondary_rate;
    terminal_primary_flag = other.terminal_primary_flag;
    terminal_secondary_flag = other.terminal_secondary_flag;
    terminal_final_rate = other.terminal_final_rate;
    return *this;
}

// IDA_EA: 0x8150EF6
// IDA_NAME: _ZN16ActiveStaticInfoD2Ev
// DEMANGLED: ActiveStaticInfo::~ActiveStaticInfo()
ActiveStaticInfo::~ActiveStaticInfo() {
    --g_ActiveStaticInfoCount;
    g_ActiveStaticInfoSize = 752 * g_ActiveStaticInfoCount;
}

// IDA_EA: 0x85BEDB0
// IDA_NAME: _ZN16ActiveStaticInfo8getHpMaxEv
// DEMANGLED: ActiveStaticInfo::getHpMax(void)
int ActiveStaticInfo::getHpMax() {
    return static_cast<int>(static_cast<long double>(hp_max_base) *
                            hp_max_rate);
}

// IDA_EA: 0x88BA4BE
// IDA_NAME: _ZN16ActiveStaticInfo8getMpMaxEv
// DEMANGLED: ActiveStaticInfo::getMpMax(void)
int ActiveStaticInfo::getMpMax() {
    return static_cast<int>(static_cast<long double>(mp_max_base) *
                            mp_max_rate);
}
