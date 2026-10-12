#include "old_data/PVF/skill/skill_script.h"

#include <algorithm>
#include <cstdio>

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

extern char g_szSkillScriptBaseDirectory[256];
extern STScriptFileList
    g_stSkillScriptFileList[oldsrv::config::kCharacterJobCount];

namespace {

typedef std::pair<int, int> FixedLevelSkillKey;
std::map<FixedLevelSkillKey, FixedLevelSkillPolicy>
    g_fixed_level_skill_policies;

struct SkillAddDataKey0828 {
    int character_job;
    int skill_index;
    int module_type;

    bool operator<(const SkillAddDataKey0828& other) const {
        if (character_job != other.character_job) {
            return character_job < other.character_job;
        }
        if (skill_index != other.skill_index) {
            return skill_index < other.skill_index;
        }
        return module_type < other.module_type;
    }
};

std::map<SkillAddDataKey0828, SkillAddDataExtension0828>
    g_skill_add_data_extensions_0828;

bool ReadSkillAddDataRules0828(
    const char* close_tag,
    std::vector<SkillAddDataRule0828>* rules) {
    if (!close_tag || !rules) return false;

    std::vector<SkillAddDataRule0828> parsed;
    while (true) {
        int target_data_index = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&target_data_index, &float_value, &text);
        if (token == DNF_TOK_TYPE) {
            if (text != close_tag) return false;
            rules->swap(parsed);
            return true;
        }
        if (token != DNF_TOK_DECIMAL && token != DNF_TOK_HEXADECIMAL) {
            return false;
        }

        SkillAddDataRule0828 rule;
        rule.target_data_index = target_data_index;
        if (ScanStr(&text) != 1) return false;
        if (text == "+") {
            rule.operation = SKILL_ADD_DATA_PLUS_0828;
        } else if (text == "+%") {
            rule.operation = SKILL_ADD_DATA_PLUS_PERCENT_0828;
        } else {
            return false;
        }

        bool success = false;
        rule.source_skill_index = ScanInt(&success);
        if (!success) return false;
        const int source_variant = ScanInt(&success);
        if (!success) return false;
        rule.source_variant = source_variant != 0;
        rule.source_data_index = ScanInt(&success);
        if (!success) return false;

        // The client record constructor is exactly 60 bytes (five protected
        // scalar cells).  Keep each record bounded and reject pathological
        // input instead of allowing an unbounded startup allocation.
        if (parsed.size() >= 4096u) return false;
        parsed.push_back(rule);
    }
}

void PublishSkillAddDataExtension0828(
    int character_job,
    int skill_index,
    int module_type,
    bool extension_seen,
    const SkillAddDataExtension0828& extension) {
    SkillAddDataKey0828 key = {character_job, skill_index, module_type};
    if (!extension_seen) {
        g_skill_add_data_extensions_0828.erase(key);
        return;
    }
    g_skill_add_data_extensions_0828[key] = extension;
}

bool ReadFixedLevelSkillPolicy(FixedLevelSkillPolicy& policy) {
    policy.interval_level = 0;
    policy.add_level_per_interval = 0;
    policy.apply_contract_skillmaster = false;
    bool interval_seen = false;
    bool add_seen = false;
    bool contract_seen = false;
    bool closed = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/fixed level skill]") {
            closed = true;
            break;
        }
        bool success = false;
        if (tag == "[interval level]") {
            if (interval_seen) return false;
            policy.interval_level = ScanInt(&success);
            interval_seen = success;
        } else if (tag == "[add level per interval]") {
            if (add_seen) return false;
            policy.add_level_per_interval = ScanInt(&success);
            add_seen = success;
        } else if (tag == "[is apply contract skillmaster]") {
            if (contract_seen) return false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) return false;
            policy.apply_contract_skillmaster = value == 1;
            contract_seen = true;
        } else {
            return false;
        }
        if (!success) return false;
    }
    return closed && interval_seen && add_seen && contract_seen &&
           policy.interval_level > 0 &&
           policy.add_level_per_interval > 0;
}

void ReadIntVector(std::vector<int>& values) {
    bool success = false;
    values.clear();
    do {
        const int value = ScanInt(&success);
        if (success) {
            values.push_back(value);
        }
    } while (success);
}

