#include "old_data/PVF/n_quest/quest_parameter_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/itemshop/quest_shop_status.h"

#include <string>

// IDA_EA: 0x8A6C146
// IDA_NAME: _ZN22pieceQuestRewardStatusC2Ev
// DEMANGLED: pieceQuestRewardStatus::pieceQuestRewardStatus(void)
pieceQuestRewardStatus::pieceQuestRewardStatus()
    : reward_type(77),
      reward_count(2),
      reward_value(0.0f) {}

// IDA_EA: 0x8A6D686
// IDA_NAME: _ZN22pieceQuestRewardStatusD2Ev
// DEMANGLED: pieceQuestRewardStatus::~pieceQuestRewardStatus()
pieceQuestRewardStatus::~pieceQuestRewardStatus() {}

// IDA_EA: 0x836F130
// IDA_NAME: _ZN20QuestParameterScript5clearEv
// DEMANGLED: QuestParameterScript::clear(void)
QuestParameterScript* QuestParameterScript::clear() {
    difficulty_weights.clear();
    exp_reward_table.clear();
    gold_reward_table.clear();
    piece_reward_levels.clear();
    piece_reward_status.clear();
    green_level_penalty = 0;
    grey_level_penalty = 0;
    bal_kun_penalty = 0;
    return this;
}

// IDA_EA: 0x836F1C2
// IDA_NAME: _ZN20QuestParameterScriptC2Ev
// DEMANGLED: QuestParameterScript::QuestParameterScript(void)
QuestParameterScript::QuestParameterScript()
    : difficulty_weights(),
      exp_reward_table(),
      gold_reward_table(),
      piece_reward_levels(),
      piece_reward_status() {
    clear();
}

// IDA_EA: 0x837D3CA
// IDA_NAME: _ZN20QuestParameterScriptD2Ev
// DEMANGLED: QuestParameterScript::~QuestParameterScript()
QuestParameterScript::~QuestParameterScript() {}

// IDA_EA: 0x8A6BD1C
// IDA_NAME: _ZN20QuestParameterScript15getRewardWeightEc
// DEMANGLED: QuestParameterScript::getRewardWeight(char)
int QuestParameterScript::getRewardWeight(char difficulty) {
    std::map<char, int>::iterator it =
        difficulty_weights.find(difficulty);
    if (it != difficulty_weights.end()) {
        return it->second;
    }
    return 100;
}

// IDA_EA: 0x8A6BD86
// IDA_NAME: _ZN20QuestParameterScript12getRewardExpEi
// DEMANGLED: QuestParameterScript::getRewardExp(int)
int QuestParameterScript::getRewardExp(int charac_level) {
    return exp_reward_table.at(charac_level);
}

// IDA_EA: 0x8A6BDA6
// IDA_NAME: _ZN20QuestParameterScript13getRewardGoldEi
// DEMANGLED: QuestParameterScript::getRewardGold(int)
int QuestParameterScript::getRewardGold(int charac_level) {
    return gold_reward_table.at(charac_level);
}

// IDA_EA: 0x8A6BDC6
// IDA_NAME: _ZN20QuestParameterScript14getRewardLevelEi
// DEMANGLED: QuestParameterScript::getRewardLevel(int)
int QuestParameterScript::getRewardLevel(int charac_level) {
    int result = 0;
    for (std::map<int, int>::iterator it = piece_reward_levels.begin();
         it != piece_reward_levels.end(); ++it) {
        if (it->second <= charac_level) {
            result = it->first;
        }
    }
    return result;
}

// IDA_EA: 0x8A6BE66
// IDA_NAME: _ZN20QuestParameterScript15getRewardStatusEi
// DEMANGLED: QuestParameterScript::getRewardStatus(int)
std::vector<pieceQuestRewardStatus> QuestParameterScript::getRewardStatus(
    int charac_level) {
    std::vector<pieceQuestRewardStatus> result;
    const int reward_level = getRewardLevel(charac_level);
    if (reward_level == -1) {
        return result;
    }

    for (std::map<int, std::vector<pieceQuestRewardStatus> >::iterator it =
             piece_reward_status.begin();
         it != piece_reward_status.end(); ++it) {
        if (it->first > reward_level) {
            break;
        }

        const std::vector<pieceQuestRewardStatus> rows = it->second;
        for (std::vector<pieceQuestRewardStatus>::const_iterator row =
                 rows.begin();
             row != rows.end(); ++row) {
            std::vector<pieceQuestRewardStatus>::iterator found =
                result.begin();
            for (; found != result.end(); ++found) {
                if (found->reward_type == row->reward_type &&
                    found->reward_count == row->reward_count) {
                    break;
                }
            }

            if (found != result.end()) {
                found->reward_value += row->reward_value;
            } else {
                result.push_back(*row);
            }
        }
    }
    return result;
}

// IDA_EA: 0x8A6B660
// IDA_NAME: _Z26importQuestParameterScriptP20QuestParameterScriptPKc
// DEMANGLED: importQuestParameterScript(QuestParameterScript *,char const*)
int importQuestParameterScript(QuestParameterScript* script,
                               const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script->clear();

    std::string tag;
    bool end_seen = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[difficulty]") {
            std::string value;
            bool is_type = false;
            while (ScanTypeOrStr(is_type, value, true) == 1) {
                if (is_type && value == "[/difficulty]") {
                    break;
                }
                if (!is_type) {
                    const int weight = ScanInt(static_cast<bool*>(0));
                    const char difficulty = value.empty() ? 0 : value[0];
                    script->difficulty_weights.insert(
                        std::make_pair(difficulty, weight));
                }
            }
        } else if (tag == "[green level penalty]") {
            script->green_level_penalty = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[grey level penalty]") {
            script->grey_level_penalty = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[bal kun penalty]") {
            script->bal_kun_penalty = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[exp reward table]") {
            script->exp_reward_table.reserve(200u);
            for (int index = 0; index <= 199; ++index) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->exp_reward_table.push_back(value);
            }
        } else if (tag == "[gold reward table]") {
            script->gold_reward_table.reserve(200u);
            for (int index = 0; index <= 199; ++index) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->gold_reward_table.push_back(value);
            }
        } else if (tag == "[piece reward table]") {
            while (1) {
                bool success = false;
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->piece_reward_levels.insert(std::make_pair(key, level));
            }
        } else if (tag == "[piece reward]") {
            bool success = false;
            const int reward_level = ScanInt(&success);
            if (!success) {
                return 0;
            }

            std::string reward_tag;
            std::vector<pieceQuestRewardStatus> rows;
            while (ScanType(reward_tag, true) == 1 &&
                   reward_tag != "[/piece reward]") {
                const int appendage_type = loadAppendageType(reward_tag);
                if (appendage_type != -1) {
                    pieceQuestRewardStatus status;
                    status.reward_type = appendage_type;
                    std::string value_type;
                    ScanStr(&value_type);
                    status.reward_count = loadValueType(value_type);
                    status.reward_value = ScanFloat(static_cast<bool*>(0));
                    rows.push_back(status);
                }
            }

            script->piece_reward_status.insert(
                std::make_pair(reward_level, rows));
        } else if (tag == "[quest parameter end]") {
            end_seen = true;
            break;
        }
    }

    return end_seen ? 1 : 0;
}
