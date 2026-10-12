#include "old_game/dungeon/tower/tower_rank_blood_abi.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

#include <cstring>
#include <string>


// IDA_EA: 0x891B6A8
// IDA_NAME: _ZN18stClearTimeAverageC2Ev
// DEMANGLED: stClearTimeAverage::stClearTimeAverage(void)
stClearTimeAverage::stClearTimeAverage() {
    clear();
}

// IDA_EA: 0x891B6BC
// IDA_NAME: _ZN18stClearTimeAverage5clearEv
// DEMANGLED: stClearTimeAverage::clear(void)
stClearTimeAverage* stClearTimeAverage::clear() {
    std::memset(values, 0, sizeof(values));
    return this;
}

// IDA_EA: 0x8370D8C
// IDA_NAME: _ZN18stBonusPointValuesC2Ev
// DEMANGLED: stBonusPointValues::stBonusPointValues(void)
stBonusPointValues::stBonusPointValues() {
    clear();
}

// IDA_EA: 0x8370DA0
// IDA_NAME: _ZN18stBonusPointValues5clearEv
// DEMANGLED: stBonusPointValues::clear(void)
stBonusPointValues* stBonusPointValues::clear() {
    threshold = 0;
    bonus = 0;
    return this;
}

// IDA_EA: 0x893FF2A
// IDA_NAME: _ZN18stBonusPointValuesD2Ev
// DEMANGLED: stBonusPointValues::~stBonusPointValues()
stBonusPointValues::~stBonusPointValues() {
}

// IDA_EA: 0x891B6F2
// IDA_NAME: _ZN16stRankSystemInfo22stDungeonClearTimeDataC2Ev
// DEMANGLED: stRankSystemInfo::stDungeonClearTimeData::stDungeonClearTimeData(void)
stRankSystemInfo::stDungeonClearTimeData::stDungeonClearTimeData() {
    clear();
}

// IDA_EA: 0x891B706
// IDA_NAME: _ZN16stRankSystemInfo22stDungeonClearTimeData5clearEv
// DEMANGLED: stRankSystemInfo::stDungeonClearTimeData::clear(void)
void* stRankSystemInfo::stDungeonClearTimeData::clear() {
    return std::memset(this, 0, 0x1Cu);
}

// IDA_EA: 0x893FBC6
// IDA_NAME: _ZN16stRankSystemInfo22stDungeonClearTimeDataD2Ev
// DEMANGLED: stRankSystemInfo::stDungeonClearTimeData::~stDungeonClearTimeData()
stRankSystemInfo::stDungeonClearTimeData::~stDungeonClearTimeData() {
}

// IDA_EA: 0x891B72A
// IDA_NAME: _ZN16stRankSystemInfo23stComboIntervalBoundaryC2Ev
// DEMANGLED: stRankSystemInfo::stComboIntervalBoundary::stComboIntervalBoundary(void)
stRankSystemInfo::stComboIntervalBoundary::stComboIntervalBoundary() {
    clear();
}

// IDA_EA: 0x891B73E
// IDA_NAME: _ZN16stRankSystemInfo23stComboIntervalBoundary5clearEv
// DEMANGLED: stRankSystemInfo::stComboIntervalBoundary::clear(void)
void* stRankSystemInfo::stComboIntervalBoundary::clear() {
    return std::memset(this, 0, 0x10u);
}

// IDA_EA: 0x8940176
// IDA_NAME: _ZN16stRankSystemInfo23stComboIntervalBoundaryD2Ev
// DEMANGLED: stRankSystemInfo::stComboIntervalBoundary::~stComboIntervalBoundary()
stRankSystemInfo::stComboIntervalBoundary::~stComboIntervalBoundary() {
}

// IDA_EA: 0x8370DCC
// IDA_NAME: _ZN16stRankSystemInfo21stDungeonRankSystem_tC2Ev
// DEMANGLED: stRankSystemInfo::stDungeonRankSystem_t::stDungeonRankSystem_t(void)
stRankSystemInfo::stDungeonRankSystem_t::stDungeonRankSystem_t() {
    clear();
}

