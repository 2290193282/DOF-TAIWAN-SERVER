#include "old_data/PVF/etc/datamanager_etc_drivers_0828.h"

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "infra/log/format/trace.h"

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

DataManagerEtcStore0828 g_datamanager_etc_store_0828;

int TraceDataManagerEtcImportFailure(const char* stage,
                                     const char* script_path) {
    cMyTrace trace("int ImportDataManagerEtcStore0828()",
                   __LINE__,
                   9,
                   true,
                   true);
    trace("[DATAMANAGER_ETC_0828][IMPORT_FAIL] stage=%s script=%s",
          stage != 0 ? stage : "unknown",
          script_path != 0 ? script_path : "unknown");
    return 0;
}

bool ScanRequiredInt(int* output) {
    bool scanned = false;
    const int value = ScanInt(&scanned);
    if (!scanned) {
        return false;
    }
    *output = value;
    return true;
}

bool ScanRequiredString(std::string* output) {
    return output != 0 && ScanStr(output) == 1;
}

bool ContainsInt(const std::vector<int>& values, int value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

bool IsFiniteFloat(float value) {
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

bool ScanIntListUntil(const char* closing_tag, std::vector<int>* values) {
    if (closing_tag == 0 || values == 0) {
        return false;
    }
    for (;;) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (!success) {
            return false;
        }
        if (is_type) {
            return token == closing_tag;
        }
        values->push_back(value);
    }
}

bool ScanIntPairsUntil(const char* closing_tag,
                       std::vector<LabyrinthIntPair0828>* output) {
    std::vector<int> values;
    if (output == 0 || !ScanIntListUntil(closing_tag, &values) ||
        values.empty() || values.size() % 2u != 0u) {
        return false;
    }
    for (std::size_t index = 0; index < values.size(); index += 2u) {
        LabyrinthIntPair0828 row;
        row.first = values[index];
        row.second = values[index + 1u];
        output->push_back(row);
    }
    return true;
}

bool ScanIntTriplesUntil(const char* closing_tag,
                         std::vector<LabyrinthIntTriple0828>* output) {
    std::vector<int> values;
    if (output == 0 || !ScanIntListUntil(closing_tag, &values) ||
        values.empty() || values.size() % 3u != 0u) {
        return false;
    }
    for (std::size_t index = 0; index < values.size(); index += 3u) {
        LabyrinthIntTriple0828 row;
        row.first = values[index];
        row.second = values[index + 1u];
        row.third = values[index + 2u];
        output->push_back(row);
    }
    return true;
}

bool IsLabyrinthEquipmentCategory(const std::string& category) {
    return category == "[weapon]" || category == "[coat]" ||
           category == "[pants]" || category == "[shoulder]" ||
           category == "[shoes]" || category == "[amulet]";
}

bool IsLabyrinthThenEffect(const std::string& tag) {
    return tag == "[target]" || tag == "[add absolute damage]" ||
           tag == "[increase critical damage]" || tag == "[duration]" ||
           tag == "[probability]" || tag == "[active status]";
}

bool IsLabyrinthPassiveEffect(const std::string& tag) {
    return tag == "[fire attack]" || tag == "[water attack]" ||
           tag == "[dark attack]" || tag == "[light attack]" ||
           tag == "[physical critical hit]" ||
           tag == "[equipment physical defense]" ||
           tag == "[HP MAX]" || tag == "[HP regen speed]" ||
           tag == "[HP drain]" || tag == "[magical critical hit]" ||
           tag == "[equipment magical defense]" ||
           tag == "[MP MAX]" || tag == "[MP regen speed]" ||
           tag == "[MP drain]" || tag == "[stuck resistance]" ||
           tag == "[stuck]" ||
           tag == "[all activestatus resistance]" ||
           tag == "[all elemental resistance]" ||
           tag == "[move speed]" || tag == "[attack speed]" ||
           tag == "[cast speed]";
}

bool ParseLabyrinthCondition(
    const std::string& tag,
    LabyrinthReinforceCondition0828* output) {
    if (output == 0) {
        return false;
    }
    if (tag == "[attack success]") {
        output->kind = LABYRINTH_CONDITION_ATTACK_SUCCESS;
        return ScanRequiredInt(&output->integer_value);
    }
    if (tag == "[attack condition]") {
        output->kind = LABYRINTH_CONDITION_ATTACK_CONDITION;
        return ScanRequiredString(&output->string_value) &&
               !output->string_value.empty();
    }
    return false;
}

bool ParseLabyrinthEffect(const std::string& tag,
                          LabyrinthReinforceEffect0828* output) {
    if (output == 0) {
        return false;
    }

    if (tag == "[target]") {
        output->kind = LABYRINTH_EFFECT_TARGET;
        std::string value;
        if (!ScanRequiredString(&value)) {
            return false;
        }
        output->string_values.push_back(value);
        return true;
    }
    if (tag == "[add absolute damage]") {
        output->kind = LABYRINTH_EFFECT_ADD_ABSOLUTE_DAMAGE;
        std::string value0;
        std::string value1;
        int value2 = 0;
        if (!ScanRequiredString(&value0) ||
            !ScanRequiredString(&value1) ||
            !ScanRequiredInt(&value2)) {
            return false;
        }
        output->string_values.push_back(value0);
        output->string_values.push_back(value1);
        output->integer_values.push_back(value2);
        return true;
    }
    if (tag == "[increase critical damage]") {
        output->kind = LABYRINTH_EFFECT_INCREASE_CRITICAL_DAMAGE;
        std::string value0;
        int value1 = 0;
        if (!ScanRequiredString(&value0) || !ScanRequiredInt(&value1)) {
            return false;
        }
        output->string_values.push_back(value0);
        output->integer_values.push_back(value1);
        return true;
    }
    if (tag == "[active status]") {
        output->kind = LABYRINTH_EFFECT_ACTIVE_STATUS;
        std::string value0;
        int value1 = 0;
        int value2 = 0;
        int value3 = 0;
        if (!ScanRequiredString(&value0) || !ScanRequiredInt(&value1) ||
            !ScanRequiredInt(&value2) || !ScanRequiredInt(&value3)) {
            return false;
        }
        output->string_values.push_back(value0);
        output->integer_values.push_back(value1);
        output->integer_values.push_back(value2);
        output->integer_values.push_back(value3);
        return true;
    }

    if (tag == "[duration]") {
        output->kind = LABYRINTH_EFFECT_DURATION;
    } else if (tag == "[probability]") {
        output->kind = LABYRINTH_EFFECT_PROBABILITY;
    } else if (tag == "[fire attack]") {
        output->kind = LABYRINTH_EFFECT_FIRE_ATTACK;
    } else if (tag == "[water attack]") {
        output->kind = LABYRINTH_EFFECT_WATER_ATTACK;
    } else if (tag == "[dark attack]") {
        output->kind = LABYRINTH_EFFECT_DARK_ATTACK;
    } else if (tag == "[light attack]") {
        output->kind = LABYRINTH_EFFECT_LIGHT_ATTACK;
    } else if (tag == "[physical critical hit]") {
        output->kind = LABYRINTH_EFFECT_PHYSICAL_CRITICAL_HIT;
    } else if (tag == "[HP MAX]") {
        output->kind = LABYRINTH_EFFECT_HP_MAX;
    } else if (tag == "[HP regen speed]") {
        output->kind = LABYRINTH_EFFECT_HP_REGEN_SPEED;
    } else if (tag == "[HP drain]") {
        output->kind = LABYRINTH_EFFECT_HP_DRAIN;
    } else if (tag == "[magical critical hit]") {
        output->kind = LABYRINTH_EFFECT_MAGICAL_CRITICAL_HIT;
    } else if (tag == "[MP MAX]") {
        output->kind = LABYRINTH_EFFECT_MP_MAX;
    } else if (tag == "[MP regen speed]") {
        output->kind = LABYRINTH_EFFECT_MP_REGEN_SPEED;
    } else if (tag == "[MP drain]") {
        output->kind = LABYRINTH_EFFECT_MP_DRAIN;
    } else if (tag == "[stuck resistance]") {
        output->kind = LABYRINTH_EFFECT_STUCK_RESISTANCE;
    } else if (tag == "[stuck]") {
        output->kind = LABYRINTH_EFFECT_STUCK;
    } else if (tag == "[all activestatus resistance]") {
        output->kind = LABYRINTH_EFFECT_ALL_ACTIVE_STATUS_RESISTANCE;
    } else if (tag == "[all elemental resistance]") {
        output->kind = LABYRINTH_EFFECT_ALL_ELEMENTAL_RESISTANCE;
    } else if (tag == "[move speed]") {
        output->kind = LABYRINTH_EFFECT_MOVE_SPEED;
    } else if (tag == "[attack speed]") {
        output->kind = LABYRINTH_EFFECT_ATTACK_SPEED;
    } else if (tag == "[cast speed]") {
        output->kind = LABYRINTH_EFFECT_CAST_SPEED;
    } else if (tag == "[equipment physical defense]") {
        output->kind = LABYRINTH_EFFECT_EQUIPMENT_PHYSICAL_DEFENSE;
        int value0 = 0;
        int value1 = 0;
        if (!ScanRequiredInt(&value0) || !ScanRequiredInt(&value1)) {
            return false;
        }
        output->integer_values.push_back(value0);
        output->integer_values.push_back(value1);
        return true;
    } else if (tag == "[equipment magical defense]") {
        output->kind = LABYRINTH_EFFECT_EQUIPMENT_MAGICAL_DEFENSE;
        int value0 = 0;
        int value1 = 0;
        if (!ScanRequiredInt(&value0) || !ScanRequiredInt(&value1)) {
            return false;
        }
        output->integer_values.push_back(value0);
        output->integer_values.push_back(value1);
        return true;
    } else {
        return false;
    }

    int value = 0;
    if (!ScanRequiredInt(&value)) {
        return false;
    }
    output->integer_values.push_back(value);
    return true;
}

bool ParseLabyrinthReinforceType(LabyrinthReinforceType0828* output) {
    if (output == 0) {
        return false;
    }
    output->clear();
    std::string tag;
    bool saw_pattern = false;
    bool saw_name = false;
    bool saw_if = false;
    bool saw_then = false;
    bool in_if = false;
    bool in_then = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[/equipment reinforce type]") {
            return !in_if && !in_then && saw_pattern && saw_name &&
                   saw_if == saw_then &&
                   (!saw_if || (!output->conditions.empty() &&
                                !output->then_effects.empty()));
        }
        if (tag == "[pattern]") {
            if (in_if || in_then || saw_pattern ||
                !ScanRequiredInt(&output->pattern) || output->pattern < 0) {
                return false;
            }
            saw_pattern = true;
        } else if (tag == "[name]") {
            if (in_if || in_then || saw_name ||
                !ScanRequiredString(&output->name) || output->name.empty()) {
                return false;
            }
            saw_name = true;
        } else if (tag == "[if]") {
            if (in_if || in_then || saw_if || saw_then || !saw_pattern ||
                !saw_name) {
                return false;
            }
            in_if = true;
            saw_if = true;
        } else if (tag == "[/if]") {
            if (!in_if || output->conditions.empty()) {
                return false;
            }
            in_if = false;
        } else if (tag == "[then]") {
            if (in_if || in_then || !saw_if || saw_then ||
                output->conditions.empty()) {
                return false;
            }
            in_then = true;
            saw_then = true;
        } else if (tag == "[/then]") {
            if (!in_then || output->then_effects.empty()) {
                return false;
            }
            in_then = false;
        } else if (in_if) {
            LabyrinthReinforceCondition0828 condition;
            if (!ParseLabyrinthCondition(tag, &condition)) {
                return false;
            }
            output->conditions.push_back(condition);
        } else if (in_then && IsLabyrinthThenEffect(tag)) {
            LabyrinthReinforceEffect0828 effect;
            if (!ParseLabyrinthEffect(tag, &effect)) {
                return false;
            }
            output->then_effects.push_back(effect);
        } else if (!in_then && IsLabyrinthPassiveEffect(tag)) {
            LabyrinthReinforceEffect0828 effect;
            if (!ParseLabyrinthEffect(tag, &effect)) {
                return false;
            }
            output->passive_effects.push_back(effect);
        } else {
            return false;
        }
    }
    return false;
}