void ReadSkillLevelFeature(std::vector<SkillLevelFeature>& features) {
    bool success = false;
    const int min_level = ScanInt(&success);
    if (!success) {
        return;
    }

    SkillLevelFeature feature;
    feature.min_level = min_level;
    feature.max_level = ScanInt(static_cast<bool*>(0));
    ScanStr(&feature.property_name);

    do {
        const int begin = ScanInt(&success);
        if (success) {
            const int end = ScanInt(static_cast<bool*>(0));
            const float value = ScanFloat(static_cast<bool*>(0));
            feature.property_ranges.push_back(std::make_pair(begin, end));
            feature.property_values.push_back(value);
        }
    } while (success);

    features.erase(
        std::remove_if(features.begin(),
                       features.end(),
                       [&feature](const SkillLevelFeature& current) {
                           return current.max_level >= feature.min_level &&
                                  current.min_level <= feature.max_level;
                       }),
        features.end());
    features.push_back(feature);
}

void ReadLevelInfo(std::vector<std::vector<float> >& level_info,
                   std::vector<float>& values) {
    level_info.clear();
    const int level_count = ScanInt(static_cast<bool*>(0));
    level_info.resize(level_count);

    bool success = false;
    values.clear();
    do {
        const float value = ScanFloat(&success);
        if (success) {
            values.push_back(value);
        }
    } while (success);

    const std::size_t value_count = values.size();
    for (int level = 0; level < level_count; ++level) {
        std::vector<float>& output = level_info[level];
        const std::size_t row_size = value_count / level_count;
        output.resize(row_size, 0.0f);
        for (std::size_t column = 0; column < row_size; ++column) {
            output[column] = values[level + column * level_count];
        }
    }
}

}  // namespace

bool GetFixedLevelSkillPolicy(int character_job,
                              int skill_index,
                              FixedLevelSkillPolicy* policy) {
    if (!policy) return false;
    const std::map<FixedLevelSkillKey, FixedLevelSkillPolicy>::const_iterator
        found = g_fixed_level_skill_policies.find(
            std::make_pair(character_job, skill_index));
    if (found == g_fixed_level_skill_policies.end()) return false;
    *policy = found->second;
    return true;
}

void ResetSkillAddDataRegistry0828() {
    g_skill_add_data_extensions_0828.clear();
}

bool GetSkillAddDataExtension0828(int character_job,
                                  int skill_index,
                                  int module_type,
                                  SkillAddDataExtension0828* extension) {
    if (!extension) return false;
    const SkillAddDataKey0828 key = {
        character_job, skill_index, module_type};
    const std::map<SkillAddDataKey0828,
                   SkillAddDataExtension0828>::const_iterator found =
        g_skill_add_data_extensions_0828.find(key);
    if (found == g_skill_add_data_extensions_0828.end()) return false;
    *extension = found->second;
    return true;
}

bool EvaluateSkillAddDataRule0828(
    const SkillAddDataRule0828& rule,
    const std::vector<std::vector<float> >& source_level_info,
    int source_skill_level,
    float base_value,
    float* result) {
    if (!result || rule.source_variant || source_skill_level <= 0 ||
        rule.source_data_index < 0 ||
        static_cast<std::size_t>(rule.source_data_index) >=
            source_level_info.size()) {
        return false;
    }
    const std::vector<float>& source_column =
        source_level_info[rule.source_data_index];
    const std::size_t level_index =
        static_cast<std::size_t>(source_skill_level - 1);
    if (level_index >= source_column.size()) return false;

    const float source_value = source_column[level_index];
    if (rule.operation == SKILL_ADD_DATA_PLUS_0828) {
        *result = base_value + source_value;
        return true;
    }
    if (rule.operation == SKILL_ADD_DATA_PLUS_PERCENT_0828) {
        *result = base_value + base_value * source_value / 100.0f;
        return true;
    }
    return false;
}

// IDA_EA: 0x83DE574
// IDA_NAME: _ZN17SkillLevelFeatureD2Ev
// DEMANGLED: SkillLevelFeature::~SkillLevelFeature()
SkillLevelFeature::~SkillLevelFeature() {}

