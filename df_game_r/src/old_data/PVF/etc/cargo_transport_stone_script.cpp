#include "old_data/PVF/etc/cargo_transport_stone_script.h"

#include <limits.h>

#include <string>
#include <utility>

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"


namespace cargo_transport {
namespace {

typedef std::map<int, Rule> RuleTable;

RuleTable& Rules() {
    static RuleTable rules;
    return rules;
}

bool& LoadedFlag() {
    static bool loaded = false;
    return loaded;
}

std::set<int>& EquipmentGrowthPrototypes() {
    static std::set<int> item_ids;
    return item_ids;
}

bool ReadIntPairs(const char* closing_tag,
                  std::map<int, int>* values,
                  bool grade_table) {
    while (1) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int key = ScanTypeOrInt(is_type, token, success, true);
        if (!success) return false;
        if (is_type) return token == closing_tag && !values->empty();

        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (!success || is_type || value <= 0) return false;
        if (grade_table) {
            if (key < 1 || key > 100 || (key - 1) % 10 != 0) return false;
        } else if (key < 0 || key > 20) {
            return false;
        }
        if (!values->insert(std::make_pair(key, value)).second) return false;
    }
}

bool ReadIntSet(const char* closing_tag, std::set<int>* values) {
    while (1) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (!success) return false;
        if (is_type) return token == closing_tag;
        if (value <= 0 || !values->insert(value).second) return false;
    }
}

bool ReadEquipmentTypes(const char* closing_tag, std::set<int>* values) {
    while (1) {
        bool is_type = false;
        std::string token;
        if (ScanTypeOrStr(is_type, token, true) != 1) return false;
        if (is_type) return token == closing_tag && !values->empty();

        const int equipment_type = GetEquipmentType(token.c_str());
        if (equipment_type < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
            equipment_type > oldsrv::config::kOrdinaryEquipmentTypeLast ||
            !values->insert(equipment_type).second) {
            return false;
        }
    }
}

bool ReadExcludedProperties(const char* closing_tag, Rule* rule) {
    while (1) {
        bool is_type = false;
        std::string token;
        if (ScanTypeOrStr(is_type, token, true) != 1) return false;
        if (is_type) return token == closing_tag;
        if (token != "growthType" || rule->excludes_growth_type) return false;
        rule->excludes_growth_type = true;
    }
}

enum RuleField {
    kFieldGrade = 1 << 0,
    kFieldRarity = 1 << 1,
    kFieldEquipmentType = 1 << 2,
    kFieldExceptIndex = 1 << 3,
    kFieldExceptEtc = 1 << 4,
    kFieldIncludeIndex = 1 << 5,
    kFieldUiIndex = 1 << 6,
};

bool ParseRule(Rule* rule) {
    int fields = 0;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/stone type]") {
            const int required_fields = kFieldGrade | kFieldRarity |
                kFieldEquipmentType | kFieldExceptIndex | kFieldExceptEtc |
                kFieldUiIndex;
            return (fields & required_fields) == required_fields &&
                   rule->ui_index > 0;
        }
        if (tag == "[cargo transport stone grade]") {
            if ((fields & kFieldGrade) != 0 ||
                !ReadIntPairs("[/cargo transport stone grade]",
                              &rule->grade_brackets, true)) {
                return false;
            }
            fields |= kFieldGrade;
        } else if (tag == "[cargo transport stone rarity weight]") {
            if ((fields & kFieldRarity) != 0 ||
                !ReadIntPairs("[/cargo transport stone rarity weight]",
                              &rule->rarity_weights, false)) {
                return false;
            }
            fields |= kFieldRarity;
        } else if (tag == "[cargo transport stone enable equip type]") {
            if ((fields & kFieldEquipmentType) != 0 ||
                !ReadEquipmentTypes(
                    "[/cargo transport stone enable equip type]",
                    &rule->enabled_equipment_types)) {
                return false;
            }
            fields |= kFieldEquipmentType;
        } else if (tag == "[cargo transport stone except index]") {
            if ((fields & kFieldExceptIndex) != 0 ||
                !ReadIntSet("[/cargo transport stone except index]",
                            &rule->excluded_item_ids)) {
                return false;
            }
            fields |= kFieldExceptIndex;
        } else if (tag == "[cargo transport stone include index]") {
            if ((fields & kFieldIncludeIndex) != 0 ||
                !ReadIntSet("[/cargo transport stone include index]",
                            &rule->included_item_ids)) {
                return false;
            }
            fields |= kFieldIncludeIndex;
        } else if (tag == "[cargo transport stone except etc]") {
            if ((fields & kFieldExceptEtc) != 0 ||
                !ReadExcludedProperties(
                    "[/cargo transport stone except etc]", rule)) {
                return false;
            }
            fields |= kFieldExceptEtc;
        } else if (tag == "[ui index]") {
            bool success = false;
            if ((fields & kFieldUiIndex) != 0) return false;
            rule->ui_index = ScanInt(&success);
            if (!success || rule->ui_index <= 0 ||
                ScanType(tag, true) != 1 || tag != "[/ui index]") {
                return false;
            }
            fields |= kFieldUiIndex;
        } else {
            return false;
        }
    }
    return false;
}

