#include "old_data/PVF/n_quest/training_quest_script.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"

#include <algorithm>
#include <ctime>
#include <string>

// IDA_EA: 0x8AAFE18
// IDA_NAME: _ZN8MyRandomC2Ei
// DEMANGLED: MyRandom::MyRandom(int)
MyRandom::MyRandom(int seed)
    : seed_(seed) {}

// IDA_EA: 0x8AAFE26
// IDA_NAME: _ZN8MyRandomclEi
// DEMANGLED: MyRandom::operator()(int)
int MyRandom::operator()(int count) {
    return seed_ % count;
}

// IDA_EA: 0x8371B76
// IDA_NAME: _ZN28ST_TrainingQuest_Select_ProbC2Ev
// DEMANGLED: ST_TrainingQuest_Select_Prob::ST_TrainingQuest_Select_Prob(void)
ST_TrainingQuest_Select_Prob::ST_TrainingQuest_Select_Prob()
    : select_prob_count(0), select_probs(), rand_values() {
    select_probs.clear();
}

// IDA_EA: 0x8371C60
// IDA_NAME: _ZN28ST_TrainingQuest_Select_ProbD2Ev
// DEMANGLED: ST_TrainingQuest_Select_Prob::~ST_TrainingQuest_Select_Prob()
ST_TrainingQuest_Select_Prob::~ST_TrainingQuest_Select_Prob() {}

// IDA_EA: 0x8371CC8
// IDA_NAME: _ZN19TrainingQuestScriptC2Ev
// DEMANGLED: TrainingQuestScript::TrainingQuestScript(void)
TrainingQuestScript::TrainingQuestScript()
    : ST_TrainingQuest_Select_Prob(),
      level_quests(),
      special_level_quests(),
      same_apply_level_cnt(0) {
    clear();
}

// IDA_EA: 0x8371DD4
// IDA_NAME: _ZN19TrainingQuestScript5clearEv
// DEMANGLED: TrainingQuestScript::clear(void)
int TrainingQuestScript::clear() {
    select_prob_count = 0;
    select_probs.clear();
    same_apply_level_cnt = 0;
    for (int index = 0;
         index < oldsrv::config::kTrainingQuestLevelCount;
         ++index) {
        level_quests[index].clear();
    }
    special_level_quests.clear();
    return 0;
}

// IDA_EA: 0x8371E48
// IDA_NAME: _ZN19TrainingQuestScript26reset_same_apply_level_cntEv
// DEMANGLED: TrainingQuestScript::reset_same_apply_level_cnt(void)
TrainingQuestScript* TrainingQuestScript::reset_same_apply_level_cnt() {
    same_apply_level_cnt = 0;
    return this;
}

// IDA_EA: 0x837D488
// IDA_NAME: _ZN19TrainingQuestScriptD2Ev
// DEMANGLED: TrainingQuestScript::~TrainingQuestScript()
TrainingQuestScript::~TrainingQuestScript() {}

