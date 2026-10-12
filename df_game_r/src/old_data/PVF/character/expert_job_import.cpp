#include "old_data/PVF/character/expert_job_script_list.h"

#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/character/expert_job_helpers.h"

// A-owned 99_misc_global values consumed by the original [potion info] tag.
// Their definitions remain outside B; see type_requests_B.md.
extern int g_alchemistInfo;
extern int g_alchemistInfo_0;

extern char g_expertJobScriptBaseDirectory[0x100];

// IDA_EA: 0x896AE21
// IDA_NAME: _Z24ImportExpertJobEtcScriptR20STExpertJobEtcScriptPKc
// DEMANGLED: ImportExpertJobEtcScript(STExpertJobEtcScript &,char const*)
int ImportExpertJobEtcScript(STExpertJobEtcScript& script,
                             const char* filename)
    __asm__("_Z24ImportExpertJobEtcScriptR20STExpertJobEtcScriptPKc");
int ImportExpertJobEtcScript(STExpertJobEtcScript& script,
                             const char* filename) {
    if (!loadRDARScriptFile(g_expertJobScriptBaseDirectory,
                                    filename)) {
        return 0;
    }

    script.Clear();
    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[expertjob level limit]") {
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success ||
                    !script.expert_job_limit_levels
                         .insert(std::make_pair(
                             static_cast<unsigned char>(index),
                             static_cast<short>(level)))
                         .second) {
                    return 0;
                }
            }
        } else if (tag == "[disjointer big win rate]") {
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int value = ScanInt(&success);
                if (!success ||
                    !script.expert_job_unknown_flags
                         .insert(std::make_pair(
                             static_cast<unsigned char>(index),
                             static_cast<unsigned char>(value)))
                         .second) {
                    return 0;
                }
            }
        }
    }
    return 1;
}
// IDA_EA: 0x896B0E4
// IDA_NAME: _Z21ImportExpertJobScriptP17STExpertJobScriptPKc
// DEMANGLED: ImportExpertJobScript(STExpertJobScript *,char const*)
int ImportExpertJobScript(STExpertJobScript* script,
                          const char* filename)
    __asm__("_Z21ImportExpertJobScriptP17STExpertJobScriptPKc");