// IDA_EA: 0x8370E34
// IDA_NAME: _ZN16stRankSystemInfo21stDungeonRankSystem_t5clearEv
// DEMANGLED: stRankSystemInfo::stDungeonRankSystem_t::clear(void)
void* stRankSystemInfo::stDungeonRankSystem_t::clear() {
    clear_time_averages.clear();
    bonus_point_values.clear();
    return std::memset(rank_grade, 0, 0x12u);
}

// IDA_EA: 0x8370E74
// IDA_NAME: _ZN16stRankSystemInfo21stDungeonRankSystem_tD2Ev
// DEMANGLED: stRankSystemInfo::stDungeonRankSystem_t::~stDungeonRankSystem_t()
stRankSystemInfo::stDungeonRankSystem_t::~stDungeonRankSystem_t() {
}

// IDA_EA: 0x8370EBA
// IDA_NAME: _ZN16stRankSystemInfoC2Ev
// DEMANGLED: stRankSystemInfo::stRankSystemInfo(void)
stRankSystemInfo::stRankSystemInfo() {
    clear();
}

// IDA_EA: 0x8370F94
// IDA_NAME: _ZN16stRankSystemInfo5clearEv
// DEMANGLED: stRankSystemInfo::clear(void)
stRankSystemInfo* stRankSystemInfo::clear() {
    dungeon_clear_time_data.clear();
    dungeon_clear_time_reduction_bonus.clear();
    rank_grade_values.clear();
    combo_interval_boundaries.clear();
    combo_practice_bonus_interval = 0;
    return this;
}

// IDA_EA: 0x837CE4C
// IDA_NAME: _ZN16stRankSystemInfoD2Ev
// DEMANGLED: stRankSystemInfo::~stRankSystemInfo()
stRankSystemInfo::~stRankSystemInfo() {
}

// IDA_EA: 0x8370970
// IDA_NAME: _ZN14stDeathTower_tC2Ev
// DEMANGLED: stDeathTower_t::stDeathTower_t(void)
stDeathTower_t::stDeathTower_t() {
    apc_exp_weight = 0.0f;
    monster_exp_weight = 0.0f;
    reward_gold_weight = 1.0f;
    for (unsigned int index = 0; index <= 0xC7u; ++index) {
        reward_exp_weight[index] = 1.0f;
        reward_card_num[index] = 0;
        illusion_reward_exp_weight[index] = 1.0f;
        illusion_reward_card_num[index] = 0;
    }
    std::memset(reward_item_prob_cumulative, 0, 0x0Cu);
}

// IDA_EA: 0x8370A3A
// IDA_NAME: _ZN16stBloodDungeon_tC2Ev
// DEMANGLED: stBloodDungeon_t::stBloodDungeon_t(void)
stBloodDungeon_t::stBloodDungeon_t() {
    clear();
}

// IDA_EA: 0x8370AB0
// IDA_NAME: _ZN16stBloodDungeon_tD2Ev
// DEMANGLED: stBloodDungeon_t::~stBloodDungeon_t()
stBloodDungeon_t::~stBloodDungeon_t() {
}

// IDA_EA: 0x8370B02
// IDA_NAME: _ZN16stBloodDungeon_t5clearEv
// DEMANGLED: stBloodDungeon_t::clear(void)
void stBloodDungeon_t::clear() {
    for (unsigned int index = 0; index <= 0xC7u; ++index) {
        reward_exp_weight[index] = 1.0f;
    }
    reward_exp_inout_count_weight.clear();
    exp_standard_monster_count = 1;
    reward_gold_weight = 1.0f;
    std::memset(reward_item_prob_cumulative, 0, 0x0Cu);
    std::memset(ultimate_difficulty_points,
                0,
                sizeof(ultimate_difficulty_points));
    ultimate_reward_prob.clear();
}

