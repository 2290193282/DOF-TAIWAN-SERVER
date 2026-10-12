#include "old_data/PVF/etc/quick_party_script.h"

#include <map>
#include <string>

#include "old_data/PVF/common/rdar_script.h"

namespace {

enum QuickPartyTag {
    kQuickPartyEnd,
    kUnableToQuickParty,
    kLevelSection,
    kBasicReward,
    kRandomReward,
    kRandomRewardProbability,
    kRequirePartyCount,
    kUnknownQuickPartyTag = -1,
};

const std::map<std::string, QuickPartyTag>& QuickPartyTags() {
    // The original provider populates this lookup on the first call while its
    // local SpinLock is held.  Function-local initialization supplies the
    // same one-time, thread-safe lookup without exposing a second global.
    static const std::map<std::string, QuickPartyTag> tags = []() {
        std::map<std::string, QuickPartyTag> result;
        result["[/quick party]"] = kQuickPartyEnd;
        result["[unable to quickparty]"] = kUnableToQuickParty;
        result["[level section]"] = kLevelSection;
        result["[basic reward]"] = kBasicReward;
        result["[random reward]"] = kRandomReward;
        result["[random reward prob]"] = kRandomRewardProbability;
        result["[require party num]"] = kRequirePartyCount;
        return result;
    }();
    return tags;
}

QuickPartyTag GetQuickPartyTag(const std::string& tag) {
    const std::map<std::string, QuickPartyTag>& tags = QuickPartyTags();
    const std::map<std::string, QuickPartyTag>::const_iterator found =
        tags.find(tag);
    return found == tags.end() ? kUnknownQuickPartyTag : found->second;
}

void ReadBasicReward(QuickParty::QuickPartyData* data) {
    std::string tag;
    while (ScanType(tag, true) == 1 && tag != "[/basic reward]") {
        if (tag == "[increase gold card]") {
            bool success = false;
            while (1) {
                // The first integer is the row level.  The old code checks it
                // only as a row terminator; party-size chooses the vector row.
                (void)ScanInt(&success);
                if (!success) {
                    break;
                }

                QuickParty::LevelData level_data;
                level_data.raw_04 = ScanInt(static_cast<bool*>(0));
                std::string percent;
                level_data.raw_00 =
                    ScanStr(&percent) == 1 && percent == "%";
                data->basic_reward.level_data.push_back(level_data);
            }
        } else if (tag == "[hell ratio]") {
            std::string ignored_ratio_name;
            if (ScanStr(&ignored_ratio_name) != 1) {
                return;
            }

            data->basic_reward.raw_18 = ScanInt(static_cast<bool*>(0));
            data->basic_reward.raw_1c = ScanInt(static_cast<bool*>(0));
            bool success = false;
            while (1) {
                // As with the gold-card table, the level column only delimits
                // a row in the original representation.
                (void)ScanInt(&success);
                if (!success) {
                    break;
                }

                QuickParty::LevelDataHell level_data;
                level_data.raw_00 = ScanInt(static_cast<bool*>(0));
                level_data.raw_04 = ScanInt(static_cast<bool*>(0));
                data->basic_reward.hell_level_data.push_back(level_data);
            }
        }
    }
}

void ReadRandomReward(QuickParty::QuickPartyData* data) {
    std::string tag;
    while (ScanType(tag, true) == 1 && tag != "[/random reward]") {
        if (tag != "[reward]") {
            continue;
        }

        std::string reward_name;
        if (ScanStr(&reward_name) != 1) {
            return;
        }

        QuickParty::LevelDataRandomReward reward;
        reward.raw_00 = QuickParty::getRandomBuffTypeByName(reward_name);
        // This string is retained by the original value type but its scanner
        // return is deliberately not a failure gate.
        ScanStr(&reward.text_04);

        bool success = false;
        while (1) {
            (void)ScanInt(&success);
            if (!success) {
                break;
            }

            QuickParty::LevelData level_data;
            level_data.raw_04 = ScanInt(static_cast<bool*>(0));
            std::string percent;
            level_data.raw_00 = ScanStr(&percent) == 1 && percent == "%";
            reward.level_data_08.push_back(level_data);
        }

        data->random_rewards.insert(std::make_pair(reward.raw_00, reward));
    }
}

void ReadRandomRewardProbability(QuickParty::QuickPartyData* data) {
    std::string buff_name;
    while (ScanStr(&buff_name) == 1) {
        const int buff_type = QuickParty::getRandomBuffTypeByName(buff_name);
        if (buff_type > 10) {
            // Old control flow consumes no probability columns for an unknown
            // buff type and resumes with the next string token.
            continue;
        }

        for (int dungeon_type = 0; dungeon_type <= 6; ++dungeon_type) {
            data->random_buff_probs[buff_type][dungeon_type] =
                ScanInt(static_cast<bool*>(0));
        }
    }
}

void ReadRequirePartyCount(QuickParty::QuickPartyData* data) {
    std::string dungeon_kind;
    while (ScanStr(&dungeon_kind) == 1) {
        int dungeon_row = -1;
        if (dungeon_kind == "[normal]") {
            dungeon_row = 0;
        } else if (dungeon_kind == "[ancient]") {
            dungeon_row = 1;
        } else {
            return;
        }

        std::string difficulty_name;
        if (ScanStr(&difficulty_name) != 1) {
            return;
        }

        int difficulty_column = -1;
        if (difficulty_name == "[easy]") {
            difficulty_column = 0;
        } else if (difficulty_name == "[medium]") {
            difficulty_column = 1;
        } else if (difficulty_name == "[hard]") {
            difficulty_column = 2;
        } else if (difficulty_name == "[ultimate]") {
            difficulty_column = 3;
        } else {
            return;
        }

        int required_count = 0;
        if (ScanInt(&required_count) != 1) {
            return;
        }
        data->require_quick_party_min_count[dungeon_row][difficulty_column] =
            required_count;
    }
}

}  // namespace

namespace QuickParty {

// IDA_EA: 0x8A6E282
// IDA_NAME: _ZN10QuickParty22importQuickPartyScriptEPKcPNS_14QuickPartyDataE
// DEMANGLED: QuickParty::importQuickPartyScript(char const*,QuickParty::QuickPartyData *)
int importQuickPartyScript(const char* filename, QuickPartyData* data) {
    // `dir__29` is the null directory pointer in the old image; the RDAR
    // loader therefore receives the same empty-base form used by the other
    // Etc importers.
    if (loadRDARScriptFile("", filename) != 1) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        switch (GetQuickPartyTag(tag)) {
        case kQuickPartyEnd:
            return 1;

        case kUnableToQuickParty: {
            bool success = false;
            while (1) {
                const int dungeon_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                // IDA writes this section to QuickPartyData +0x0C.
                data->dungeon_idx_list.push_back(dungeon_index);
            }
            break;
        }

        case kLevelSection: {
            bool success = false;
            while (1) {
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                // IDA writes this section to QuickPartyData +0x00.
                data->unable_to_quick_party.push_back(level);
            }
            break;
        }

        case kBasicReward:
            ReadBasicReward(data);
            break;

        case kRandomReward:
            ReadRandomReward(data);
            break;

        case kRandomRewardProbability:
            ReadRandomRewardProbability(data);
            break;

        case kRequirePartyCount:
            ReadRequirePartyCount(data);
            break;

        case kUnknownQuickPartyTag:
        default:
            // The original tag lookup also ignores unrecognised outer tags.
            break;
        }
    }

    // A successful old import ends only at the explicit closing tag above.
    return 0;
}

}  // namespace QuickParty