// IDA_EA: 0x8A9B164
// IDA_NAME: _ZN17SkillLevelFeature5clearEv
// DEMANGLED: SkillLevelFeature::clear(void)
SkillLevelFeature* SkillLevelFeature::clear() {
    min_level = 0;
    max_level = 0;
    property_name.clear();
    property_ranges.clear();
    property_values.clear();
    return this;
}

// IDA_EA: 0x8A9B1C2
// IDA_NAME: _ZN17SkillLevelFeatureC2Ev
// DEMANGLED: SkillLevelFeature::SkillLevelFeature(void)
SkillLevelFeature::SkillLevelFeature()
    : min_level(0),
      max_level(0),
      property_name(),
      property_ranges(),
      property_values() {
    clear();
}

// IDA_EA: 0x8A9B6D6
// IDA_NAME: _ZN17SkillLevelFeatureC2ERKS_
// DEMANGLED: SkillLevelFeature::SkillLevelFeature(SkillLevelFeature const&)
SkillLevelFeature::SkillLevelFeature(const SkillLevelFeature& other)
    : min_level(other.min_level),
      max_level(other.max_level),
      property_name(other.property_name),
      property_ranges(other.property_ranges),
      property_values(other.property_values) {}

// IDA_EA: 0x8A9B7D4
// IDA_NAME: _ZN17SkillLevelFeatureaSERKS_
// DEMANGLED: SkillLevelFeature::operator=(SkillLevelFeature const&)
SkillLevelFeature& SkillLevelFeature::operator=(
    const SkillLevelFeature& other) {
    min_level = other.min_level;
    max_level = other.max_level;
    property_name = other.property_name;
    property_ranges = other.property_ranges;
    property_values = other.property_values;
    return *this;
}

// IDA_EA: 0x8A9B256
// IDA_NAME: _ZN20STPassiveFeatureData5clearEv
// DEMANGLED: STPassiveFeatureData::clear(void)
STPassiveFeatureData* STPassiveFeatureData::clear() {
    applies_to_all = true;
    skill_index = 0;
    operation = 0;
    value = 0;
    return this;
}

// IDA_EA: 0x8A9B280
// IDA_NAME: _ZN20STPassiveFeatureDataC2Ev
// DEMANGLED: STPassiveFeatureData::STPassiveFeatureData(void)
STPassiveFeatureData::STPassiveFeatureData() { clear(); }

// IDA_EA: 0x8A9CC80
// IDA_NAME: _ZN20STPassiveFeatureDataD2Ev
// DEMANGLED: STPassiveFeatureData::~STPassiveFeatureData()
STPassiveFeatureData::~STPassiveFeatureData() {}