bool ParseLabyrinthEquipmentDefinition(
    LabyrinthEquipmentDefinition0828* output) {
    if (output == 0) {
        return false;
    }
    output->clear();
    std::string tag;
    bool saw_category = false;
    bool saw_fp_cost = false;
    bool saw_reinforce_max = false;
    bool saw_use_patterns = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[/equipment definition]") {
            return saw_category && saw_fp_cost && saw_reinforce_max &&
                   saw_use_patterns && !output->use_patterns.empty();
        }
        if (tag == "[category]") {
            if (saw_category || ScanType(output->category, true) != 1 ||
                !IsLabyrinthEquipmentCategory(output->category)) {
                return false;
            }
            saw_category = true;
        } else if (tag == "[fp cost]") {
            if (saw_fp_cost || !ScanRequiredInt(&output->fp_cost) ||
                output->fp_cost < 0) {
                return false;
            }
            saw_fp_cost = true;
        } else if (tag == "[reinforce max]") {
            if (saw_reinforce_max ||
                !ScanRequiredInt(&output->reinforce_max) ||
                output->reinforce_max < 0) {
                return false;
            }
            saw_reinforce_max = true;
        } else if (tag == "[use patterns]") {
            int count = 0;
            if (saw_use_patterns || !ScanRequiredInt(&count) || count <= 0) {
                return false;
            }
            for (int index = 0; index < count; ++index) {
                int pattern = -1;
                if (!ScanRequiredInt(&pattern) || pattern < 0) {
                    return false;
                }
                output->use_patterns.push_back(pattern);
            }
            saw_use_patterns = true;
        } else {
            return false;
        }
    }
    return false;
}

