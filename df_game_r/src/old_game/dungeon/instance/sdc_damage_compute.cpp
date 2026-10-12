#include "old_game/dungeon/instance/sdc_damage_compute.h"

#include <algorithm>

#include "old_data/PVF/creature/active_status_sync_server.h"
#include "old_data/PVF/etc/datamanager_etc_drivers_0828.h"

namespace {

// IDA data: g_maxTotalDefenseRate @0x936E4A8 = 0.01f in the 0627 ELF.
// The 0828 EtcParameter layout moves that same defense floor to ordered
// damage-formula index 28.  The critical multiplier remains immutable data.
const float kCriticalDamageDefaultRate = 1.5f;

bool HasObjectType(uint32_t object_type_mask, uint32_t required_mask) {
    // IDA_EA: 0x80950E8, _SDC::isObjectType(int,int).
    return (object_type_mask & required_mask) == required_mask;
}

float GetAbilityConvertRate() {
    // IDA_EA: 0x80950FB, getAbilityConvertRateSDC(void *,ENUM_CONVERT_TABLE_TYPE).
    return 1.0f;
}

float GetCreatureDamageRateFromModule(int module_type) {
    std::map<int, float>::const_iterator it =
        g_creatureModuleDamageRateMap.find(module_type);
    return it == g_creatureModuleDamageRateMap.end() ? 1.0f : it->second;
}

float GetCreatureDamageRateFromIndex(int index) {
    std::map<int, float>::const_iterator it =
        g_creatureIndexDamageRateMap.find(index);
    return it == g_creatureIndexDamageRateMap.end() ? 1.0f : it->second;
}

float GetDamageRateCreature(int creature_level,
                            int creature_level_bonus,
                            int creature_damage_basis) {
    if (creature_level > 50) {
        creature_level = 50;
    }
    const int level = creature_level + creature_level_bonus;
    if (g_creature_damage_vector_.size() <= 6u) {
        return 1.0f;
    }

    const double value_0 =
        static_cast<double>(g_creature_damage_vector_.at(0)) +
        static_cast<double>(creature_damage_basis);
    const double value_1 =
        static_cast<double>(g_creature_damage_vector_.at(1)) * level + value_0;
    const double value_2 = static_cast<double>(g_creature_damage_vector_.at(2));
    const double value_3 =
        static_cast<double>(g_creature_damage_vector_.at(3)) * level + value_2;
    const double value_4 =
        (value_3 / static_cast<double>(g_creature_damage_vector_.at(4)) + 1.0) *
        value_1;
    const double value_5 = static_cast<double>(g_creature_damage_vector_.at(5));
    const double value_6 =
        static_cast<double>(g_creature_damage_vector_.at(3)) * level + value_5;
    return static_cast<float>(
        (value_6 / static_cast<double>(g_creature_damage_vector_.at(6)) + value_4) /
        100.0);
}

float GetDamageInflictCreature(const STComputeDamageVariable& variable) {
    if (!variable.has_creature_damage) {
        return 0.0f;
    }

    float damage = variable.base_damage;
    damage *= GetDamageRateCreature(
        variable.creature_level,
        variable.creature_level_bonus,
        static_cast<int>(variable.creature_damage_basis));
    damage *= variable.creature_damage_multiplier;
    damage *= GetCreatureDamageRateFromModule(variable.module_type);
    if (variable.module_type == 9) {
        damage *= GetCreatureDamageRateFromIndex(variable.creature_index);
    }
    return damage;
}

float GetDamageInflictCollisionObject(const STComputeDamageVariable& variable) {
    float damage = variable.base_damage;
    if (variable.use_ability_bonus) {
        if (variable.attack_type == 0 || variable.attack_type == 2) {
            damage += variable.physical_ability_bonus;
        } else if (variable.attack_type == 1) {
            damage += variable.magical_ability_bonus;
        }
    }

    damage = (variable.attack_rate_percent * 0.01f + 1.0f) * damage;
    if (variable.attack_type == 0) {
        damage = (variable.physical_attack_percent * 0.01f + 1.0f) * damage;
        float power = variable.physical_attack_power;
        if (variable.module_type != 4 && variable.module_type != 5) {
            power *= GetAbilityConvertRate();
        }
        damage = (power / g_physicalDamageRateConst_ + 1.0f) * damage;
    } else if (variable.attack_type == 1) {
        damage = (variable.magical_attack_percent * 0.01f + 1.0f) * damage;
        float power = variable.magical_attack_power;
        if (variable.module_type != 4 && variable.module_type != 5) {
            power *= GetAbilityConvertRate();
        }
        damage = (power / g_magicalDamageRateConst_ + 1.0f) * damage;
    }
    const float converted_damage = GetAbilityConvertRate() * damage;
    return variable.collision_damage_multiplier * converted_damage +
           converted_damage;
}

float GetDamageInflictPassiveObject(const STComputeDamageVariable& variable) {
    return variable.has_creature_damage && variable.use_creature_damage
               ? GetDamageInflictCreature(variable)
               : GetDamageInflictCollisionObject(variable);
}

float GetDamageInflict(const STComputeDamageVariable& variable) {
    if (HasObjectType(variable.source_object_type_mask, 33u)) {
        return GetDamageInflictPassiveObject(variable);
    }
    if (HasObjectType(variable.source_object_type_mask, 129u)) {
        return GetDamageInflictCreature(variable);
    }
    return GetDamageInflictCollisionObject(variable);
}

bool IsValidAttackInfoElement(const STComputeDamageVariable& variable,
                              int element) {
    if (element != 4) {
        return variable.attack_element_enabled[element] != 0;
    }
    return !variable.attack_element_enabled[0] &&
           !variable.attack_element_enabled[1] &&
           !variable.attack_element_enabled[2] &&
           !variable.attack_element_enabled[3];
}

int GetComputeDamageBasisLevelAttacker(
    const STComputeDamageVariable& variable,
    bool use_alternate_level) {
    if (use_alternate_level &&
        HasObjectType(variable.actor_object_type_mask, 273u)) {
        return variable.alternate_attacker_level;
    }
    return variable.attacker_level;
}

void GetDefenseRateDamager(float* defense_rate,
                           float* reduction_rate,
                           const STComputeDamageVariable& variable) {
    const int level = GetComputeDamageBasisLevelAttacker(variable, false);
    float defense = 0.0f;
    float level_rate = 0.0f;
    float reduction_base = 0.0f;

    if (variable.attack_type == 0) {
        defense = static_cast<float>(variable.physical_defense);
        defense = (static_cast<float>(variable.physical_defense_percent) *
                       0.01f +
                   1.0f) *
                  defense;
        reduction_base = g_physicalReduceRateConst_;
        level_rate = variable.physical_defense_reduction * 0.01f;
        if (variable.module_type == 9) {
            level_rate *= GetAbilityConvertRate();
        }
    } else if (variable.attack_type == 1) {
        defense = static_cast<float>(variable.magical_defense);
        defense = (static_cast<float>(variable.magical_defense_percent) *
                       0.01f +
                   1.0f) *
                  defense;
        reduction_base = g_magicalReduceRateConst_;
        level_rate = variable.magical_defense_reduction * 0.01f;
        if (variable.module_type == 9) {
            level_rate *= GetAbilityConvertRate();
        }
    }

    defense *= GetAbilityConvertRate();
    if (variable.double_defense_rate) {
        reduction_base += reduction_base;
    }
    defense = std::max(0.0f, defense);
    const float denominator = std::max(
        static_cast<float>(level) * reduction_base + defense, 1.0f);
    *defense_rate = defense / denominator;
    if (defense < 0.0f && level <= 9) {
        *defense_rate = defense / 400.0f;
    }
    *reduction_rate = level_rate;
}

bool CheckIsCritical(STComputeDamageVariable* variable,
                     float* damage,
                     uint32_t* random_state) {
    if (!damage) {
        return false;
    }

    float rate = variable->critical_rate + 3.0f;
    if (variable->use_creature_damage) {
        if (HasObjectType(variable->actor_object_type_mask, 129u)) {
            rate += variable->creature_critical_rate;
        }
    } else if (HasObjectType(variable->actor_object_type_mask, 17u)) {
        if (variable->attack_type == 0) {
            rate += variable->physical_critical_rate;
        } else if (variable->attack_type == 1) {
            rate += variable->magical_critical_rate;
        }
    }

    if (variable->use_additional_critical) {
        rate += variable->additional_critical_rate;
        if (HasObjectType(variable->actor_object_type_mask, 17u)) {
            if (variable->attack_type == 0) {
                rate += variable->physical_extra_critical_rate;
            } else if (variable->attack_type == 1) {
                rate += variable->magical_extra_critical_rate;
            }
        }
    }
    if (HasObjectType(variable->target_object_type_mask, 17u)) {
        rate -= variable->target_critical_resistance;
    }

    if (rate * 100.0f <= static_cast<float>(SDC_Rand(random_state) % 10000)) {
        variable->is_critical = 0;
        return false;
    }

    float critical_multiplier = kCriticalDamageDefaultRate;
    if (HasObjectType(variable->actor_object_type_mask, 17u)) {
        critical_multiplier *= variable->critical_damage_multiplier;
    }
    *damage *= variable->critical_damage_add + critical_multiplier;
    variable->is_critical = 1;
    return true;
}

}  // namespace

