#include "old_game/progression/career/disjoint_abi.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"
#include "common/manager/global_tools.h"

#include <stdint.h>
#include <string>
#include <vector>

// IDA_EA: 0x89BAB64
// IDA_NAME: _Z17createLotteryInfoP11LotteryInfoPSt6vectorIiSaIiEE
// DEMANGLED: createLotteryInfo(LotteryInfo *,std::vector<int,std::allocator<int>> *)
extern bool createLotteryInfo(LotteryInfo* info, std::vector<int>* values)
    __asm__("_Z17createLotteryInfoP11LotteryInfoPSt6vectorIiSaIiEE");

// IDA_EA: 0x88E4D88
// IDA_NAME: _Z20importDisjointScriptP14DisjointScriptPKc
// DEMANGLED: importDisjointScript(DisjointScript *,char const*)
int importDisjointScript(DisjointScript* script, const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    script->clear();
    std::string tag;
    std::string text;
    int value = 0;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[cube index]") {
            ScanStr(&text);
            const int element = getElement(text.c_str());
            script->cube_indexes[element].clear();
            while (1) {
                value = ScanInt(&success);
                if (!success) break;
                script->cube_indexes[element].push_back(value);
            }
        } else if (tag == "[legacy info]") {
            const int legacy_index = ScanInt(static_cast<bool*>(0));
            std::vector<int> lottery_values;
            while (1) {
                value = ScanInt(&success);
                if (!success) break;
                lottery_values.push_back(value);
            }
            if (legacy_index - 1 <= 3) {
                createLotteryInfo(&script->legacy_infos[legacy_index - 1],
                                  &lottery_values);
            }
        } else if (tag == "[skill info]") {
            script->skill_infos.clear();
            script->skill_info_count = ScanInt(static_cast<bool*>(0));
            DisjointSkillScript skill;
            while (ScanStr(&text) == 1) {
                skill.character_job = getCharacterJob(text.c_str());
                skill.skill_index = ScanInt(static_cast<bool*>(0));
                skill.skill_level = ScanInt(static_cast<bool*>(0));
                ScanStr(&text);
                skill.element = getElement(text.c_str());
                script->skill_infos.push_back(skill);
            }
        } else if (tag == "[skill prob]") {
            script->skill_prob[0] = ScanFloat(static_cast<bool*>(0));
            script->skill_prob[1] = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[cube creation const]") {
            script->cube_creation_const = ScanInt(static_cast<bool*>(0));
            for (int index = 0; index <= 5; ++index)
                script->cube_creation_rates[index] = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[additional result]") {
            for (int rarity = 0; rarity <= 5; ++rarity) {
                script->additional_results[rarity].count = ScanInt(&success);
                if (!success) break;
                script->additional_results[rarity].items.clear();
                script->additional_results[rarity].items.reserve(
                    script->additional_results[rarity].count);
                for (int index = 0;
                     script->additional_results[rarity].count > index; ++index) {
                    value = ScanInt(static_cast<bool*>(0));
                    script->additional_results[rarity].items.push_back(value);
                }
            }
        } else if (tag == "[additional result const]") {
            for (int rarity = 0; rarity <= 5; ++rarity) {
                script->additional_result_grade_a[rarity] =
                    ScanFloat(static_cast<bool*>(0));
                if (script->additional_result_grade_a[rarity] == 0.0f) {
                    script->additional_result_grade_b[rarity] = 0.0f;
                    script->additional_result_const[3 * rarity] = 0;
                    script->additional_result_const[3 * rarity + 1] = 0;
                } else {
                    script->additional_result_grade_b[rarity] =
                        ScanFloat(static_cast<bool*>(0));
                    script->additional_result_const[3 * rarity] =
                        ScanInt(static_cast<bool*>(0));
                    script->additional_result_const[3 * rarity + 1] =
                        ScanInt(static_cast<bool*>(0));
                }
            }
        } else if (tag == "[avatar disjoint info]") {
            while (ScanType(tag, true) == 1 && tag != "[/avatar disjoint info]") {
                const int character_job = getCharacterJob(tag.c_str());
                if (character_job == oldsrv::config::kCharacterJobCommon) return 0;
                for (int grade = 1; grade <= 3; ++grade) {
                    for (int element = 0; element <= 4; ++element) {
                        script->avatar_disjoint_info[character_job][grade][element] =
                            ScanInt(&success);
                        if (!success) return 0;
                    }
                }
                ScanInt(&success);
            }
        } else if (tag == "[avatar disjoint for parts info]") {
            for (int grade = 1; grade <= 3; ++grade) {
                for (int part = 0; part <= 9; ++part) {
                    script->avatar_parts_disjoint_info[grade][part] =
                        ScanInt(&success);
                    if (!success) return 0;
                }
            }
            ScanInt(&success);
        }
    }
    return 1;
}