template <typename RowType>
bool HasUniqueFirstColumn(const std::vector<RowType>& rows) {
    std::set<int> keys;
    for (typename std::vector<RowType>::const_iterator it = rows.begin();
         it != rows.end(); ++it) {
        if (!keys.insert(it->first).second) {
            return false;
        }
    }
    return true;
}

bool ValidateLabyrinthConfig(const LabyrinthConfig0828& config) {
    if (config.character_exp_table.empty() ||
        config.monster_fp_exp_points.empty() ||
        config.barrack_fp_exp_points.empty() ||
        config.tower_fp_exp_points.empty() ||
        config.reinforce_types.empty() ||
        config.equipment_definitions.empty() ||
        config.level_up_skill_point < 0 || config.barrack_count.second < 0 ||
        config.tower_count.second < 0 ||
        !HasUniqueFirstColumn(config.character_exp_table) ||
        !HasUniqueFirstColumn(config.monster_fp_exp_points) ||
        !HasUniqueFirstColumn(config.barrack_fp_exp_points) ||
        !HasUniqueFirstColumn(config.tower_fp_exp_points)) {
        return false;
    }

    std::set<int> defined_patterns;
    for (std::vector<LabyrinthReinforceType0828>::const_iterator it =
             config.reinforce_types.begin();
         it != config.reinforce_types.end(); ++it) {
        if (!defined_patterns.insert(it->pattern).second) {
            return false;
        }
    }

    std::set<std::string> categories;
    std::set<int> referenced_patterns;
    for (std::vector<LabyrinthEquipmentDefinition0828>::const_iterator it =
             config.equipment_definitions.begin();
         it != config.equipment_definitions.end(); ++it) {
        if (!categories.insert(it->category).second) {
            return false;
        }
        for (std::vector<int>::const_iterator pattern =
                 it->use_patterns.begin();
             pattern != it->use_patterns.end(); ++pattern) {
            if (defined_patterns.find(*pattern) == defined_patterns.end() ||
                !referenced_patterns.insert(*pattern).second) {
                return false;
            }
        }
    }
    return referenced_patterns == defined_patterns;
}

bool ScanChaosFPShopEntries(const char* closing_tag,
                            std::vector<ChaosFPShopEntry0828>* output) {
    std::vector<int> values;
    if (output == 0 || !ScanIntListUntil(closing_tag, &values) ||
        values.empty() || values.size() % 3u != 0u) {
        return false;
    }
    for (std::size_t index = 0; index < values.size(); index += 3u) {
        ChaosFPShopEntry0828 entry;
        entry.value0 = values[index];
        entry.value1 = values[index + 1u];
        entry.value2 = values[index + 2u];
        output->push_back(entry);
    }
    return true;
}

int ImportPartyMatchingInfoWithCommon(
    const char* filename,
    std::vector<int>* common_dungeon_indices,
    std::vector<stPartyMatchingInfo>* output) {
    if (filename == 0 || common_dungeon_indices == 0 || output == 0 ||
        !loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::vector<int> next_common;
    std::vector<stPartyMatchingInfo> next_infos;
    stPartyMatchingInfo current;
    std::string tag;
    bool in_channel = false;
    bool in_region = false;
    bool channel_has_region = false;
    bool region_has_dungeons = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[common dungeon index]") {
            if (in_channel || !next_common.empty()) {
                return 0;
            }
            if (!ScanIntListUntil("[/common dungeon index]", &next_common) ||
                next_common.empty()) {
                return 0;
            }
        } else if (tag == "[channel name]") {
            if (in_channel) {
                return 0;
            }
            current.clear();
            if (!ScanRequiredString(&current.channel_name) ||
                current.channel_name.empty()) {
                return 0;
            }
            in_channel = true;
            channel_has_region = false;
            region_has_dungeons = false;
        } else if (tag == "[region name]") {
            if (!in_channel || in_region || channel_has_region) {
                return 0;
            }
            if (!ScanRequiredString(&current.region_name) ||
                current.region_name.empty()) {
                return 0;
            }
            in_region = true;
            channel_has_region = true;
        } else if (tag == "[dungeon index]") {
            if (!in_channel || !in_region || region_has_dungeons) {
                return 0;
            }
            if (!ScanIntListUntil("[/dungeon index]",
                                  &current.dungeon_indices) ||
                current.dungeon_indices.empty()) {
                return 0;
            }
            region_has_dungeons = true;
        } else if (tag == "[/region name]") {
            if (!in_region || !region_has_dungeons) {
                return 0;
            }
            in_region = false;
        } else if (tag == "[/channel name]") {
            if (!in_channel || in_region || !channel_has_region ||
                !region_has_dungeons) {
                return 0;
            }
            next_infos.push_back(current);
            current.clear();
            in_channel = false;
        } else {
            // Both official parsers have a closed tag vocabulary here.
            return 0;
        }
    }

    if (in_channel || in_region || next_common.empty() ||
        next_infos.empty()) {
        return 0;
    }

    common_dungeon_indices->swap(next_common);
    output->swap(next_infos);
    return 1;
}

}  // namespace