// IDA_EA: 0x89089F3
// IDA_NAME: _Z22importDeathTowerScriptPKcR14stDeathTower_t
// DEMANGLED: importDeathTowerScript(char const*,stDeathTower_t &)
int importDeathTowerScript(const char* filename, stDeathTower_t& script) {
    bool success = false;
    std::string tag;
    if (loadRDARScriptFile("", filename)) {
        while (ScanType(tag, true) == 1) {
            if (tag == "[apc exp weight]") {
                script.apc_exp_weight = ScanFloat(&success);
                if (!success) {
                    return 6279;
                }
            } else if (tag == "[reward exp weight]") {
                int index = 1;
                do {
                    script.reward_exp_weight[index] = ScanFloat(&success);
                    ++index;
                } while (success);
            } else if (tag == "[reward card num]") {
                int index = 1;
                do {
                    script.reward_card_num[index] = ScanInt(&success);
                    ++index;
                } while (success);
            } else if (tag == "[illusion reward exp weight]") {
                int index = 1;
                do {
                    script.illusion_reward_exp_weight[index] =
                        ScanFloat(&success);
                    ++index;
                } while (success);
            } else if (tag == "[illusion reward card num]") {
                int index = 1;
                do {
                    script.illusion_reward_card_num[index] =
                        ScanInt(&success);
                    ++index;
                } while (success);
            } else if (tag == "[monster exp weight]") {
                script.monster_exp_weight = ScanFloat(&success);
                if (!success) {
                    return 6328;
                }
            } else if (tag == "[reward item prob]") {
                script.reward_item_prob_cumulative[0] = ScanInt(&success);
                if (!success) {
                    return 6334;
                }
                script.reward_item_prob_cumulative[1] =
                    script.reward_item_prob_cumulative[0] + ScanInt(&success);
                if (!success) {
                    return 6337;
                }
                script.reward_item_prob_cumulative[2] =
                    script.reward_item_prob_cumulative[1] + ScanInt(&success);
                if (!success) {
                    return 6340;
                }
            } else {
                if (tag != "[reward gold weight]") {
                    return 6349;
                }
                script.reward_gold_weight = ScanFloat(&success);
                if (!success) {
                    return 6346;
                }
            }
        }
        return 0;
    }
    return 6270;
}

// IDA_EA: 0x8909016
// IDA_NAME: _Z24importBloodDungeonScriptPKcR16stBloodDungeon_t
// DEMANGLED: importBloodDungeonScript(char const*,stBloodDungeon_t &)
int importBloodDungeonScript(const char* filename, stBloodDungeon_t& script) {
    bool success = false;
    std::string tag;
    if (loadRDARScriptFile("", filename)) {
        while (ScanType(tag, true) == 1) {
            if (tag == "[reward exp weight]") {
                int index = 0;
                do {
                    script.reward_exp_weight[index] = ScanFloat(&success);
                    ++index;
                } while (success);
            } else if (tag == "[exp standard monster cnt]") {
                script.exp_standard_monster_count = ScanInt(&success);
                if (!success) {
                    return 6447;
                }
            } else if (tag == "[reward item prob]") {
                script.reward_item_prob_cumulative[0] = ScanInt(&success);
                if (!success) {
                    return 6454;
                }
                script.reward_item_prob_cumulative[1] =
                    script.reward_item_prob_cumulative[0] + ScanInt(&success);
                if (!success) {
                    return 6457;
                }
                script.reward_item_prob_cumulative[2] =
                    script.reward_item_prob_cumulative[1] + ScanInt(&success);
                if (!success) {
                    return 6460;
                }
            } else if (tag == "[reward gold weight]") {
                script.reward_gold_weight = ScanFloat(&success);
                if (!success) {
                    return 6466;
                }
            } else if (tag == "[reward exp inout count weight]") {
                while (1) {
                    float value = ScanFloat(&success);
                    if (!success) {
                        break;
                    }
                    script.reward_exp_inout_count_weight.push_back(value);
                }
            } else if (tag == "[ultimate difficulty point]") {
                script.ultimate_difficulty_points[1] =
                    ScanInt(static_cast<bool*>(0));
                script.ultimate_difficulty_points[2] =
                    ScanInt(static_cast<bool*>(0));
            } else if (tag == "[ultimate reward prob]") {
                while (1) {
                    const int key = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    STUltimateRewardProb prob;
                    prob.first_dword = ScanInt(static_cast<bool*>(0));
                    prob.second_dword = ScanInt(static_cast<bool*>(0));
                    if (script.ultimate_reward_prob.find(key) !=
                        script.ultimate_reward_prob.end()) {
                        return 6510;
                    }
                    script.ultimate_reward_prob.insert(
                        std::make_pair(key, prob));
                }
            }
        }
        return 0;
    }
    return 6428;
}

