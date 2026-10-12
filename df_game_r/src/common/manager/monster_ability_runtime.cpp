#include "common/manager/active_static_info_abi.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "common/manager/monster_script_public_abi.h"

namespace {

const int kChampionFeatureList[37] = {
    0x0001ff00, 0x0000ff01, 0x0002ff02, 0x0002ff03, 0x00030004,
    0x00030005, 0x00030006, 0x00030007, 0x00040008, 0x0003fe09,
    0x0003fe0a, 0x0003fe0b, 0x0003fe0c, 0x0002000d, 0x0003000e,
    0x0003000f, 0x00020010, 0x00040011, 0x00010012, 0x00020013,
    0x0002fd14, 0x0002fd15, 0x0002fd16, 0x0002fd17, 0x0002fd18,
    0x0002fd19, 0x0002fd1a, 0x0003001b, 0x0004fd1c, 0x0004fd1d,
    0x0004fd1e, 0x0004fd1f, 0x0004fd20, 0x0004fd21, 0x0004fd22,
    0x0004fd23, 0x0004fd24
};

// IDA global _ZL21CHAMPFEAT_POINT_MINUS_425.
const int kChampionFeaturePointCost[5] = {10, 8, 5, 3, 2};

int GetToleranceActiveStatus(int property) {
    if (property >= 0 && property <= 11) {
        return property;
    }
    if (property == 14) {
        return 15;
    }
    if (property == 15) {
        return 16;
    }
    if (property == 16) {
        return 17;
    }
    return 18;
}

int GetLinearDataForMonsterAbility(int begin,
                                   int end,
                                   int relative_level,
                                   int level_span) {
    if (relative_level >= level_span) {
        return end;
    }
    if (relative_level > 0) {
        return begin + relative_level * (end - begin) / level_span;
    }
    return begin;
}

bool IsNamedMonsterIndex(int monster_index) {
    if (monster_index > 49999 && monster_index <= 51000) {
        return true;
    }
    return monster_index > 55999 && monster_index <= 60000;
}

int GetMonsterTableIndex(const STMonsterScript* script,
                         int dungeon_index,
                         int monster_kind,
                         bool force_boss,
                         bool force_tower) {
    int table_index = 0;
    if (script->value_3bc == 1) {
        table_index = 3;
    }
    if (force_boss) {
        table_index = 3;
    }
    if (monster_kind == 3) {
        table_index = 2;
    }
    if (IsNamedMonsterIndex(script->index)) {
        table_index = 1;
    }
    if ((dungeon_index > 1999 && dungeon_index <= 2002) || force_tower) {
        return 4;
    }
    return table_index;
}

bool HaveChampionFeature(const std::vector<int>& features, int feature) {
    return std::find(features.begin(), features.end(), feature) !=
           features.end();
}

bool AddChampionFeature(int feature,
                        int& remaining_points,
                        std::vector<int>& features) {
    for (std::vector<int>::const_iterator it = features.begin();
         it != features.end();
         ++it) {
        const int current_group = (*it & 0xff00) >> 8;
        const int new_group = (feature & 0xff00) >> 8;
        if ((current_group == 0xff && new_group == 0xff) ||
            (current_group == 0xfe && new_group == 0xfe) ||
            (current_group == 0xfd && new_group == 0xfd) ||
            (current_group == 0xfc && new_group == 0xfc) ||
            *it == feature) {
            return false;
        }
    }

    const unsigned int cost_index =
        static_cast<unsigned int>((feature & 0xff0000) >> 16);
    if (cost_index >=
        sizeof(kChampionFeaturePointCost) /
            sizeof(kChampionFeaturePointCost[0])) {
        return false;
    }
    const int cost = kChampionFeaturePointCost[cost_index];
    if (remaining_points < cost) {
        return false;
    }
    features.push_back(feature);
    remaining_points -= cost;
    return true;
}

void RandomSetChampionFeature(unsigned int seed,
                              int& remaining_points,
                              std::vector<int>& features,
                              int dungeon_difficulty) {
    std::srand(seed);
    if (dungeon_difficulty <= 3) {
        int guard = 3700;
        while (remaining_points > 1 && guard-- > 0) {
            const int feature = kChampionFeatureList[std::rand() % 28];
            if (feature > 196615) {
                if (feature >= 261641 &&
                    (feature <= 261644 || feature == 262152)) {
                    continue;
                }
            } else if (feature >= 196612 ||
                       feature == 130816 ||
                       feature == 131091 ||
                       feature == 65281) {
                continue;
            }
            AddChampionFeature(feature, remaining_points, features);
        }
        return;
    }
    AddChampionFeature(kChampionFeatureList[std::rand() % 7 + 28],
                       remaining_points,
                       features);
}

void ApplyBaseDifficulty(int dungeon_difficulty,
                         ActiveStaticInfo* ability) {
    ability->hp_max_base = static_cast<int>(
        static_cast<long double>(ability->hp_max_base) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 20]);
    ability->active_status.resistance[13] += static_cast<int>(
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 15]);
    ability->motion.hit_recovery.value += static_cast<int>(
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 50]);
    for (int index = 0; index <= 16; ++index) {
        if (GetToleranceActiveStatus(index) != 18) {
            ability->active_status.resistance[index] += static_cast<int>(
                DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 35]);
        }
    }
    ability->motion.attack_speed.value = static_cast<int>(
        static_cast<long double>(ability->motion.attack_speed.value) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 25]);
    if (ability->motion.move_speed.value) {
        ability->motion.move_speed.value += static_cast<int>(
            DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 30]);
    }
    ability->motion.equipment_physical_attack.value = static_cast<int>(
        static_cast<long double>(
            ability->motion.equipment_physical_attack.value) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 40]);
    ability->motion.equipment_magical_attack.value = static_cast<int>(
        static_cast<long double>(
            ability->motion.equipment_magical_attack.value) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 40]);
    ability->motion.equipment_physical_defense.value = static_cast<int>(
        static_cast<long double>(
            ability->motion.equipment_physical_defense.value) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 45]);
    ability->motion.equipment_magical_defense.value = static_cast<int>(
        static_cast<long double>(
            ability->motion.equipment_magical_defense.value) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 45]);
}