PremiumServiceData::PremiumServiceData()
    : type(0),
      main_premium_image_index(-1, -1),
      additional_premium_image_index(-1),
      icon_index(-1),
      icon_explain(),
      add_equipment_list(),
      selectable_equipment_list() {}

PremiumServiceData* PremiumServiceData::clear() {
    type = 0;
    main_premium_image_index.first = -1;
    main_premium_image_index.second = -1;
    additional_premium_image_index = -1;
    icon_index = -1;
    icon_explain.clear();
    add_equipment_list.clear();
    selectable_equipment_list.clear();
    return this;
}

LabyrinthIntPair0828::LabyrinthIntPair0828() : first(0), second(0) {}

LabyrinthIntTriple0828::LabyrinthIntTriple0828()
    : first(0), second(0), third(0) {}

LabyrinthReinforceCondition0828::LabyrinthReinforceCondition0828()
    : kind(LABYRINTH_CONDITION_ATTACK_SUCCESS),
      integer_value(0),
      string_value() {}

LabyrinthReinforceEffect0828::LabyrinthReinforceEffect0828()
    : kind(LABYRINTH_EFFECT_TARGET), string_values(), integer_values() {}

LabyrinthReinforceType0828::LabyrinthReinforceType0828()
    : pattern(-1),
      name(),
      conditions(),
      then_effects(),
      passive_effects() {}

void LabyrinthReinforceType0828::clear() {
    pattern = -1;
    name.clear();
    conditions.clear();
    then_effects.clear();
    passive_effects.clear();
}

LabyrinthEquipmentDefinition0828::LabyrinthEquipmentDefinition0828()
    : category(), fp_cost(0), reinforce_max(0), use_patterns() {}

void LabyrinthEquipmentDefinition0828::clear() {
    category.clear();
    fp_cost = 0;
    reinforce_max = 0;
    use_patterns.clear();
}

LabyrinthConfig0828::LabyrinthConfig0828()
    : character_exp_table(),
      monster_fp_exp_points(),
      level_up_skill_point(0),
      barrack_count(),
      tower_count(),
      barrack_fp_exp_points(),
      tower_fp_exp_points(),
      barrack_destroy_point(),
      reinforce_types(),
      equipment_definitions() {}

void LabyrinthConfig0828::clear() {
    character_exp_table.clear();
    monster_fp_exp_points.clear();
    level_up_skill_point = 0;
    barrack_count = LabyrinthIntPair0828();
    tower_count = LabyrinthIntPair0828();
    barrack_fp_exp_points.clear();
    tower_fp_exp_points.clear();
    barrack_destroy_point = LabyrinthIntPair0828();
    reinforce_types.clear();
    equipment_definitions.clear();
}

void LabyrinthConfig0828::swap(LabyrinthConfig0828& other) {
    character_exp_table.swap(other.character_exp_table);
    monster_fp_exp_points.swap(other.monster_fp_exp_points);
    std::swap(level_up_skill_point, other.level_up_skill_point);
    std::swap(barrack_count.first, other.barrack_count.first);
    std::swap(barrack_count.second, other.barrack_count.second);
    std::swap(tower_count.first, other.tower_count.first);
    std::swap(tower_count.second, other.tower_count.second);
    barrack_fp_exp_points.swap(other.barrack_fp_exp_points);
    tower_fp_exp_points.swap(other.tower_fp_exp_points);
    std::swap(barrack_destroy_point.first,
              other.barrack_destroy_point.first);
    std::swap(barrack_destroy_point.second,
              other.barrack_destroy_point.second);
    reinforce_types.swap(other.reinforce_types);
    equipment_definitions.swap(other.equipment_definitions);
}

ChaosFPShopEntry0828::ChaosFPShopEntry0828()
    : value0(0), value1(0), value2(0) {}

void ChaosFPShopCatalog0828::clear() {
    consume_items.clear();
    fairy_items.clear();
    improve_items.clear();
    special_items.clear();
}

void ChaosFPShopCatalog0828::swap(ChaosFPShopCatalog0828& other) {
    consume_items.swap(other.consume_items);
    fairy_items.swap(other.fairy_items);
    improve_items.swap(other.improve_items);
    special_items.swap(other.special_items);
}

