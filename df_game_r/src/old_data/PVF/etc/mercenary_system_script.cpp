#include "old_data/PVF/etc/mercenary_system_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <stdint.h>

#include <string>
#include <utility>


// IDA_EA: 0x89F6C50
// IDA_NAME: _Z30importMercenarySystemEtcScriptPKcP21stMercenarySystemInfo
// DEMANGLED: importMercenarySystemEtcScript(char const*,stMercenarySystemInfo*)
int importMercenarySystemEtcScript(const char* filename,
                                   stMercenarySystemInfo* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 9;
    }
    if (!info) {
        return 12;
    }

    std::string tag;
    std::string area_tag;
    std::string reward_tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[base time unit]") {
            const int base_time_unit = ScanInt(&success);
            if (!success) {
                return 41;
            }
            if (!base_time_unit) {
                return 44;
            }
            info->duration_sec = static_cast<uint32_t>(base_time_unit);
        } else if (tag == "[level base GoldItem Prob]") {
            const int level = ScanInt(&success);
            if (!success) {
                return 54;
            }
            const int first = ScanInt(&success);
            if (!success) {
                return 58;
            }
            const int second = ScanInt(&success);
            if (!success) {
                return 62;
            }
            info->level_ranges.push_back(std::make_pair(
                level, std::make_pair(first, second)));
            if (static_cast<int>(info->default_count) > level) {
                info->default_count = static_cast<uint16_t>(level);
            }
        } else if (tag == "[mercenary reward per period]") {
            while (1) {
                const int period = ScanInt(&success);
                if (!success) {
                    break;
                }
                const float reward = ScanFloat(&success);
                if (!success) {
                    return 80;
                }
                info->first_rates.push_back(
                    std::make_pair(period, reward));
            }
        } else if (tag == "[mercenary reward avatar bonus]") {
            while (1) {
                const int avatar_bonus = ScanInt(&success);
                if (!success) {
                    break;
                }
                const float reward = ScanFloat(&success);
                if (!success) {
                    return 97;
                }
                info->second_rates.push_back(
                    std::make_pair(avatar_bonus, reward));
            }
        } else if (tag == "[mercenary reward critical]") {
            if (ScanStr(&info->script_name) != 1) {
                return 108;
            }
            info->value_3c = 0;
            while (1) {
                const int rate = ScanInt(&success);
                if (!success) {
                    break;
                }
                const float reward = ScanFloat(&success);
                if (!success) {
                    return 119;
                }
                info->value_3c += static_cast<uint32_t>(rate);
                info->third_rates.push_back(std::make_pair(rate, reward));
            }
        } else if (tag == "[competition area]") {
            stMercenaryCompetitionArea area;
            int group_probability_total = 0;
            while (ScanType(area_tag, true) == 1) {
                if (area_tag == "[reward group]") {
                    stMercenaryRewardGroup reward_group;
                    reward_group.first_dword =
                        static_cast<uint32_t>(ScanInt(&success));
                    if (!success) {
                        return 143;
                    }
                    group_probability_total +=
                        static_cast<int>(reward_group.first_dword);
                    if (ScanStr(&reward_group.group_name) != 1) {
                        return 146;
                    }

                    while (1) {
                        if (ScanType(reward_tag, true) != 1) {
                            return 151;
                        }
                        if (reward_tag == "[mob index]") {
                            while (1) {
                                const int index = ScanInt(&success);
                                if (!success) {
                                    break;
                                }
                                const int probability = ScanInt(&success);
                                if (!success) {
                                    return 163;
                                }
                                reward_group.second_dword +=
                                    static_cast<uint32_t>(probability);
                                reward_group.first_rewards.push_back(
                                    std::make_pair(probability, index));
                            }
                        } else if (reward_tag == "[item index]") {
                            while (1) {
                                const int index = ScanInt(&success);
                                if (!success) {
                                    break;
                                }
                                const int probability = ScanInt(&success);
                                if (!success) {
                                    return 179;
                                }
                                reward_group.third_dword +=
                                    static_cast<uint32_t>(probability);
                                reward_group.second_rewards.push_back(
                                    std::make_pair(probability, index));
                            }
                        } else if (reward_tag == "[/reward group]") {
                            area.reward_groups.push_back(reward_group);
                            break;
                        }
                    }
                } else if (area_tag == "[world map]") {
                    area.area_index =
                        static_cast<uint16_t>(ScanInt(&success));
                } else if (area_tag == "[area level]") {
                    area.value_14 =
                        static_cast<uint16_t>(ScanInt(&success));
                } else if (area_tag == "[/competition area]") {
                    area.value_10 =
                        static_cast<uint32_t>(group_probability_total);
                    info->competition_areas.push_back(area);
                    break;
                } else {
                    return 209;
                }
            }
        }
    }

    return 0;
}