// IDA_EA: 0x890B8D0
// IDA_NAME: _Z15importTowerInfoPKcS0_RSt3mapIj14stDeathTower_tSt4lessIjESaISt4pairIKjS2_EEE
// DEMANGLED: importTowerInfo(char const*,char const*,std::map<uint,stDeathTower_t> &)
int importTowerInfo(const char* dir,
                    const char* filename,
                    std::map<unsigned int, stDeathTower_t>& tower_info) {
    STScriptFileList script_file_list;
    if (loadRDARScriptListFile(&script_file_list, dir, filename)) {
        if (script_file_list.empty()) {
            return 7822;
        }

        for (STScriptFileList::iterator it = script_file_list.begin();
             it != script_file_list.end();
             ++it) {
            stDeathTower_t tower;
            const unsigned int tower_index =
                static_cast<unsigned int>(it->first);
            const std::string child_filename = std::string(dir) + it->second;
            if (importDeathTowerScript(child_filename.c_str(), tower)) {
                return 7842;
            }
            tower_info.insert(std::make_pair(tower_index, tower));
        }
        return 0;
    }
    return 7851;
}

// IDA_EA: 0x890ED7F
// IDA_NAME: _Z22importRankSystemScriptPKcP16stRankSystemInfo
// DEMANGLED: importRankSystemScript(char const*,stRankSystemInfo *)
int importRankSystemScript(const char* filename, stRankSystemInfo* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    bool success = false;
    std::string tag;
    std::string text;
    while (ScanType(tag, true) == 1) {
        if (tag == "[dungeon clear time average]") {
            while (1) {
                stRankSystemInfo::stDungeonClearTimeData data;
                data.values[0] = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.values[1] = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.values[2] = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.values[3] = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.values[4] = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.values[5] = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.values[6] = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->dungeon_clear_time_data.push_back(data);

                stClearTimeAverageMapKey key;
                key.dungeon_index = data.values[0];
                key.difficulty = data.values[1];
                stClearTimeAverage average;
                average.values[0] = data.values[2];
                average.values[1] = data.values[3];
                average.values[2] = data.values[4];
                average.values[3] = data.values[5];
                average.values[4] = data.values[6];
                if (info->dungeon_rank_system.clear_time_averages.find(key) !=
                    info->dungeon_rank_system.clear_time_averages.end()) {
                    return 0;
                }
                info->dungeon_rank_system.clear_time_averages.insert(
                    std::make_pair(key, average));
            }
        } else if (tag == "[dungeon clear time reduction bonus]") {
            while (1) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->dungeon_clear_time_reduction_bonus.insert(
                    std::make_pair(first, second));
                stBonusPointValues values;
                values.threshold = first;
                values.bonus = second;
                info->dungeon_rank_system.bonus_point_values.push_back(values);
            }
        } else if (tag == "[rank grade]") {
            for (int index = 0;; ++index) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->rank_grade_values.push_back(value);
                if (index > 8) {
                    break;
                }
                info->dungeon_rank_system.rank_grade[index] =
                    static_cast<uint16_t>(value);
            }
        } else if (tag == "[combo interval boundaries]") {
            while (1) {
                stRankSystemInfo::stComboIntervalBoundary boundary;
                boundary.values[0] = ScanInt(&success);
                if (!success) {
                    break;
                }
                boundary.values[1] = ScanInt(&success);
                if (!success) {
                    break;
                }
                boundary.values[2] = ScanInt(&success);
                if (!success) {
                    break;
                }
                boundary.values[3] = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->combo_interval_boundaries.push_back(boundary);
            }
        } else if (tag == "[combo practice bonus inerval]") {
            info->combo_practice_bonus_interval = ScanInt(&success);
            if (!success) {
                break;
            }
        }
    }
    return 1;
}