int ImportLabyrinthConfig0828(LabyrinthConfig0828* output) {
    if (output == 0 ||
        !loadRDARScriptFile("", "Etc/Labyrinth.etc")) {
        return 0;
    }

    LabyrinthConfig0828 next;
    std::string tag;
    bool valid = true;
    bool saw_character_exp = false;
    bool saw_monster_fp = false;
    bool saw_level_skill = false;
    bool saw_barrack_count = false;
    bool saw_tower_count = false;
    bool saw_barrack_fp = false;
    bool saw_tower_fp = false;
    bool saw_barrack_destroy = false;

    while (valid && ScanType(tag, true) == 1) {
        if (tag == "[character exp table]") {
            valid = !saw_character_exp &&
                    ScanIntPairsUntil("[/character exp table]",
                                      &next.character_exp_table);
            saw_character_exp = valid;
        } else if (tag == "[monster fp exp point]") {
            valid = !saw_monster_fp &&
                    ScanIntTriplesUntil("[/monster fp exp point]",
                                        &next.monster_fp_exp_points);
            saw_monster_fp = valid;
        } else if (tag == "[level up skill point]") {
            valid = !saw_level_skill &&
                    ScanRequiredInt(&next.level_up_skill_point);
            saw_level_skill = valid;
        } else if (tag == "[barrack count]") {
            valid = !saw_barrack_count &&
                    ScanRequiredInt(&next.barrack_count.first) &&
                    ScanRequiredInt(&next.barrack_count.second);
            saw_barrack_count = valid;
        } else if (tag == "[tower count]") {
            valid = !saw_tower_count &&
                    ScanRequiredInt(&next.tower_count.first) &&
                    ScanRequiredInt(&next.tower_count.second);
            saw_tower_count = valid;
        } else if (tag == "[barrack fp exp point]") {
            valid = !saw_barrack_fp &&
                    ScanIntTriplesUntil("[/barrack fp exp point]",
                                        &next.barrack_fp_exp_points);
            saw_barrack_fp = valid;
        } else if (tag == "[tower fp exp point]") {
            valid = !saw_tower_fp &&
                    ScanIntTriplesUntil("[/tower fp exp point]",
                                        &next.tower_fp_exp_points);
            saw_tower_fp = valid;
        } else if (tag == "[barrack destroy point]") {
            std::vector<int> values;
            valid = !saw_barrack_destroy &&
                    ScanIntListUntil("[/barrack destroy point]", &values) &&
                    values.size() == 2u;
            if (valid) {
                next.barrack_destroy_point.first = values[0];
                next.barrack_destroy_point.second = values[1];
            }
            saw_barrack_destroy = valid;
        } else if (tag == "[equipment reinforce type]") {
            LabyrinthReinforceType0828 value;
            valid = ParseLabyrinthReinforceType(&value);
            if (valid) {
                next.reinforce_types.push_back(value);
            }
        } else if (tag == "[equipment definition]") {
            LabyrinthEquipmentDefinition0828 value;
            valid = ParseLabyrinthEquipmentDefinition(&value);
            if (valid) {
                next.equipment_definitions.push_back(value);
            }
        } else {
            valid = false;
        }
    }

    unloadRDARScriptFile();
    valid = valid && saw_character_exp && saw_monster_fp &&
            saw_level_skill && saw_barrack_count && saw_tower_count &&
            saw_barrack_fp && saw_tower_fp && saw_barrack_destroy &&
            ValidateLabyrinthConfig(next);
    if (!valid) {
        return 0;
    }
    output->swap(next);
    return 1;
}

int ImportChaosFPShopCatalog0828(ChaosFPShopCatalog0828* output) {
    if (output == 0 ||
        !loadRDARScriptFile("", "Etc/ChaosFPShop.etc")) {
        return 0;
    }

    ChaosFPShopCatalog0828 next;
    std::string tag;
    bool valid = true;
    bool saw_consume = false;
    bool saw_fairy = false;
    bool saw_improve = false;
    bool saw_special = false;
    while (valid && ScanType(tag, true) == 1) {
        if (tag == "[consume items]") {
            valid = !saw_consume && ScanChaosFPShopEntries(
                "[/consume items]", &next.consume_items);
            saw_consume = valid;
        } else if (tag == "[fairy items]") {
            valid = !saw_fairy && ScanChaosFPShopEntries(
                "[/fairy items]", &next.fairy_items);
            saw_fairy = valid;
        } else if (tag == "[improve items]") {
            valid = !saw_improve && ScanChaosFPShopEntries(
                "[/improve items]", &next.improve_items);
            saw_improve = valid;
        } else if (tag == "[special items]") {
            valid = !saw_special && ScanChaosFPShopEntries(
                "[/special items]", &next.special_items);
            saw_special = valid;
        } else {
            valid = false;
        }
    }
    unloadRDARScriptFile();

    if (!valid || !saw_consume || !saw_fairy || !saw_improve ||
        !saw_special) {
        return 0;
    }
    output->swap(next);
    return 1;
}

DamageFormulaConstants0828::DamageFormulaConstants0828() {
    clear();
}

void DamageFormulaConstants0828::clear() {
    for (int index = 0; index < DAMAGE_FORMULA_0828_COUNT; ++index) {
        values[index] = 0.0f;
    }
}

// IDA_EA: 0x80966FA (0627 server prefix).
// The old importer reads the first 26 values directly into old process
// globals.  The 0828 official file has exactly 29 values; parse the complete
// generation into temporary sidecar storage and publish only after the width
// and the next official tag have both been verified.
int ImportDamageFormulaConstants0828(DamageFormulaConstants0828* output) {
    if (output == 0 ||
        !loadRDARScriptFile("", "Etc/EtcParameter.etc")) {
        return 0;
    }

    DamageFormulaConstants0828 next;
    std::string tag;
    bool found = false;
    bool complete = false;
    for (;;) {
        int skipped_integer = 0;
        float skipped_float = 0.0f;
        const DNFLEX_TOKEN token =
            ScanAll(&skipped_integer, &skipped_float, &tag);
        if (token == DNF_TOK_EOF || token == DNF_TOK_ERROR) {
            break;
        }
        if (token != DNF_TOK_TYPE ||
            tag != "[damage fomula constant]") {
            continue;
        }
        if (found) {
            break;
        }
        found = true;

        bool valid = true;
        for (int index = 0; index < DAMAGE_FORMULA_0828_COUNT; ++index) {
            bool is_type = false;
            bool success = false;
            std::string type;
            const long double value =
                ScanTypeOrFloat(is_type, type, success, true);
            if (!success || is_type) {
                valid = false;
                break;
            }
            next.values[index] = static_cast<float>(value);
            if (!IsFiniteFloat(next.values[index])) {
                valid = false;
                break;
            }
        }
        if (!valid) {
            break;
        }

        // A 30th numeric silently accepted here would shift every following
        // tag.  The official 0828 layout places summoned-monster status next.
        bool next_is_type = false;
        bool next_success = false;
        std::string next_type;
        ScanTypeOrFloat(next_is_type, next_type, next_success, true);
        if (!next_success || !next_is_type ||
            next_type != "[summoned monster status up rate]") {
            break;
        }

        if (next.values[DAMAGE_FORMULA_PHYSICAL_DAMAGE_RATE] <= 0.0f ||
            next.values[DAMAGE_FORMULA_PHYSICAL_REDUCE_RATE] <= 0.0f ||
            next.values[DAMAGE_FORMULA_MAGICAL_DAMAGE_RATE] <= 0.0f ||
            next.values[DAMAGE_FORMULA_MAGICAL_REDUCE_RATE] <= 0.0f ||
            next.values[DAMAGE_FORMULA_0828_ORDINAL_27] < 0.0f ||
            next.values[DAMAGE_FORMULA_0828_ORDINAL_28] < 0.0f ||
            next.values[DAMAGE_FORMULA_MAX_TOTAL_DEFENSE_RATE] <= 0.0f ||
            next.values[DAMAGE_FORMULA_MAX_TOTAL_DEFENSE_RATE] > 1.0f) {
            break;
        }
        complete = true;
        break;
    }

    unloadRDARScriptFile();
    if (!found || !complete) {
        return 0;
    }
    *output = next;
    return 1;
}