void ApplyDifficultyRow(int dungeon_difficulty,
                        ActiveStaticInfo* ability,
                        const std::vector<float>& row) {
    ability->hp_max_base = static_cast<int>(
        row.at(4) *
        (static_cast<long double>(ability->hp_max_base) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 20]));
    ability->active_status.resistance[13] += static_cast<int>(
        row.at(3) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 15]);
    ability->motion.hit_recovery.value += static_cast<int>(
        row.at(10) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 50]);
    for (int index = 0; index <= 16; ++index) {
        if (GetToleranceActiveStatus(index) != 18) {
            ability->active_status.resistance[index] += static_cast<int>(
                row.at(7) *
                DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 35]);
        }
    }
    ability->motion.attack_speed.value = static_cast<int>(
        row.at(5) *
        (static_cast<long double>(ability->motion.attack_speed.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 25]));
    if (ability->motion.move_speed.value) {
        ability->motion.move_speed.value += static_cast<int>(
            row.at(6) *
            DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 30]);
    }
    ability->motion.equipment_physical_attack.value = static_cast<int>(
        row.at(8) *
        (static_cast<long double>(
             ability->motion.equipment_physical_attack.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 40]));
    ability->motion.equipment_magical_attack.value = static_cast<int>(
        row.at(8) *
        (static_cast<long double>(
             ability->motion.equipment_magical_attack.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 40]));
    ability->motion.equipment_physical_defense.value = static_cast<int>(
        row.at(9) *
        (static_cast<long double>(
             ability->motion.equipment_physical_defense.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 45]));
    ability->motion.equipment_magical_defense.value = static_cast<int>(
        row.at(9) *
        (static_cast<long double>(
             ability->motion.equipment_magical_defense.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 45]));
}

}  // namespace