// IDA_EA: 0x808BD3F
// IDA_NAME: _Z8SDC_RandPj
// DEMANGLED: SDC_Rand(unsigned int *)
int SDC_Rand(uint32_t* state) {
    if (!state) {
        return 0;
    }
    const uint32_t first = 1103515245u * *state + 12345u;
    const int high = static_cast<int>((first >> 16) & 0x7ffu);
    const uint32_t second = 1103515245u * first + 12345u;
    const uint32_t third = 1103515245u * second + 12345u;
    *state = third;
    return static_cast<int>(((third >> 16) & 0x3ffu) ^
                            ((((second >> 16) & 0x3ffu) ^ (high << 10)) <<
                             10));
}

// IDA_EA: 0x8095C32
// IDA_NAME: _Z18computeHpDamageSDCP23STComputeDamageVariablePvS1_
// DEMANGLED: computeHpDamageSDC(STComputeDamageVariable *,void *,void *)
int computeHpDamageSDC(STComputeDamageVariable* variable, void*, void*) {
    uint32_t random_state[3] = {variable->random_seed, 0, 0};
    float damage = GetDamageInflict(*variable);

    if (variable->attack_element != 4 &&
        IsValidAttackInfoElement(*variable, variable->attack_element)) {
        damage *= 1.05f;
    }
    if (HasObjectType(variable->target_object_type_mask, 17u) &&
        !IsValidAttackInfoElement(*variable, 4)) {
        float best_element_damage = -1.0f;
        for (int index = 0; index <= 3; ++index) {
            if (variable->attack_element_enabled[index]) {
                const float element_damage =
                    (g_commonElementBaseConst_ -
                     static_cast<float>(variable->element_resistance[index]) *
                         variable->element_resistance_rate) *
                    0.01f * damage;
                if (element_damage > best_element_damage) {
                    best_element_damage = element_damage;
                }
            }
        }
        damage = best_element_damage;
    }

    float random_rate = 0.0f;
    if (variable->attack_type == 0) {
        random_rate = g_physicalRandomBaseConst_ +
                      static_cast<float>(GetComputeDamageBasisLevelAttacker(
                          *variable, variable->use_ability_bonus != 0)) *
                          g_physicalRandomRateConst_;
    } else if (variable->attack_type == 1) {
        random_rate = g_magicalRandomBaseConst_ +
                      static_cast<float>(GetComputeDamageBasisLevelAttacker(
                          *variable, variable->use_ability_bonus != 0)) *
                          g_magicalRandomRateConst_;
    } else if (variable->attack_type == 2) {
        random_rate = damage * 0.07f;
    }
    if (variable->module_type == 14) {
        random_rate = 0.0f;
    }
    damage += GetUniformVelocityFloatSDC(-random_rate,
                                         random_rate,
                                         SDC_Rand(random_state) % 1000,
                                         999);

    if (HasObjectType(variable->target_object_type_mask, 17u) &&
        variable->attack_type != 2 && !variable->suppress_defense_element) {
        float defense_rate = 0.0f;
        float reduction_rate = 0.0f;
        GetDefenseRateDamager(&defense_rate, &reduction_rate, *variable);
        float total_defense_rate =
            1.0f - defense_rate - variable->target_defense_rate * reduction_rate;
        const float max_total_defense_rate = GetMaxTotalDefenseRate0828();
        if (max_total_defense_rate > total_defense_rate) {
            total_defense_rate = max_total_defense_rate;
        }
        if (total_defense_rate < 0.0f) {
            total_defense_rate = 0.0f;
        }
        damage *= total_defense_rate;
    }

    if (variable->attack_type == 0) {
        float skill_damage = variable->skill_base_damage;
        if (variable->use_ability_bonus) {
            skill_damage += static_cast<float>(variable->physical_ability) *
                            variable->ability_level_rate;
        }
        skill_damage *= variable->attack_rate_percent * 0.01f + 1.0f;
        skill_damage *= variable->physical_attack_percent * 0.01f + 1.0f;
        damage += GetAbilityConvertRate() * skill_damage;
        if (HasObjectType(variable->target_object_type_mask, 17u) &&
            !variable->suppress_defense_element) {
            const float reduction =
                static_cast<float>(variable->physical_defense_add) *
                    variable->target_defense_rate +
                static_cast<float>(variable->physical_damage_add);
            damage -= GetAbilityConvertRate() * reduction;
        }
        damage -= variable->physical_damage_reduction * damage;
    } else if (variable->attack_type == 1) {
        float skill_damage = variable->skill_base_damage;
        if (variable->use_ability_bonus) {
            skill_damage += static_cast<float>(variable->magical_ability) *
                            variable->ability_level_rate;
        }
        skill_damage *= variable->attack_rate_percent * 0.01f + 1.0f;
        skill_damage *= variable->magical_attack_percent * 0.01f + 1.0f;
        damage += GetAbilityConvertRate() * skill_damage;
        if (HasObjectType(variable->target_object_type_mask, 17u) &&
            !variable->suppress_defense_element) {
            const float reduction =
                static_cast<float>(variable->magical_defense_add) *
                    variable->target_defense_rate +
                static_cast<float>(variable->magical_damage_add);
            damage -= GetAbilityConvertRate() * reduction;
        }
        damage -= variable->magical_damage_reduction * damage;
    }

    if (damage < 0.0f) {
        damage = 0.0f;
    }
    CheckIsCritical(variable, &damage, random_state);
    if (variable->has_critical_bonus) {
        variable->critical_bonus_damage =
            static_cast<int>(damage * 1.1f - damage);
        damage *= 1.1f;
    }

    damage = variable->final_damage_percent * damage * 0.01f;
    damage = static_cast<float>(variable->damage_percent) * damage * 0.01f;
    if (variable->conditional_rate_enabled[0]) {
        damage = variable->conditional_rate_1 * 0.01f * damage;
    }
    if (variable->conditional_rate_enabled[1]) {
        damage = variable->conditional_rate_2 * 0.01f * damage;
    }
    if (variable->conditional_rate_enabled[2]) {
        damage = variable->conditional_rate_3 * 0.01f * damage;
    }
    damage = variable->final_damage_multiplier * damage;
    if (damage < 1.0f) {
        damage = 1.0f;
    }
    if (HasObjectType(variable->damage_rate_object_type_mask, 273u)) {
        const int rate_set = variable->is_critical != 0 ? 1 : 0;
        for (int index = 0; index <= 25; ++index) {
            damage = variable->critical_damage_rate[rate_set][index] * damage;
        }
    }
    if (HasObjectType(variable->actor_object_type_mask, 17u) &&
        variable->actor_damage_multiplier != 1.0f) {
        damage = variable->actor_damage_multiplier * damage;
    }
    if (HasObjectType(variable->actor_object_type_mask, 17u) &&
        variable->actor_damage_add) {
        damage += static_cast<float>(variable->actor_damage_add);
    }
    damage = variable->post_damage_multiplier * damage;
    if (variable->use_quarter_damage_bonus &&
        HasObjectType(variable->actor_object_type_mask, 273u)) {
        damage += static_cast<float>(static_cast<int>(damage * 0.25f));
    }
    if (variable->use_fixed_damage_add) {
        damage += static_cast<float>(variable->fixed_damage_add);
    }
    if (variable->use_adjacent_multiplier &&
        HasObjectType(variable->actor_object_type_mask, 273u)) {
        damage = variable->adjacent_multiplier * damage;
    }
    damage += variable->adjacent_damage_percent * damage / 100.0f;
    damage += variable->adjacent_damage_add;
    variable->computed_damage = static_cast<int>(damage);
    return variable->computed_damage;
}