// IDA_EA: 0x8912E47 (0627 server); 0828 client sub_5D6FB0.
int importPremiumServiceData(std::vector<PremiumServiceData>& output) {
    if (!loadRDARScriptFile("", "Etc/PremiumServiceEffect.etc")) {
        return 0;
    }

    std::vector<PremiumServiceData> next;
    PremiumServiceData current;
    std::string tag;
    bool in_service = false;
    bool saw_type = false;
    bool saw_main_image = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[premium service]") {
            if (in_service) {
                return 0;
            }
            current.clear();
            in_service = true;
            saw_type = false;
            saw_main_image = false;
        } else if (tag == "[/premium service]") {
            if (!in_service || !saw_type || !saw_main_image) {
                return 0;
            }
            next.push_back(current);
            current.clear();
            in_service = false;
        } else if (tag == "[type]") {
            if (!in_service || saw_type ||
                !ScanRequiredInt(&current.type)) {
                return 0;
            }
            saw_type = true;
        } else if (tag == "[icon index]") {
            if (!in_service || !ScanRequiredInt(&current.icon_index)) {
                return 0;
            }
        } else if (tag == "[icon explain]") {
            if (!in_service || !ScanRequiredString(&current.icon_explain)) {
                return 0;
            }
        } else if (tag == "[main premium image index]") {
            if (!in_service || saw_main_image ||
                !ScanRequiredInt(&current.main_premium_image_index.first) ||
                !ScanRequiredInt(&current.main_premium_image_index.second)) {
                return 0;
            }
            saw_main_image = true;
        } else if (tag == "[additional premium image index]") {
            if (!in_service ||
                !ScanRequiredInt(&current.additional_premium_image_index)) {
                return 0;
            }
        } else if (tag == "[add equipment list]") {
            if (!in_service) {
                return 0;
            }
            if (!ScanIntListUntil("[/add equipment list]",
                                  &current.add_equipment_list)) {
                return 0;
            }
        } else if (tag == "[add selectAble equipment list]") {
            if (!in_service) {
                return 0;
            }
            std::vector<int> selectable_values;
            if (!ScanIntListUntil("[/add selectAble equipment list]",
                                  &selectable_values) ||
                selectable_values.size() % 2u != 0u) {
                return 0;
            }
            for (std::size_t index = 0;
                 index < selectable_values.size(); index += 2u) {
                current.selectable_equipment_list.insert(std::make_pair(
                    selectable_values[index],
                    std::make_pair(false, selectable_values[index + 1u])));
            }
        } else {
            // The scalar tags in the official format intentionally have no
            // closing tag; any other token would otherwise be silently lost.
            return 0;
        }
    }

    if (in_service || next.empty()) {
        return 0;
    }

    output.swap(next);
    return 1;
}

// IDA_EA: 0x890F311 (0627 server); 0828 client sub_1501F20.
int importPartyMatchingInfo(const char* filename,
                            std::vector<stPartyMatchingInfo>* output) {
    std::vector<int> discarded_common_dungeons;
    return ImportPartyMatchingInfoWithCommon(
        filename, &discarded_common_dungeons, output);
}

// IDA_EA: 0x89174F5 (0627 server); 0828 client sub_5CD100.
int importQuestMovieList(QuestMovieMap& output) {
    if (!loadRDARScriptFile("", "Etc/QuestMovieList.etc")) {
        return 0;
    }

    QuestMovieMap next;
    QuestMovieInfo current;
    std::string tag;
    int quest_index = -1;
    ENUM_QUEST_MOVIE_EVENT event = QUEST_MOVIE_EVENT_INVALID;
    bool in_movie = false;
    bool in_play_info = false;
    bool saw_quest_index = false;
    bool saw_play_info = false;
    bool saw_condition = false;
    bool saw_title = false;
    bool saw_file_path = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[movie play]") {
            if (in_movie || in_play_info) {
                return 0;
            }
            quest_index = -1;
            in_movie = true;
            saw_quest_index = false;
            saw_play_info = false;
        } else if (tag == "[/movie play]") {
            if (!in_movie || in_play_info || !saw_quest_index ||
                !saw_play_info) {
                return 0;
            }
            in_movie = false;
        } else if (tag == "[quest index]") {
            if (!in_movie || in_play_info || saw_quest_index ||
                !ScanRequiredInt(&quest_index)) {
                return 0;
            }
            saw_quest_index = true;
        } else if (tag == "[/quest index]") {
            if (!in_movie || !saw_quest_index) {
                return 0;
            }
        } else if (tag == "[play info]") {
            if (!in_movie || in_play_info || !saw_quest_index) {
                return 0;
            }
            current.clear();
            event = QUEST_MOVIE_EVENT_INVALID;
            in_play_info = true;
            saw_condition = false;
            saw_title = false;
            saw_file_path = false;
        } else if (tag == "[/play info]") {
            if (!in_play_info || !saw_condition || !saw_title ||
                !saw_file_path || event == QUEST_MOVIE_EVENT_INVALID) {
                return 0;
            }
            next.insert(std::make_pair(
                QuestMovieKey(quest_index, event), current));
            in_play_info = false;
            saw_play_info = true;
        } else if (tag == "[condition]") {
            if (!in_play_info || saw_condition) {
                return 0;
            }
            std::string condition;
            if (!ScanRequiredString(&condition)) {
                return 0;
            }
            if (condition == "[on accept]") {
                event = QUEST_MOVIE_EVENT_ON_ACCEPT;
            } else if (condition == "[clear]") {
                event = QUEST_MOVIE_EVENT_CLEAR;
            } else if (condition == "[on complete]") {
                event = QUEST_MOVIE_EVENT_ON_COMPLETE;
            } else {
                return 0;
            }
            saw_condition = true;
        } else if (tag == "[/condition]") {
            if (!in_play_info || !saw_condition) {
                return 0;
            }
        } else if (tag == "[title]") {
            if (!in_play_info || saw_title ||
                !ScanRequiredString(&current.title)) {
                return 0;
            }
            saw_title = true;
        } else if (tag == "[/title]") {
            if (!in_play_info || !saw_title) {
                return 0;
            }
        } else if (tag == "[file path]") {
            if (!in_play_info || saw_file_path ||
                !ScanRequiredString(&current.file_path)) {
                return 0;
            }
            saw_file_path = true;
        } else if (tag == "[/file path]") {
            if (!in_play_info || !saw_file_path) {
                return 0;
            }
        } else {
            return 0;
        }
    }

    if (in_movie || in_play_info || next.empty()) {
        return 0;
    }

    output.swap(next);
    return 1;
}