// IDA_EA: 0x83750B4
// IDA_NAME: _ZN13STSkillScript5ClearEv
// DEMANGLED: STSkillScript::Clear(void)
STSkillScript* STSkillScript::Clear() {
    int i;
    int j;

    skill_index = 0;
    name.clear();
    name2.clear();
    explain.clear();
    basic_explain.clear();
    maximum_level = 0;
    purchase_cost.clear();
    special_purchase_cost.clear();
    required_level = 1;
    required_level_range = 1;
    required_rank = 0;
    required_guild_level = 1;
    required_guild_level_range = 1;
    pre_required_skill_indices.clear();
    pre_required_skill_levels.clear();
    type = 0;
    skill_class = 0;
    for (i = 0; i != 6; ++i) {
        growtype_fitness[i] = 0;
        growtype_maximum_level[i] = 0;
        for (j = 0; j != 2; ++j) {
            second_growtype[i].fitness[j] = 0;
            second_growtype[i].maximum_level[j] = 0;
        }
    }
    skill_fitness_second_growtype.clear();
    for (i = 0; i != 2; ++i) {
        casting_time[i] = 0;
        maintain_mp[i] = 0;
        consume_mp[i] = 0.0f;
        cool_time[i] = 0;
    }
    durability_decrease_rate = 0;
    consume_items.clear();
    limit_count.clear();
    auto_limit_count_reduce = 1;
    skill_under_cooltime_effect.clear();
    skill_under_cooltime_effect_each.clear();
    auto_cooltime_apply = 1;
    weapon_effect_type = 0;
    icon_name.clear();
    icon_sprite.clear();
    icon_x = 0;
    icon_y = 0;
    command.clear();
    command_key_explain.clear();
    static_data.clear();
    static_data_type = -1;
    cooltime_level_info.clear();
    level_properties.clear();
    basic_properties.clear();
    value_1cc = 0;
    value_1d0 = 0;
    value_1d4 = false;
    value_1d5 = false;
    physical_magical_change = -1;
    skill_fitness_growtype.clear();
    steel_learning_enabled = false;
    steel_learning_skill = 0;
    steel_learning_value = 0;
    steel_learning_value_2 = 0;
    preloading_images.clear();
    purchase_gsp.clear();
    purchase_gold.clear();
    start_cool_time = 0;
    start_cool_time_value = 0;
    module_type = -1;
    for (i = 0; i != 17; ++i) {
        module_tag_enabled[i] = false;
    }
    module_tag_enabled[3] = true;
    league_ban = 0;
    skill_command_advantage = 0;
    skill_command_advantage_value = 0;
    is_linked_blast = 0;
    linked_blast_add_point = 0;
    linked_blast_dec_point = 0;
    value_254 = 0;
    feature_skill_index = -1;
    child_skill_index = -1;
    feature_skill_type = false;
    online = 0;
    special_level_up.clear();
    explain_ex.clear();
    basic_explain_ex.clear();
    nut_skill_script.clear();
    command_customizing = true;
    executable_states.clear();
    combo_set_enabled = false;
    combo_set.clear();
    value_2a0.clear();
    usable_passiveobject_ids.clear();
    ignore_active_status = false;
    ignored_active_status_states.clear();
    return this;
}

// IDA_EA: 0x8375642
// IDA_NAME: _ZN13STSkillScriptC2Ev
// DEMANGLED: STSkillScript::STSkillScript(void)
STSkillScript::STSkillScript()
    : name(),
      name2(),
      explain(),
      basic_explain(),
      purchase_cost(),
      special_purchase_cost(),
      pre_required_skill_indices(),
      pre_required_skill_levels(),
      skill_fitness_second_growtype(),
      consume_items(),
      limit_count(),
      skill_under_cooltime_effect(),
      skill_under_cooltime_effect_each(),
      icon_name(),
      icon_sprite(),
      command(),
      command_key_explain(),
      static_data(),
      cooltime_level_info(),
      level_properties(),
      basic_properties(),
      skill_fitness_growtype(),
      preloading_images(),
      purchase_gsp(),
      purchase_gold(),
      special_level_up(),
      explain_ex(),
      basic_explain_ex(),
      nut_skill_script(),
      executable_states(),
      combo_set(),
      value_2a0(),
      usable_passiveobject_ids(),
      ignore_active_status(false),
      ignored_active_status_states() {
    Clear();
}

// IDA_EA: 0x837A0FA
// IDA_NAME: _ZN13STSkillScriptD2Ev
// DEMANGLED: STSkillScript::~STSkillScript()
STSkillScript::~STSkillScript() {}