// IDA_EA: 0x88B7C24
// IDA_NAME: _Z27upgradeAbilityByBossFeatureP16ActiveStaticInfo
void upgradeAbilityByBossFeature(ActiveStaticInfo*) {
    // The original function is an intentional empty provider.
}

// IDA_EA: 0x88B7C29
// IDA_NAME: _Z31upgradeAbilityByChampionFeatureiiPSt6vectorIiSaIiEEP16ActiveStaticInfo
ActiveStaticInfo* upgradeAbilityByChampionFeature(
    int champion_element,
    int monster_kind,
    std::vector<int>* features,
    ActiveStaticInfo* ability) {
    static const float kElementRates[4] = {0.0f, 0.3f, 0.5f, 0.0f};
    static const float kDefenseRates[4] = {0.0f, 1.5f, 2.0f, 0.0f};
    static const float kHpRates[4] = {0.0f, 1.5f, 3.0f, 0.0f};

    if (champion_element >= 0 && champion_element <= 3) {
        const int current = ability->elements.tolerance[champion_element];
        ability->elements.tolerance[champion_element] = static_cast<int>(
            static_cast<long double>(current) +
            static_cast<long double>(150 - current) *
                kElementRates[monster_kind]);
    } else if (champion_element == 4) {
        ability->physical_attack.value = static_cast<int>(
            static_cast<long double>(ability->physical_attack.value) *
            kDefenseRates[monster_kind]);
        ability->physical_defense.value = static_cast<int>(
            static_cast<long double>(ability->physical_defense.value) *
            kDefenseRates[monster_kind]);
    }

    ability->hp_max_base = static_cast<int>(
        static_cast<long double>(ability->hp_max_base) *
        kHpRates[monster_kind]);
    if (HaveChampionFeature(*features, 131085)) {
        ability->motion.move_speed.value = static_cast<int>(
            static_cast<long double>(ability->motion.move_speed.value) * 1.5);
    }
    if (HaveChampionFeature(*features, 196622)) {
        ability->motion.attack_speed.value +=
            ability->motion.attack_speed.value;
    }
    if (HaveChampionFeature(*features, 196354)) {
        ability->motion.equipment_physical_defense.value = static_cast<int>(
            static_cast<long double>(
                ability->motion.equipment_physical_defense.value) * 2.5);
    }
    if (HaveChampionFeature(*features, 196355)) {
        ability->motion.equipment_magical_defense.value = static_cast<int>(
            static_cast<long double>(
                ability->motion.equipment_magical_defense.value) * 2.5);
    }
    if (HaveChampionFeature(*features, 196623)) {
        ability->motion.hit_recovery.value +=
            ability->motion.hit_recovery.value;
    }
    ability->active_status.resistance[13] += 50;
    return ability;
}