// IDA_EA: 0x8902543 (0627 server); 0828 client sub_1511590.
int ImportWarRoomWPTable(stBloodWarGrowTable* output) {
    if (output == 0 ||
        !loadRDARScriptFile("", "Etc/WarRoomWPTable.tbl")) {
        return 0;
    }

    stBloodWarGrowTable next;
    std::string tag;
    bool saw_table = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[WP TABLE]") {
            if (saw_table) {
                return 0;
            }
            saw_table = true;
            std::vector<int> values;
            if (!ScanIntListUntil("[/WP TABLE]", &values) ||
                values.empty() || values.size() % 10u != 0u) {
                return 0;
            }
            for (std::size_t value_index = 0;
                 value_index < values.size(); value_index += 10u) {
                std::vector<std::pair<int, int> > row;
                for (std::size_t pair_offset = 0;
                     pair_offset < 10u; pair_offset += 2u) {
                    row.push_back(std::make_pair(
                        values[value_index + pair_offset],
                        values[value_index + pair_offset + 1u]));
                }
                next.rows.push_back(row);
            }
        } else {
            return 0;
        }
    }

    if (!saw_table || next.rows.empty()) {
        return 0;
    }
    output->rows.swap(next.rows);
    return 1;
}

void DataManagerEtcStore0828::clear() {
    damage_formula_constants.clear();
    premium_services.clear();
    common_party_matching_dungeon_indices.clear();
    party_matching_infos.clear();
    quest_movies.clear();
    war_room_wp_table.rows.clear();
    labyrinth.clear();
    chaos_fp_shop.clear();
}

void DataManagerEtcStore0828::swap(DataManagerEtcStore0828& other) {
    for (int index = 0; index < DAMAGE_FORMULA_0828_COUNT; ++index) {
        std::swap(damage_formula_constants.values[index],
                  other.damage_formula_constants.values[index]);
    }
    premium_services.swap(other.premium_services);
    common_party_matching_dungeon_indices.swap(
        other.common_party_matching_dungeon_indices);
    party_matching_infos.swap(other.party_matching_infos);
    quest_movies.swap(other.quest_movies);
    war_room_wp_table.rows.swap(other.war_room_wp_table.rows);
    labyrinth.swap(other.labyrinth);
    chaos_fp_shop.swap(other.chaos_fp_shop);
}

int ImportDataManagerEtcStore0828() {
    DataManagerEtcStore0828 next;
    if (ImportDamageFormulaConstants0828(
            &next.damage_formula_constants) != 1) {
        return TraceDataManagerEtcImportFailure(
            "damage_formula", "Etc/EtcParameter.etc");
    }
    if (importPremiumServiceData(next.premium_services) != 1) {
        return TraceDataManagerEtcImportFailure(
            "premium_service", "Etc/PremiumServiceEffect.etc");
    }
    if (ImportPartyMatchingInfoWithCommon(
            "Etc/PartyMatchingInfo.etc",
            &next.common_party_matching_dungeon_indices,
            &next.party_matching_infos) != 1) {
        return TraceDataManagerEtcImportFailure(
            "party_matching", "Etc/PartyMatchingInfo.etc");
    }
    if (importQuestMovieList(next.quest_movies) != 1) {
        return TraceDataManagerEtcImportFailure(
            "quest_movie", "Etc/QuestMovieList.etc");
    }
    if (ImportWarRoomWPTable(&next.war_room_wp_table) != 1) {
        return TraceDataManagerEtcImportFailure(
            "war_room_wp", "Etc/WarRoomWPTable.tbl");
    }
    if (ImportLabyrinthConfig0828(&next.labyrinth) != 1) {
        return TraceDataManagerEtcImportFailure(
            "labyrinth", "Etc/Labyrinth.etc");
    }
    if (ImportChaosFPShopCatalog0828(&next.chaos_fp_shop) != 1) {
        return TraceDataManagerEtcImportFailure(
            "chaos_fp_shop", "Etc/ChaosFPShop.etc");
    }

    g_datamanager_etc_store_0828.swap(next);
    return 1;
}

const DataManagerEtcStore0828& GetDataManagerEtcStore0828() {
    return g_datamanager_etc_store_0828;
}

const LabyrinthConfig0828& GetLabyrinthConfig0828() {
    return g_datamanager_etc_store_0828.labyrinth;
}

const LabyrinthReinforceType0828* FindLabyrinthReinforceType0828(
    int pattern) {
    const std::vector<LabyrinthReinforceType0828>& values =
        GetLabyrinthConfig0828().reinforce_types;
    for (std::vector<LabyrinthReinforceType0828>::const_iterator it =
             values.begin();
         it != values.end(); ++it) {
        if (it->pattern == pattern) {
            return &*it;
        }
    }
    return 0;
}

