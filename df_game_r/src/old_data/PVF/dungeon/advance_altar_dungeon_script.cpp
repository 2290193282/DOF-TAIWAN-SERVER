#include "old_data/PVF/dungeon/advance_altar_dungeon_script.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/common/rdar_script.h"

namespace advancealtar {

// IDA_EA: 0x88990BA
// IDA_NAME: _ZN12advancealtar9StageType12getStageTypeERKSsRNS0_1TE
// DEMANGLED: advancealtar::StageType::getStageType(std::string const&,advancealtar::StageType::T &)
bool StageType::getStageType(const std::string& name, T& type) {
    // The original accepts every text token.  Only "survival" selects the
    // survival value; all other tokens, including "normal", select normal.
    type = name == "survival" ? SURVIVAL : NORMAL;
    return true;
}

}  // namespace advancealtar

namespace {

bool scanRewardItem(advancealtar::_RewardItem& item) {
    std::string type_name;
    if (ScanStr(&type_name) != 1) {
        return false;
    }

    advancealtar::RewardItemType::T type =
        advancealtar::RewardItemType::NORMAL;
    if (!advancealtar::RewardItemType::getRewardItemType(type_name, type)) {
        return false;
    }

    bool scanned = false;
    const int item_id = ScanInt(&scanned);
    if (!scanned) {
        return false;
    }
    const int item_count = ScanInt(&scanned);
    if (!scanned) {
        return false;
    }

    item.reward_type = type;
    item.item_id = item_id;
    item.item_count = item_count;
    return true;
}

}  // namespace

// IDA_EA: 0x88F20A4
// IDA_NAME: _Z28ImportAdvanceAltarDungeonTagP15STDungeonScriptSsPKc
// DEMANGLED: ImportAdvanceAltarDungeonTag(STDungeonScript *,std::string,char const*)
int ImportAdvanceAltarDungeonTag(STDungeonScript* script,
                                 std::string tag,
                                 const char* filename) {
    (void)filename;

    if (tag == "[defense dungeon]") {
        script->defense_dungeon = ScanInt(static_cast<bool*>(0)) != 0;
        return 1;
    }

    if (tag == "[advance altar type]") {
        std::string type_name;
        if (ScanStr(&type_name) == 1) {
            advancealtar::StageType::T type = advancealtar::StageType::NORMAL;
            if (advancealtar::StageType::getStageType(type_name, type)) {
                script->value_2ac = type;
            }
        }
        return 1;
    }

    if (tag == "[advance altar map]") {
        for (;;) {
            std::string difficulty_name;
            if (ScanStr(&difficulty_name) != 1) {
                break;
            }

            advancealtar::StageDifficulty::T difficulty =
                advancealtar::StageDifficulty::NONE;
            if (!advancealtar::StageDifficulty::getDifficulty(difficulty_name,
                                                               difficulty)) {
                break;
            }

            bool scanned = false;
            const int first_value = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            const int second_value = ScanInt(&scanned);
            if (!scanned) {
                break;
            }

            script->value_2b0[difficulty] = first_value;
            script->value_2bc[difficulty] = second_value;
        }
        return 1;
    }

    if (tag == "[advance altar clear reward]") {
        for (;;) {
            std::string difficulty_tag;
            if (ScanType(difficulty_tag, true) != 1) {
                return 1;
            }

            int difficulty = -1;
            if (difficulty_tag == "[easy]") {
                difficulty = 0;
            } else if (difficulty_tag == "[medium]") {
                difficulty = 1;
            } else if (difficulty_tag == "[hard]") {
                difficulty = 2;
            } else {
                return 1;
            }

            for (;;) {
                advancealtar::_RewardItem reward;
                if (!scanRewardItem(reward)) {
                    break;
                }
                script->clear_reward_data.reward_items[difficulty].push_back(
                    reward);
            }
        }
    }

    if (tag == "[advance altar survival map]") {
        for (;;) {
            bool scanned = false;
            const int map_index = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            script->value_2c8.push_back(map_index);
        }
        return 1;
    }

    if (tag != "[advance altar survival clear reward]") {
        return 0;
    }

    for (;;) {
        std::string outer_tag;
        if (ScanType(outer_tag, true) != 1 || outer_tag != "[type]") {
            return 1;
        }

        advancealtar::RoundRewardData round_reward;
        for (;;) {
            std::string inner_tag;
            if (ScanType(inner_tag, true) != 1) {
                break;
            }

            if (inner_tag == "[/type]") {
                script->survival_reward_data.round_reward_data.push_back(
                    round_reward);
                break;
            }

            if (inner_tag == "[round]") {
                bool scanned = false;
                const int begin_round = ScanInt(&scanned);
                if (!scanned) {
                    break;
                }
                const int end_round = ScanInt(&scanned);
                if (!scanned) {
                    break;
                }
                round_reward.begin_round = begin_round;
                round_reward.end_round = end_round;
                continue;
            }

            if (inner_tag == "[list]") {
                for (;;) {
                    advancealtar::_RewardItem reward;
                    if (!scanRewardItem(reward)) {
                        break;
                    }
                    if (static_cast<unsigned int>(reward.reward_type) <= 1u) {
                        round_reward.reward_items.push_back(reward);
                    }
                }
            }
        }
    }
}