int ImportExpertJobScript(STExpertJobScript* script,
                          const char* filename) {
    if (!loadRDARScriptFile(g_expertJobScriptBaseDirectory,
                                    filename)) {
        return 0;
    }

    script->Clear();
    std::multimap<int, stItemSelectRate> additional_results;
    std::multimap<int, stItemSelectRate> big_win_results;
    std::string tag;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[expertness exp]") {
            while (true) {
                ExpertJobExp value;
                value.exp = static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.level = static_cast<unsigned char>(ScanInt(&success));
                if (!success || ScanStr(&value.text) != 1) {
                    return 0;
                }
                script->expert_job_exp.push_back(value);
            }
        } else if (tag == "[skill]") {
            script->unknown_exp_pairs.clear();
            while (true) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->unknown_exp_pairs.push_back(std::make_pair(first, second));
            }
        } else if (tag == "[additional result]" ||
                   tag == "[big win result]") {
            std::multimap<int, stItemSelectRate>& destination =
                tag == "[additional result]" ? additional_results : big_win_results;
            destination.clear();
            while (true) {
                const int group = ScanInt(&success);
                if (!success) {
                    break;
                }
                stItemSelectRate value;
                value.min_grade = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.max_grade = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.item_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.select_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.grade_divisor = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                destination.insert(std::make_pair(group, value));
            }
        } else if (tag == "[base const]") {
            script->dword_1D4 = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[additional const]") {
            script->dword_1D8 = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[disjoint result]") {
            script->disjoint_results.clear();
            while (true) {
                DisjointMachineGrade grade;
                stDisjointResult result;
                grade.disjoint_machine_grade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                grade.rarity = static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                grade.unidentified =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                result.result_item_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                result.base_cost_rate = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                int group = ScanInt(&success);
                if (!success ||
                    (group != 0 &&
                     expert_job::get_disjoint_additional_items(
                         additional_results, group, result.additional_items) != 1)) {
                    return 0;
                }
                group = ScanInt(&success);
                if (!success ||
                    (group != 0 &&
                     expert_job::get_disjoint_additional_items(
                         big_win_results, group, result.big_win_items) != 1)) {
                    return 0;
                }
                result.big_win_rate = ScanInt(&success);
                if (!success ||
                    !script->disjoint_results.insert(std::make_pair(grade, result)).second) {
                    return 0;
                }
            }
        } else if (tag == "[clear quests]") {
            script->dword_vector_1DC.clear();
            while (true) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->dword_vector_1DC.push_back(value);
            }
        } else if (tag == "[giveup cost]") {
            script->dword_vector_1E8.clear();
            while (true) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->dword_vector_1E8.push_back(value);
                if (script->dword_vector_1E8.size() > 9u) {
                    std::printf("SCRIPT ERROR over count [%s] %s\n",
                                filename, tag.c_str());
                    return 0;
                }
            }
        } else if (tag == "[compound rate]") {
            script->compound_rates.clear();
            while (true) {
                stCompoundRate value;
                value.boundary = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.rates[0] = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.rates[1] = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.rates[2] = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->compound_rates.push_back(value);
            }
        } else if (tag == "[items]") {
            script->recipe_infos.clear();
            script->recipe_info_indices.clear();
            script->int_map_230.clear();
            script->ulong_map_248.clear();
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int recipe_item = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int result_item = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int required_level = ScanInt(&success);
                if (!success || index > 239) {
                    std::printf("SCRIPT ERROR over count [%s] %s\n",
                                filename, tag.c_str());
                    return 0;
                }
                stRecipeInfo info;
                info.recipe_item_index = static_cast<unsigned int>(recipe_item);
                info.result_item_index = static_cast<unsigned int>(result_item);
                info.required_level = static_cast<unsigned int>(required_level);
                script->recipe_infos.insert(std::make_pair(index, info));
                script->recipe_info_indices.insert(
                    std::make_pair(static_cast<unsigned long>(recipe_item), index));
                script->int_map_230.insert(
                    std::make_pair(recipe_item, required_level));
                script->ulong_map_248.insert(std::make_pair(
                    static_cast<unsigned long>(result_item),
                    static_cast<unsigned long>(recipe_item)));
            }
        } else if (tag == "[endurance repair cost]") {
            script->endurance_repair_costs.clear();
            while (true) {
                stEnduranceRepairCost value;
                value.full_repair_cost = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.max_endurance = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->endurance_repair_costs.push_back(value);
            }
        } else if (tag == "[endurance reduce]") {
            script->dword_26C = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
            script->dword_270 = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[endurance initial value]") {
            script->dword_274 = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[upgrade cost]") {
            script->upgrade_costs.clear();
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (index - 1 != static_cast<int>(script->upgrade_costs.size())) {
                    return 0;
                }
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->upgrade_costs.push_back(value);
            }
        } else if (tag == "[potion info]") {
            while (true) {
                const unsigned int item_index =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                if (item_index == 2600025u) {
                    g_alchemistInfo = value;
                } else if (item_index == 10000410u ||
                           item_index == 10000525u ||
                           item_index == 690000190u ||
                           item_index == 10000517u ||
                           item_index == 2600018u || item_index == 8078u) {
                    g_alchemistInfo_0 = value;
                } else {
                    return 0;
                }
            }
        } else if (tag == "[gain exp]") {
            script->dword_284 = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
            script->dword_288 = static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[limit disjoint gold]") {
            script->max_disjoint_machine_cost =
                static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[portable disjoint info]") {
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success ||
                    !script->portable_disjoint_possible_levels
                         .insert(std::make_pair(
                             index, static_cast<unsigned short>(level)))
                         .second) {
                    return 0;
                }
            }
        } else if (tag == "[disjoint self service]") {
            script->self_disjoin_material_info.item_index = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->self_disjoin_material_info.item_count = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->self_disjoin_material_info.gold = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[shop doll info]") {
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int slot = ScanInt(&success);
                if (!success ||
                    !script->shop_doll_slots
                         .insert(std::make_pair(
                             index, static_cast<unsigned char>(slot)))
                         .second) {
                    return 0;
                }
            }
        } else if (tag == "[unseal scroll level limit info]") {
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success ||
                    !script->unseal_scroll_possible_levels
                         .insert(std::make_pair(
                             index, static_cast<unsigned short>(level)))
                         .second) {
                    return 0;
                }
            }
        } else if (tag == "[monstercard bind info]") {
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                STExpertJobScript::stMonsterCardBindRate value;
                value.common_rate =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                value.special_rate =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success ||
                    !script->monster_card_bind_rates
                         .insert(std::make_pair(index, value))
                         .second) {
                    return 0;
                }
            }
        } else if (tag == "[monstercard bind only]" ||
                   tag == "[monstercard bind list]") {
            STExpertJobScript::stMonsterCardBindInfo& info =
                tag == "[monstercard bind only]" ? script->special_bind_info
                                                 : script->common_bind_info;
            int counts[4] = {0, 0, 0, 0};
            while (true) {
                STExpertJobScript::stMonsterCardBindResult value;
                value.item_index =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                const int group = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.probability =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                info.results[group].push_back(value);
                counts[group] += value.probability;
            }
            for (int index = 0; index <= 3; ++index) {
                info.group_counts[index] = counts[index];
            }
        } else if (tag == "[alchemist extraction info]" ||
                   tag == "[enchanter extraction info]" ||
                   tag == "[doll_controller extraction info]") {
            std::map<int, unsigned short>* destination = 0;
            if (tag == "[alchemist extraction info]") {
                destination = &script->alchemist_extraction_possible_levels;
            } else if (tag == "[enchanter extraction info]") {
                destination = &script->enchanter_extraction_possible_levels;
            } else {
                destination = &script->doll_controller_extraction_possible_levels;
            }
            while (true) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success ||
                    !destination
                         ->insert(std::make_pair(
                             index, static_cast<unsigned short>(level)))
                         .second) {
                    return 0;
                }
            }
        } else if (tag == "[alchemist extraction result item]") {
            int counts[3] = {0, 0, 0};
            while (true) {
                STExpertJobScript::stMonsterCardBindResult value;
                value.item_index =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                std::string result_type;
                if (ScanStr(&result_type) != 1) {
                    return 0;
                }
                value.probability =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                int group = -1;
                if (result_type == "food") {
                    group = 0;
                } else if (result_type == "life") {
                    group = 1;
                } else if (result_type == "jewel") {
                    group = 2;
                } else {
                    return 0;
                }
                script->alchemist_extraction_result_info.results[group].push_back(value);
                counts[group] += value.probability;
            }
            for (int index = 0; index <= 2; ++index) {
                script->alchemist_extraction_result_info.group_counts[index] =
                    counts[index];
            }
        } else if (tag == "[enchanter extraction result item]") {
            int count = 0;
            while (true) {
                STExpertJobScript::stMonsterCardBindResult value;
                value.item_index =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.probability =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                count += value.probability;
                script->enchanter_extraction_result_info.results.push_back(value);
            }
            script->enchanter_extraction_result_info.group_count = count;
        } else if (tag == "[doll_controller extraction result item]") {
            int counts[3] = {0, 0, 0};
            while (true) {
                STExpertJobScript::stDollControllerResult value;
                value.item_index =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.grade =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                value.rate =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                value.count =
                    static_cast<unsigned short>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                int group = -1;
                if (value.grade != 0 && value.rate <= 30u) {
                    group = 0;
                } else if (value.grade > 30u && value.rate <= 50u) {
                    group = 1;
                } else if (value.grade > 50u && value.rate <= 80u) {
                    group = 2;
                } else {
                    return 0;
                }
                script->doll_controller_extraction_result_info.results[group]
                    .push_back(value);
                counts[group] += value.count;
            }
            for (int index = 0; index <= 2; ++index) {
                script->doll_controller_extraction_result_info.group_counts[index] =
                    counts[index];
            }
        }
    }
    return 1;
}