const LabyrinthEquipmentDefinition0828*
FindLabyrinthEquipmentDefinition0828(const std::string& category) {
    const std::vector<LabyrinthEquipmentDefinition0828>& values =
        GetLabyrinthConfig0828().equipment_definitions;
    for (std::vector<LabyrinthEquipmentDefinition0828>::const_iterator it =
             values.begin();
         it != values.end(); ++it) {
        if (it->category == category) {
            return &*it;
        }
    }
    return 0;
}

const ChaosFPShopCatalog0828& GetChaosFPShopCatalog0828() {
    return g_datamanager_etc_store_0828.chaos_fp_shop;
}

const std::vector<ChaosFPShopEntry0828>* GetChaosFPShopSection0828(
    ChaosFPShopSection0828 section) {
    const ChaosFPShopCatalog0828& catalog = GetChaosFPShopCatalog0828();
    switch (section) {
        case CHAOS_FP_SHOP_CONSUME:
            return &catalog.consume_items;
        case CHAOS_FP_SHOP_FAIRY:
            return &catalog.fairy_items;
        case CHAOS_FP_SHOP_IMPROVE:
            return &catalog.improve_items;
        case CHAOS_FP_SHOP_SPECIAL:
            return &catalog.special_items;
        default:
            return 0;
    }
}

const DamageFormulaConstants0828& GetDamageFormulaConstants0828() {
    return g_datamanager_etc_store_0828.damage_formula_constants;
}

float GetDamageFormulaConstant0828(unsigned int index) {
    if (index >= static_cast<unsigned int>(DAMAGE_FORMULA_0828_COUNT)) {
        return 0.0f;
    }
    return GetDamageFormulaConstants0828().values[index];
}

float GetMaxTotalDefenseRate0828() {
    const float value = GetDamageFormulaConstant0828(
        DAMAGE_FORMULA_MAX_TOTAL_DEFENSE_RATE);
    // Gameplay cannot reach this fallback after a successful DataManager
    // startup; it preserves the immutable 0627 value for pre-init callers.
    return value > 0.0f ? value : 0.01f;
}

int CDataManager::import_datamanager_etc_0828() {
    return ImportDataManagerEtcStore0828();
}

const DamageFormulaConstants0828&
CDataManager::get_damage_formula_constants_0828() const {
    return GetDamageFormulaConstants0828();
}

float CDataManager::get_damage_formula_constant_0828(
    unsigned int index) const {
    return GetDamageFormulaConstant0828(index);
}

const std::vector<PremiumServiceData>&
CDataManager::get_premium_service_data_0828() const {
    return GetDataManagerEtcStore0828().premium_services;
}

const PremiumServiceData*
CDataManager::find_premium_service_data_0828(int type) const {
    const std::vector<PremiumServiceData>& services =
        GetDataManagerEtcStore0828().premium_services;
    for (std::vector<PremiumServiceData>::const_iterator it = services.begin();
         it != services.end(); ++it) {
        if (it->type == type) {
            return &*it;
        }
    }
    return 0;
}

const std::vector<int>&
CDataManager::get_common_party_matching_dungeons_0828() const {
    return GetDataManagerEtcStore0828()
        .common_party_matching_dungeon_indices;
}

bool CDataManager::is_common_party_matching_dungeon_0828(
    int dungeon_index) const {
    return ContainsInt(
        GetDataManagerEtcStore0828()
            .common_party_matching_dungeon_indices,
        dungeon_index);
}

const std::vector<stPartyMatchingInfo>&
CDataManager::get_party_matching_info_0828() const {
    return GetDataManagerEtcStore0828().party_matching_infos;
}

const stPartyMatchingInfo*
CDataManager::find_party_matching_info_0828(
    const std::string& channel_name,
    const std::string& region_name) const {
    const std::vector<stPartyMatchingInfo>& infos =
        GetDataManagerEtcStore0828().party_matching_infos;
    for (std::vector<stPartyMatchingInfo>::const_iterator it = infos.begin();
         it != infos.end(); ++it) {
        if (it->channel_name == channel_name &&
            it->region_name == region_name) {
            return &*it;
        }
    }
    return 0;
}

const QuestMovieMap& CDataManager::get_quest_movie_list_0828() const {
    return GetDataManagerEtcStore0828().quest_movies;
}

const QuestMovieInfo* CDataManager::find_quest_movie_0828(
    int quest_index,
    ENUM_QUEST_MOVIE_EVENT event) const {
    const QuestMovieMap& movies =
        GetDataManagerEtcStore0828().quest_movies;
    const QuestMovieMap::const_iterator it =
        movies.find(QuestMovieKey(quest_index, event));
    return it == movies.end() ? 0 : &it->second;
}

const stBloodWarGrowTable&
CDataManager::get_war_room_wp_table_0828() const {
    return GetDataManagerEtcStore0828().war_room_wp_table;
}

bool CDataManager::get_war_room_wp_pair_0828(
    unsigned int row_index,
    unsigned int pair_index,
    std::pair<int, int>* output) const {
    if (output == 0) {
        return false;
    }
    const std::vector<std::vector<std::pair<int, int> > >& rows =
        GetDataManagerEtcStore0828().war_room_wp_table.rows;
    if (row_index >= rows.size() || pair_index >= rows[row_index].size()) {
        return false;
    }
    *output = rows[row_index][pair_index];
    return true;
}

const LabyrinthConfig0828&
CDataManager::get_labyrinth_config_0828() const {
    return GetLabyrinthConfig0828();
}

const LabyrinthReinforceType0828*
CDataManager::find_labyrinth_reinforce_type_0828(int pattern) const {
    return FindLabyrinthReinforceType0828(pattern);
}

const LabyrinthEquipmentDefinition0828*
CDataManager::find_labyrinth_equipment_definition_0828(
    const std::string& category) const {
    return FindLabyrinthEquipmentDefinition0828(category);
}

const ChaosFPShopCatalog0828&
CDataManager::get_chaos_fp_shop_catalog_0828() const {
    return GetChaosFPShopCatalog0828();
}

const std::vector<ChaosFPShopEntry0828>*
CDataManager::get_chaos_fp_shop_section_0828(
    ChaosFPShopSection0828 section) const {
    return GetChaosFPShopSection0828(section);
}