// IDA_EA: 0x88B7F36
// IDA_NAME: _Z40upgradeAbilityByMonsterParameterCategoryPKSt6vectorI26STMonsterParameterCategorySaIS0_EEP16ActiveStaticInfob
bool upgradeAbilityByMonsterParameterCategory(
    const std::vector<STMonsterParameterCategory>* categories,
    ActiveStaticInfo* ability,
    bool boss) {
    bool has_hp_rate = false;
    for (std::vector<STMonsterParameterCategory>::const_iterator it =
             categories->begin();
         it != categories->end();
         ++it) {
        if (it->parameter_type == 13) {
            has_hp_rate = true;
            break;
        }
    }

    for (std::vector<STMonsterParameterCategory>::const_iterator it =
             categories->begin();
         it != categories->end();
         ++it) {
        int* target = 0;
        switch (it->parameter_type) {
            case 0:
                target = &ability->motion.attack_speed.value;
                break;
            case 1:
                target = &ability->motion.move_speed.value;
                break;
            case 2:
                target = &ability->physical_attack.value;
                break;
            case 3:
                target = &ability->physical_defense.value;
                break;
            case 4:
                target = &ability->magical_attack.value;
                break;
            case 5:
                target = &ability->magical_defense.value;
                break;
            case 6:
                target = &ability->motion.equipment_physical_attack.value;
                break;
            case 7:
                target = &ability->motion.equipment_physical_defense.value;
                break;
            case 8:
                target = &ability->motion.equipment_magical_attack.value;
                break;
            case 9:
                target = &ability->motion.equipment_magical_defense.value;
                break;
            case 10:
                target = &ability->hp_max_base;
                break;
            case 11:
                target = &ability->motion.hit_recovery.value;
                break;
            case 12:
                target = &ability->active_status.resistance[13];
                break;
            case 13:
                target = &ability->hp_max_base;
                break;
            default: break;
        }
        if (!target ||
            (has_hp_rate && boss && it->parameter_type == 10) ||
            (has_hp_rate && !boss && it->parameter_type == 13)) {
            continue;
        }
        if (it->parameter_index) {
            *target = static_cast<int>(
                static_cast<long double>(*target) *
                it->getValueFloat());
        } else {
            *target += it->getValueInt();
        }
    }
    return true;
}

// IDA_EA: 0x88B8C37
// IDA_NAME: _Z35upgradeAbilityByDifficultyOfDungeoniP16ActiveStaticInfo
ActiveStaticInfo* upgradeAbilityByDifficultyOfDungeon(
    int dungeon_difficulty,
    ActiveStaticInfo* ability) {
    ApplyBaseDifficulty(dungeon_difficulty, ability);
    return ability;
}

// IDA_EA: 0x88B85CF
// IDA_NAME: _Z35upgradeAbilityByDifficultyOfDungeoniP16ActiveStaticInfoi
ActiveStaticInfo* upgradeAbilityByDifficultyOfDungeon(
    int dungeon_difficulty,
    ActiveStaticInfo* ability,
    int monster_level) {
    const std::vector<std::vector<float> >& table =
        MONSTER_DIFFICULTY_BONUS[dungeon_difficulty];
    if (table.size() < static_cast<unsigned int>(monster_level)) {
        ApplyBaseDifficulty(dungeon_difficulty, ability);
    } else {
        ApplyDifficultyRow(dungeon_difficulty,
                           ability,
                           table.at(monster_level - 1));
    }
    return ability;
}

// IDA_EA: 0x88B813A
// IDA_NAME: _Z35upgradeAbilityByDifficultyOfDungeoniP16ActiveStaticInfoiRKSt6vectorIS1_IfSaIfEESaIS3_EE
ActiveStaticInfo* upgradeAbilityByDifficultyOfDungeon(
    int dungeon_difficulty,
    ActiveStaticInfo* ability,
    int monster_level,
    const std::vector<std::vector<float> >& dungeon_bonus) {
    if (dungeon_bonus.empty()) {
        return upgradeAbilityByDifficultyOfDungeon(
            dungeon_difficulty, ability, monster_level);
    }

    ability->hp_max_base = static_cast<int>(
        dungeon_bonus.at(4).at(dungeon_difficulty) *
        (static_cast<long double>(ability->hp_max_base) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 20]));
    ability->active_status.resistance[13] += static_cast<int>(
        dungeon_bonus.at(3).at(dungeon_difficulty) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 15]);
    ability->motion.hit_recovery.value += static_cast<int>(
        dungeon_bonus.at(10).at(dungeon_difficulty) *
        DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 50]);
    for (int index = 0; index <= 16; ++index) {
        if (GetToleranceActiveStatus(index) != 18) {
            ability->active_status.resistance[index] += static_cast<int>(
                dungeon_bonus.at(7).at(dungeon_difficulty) *
                DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 35]);
        }
    }
    ability->motion.attack_speed.value = static_cast<int>(
        dungeon_bonus.at(5).at(dungeon_difficulty) *
        (static_cast<long double>(ability->motion.attack_speed.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 25]));
    if (ability->motion.move_speed.value) {
        ability->motion.move_speed.value += static_cast<int>(
            dungeon_bonus[6][dungeon_difficulty] *
            DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 30]);
    }
    ability->motion.equipment_physical_attack.value = static_cast<int>(
        dungeon_bonus.at(8).at(dungeon_difficulty) *
        (static_cast<long double>(
             ability->motion.equipment_physical_attack.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 40]));
    ability->motion.equipment_magical_attack.value = static_cast<int>(
        dungeon_bonus.at(8).at(dungeon_difficulty) *
        (static_cast<long double>(
             ability->motion.equipment_magical_attack.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 40]));
    ability->motion.equipment_physical_defense.value = static_cast<int>(
        dungeon_bonus.at(9).at(dungeon_difficulty) *
        (static_cast<long double>(
             ability->motion.equipment_physical_defense.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 45]));
    ability->motion.equipment_magical_defense.value = static_cast<int>(
        dungeon_bonus.at(9).at(dungeon_difficulty) *
        (static_cast<long double>(
             ability->motion.equipment_magical_defense.value) *
         DIFFICULTY_OF_DUNGEON_BONUS[dungeon_difficulty + 45]));
    return ability;
}

