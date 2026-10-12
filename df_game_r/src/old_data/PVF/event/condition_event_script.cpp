#include "old_data/PVF/event/condition_event_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>


// IDA_EA: 0x890E868
// IDA_NAME: _Z26importConditionEventScriptPKcP20stConditionEventInfo
// DEMANGLED: importConditionEventScript(char const*,stConditionEventInfo *)
int importConditionEventScript(const char* filename,
                               stConditionEventInfo* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string unused;
    (void)unused;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[id]") {
            const int value = ScanInt(&success);
            if (!success) {
                break;
            }
            info->first_dword = value;
        } else if (tag == "[condition match]") {
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->condition_matches.insert(
                    std::make_pair(key, std::make_pair(first, second)));
            }
        } else if (tag == "[condition reward]") {
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int mail_reward = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int quick_party = ScanInt(&success);
                if (!success) {
                    break;
                }

                stConditionEventRewardInfo reward;
                reward.first_dword = item_id;
                reward.second_dword = item_count;
                reward.first_flag = mail_reward != 0;
                reward.second_flag = quick_party != 0;
                info->condition_rewards.insert(std::make_pair(key, reward));
            }
        } else if (tag == "[condition quick party add reward]") {
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int mail_reward = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int quick_party = ScanInt(&success);
                if (!success) {
                    break;
                }

                stConditionEventRewardInfo reward;
                reward.first_dword = item_id;
                reward.second_dword = item_count;
                reward.first_flag = mail_reward != 0;
                reward.second_flag = quick_party != 0;
                info->quick_party_rewards.insert(
                    std::make_pair(key, reward));
            }
        }
    }

    return 1;
}

// IDA_EA: 0x8913891
// IDA_NAME: _Z36importConditionEventChkDungeonScriptPKcP24ConditionLevelChkDungeon
// DEMANGLED: importConditionEventChkDungeonScript(char const*,ConditionLevelChkDungeon *)
int importConditionEventChkDungeonScript(const char* filename,
                                         ConditionLevelChkDungeon* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string unused;
    (void)unused;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[apply level]") {
            const int value = ScanInt(&success);
            info->min_level = value;
        } else if (tag == "[unable dungeon]") {
            info->dungeon_indexes.clear();
            while (1) {
                const int dungeon_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                info->dungeon_indexes.push_back(dungeon_index);
            }
        }
    }

    return 1;
}