bool ValidateRuleTable(const RuleTable& rules) {
    if (rules.size() != 3u) return false;
    for (int type = 0; type != 3; ++type) {
        RuleTable::const_iterator found = rules.find(type);
        if (found == rules.end()) return false;
        const Rule& rule = found->second;
        if (rule.grade_brackets.empty() || rule.rarity_weights.empty() ||
            rule.enabled_equipment_types.empty()) {
            return false;
        }
        for (std::map<int, int>::const_iterator grade =
                 rule.grade_brackets.begin();
             grade != rule.grade_brackets.end(); ++grade) {
            for (std::map<int, int>::const_iterator rarity =
                     rule.rarity_weights.begin();
                 rarity != rule.rarity_weights.end(); ++rarity) {
                if (grade->second > INT_MAX / rarity->second ||
                    grade->second * rarity->second > 0xffff) {
                    return false;
                }
            }
        }
    }
    return true;
}

}  // namespace

Rule::Rule()
    : stone_type(-1),
      grade_brackets(),
      rarity_weights(),
      enabled_equipment_types(),
      excluded_item_ids(),
      included_item_ids(),
      excludes_growth_type(false),
      ui_index(0) {}

bool ImportCargoTransportStoneScript(const char* filename) {
    if (!filename || !loadRDARScriptFile("", filename)) return false;

    RuleTable parsed_rules;
    bool valid = true;
    std::string tag;
    while (valid && ScanType(tag, true) == 1) {
        if (tag != "[stone type]") {
            valid = false;
            break;
        }
        bool success = false;
        Rule rule;
        rule.stone_type = ScanInt(&success);
        if (!success || rule.stone_type < 0 || rule.stone_type > 2 ||
            !ParseRule(&rule) ||
            !parsed_rules.insert(std::make_pair(rule.stone_type, rule)).second) {
            valid = false;
        }
    }
    unloadRDARScriptFile();

    if (!valid || !ValidateRuleTable(parsed_rules)) return false;
    Rules().swap(parsed_rules);
    LoadedFlag() = true;
    return true;
}

bool IsLoaded() {
    return LoadedFlag();
}

const Rule* FindRule(int stone_type) {
    if (!LoadedFlag()) return 0;
    const RuleTable& rules = Rules();
    RuleTable::const_iterator found = rules.find(stone_type);
    return found == rules.end() ? 0 : &found->second;
}

int GetRequiredMaterialCount(const Rule& rule,
                             const CEquipItem& equipment,
                             int item_id) {
    if (rule.excluded_item_ids.find(item_id) !=
        rule.excluded_item_ids.end()) {
        return -1;
    }

    const int level = equipment.getUsableLevel();
    if (level <= 0) return -1;
    const int bracket_start = ((level - 1) / 10) * 10 + 1;
    const std::map<int, int>::const_iterator grade =
        rule.grade_brackets.find(bracket_start);
    const std::map<int, int>::const_iterator rarity =
        rule.rarity_weights.find(equipment.get_rarity());
    if (grade == rule.grade_brackets.end() ||
        rarity == rule.rarity_weights.end()) {
        return -1;
    }

    bool eligible = rule.enabled_equipment_types.find(
                        equipment.GetItemType()) !=
                    rule.enabled_equipment_types.end();
    if (eligible && rule.excludes_growth_type &&
        IsEquipmentGrowthPrototype(item_id)) {
        eligible = false;
    }
    // CLIENT_IDA_EVIDENCE: sub_15EDDD0 walks include-index last and turns
    // the result back on for an exact item ID.  Except-index already failed
    // above in the required-count stage and therefore cannot be overridden.
    if (!eligible && rule.included_item_ids.find(item_id) !=
                         rule.included_item_ids.end()) {
        eligible = true;
    }
    if (!eligible) return -1;
    return grade->second * rarity->second;
}

void RegisterEquipmentGrowthPrototype(int item_id,
                                      uint8_t growth_type,
                                      uint8_t growth_stage,
                                      bool has_expand_path) {
    std::set<int>& item_ids = EquipmentGrowthPrototypes();
    item_ids.erase(item_id);
    if (item_id > 0 &&
        (has_expand_path || (growth_type > 0 && growth_stage > 0))) {
        item_ids.insert(item_id);
    }
}

bool IsEquipmentGrowthPrototype(int item_id) {
    return EquipmentGrowthPrototypes().find(item_id) !=
           EquipmentGrowthPrototypes().end();
}

}  // namespace cargo_transport