// IDA_EA: 0x88B9603
// IDA_NAME: _Z22setBasicAbilityByLevelPK15STMonsterScriptP16ActiveStaticInfoiiibb
char setBasicAbilityByLevel(const STMonsterScript* script,
                            ActiveStaticInfo* ability,
                            int monster_level,
                            int dungeon_index,
                            int monster_kind,
                            bool force_boss,
    bool force_tower) {
    ability->clear();
    ability->initial_state = monster_level;

    const int relative_level =
        monster_level - script->level_range[0];
    const int level_span =
        script->level_range[1] - script->level_range[0];
    const int table_index = GetMonsterTableIndex(
        script, dungeon_index, monster_kind, force_boss, force_tower);
    if (table_index == 4) {
        monster_level = dungeon_index;
    }

    const MonsterBaseParameter* base = 0;
    const std::vector<MonsterBaseParameter>& global_table =
        g_monsterBaseParameters[table_index];
    for (unsigned int index = 0; index < global_table.size(); ++index) {
        if (global_table[index].monster_level == monster_level) {
            base = &global_table[index];
            if (!script->base_parameters.empty()) {
                base = &script->base_parameters[index];
            }
            break;
        }
    }
    if (!base) {
        return 0;
    }

    ability->hp_max_base = base->hp_max;
    ability->mp_max_base = base->mp_max;
    ability->physical_attack.value = base->physical_attack;
    ability->physical_defense.value = base->physical_defense;
    ability->magical_attack.value = base->magical_attack;
    ability->magical_defense.value = base->magical_defense;
    ability->motion.equipment_physical_attack.value =
        base->equipment_physical_attack;
    ability->motion.equipment_physical_defense.value =
        base->equipment_physical_defense;
    ability->motion.equipment_magical_attack.value =
        base->equipment_magical_attack;
    ability->motion.equipment_magical_defense.value =
        base->equipment_magical_defense;
    ability->primary_state_enabled = script->value_3b8;

    for (int index = 0; index <= 3; ++index) {
        ability->elements.tolerance[index] = GetLinearDataForMonsterAbility(
            script->values_3f4[2 * index],
            script->values_3f4[2 * index + 1],
            relative_level,
            level_span);
    }
    for (int index = 0; index <= 16; ++index) {
        ability->active_status.resistance[index] =
            GetLinearDataForMonsterAbility(
            script->values_414[2 * index],
            script->values_414[2 * index + 1],
            relative_level,
            level_span);
    }
    ability->motion.move_speed.value = GetLinearDataForMonsterAbility(
        script->values_49c[4], script->values_49c[5],
        relative_level, level_span);
    ability->motion.attack_speed.value = GetLinearDataForMonsterAbility(
        script->values_49c[6], script->values_49c[7],
        relative_level, level_span);
    ability->motion.cast_speed.value = GetLinearDataForMonsterAbility(
        script->values_49c[8], script->values_49c[9],
        relative_level, level_span);
    ability->motion.hit_recovery.value = GetLinearDataForMonsterAbility(
        script->values_49c[10], script->values_49c[11],
        relative_level, level_span);
    ability->motion.jump_power.value = GetLinearDataForMonsterAbility(
        script->values_49c[12], script->values_49c[13],
        relative_level, level_span);
    ability->motion.jump_speed.value = GetLinearDataForMonsterAbility(
        script->values_49c[14], script->values_49c[15],
        relative_level, level_span);
    ability->motion.weight.value = GetLinearDataForMonsterAbility(
        script->values_49c[20], script->values_49c[21],
        relative_level, level_span);

    const float hp_rate =
        static_cast<float>(GetLinearDataForMonsterAbility(
            script->values_49c[0], script->values_49c[1],
            relative_level, level_span)) /
        1000.0f;
    const float mp_rate =
        static_cast<float>(GetLinearDataForMonsterAbility(
            script->values_49c[2], script->values_49c[3],
            relative_level, level_span)) /
        1000.0f;
    ability->motion.hp_regeneration.value = static_cast<int>(
        static_cast<long double>(ability->getHpMax()) * hp_rate);
    ability->motion.mp_regeneration.value = static_cast<int>(
        static_cast<long double>(ability->getMpMax()) * mp_rate);
    ability->base_table_fractional_bonus +=
        static_cast<float>(base->fractional_combat_bonus_x100) / 100.0f;
    ability->active_status.resistance[13] += static_cast<int>(
        static_cast<long double>(base->stuck_resistance_bonus_x10) / 10.0);

    for (int index = 0; index <= 3; ++index) {
        ability->elements.attack[index] = GetLinearDataForMonsterAbility(
            script->values_508[2 * index],
            script->values_508[2 * index + 1],
            relative_level,
            level_span);
        ability->elements.tolerance[index] +=
            base->element_tolerance[index];
        ability->elements.attack[index] += base->element_attack[index];
    }
    ability->motion.hit_recovery.value += base->hit_recovery;
    ability->motion.rigidity.value = GetLinearDataForMonsterAbility(
        script->values_528[0], script->values_528[1],
        relative_level, level_span);
    ability->motion.rigidity.value += base->rigidity;
    for (int index = 0; index <= 16; ++index) {
        if (index != 13) {
            ability->active_status.resistance[index] +=
                base->active_status_resistance;
        }
    }
    return 1;
}