// IDA_EA: 0x8A9792F
// IDA_NAME: _Z17ImportSkillScriptP13STSkillScriptPKc17ENUM_CHARACTERJOBii
// DEMANGLED: ImportSkillScript(STSkillScript *,char const*,ENUM_CHARACTERJOB,int,int)
int ImportSkillScript(STSkillScript* script,
                      const char* filename,
                      ENUM_CHARACTERJOB character_job,
                      int module_type,
                      int script_index) {
    if (loadRDARScriptFile(g_szSkillScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    script->Clear();
    std::vector<float> level_values;
    level_values.reserve(4000);
    std::string tag;
    FixedLevelSkillPolicy fixed_level_policy = {0, 0, false};
    bool fixed_level_policy_seen = false;
    SkillAddDataExtension0828 add_data_extension;
    bool add_static_data_seen = false;
    bool add_level_info_seen = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->name);
        } else if (tag == "[name2]") {
            ScanStr(&script->name2);
        } else if (tag == "[explain]") {
            ScanStr(&script->explain);
        } else if (tag == "[basic explain]") {
            ScanStr(&script->basic_explain);
        } else if (tag == "[basic property]") {
            ReadSkillLevelFeature(script->basic_properties);
        } else if (tag == "[maximum level]") {
            script->maximum_level = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[purchase cost]") {
            ReadIntVector(script->purchase_cost);
        } else if (tag == "[special purchase cost]") {
            ReadIntVector(script->special_purchase_cost);
        } else if (tag == "[required level]") {
            script->required_level = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required level range]") {
            script->required_level_range = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required rank]") {
            script->required_rank = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required guild level]") {
            script->required_guild_level = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required guild level range]") {
            script->required_guild_level_range = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[fixed level skill]") {
            if (fixed_level_policy_seen ||
                !ReadFixedLevelSkillPolicy(fixed_level_policy)) return 0;
            fixed_level_policy_seen = true;
        } else if (tag == "[pre required skill]") {
            script->pre_required_skill_indices.clear();
            script->pre_required_skill_levels.clear();
            bool success = false;
            do {
                const int skill = ScanInt(&success);
                if (success) {
                    script->pre_required_skill_indices.push_back(skill);
                    script->pre_required_skill_levels.push_back(
                        ScanInt(static_cast<bool*>(0)));
                }
            } while (success);
        } else if (tag == "[steel learning skill]") {
            script->steel_learning_enabled =
                ScanInt(static_cast<bool*>(0)) == 1;
            script->steel_learning_skill = ScanInt(static_cast<bool*>(0));
            script->steel_learning_value = ScanInt(static_cast<bool*>(0));
            script->steel_learning_value_2 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[type]") {
            std::string type;
            ScanStr(&type);
            if (type == "[active]") {
                script->type = 0;
            } else if (type == "[passive]") {
                script->type = 1;
            }
        } else if (tag == "[skill class]") {
            script->skill_class = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[growtype fitness]") {
            for (int index = 0; index <= 5; ++index) {
                script->growtype_fitness[index] = ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[second growtype fitness]") {
            for (int growtype = 0; growtype <= 5; ++growtype) {
                for (int index = 0; index <= 1; ++index) {
                    script->second_growtype[growtype].fitness[index] =
                        ScanInt(static_cast<bool*>(0));
                }
            }
        } else if (tag == "[growtype maximum level]") {
            for (int index = 0; index <= 5; ++index) {
                script->growtype_maximum_level[index] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[second growtype maximum level]") {
            for (int growtype = 0; growtype <= 5; ++growtype) {
                for (int second_growtype = 1; second_growtype <= 2;
                     ++second_growtype) {
                    script->second_growtype[growtype]
                        .maximum_level[second_growtype - 1] =
                            ScanInt(static_cast<bool*>(0));
                }
            }
        } else if (tag == "[skill fitness growtype]") {
            ReadIntVector(script->skill_fitness_growtype);
        } else if (tag == "[skill fitness second growtype]") {
            ReadIntVector(script->skill_fitness_second_growtype);
        } else if (tag == "[casting time]") {
            for (int index = 0; index <= 1; ++index) {
                script->casting_time[index] = ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[durability decrease rate]") {
            script->durability_decrease_rate = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[consume MP]") {
            for (int index = 0; index <= 1; ++index) {
                script->consume_mp[index] = ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[maintain MP]") {
            for (int index = 0; index <= 1; ++index) {
                script->maintain_mp[index] = ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[consume item]") {
            script->consume_items.clear();
            bool success = false;
            do {
                SkillUseItem item;
                item.item_index = ScanInt(&success);
                if (success) {
                    item.value_04 = ScanInt(static_cast<bool*>(0));
                    item.value_08 = ScanInt(static_cast<bool*>(0));
                    script->consume_items.push_back(item);
                }
            } while (success);
        } else if (tag == "[usable passiveobject]") {
            // CLIENT_EA: ImportSkillScript @0x15FE191 stores every integer in
            // the target STSkillScript passive-object vector.  The failed
            // terminal ScanInt owns the closing TYPE token, matching every
            // other original integer-vector reader.
            ReadIntVector(script->usable_passiveobject_ids);
        } else if (tag == "[ignore active status]") {
            script->ignore_active_status = true;
        } else if (tag == "[ignore active status state]") {
            ReadIntVector(script->ignored_active_status_states);
        } else if (tag == "[limit count]") {
            ReadIntVector(script->limit_count);
        } else if (tag == "[auto limit count reduce]") {
            script->auto_limit_count_reduce = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cool time]") {
            for (int index = 0; index <= 1; ++index) {
                script->cool_time[index] = ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[league ban]") {
            script->league_ban = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[skill under cooltime effect]") {
            ReadIntVector(script->skill_under_cooltime_effect);
        } else if (tag == "[skill under cooltime effect each]") {
            ReadIntVector(script->skill_under_cooltime_effect_each);
        } else if (tag == "[auto cooltime apply]") {
            script->auto_cooltime_apply = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[weapon effect type]") {
            std::string effect_type;
            ScanStr(&effect_type);
            if (effect_type == "[none]") {
                script->weapon_effect_type = 0;
            } else if (effect_type == "[physical]") {
                script->weapon_effect_type = 1;
            } else if (effect_type == "[magical]") {
                script->weapon_effect_type = 2;
            }
        } else if (tag == "[icon]") {
            ScanStr(&script->icon_name);
            script->icon_x = ScanInt(static_cast<bool*>(0));
            ScanStr(&script->icon_sprite);
            script->icon_y = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[command key explain]") {
            ScanStr(&script->command_key_explain);
        } else if (tag == "[command]") {
            importCommand(&script->command);
        } else if (tag == "[static data]") {
            ReadIntVector(script->static_data);
        } else if (tag == "[add static data]") {
            if (add_static_data_seen ||
                !ReadSkillAddDataRules0828(
                    "[/add static data]",
                    &add_data_extension.static_data_rules)) {
                return 0;
            }
            add_static_data_seen = true;
        } else if (tag == "[cooltime level info]") {
            script->static_data_type = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[level info]") {
            ReadLevelInfo(script->cooltime_level_info, level_values);
        } else if (tag == "[add level info]") {
            if (add_level_info_seen ||
                !ReadSkillAddDataRules0828(
                    "[/add level info]",
                    &add_data_extension.level_info_rules)) {
                return 0;
            }
            add_level_info_seen = true;
        } else if (tag == "[level property]") {
            ReadSkillLevelFeature(script->level_properties);
        } else if (tag == "[shake screen]") {
            script->value_1cc = ScanInt(static_cast<bool*>(0));
            script->value_1d0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[seal enable]") {
            script->value_1d4 = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[active seal enable]") {
            script->value_1d5 = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[physical magical change]") {
            script->physical_magical_change = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[skill preloading image]") {
            std::string image;
            while (ScanStr(&image) == 1) {
                script->preloading_images.push_back(image);
            }
        } else if (tag == "[purchase gsp]") {
            ReadIntVector(script->purchase_gsp);
        } else if (tag == "[purchase gold]") {
            ReadIntVector(script->purchase_gold);
        } else if (tag == "[start cool time]") {
            script->start_cool_time = ScanInt(static_cast<bool*>(0));
            script->start_cool_time_value = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[skill command advantage]") {
            script->skill_command_advantage = ScanInt(static_cast<bool*>(0));
            script->skill_command_advantage_value =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[is linked blast]") {
            script->is_linked_blast = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[linked blast add point]") {
            script->linked_blast_add_point = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[linked blast dec point]") {
            script->linked_blast_dec_point = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[feature skill index]") {
            script->feature_skill_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[child skill index]") {
            script->child_skill_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[feature skill type]") {
            script->feature_skill_type =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[special level up]") {
            script->special_level_up.clear();
            bool success = false;
            do {
                STPassiveFeatureData feature;
                const int applies_to_all = ScanInt(&success);
                if (success) {
                    feature.applies_to_all = applies_to_all == -1;
                    feature.skill_index = ScanInt(&success);
                    std::string operation;
                    ScanStr(&operation);
                    if (operation == "+") {
                        feature.operation = 0;
                    } else if (operation == "-") {
                        feature.operation = 1;
                    } else if (operation == "%") {
                        feature.operation = 2;
                    }
                    feature.value = ScanInt(static_cast<bool*>(0));
                    script->special_level_up.push_back(feature);
                }
            } while (success);
        } else if (tag == "[explain ex]") {
            ScanStr(&script->explain_ex);
        } else if (tag == "[basic explain ex]") {
            ScanStr(&script->basic_explain_ex);
        } else if (tag == "[nut skill script]") {
            ScanStr(&script->nut_skill_script);
        } else if (tag == "[command customizing]") {
            script->command_customizing = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[executable states]") {
            script->executable_states.clear();
            while (true) {
                int state = -1;
                std::string state_name;
                const DNFLEX_TOKEN token =
                    ScanAll(&state, static_cast<float*>(0), &state_name);

                if (token == DNF_TOK_DECIMAL ||
                    token == DNF_TOK_HEXADECIMAL) {
                    script->executable_states.push_back(state);
                    continue;
                }
                if (token != DNF_TOK_STRING && token != DNF_TOK_TYPE &&
                    token != DNF_TOK_CUSTOM_DATA) {
                    break;
                }
                if (state_name == "[/executable states]") {
                    break;
                }

                if (state_name == "[stand]") {
                    state = 0;
                } else if (state_name == "[damage]") {
                    state = 3;
                } else if (state_name == "[jump]") {
                    state = 6;
                } else if (state_name == "[attack]") {
                    state = 8;
                } else if (state_name == "[dash]") {
                    state = 14;
                }
                if (state >= 0) {
                    script->executable_states.push_back(state);
                }
            }
        } else if (tag == "[combo set]") {
            bool success = false;
            do {
                const int value = ScanInt(&success);
                if (success) {
                    script->combo_set.push_back(value);
                }
            } while (success);
            script->combo_set_enabled = true;
        } else {
            int tag_index = -1;
            int parsed_module_type = -1;
            processModuleTagSet(tag_index,
                                parsed_module_type,
                                &tag,
                                module_type,
                                true);
            if (tag_index != -1) {
                script->module_tag_enabled[tag_index] = true;
            }
            if (parsed_module_type != -1) {
                script->module_type = parsed_module_type;
            }
        }
    }

    for (int index = 0; index <= 5; ++index) {
        const int growtype_maximum_level =
            script->growtype_maximum_level[index];
        if (growtype_maximum_level > script->maximum_level) {
            std::printf("SKILL ERROR : %s\\n", filename);
            script->maximum_level = growtype_maximum_level;
        }
    }

    while (!script->purchase_cost.empty() &&
           static_cast<int>(script->purchase_cost.size()) <
               script->maximum_level) {
        script->purchase_cost.push_back(script->purchase_cost.back());
    }
    while (!script->special_purchase_cost.empty() &&
           static_cast<int>(script->special_purchase_cost.size()) <
               script->maximum_level) {
        script->special_purchase_cost.push_back(
            script->special_purchase_cost.back());
    }
    if (!script->purchase_gsp.empty()) {
        while (static_cast<int>(script->purchase_gsp.size()) <
               script->maximum_level) {
            script->purchase_gsp.push_back(script->purchase_gsp.back());
        }
    }
    if (!script->purchase_gold.empty()) {
        while (static_cast<int>(script->purchase_gold.size()) <
               script->maximum_level) {
            script->purchase_gold.push_back(script->purchase_gold.back());
        }
    }
    if (!script->limit_count.empty()) {
        while (static_cast<int>(script->limit_count.size()) <
               script->maximum_level) {
            script->limit_count.push_back(script->limit_count.back());
        }
    }

    const int job = static_cast<int>(character_job);
    if (job == oldsrv::config::kCharacterJobCommon) {
        script->skill_index = -1;
    } else if (script_index == -1) {
        script->skill_index =
            g_stSkillScriptFileList[job].FindIndexByFullScan(filename);
    } else {
        script->skill_index = script_index;
    }
    if (job >= 0 && job < oldsrv::config::kCharacterJobCount &&
        script->skill_index >= 0) {
        const FixedLevelSkillKey key =
            std::make_pair(job, script->skill_index);
        if (fixed_level_policy_seen) {
            g_fixed_level_skill_policies[key] = fixed_level_policy;
        } else {
            g_fixed_level_skill_policies.erase(key);
        }
        PublishSkillAddDataExtension0828(
            job,
            script->skill_index,
            module_type,
            add_static_data_seen || add_level_info_seen,
            add_data_extension);
    }
    return 1;
}