// IDA_EA: 0x8AAF8B6
// IDA_NAME: _ZN19TrainingQuestScript13getApplyLevelEPSt8multisetIiSt4lessIiESaIiEE
// DEMANGLED: TrainingQuestScript::getApplyLevel(std::multiset<int,std::less<int>,std::allocator<int>> *)
int TrainingQuestScript::getApplyLevel(std::multiset<int>* apply_levels) {
    if (select_prob_count <= 0 || select_probs.empty()) {
        return 0;
    }

    std::vector<int> selected_levels;
    const int day_index = computeDayIndex();
    for (int index = 0; index < select_prob_count; ++index) {
        const int random_value = get_random_fake_value(index, day_index);
        int accumulated = 0;
        for (std::vector<std::pair<int, int> >::iterator it =
                 select_probs.begin();
             it != select_probs.end(); ++it) {
            accumulated += it->second;
            if (random_value < accumulated) {
                selected_levels.push_back(it->first);
                break;
            }
        }
    }

    for (int level = oldsrv::config::kCharacterLevelMinimum;
         level <= oldsrv::config::kTrainingQuestLevelMaximum;
         ++level) {
        for (std::vector<int>::iterator it = selected_levels.begin();
             it != selected_levels.end(); ++it) {
            if (*it == -100) {
                apply_levels->insert(*it);
            } else if (level + *it > 0) {
                apply_levels->insert(level + *it);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8AAFB44
// IDA_NAME: _ZN19TrainingQuestScript16getTrainingQuestEiii
// DEMANGLED: TrainingQuestScript::getTrainingQuest(int,int,int)
int TrainingQuestScript::getTrainingQuest(int charac_level,
                                          int apply_level,
                                          int prev_apply_level) {
    if (apply_level == prev_apply_level) {
        ++same_apply_level_cnt;
    } else {
        same_apply_level_cnt = 0;
    }

    if (apply_level == -100) {
        const unsigned int index =
            static_cast<unsigned int>(same_apply_level_cnt);
        if (index < special_level_quests.size()) {
            return special_level_quests.at(index);
        }
        return 0x7FFFFFFF;
    }

    if (apply_level + charac_level - 1 >= 0) {
        std::vector<int>& quests =
            level_quests[apply_level + charac_level - 1];
        const unsigned int index =
            static_cast<unsigned int>(same_apply_level_cnt);
        if (index < quests.size()) {
            return quests.at(index);
        }
        return 0x7FFFFFFF;
    }

    return 0x7FFFFFFF;
}

// IDA_EA: 0x8AAFC64
// IDA_NAME: _ZN19TrainingQuestScript15computeDayIndexEv
// DEMANGLED: TrainingQuestScript::computeDayIndex(void)
int TrainingQuestScript::computeDayIndex() {
    std::time_t timer = std::time(0);
    struct tm* local = std::localtime(&timer);
    return local->tm_yday;
}

// IDA_EA: 0x8AAFC90
// IDA_NAME: _ZN19TrainingQuestScript21get_random_fake_valueEii
// DEMANGLED: TrainingQuestScript::get_random_fake_value(int,int)
int TrainingQuestScript::get_random_fake_value(int index, int day_index) {
    return rand_values[day_index].at(index);
}

// IDA_EA: 0x8AAFCBC
// IDA_NAME: _ZN19TrainingQuestScript20suffleTrainingQuestsEv
// DEMANGLED: TrainingQuestScript::suffleTrainingQuests(void)
bool TrainingQuestScript::suffleTrainingQuests() {
    const int day_index = computeDayIndex();
    MyRandom random(rand_values[day_index].at(0));
    for (int index = 0; index <= 69; ++index) {
        std::vector<int>& quests = level_quests[index];
        if (!quests.empty()) {
            std::random_shuffle(quests.begin(), quests.end(), random);
        }
    }
    return false;
}

// IDA_EA: 0x8AAF554
// IDA_NAME: _Z25importTrainingQuestScriptP19TrainingQuestScriptPKc
// DEMANGLED: importTrainingQuestScript(TrainingQuestScript *,char const*)
int importTrainingQuestScript(TrainingQuestScript* script,
                              const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script->clear();

    std::string tag;
    bool success = false;
    int previous_level = 0;
    while (ScanType(tag, true) == 1) {
        if (tag == "[select prob]") {
            script->select_prob_count = ScanInt(&success);
            if (!success) {
                break;
            }

            while (1) {
                std::pair<int, int> row;
                row.first = ScanInt(&success);
                if (!success) {
                    break;
                }
                row.second = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->select_probs.push_back(row);
            }
        } else if (tag == "[rand value]") {
            for (int day = 0; day <= 365; ++day) {
                for (int index = 0; index < script->select_prob_count; ++index) {
                    const int value = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script->rand_values[day].push_back(value);
                }
                if (!success) {
                    break;
                }
            }
        } else if (tag == "[level]") {
            const int level = ScanInt(static_cast<bool*>(0));
            if (previous_level + 1 == level) {
                while (1) {
                    const int quest_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script->level_quests[previous_level].push_back(quest_index);
                }
                previous_level = level;
            } else if (level == -100) {
                while (1) {
                    const int quest_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script->special_level_quests.push_back(quest_index);
                }
            }
        } else if (tag == "[training quest end]") {
            success = true;
            break;
        }
    }

    return success ? 1 : 0;
}