// IDA_EA: 0x88B9CCE
// IDA_NAME: _Z24getRandomChampionElementPK15STMonsterScripti
int getRandomChampionElement(const STMonsterScript* script, int seed) {
    if (!script) {
        return 4;
    }
    const std::vector<int>& elements = script->values_c40;
    std::srand(static_cast<unsigned int>(seed));
    if (elements.empty()) {
        return std::rand() % 4 + 1;
    }
    return elements[std::rand() % elements.size()];
}

// IDA_EA: 0x88BA0FD
// IDA_NAME: _Z23makeChampionFeatureListRSt6vectorIiSaIiEEiiS2_i
char makeChampionFeatureList(std::vector<int>& scripted_features,
                             int monster_level,
                             int seed,
                             std::vector<int>& output,
                             int dungeon_difficulty) {
    int remaining_points = static_cast<int>(
        static_cast<long double>(monster_level - 1) * 0.2 + 2.0);
    if (!scripted_features.empty()) {
        for (std::vector<int>::const_iterator it =
                 scripted_features.begin();
             it != scripted_features.end();
             ++it) {
            AddChampionFeature(*it, remaining_points, output);
        }
    } else {
        RandomSetChampionFeature(static_cast<unsigned int>(seed),
                                 remaining_points,
                                 output,
                                 dungeon_difficulty);
    }
    return 1;
}
