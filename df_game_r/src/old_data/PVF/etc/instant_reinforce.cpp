#include "old_data/PVF/etc/instant_reinforce.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/manager/change_status_data.h"
#include "old_data/PVF/common/rdar_script.h"

namespace instant_reinforce {
namespace {

typedef std::map<int, std::vector<Rule> > RuleTable;

RuleTable& Rules() {
    static RuleTable rules;
    return rules;
}

bool& LoadedFlag() {
    static bool loaded = false;
    return loaded;
}

unsigned int& RuleCount() {
    static unsigned int count = 0;
    return count;
}

class ScriptUnloadGuard {
public:
    ScriptUnloadGuard() {}
    ~ScriptUnloadGuard() { unloadRDARScriptFile(); }

private:
    ScriptUnloadGuard(const ScriptUnloadGuard&);
    ScriptUnloadGuard& operator=(const ScriptUnloadGuard&);
};

bool ReadInt(int* output) {
    bool success = false;
    const int value = ScanInt(&success);
    if (!success) return false;
    *output = value;
    return true;
}

bool ReadType(std::string* output) {
    int token_type = 3;
    int unused_value = 0;
    return ScanAnyData(token_type, *output, unused_value, true) == 1 &&
           token_type == 2;
}

bool ReadTypeOrInt(bool* is_type,
                   std::string* type_text,
                   int* int_value) {
    int token_type = 3;
    if (ScanAnyData(token_type, *type_text, *int_value, true) != 1 ||
        (token_type != 1 && token_type != 2)) {
        return false;
    }
    *is_type = token_type == 2;
    return true;
}

bool IsProjectedStatusType(int type) {
    switch (type) {
        case 0:   // physical attack
        case 1:   // magical attack
        case 10:  // attack speed
        case 11:  // move speed
        case 12:  // cast speed
        case 13:  // hp max
        case 48:  // mp max
            return true;
        default:
            return false;
    }
}

bool ParseCondition(Rule* rule) {
    bool saw_revive_condition = false;
    bool saw_select_index = false;
    std::string tag;
    while (ReadType(&tag)) {
        if (tag == "[/condition]") {
            rule->on_revive_after_corpse = saw_revive_condition;
            return saw_revive_condition;
        }
        if (tag == "[on revive after corpse]") {
            if (saw_revive_condition) return false;
            saw_revive_condition = true;
        } else if (tag == "[select index]") {
            if (saw_select_index || !ReadInt(&rule->select_index) ||
                rule->select_index < 0 || rule->select_index > 255) {
                return false;
            }
            saw_select_index = true;
        } else {
            return false;
        }
    }
    return false;
}

bool ParseSkills(Rule* rule) {
    if (!ReadInt(&rule->skill_header) || rule->skill_header < 0 ||
        rule->skill_header > 255) {
        return false;
    }

    std::set<int> skill_ids;
    while (1) {
        bool is_type = false;
        std::string token;
        int skill_id = 0;
        if (!ReadTypeOrInt(&is_type, &token, &skill_id)) return false;
        if (is_type) {
            return token == "[/skill]" && !rule->skills.empty();
        }

        int level = 0;
        if (!ReadTypeOrInt(&is_type, &token, &level) || is_type ||
            skill_id <= 0 || skill_id > 1000000 ||
            level <= 0 || level > 255 ||
            !skill_ids.insert(skill_id).second ||
            rule->skills.size() >= 256u) {
            return false;
        }
        rule->skills.push_back(SkillGrant(skill_id, level));
    }
}

bool ParseStatuses(Rule* rule) {
    std::set<int> status_types;
    std::string tag;
    while (ReadType(&tag)) {
        if (tag == "[/status]") return !rule->statuses.empty();
        if (tag.size() < 3u || tag[0] != '[' ||
            tag[tag.size() - 1u] != ']' || tag[1] == '/') {
            return false;
        }

        std::string status_name = tag.substr(1u, tag.size() - 2u);
        const int status_type = getChangeStatusType(status_name);
        std::string operation;
        int additive_value = 0;
        if (!IsProjectedStatusType(status_type) ||
            !status_types.insert(status_type).second ||
            ScanStr(&operation) != 1 || operation != "+" ||
            !ReadInt(&additive_value) || additive_value <= 0) {
            return false;
        }
        rule->statuses.push_back(StatusGrant(status_type, additive_value));
    }
    return false;
}

bool ParseEffects(Rule* rule) {
    std::set<int> effect_ids;
    while (1) {
        bool is_type = false;
        std::string token;
        int effect_id = 0;
        if (!ReadTypeOrInt(&is_type, &token, &effect_id)) return false;
        if (is_type) {
            return token == "[/effect]" && !rule->effects.empty();
        }
        if (effect_id < 0 || effect_id > 1000000 ||
            !effect_ids.insert(effect_id).second) {
            return false;
        }
        rule->effects.push_back(effect_id);
    }
}

bool ParseRuleAfterMapTag(Rule* rule) {
    std::string tag;
    if (!ReadInt(&rule->map_index) || rule->map_index <= 0 ||
        !ReadType(&tag) ||
        tag != "[condition]" || !ParseCondition(rule) ||
        !ReadType(&tag) || tag != "[skill]" ||
        !ParseSkills(rule) || !ReadType(&tag) ||
        tag != "[status]" || !ParseStatuses(rule) ||
        !ReadType(&tag) || tag != "[effect]" ||
        !ParseEffects(rule)) {
        return false;
    }
    return true;
}

bool ValidateRules(const RuleTable& table, unsigned int count) {
    if (table.empty() || count == 0u || count > 4096u) return false;

    unsigned int checked_count = 0;
    for (RuleTable::const_iterator map = table.begin(); map != table.end();
         ++map) {
        const std::vector<Rule>& variants = map->second;
        if (variants.empty() || variants.size() > 256u) return false;
        checked_count += static_cast<unsigned int>(variants.size());

        std::set<int> selectors;
        for (std::vector<Rule>::const_iterator rule = variants.begin();
             rule != variants.end(); ++rule) {
            if (rule->map_index != map->first ||
                !rule->on_revive_after_corpse || rule->skills.empty() ||
                rule->statuses.empty() || rule->effects.empty()) {
                return false;
            }
            if (variants.size() > 1u) {
                if (rule->select_index < 0 ||
                    !selectors.insert(rule->select_index).second) {
                    return false;
                }
            } else if (rule->select_index >= 0) {
                selectors.insert(rule->select_index);
            }
        }
    }
    return checked_count == count;
}

}  // namespace

SkillGrant::SkillGrant() : skill_index(0), level(0) {}

SkillGrant::SkillGrant(int skill, int granted_level)
    : skill_index(skill), level(granted_level) {}

StatusGrant::StatusGrant() : status_type(77), additive_value(0) {}

StatusGrant::StatusGrant(int type, int value)
    : status_type(type), additive_value(value) {}

Rule::Rule()
    : map_index(0),
      select_index(-1),
      on_revive_after_corpse(false),
      skill_header(12),
      skills(),
      statuses(),
      effects() {}

bool ImportConfig(const char* filename) {
    if (!filename || !loadRDARScriptFile("", filename)) return false;
    ScriptUnloadGuard unload_guard;

    RuleTable parsed;
    unsigned int parsed_count = 0;
    std::string tag;
    if (!ReadType(&tag) || tag != "[reinforce info]") {
        return false;
    }

    bool saw_close = false;
    while (ReadType(&tag)) {
        if (tag == "[/reinforce info]") {
            saw_close = true;
            break;
        }
        if (tag != "[map index]") return false;

        Rule rule;
        if (!ParseRuleAfterMapTag(&rule) || parsed_count >= 4096u) {
            return false;
        }
        parsed[rule.map_index].push_back(rule);
        ++parsed_count;
    }

    if (!saw_close || !ValidateRules(parsed, parsed_count)) return false;
    // ScanType must observe EOF after the top-level closing tag.  Extra PVF
    // sections are rejected rather than silently changing the packet policy.
    int extra_type = 3;
    int extra_value = 0;
    if (ScanAnyData(extra_type, tag, extra_value, false) == 1) return false;

    Rules().swap(parsed);
    RuleCount() = parsed_count;
    LoadedFlag() = true;
    return true;
}

bool IsConfigLoaded() { return LoadedFlag(); }

unsigned int GetRuleCount() {
    return LoadedFlag() ? RuleCount() : 0u;
}

unsigned int GetMapVariantCount(int map_index) {
    if (!LoadedFlag()) return 0u;
    const RuleTable& table = Rules();
    const RuleTable::const_iterator found = table.find(map_index);
    return found == table.end()
               ? 0u
               : static_cast<unsigned int>(found->second.size());
}

bool HasMap(int map_index) {
    return GetMapVariantCount(map_index) != 0u;
}

bool ResolveRule(int map_index, int select_index, Rule* output) {
    if (!LoadedFlag() || !output) return false;
    const RuleTable& table = Rules();
    const RuleTable::const_iterator found = table.find(map_index);
    if (found == table.end() || found->second.empty()) return false;

    const std::vector<Rule>& variants = found->second;
    if (variants.size() == 1u && variants[0].select_index < 0) {
        *output = variants[0];
        return true;
    }

    for (std::vector<Rule>::const_iterator rule = variants.begin();
         rule != variants.end(); ++rule) {
        if (rule->select_index == select_index) {
            *output = *rule;
            return true;
        }
    }
    return false;
}

}  // namespace instant_reinforce_0828
