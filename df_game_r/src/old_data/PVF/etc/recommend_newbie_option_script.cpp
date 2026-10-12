#include "old_data/PVF/etc/recommend_newbie_option_script.h"

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string>

namespace {

const char* const kAvatarVariationTags[47] = {
    "[none]", "[HP MAX]", "[MP MAX]", "[PHYSICAL_ATTACK]",
    "[PHYSICAL_DEFENSE]", "[MAGICAL_ATTACK]", "[MAGICAL_DEFENSE]",
    "[EQUIPMENT_PHYSICAL_ATTACK]", "[EQUIPMENT_PHYSICAL_DEFENSE]",
    "[EQUIPMENT_MAGICAL_ATTACK]", "[EQUIPMENT_MAGICAL_DEFENSE]",
    "[ELEMENT_TOLERANCE_FIRE]", "[ELEMENT_TOLERANCE_WATER]",
    "[ELEMENT_TOLERANCE_DARK]", "[ELEMENT_TOLERANCE_LIGHT]",
    "[ELEMENT_TOLERANCE_ALL]", "[ACTIVESTATUS_TOLERANCE_SLOW]",
    "[ACTIVESTATUS_TOLERANCE_FREEZE]",
    "[ACTIVESTATUS_TOLERANCE_POISON]",
    "[ACTIVESTATUS_TOLERANCE_STUN]",
    "[ACTIVESTATUS_TOLERANCE_CURSE]",
    "[ACTIVESTATUS_TOLERANCE_BLIND]",
    "[ACTIVESTATUS_TOLERANCE_LIGHTNING]",
    "[ACTIVESTATUS_TOLERANCE_STONE]",
    "[ACTIVESTATUS_TOLERANCE_SLEEP]",
    "[ACTIVESTATUS_TOLERANCE_BURN]",
    "[ACTIVESTATUS_TOLERANCE_WEAPON_BREAK]",
    "[ACTIVESTATUS_TOLERANCE_BLEEDING]",
    "[ACTIVESTATUS_TOLERANCE_PIERCE]",
    "[ACTIVESTATUS_TOLERANCE_STUCK]",
    "[ACTIVESTATUS_TOLERANCE_CONFUSE]",
    "[ACTIVESTATUS_TOLERANCE_HOLD]",
    "[ACTIVESTATUS_TOLERANCE_ALL]", "[INVENTORY_MAX_WEIGHT]",
    "[HP_REGENRATE]", "[MP_REGENRATE]", "[MP_REGENRATE_PERCENT_UP]",
    "[MOVE_SPEED]", "[ATTACK_SPEED]", "[CAST_SPEED]", "[HIT_RECOVERY]",
    "[JUMP_POWER]", "[CRITICAL]", "[SKILL_LEVEL]",
    "[PHYSICAL ABSOLUTE DEFENSE]", "[MAGICAL ABSOLUTE DEFENSE]",
    "[STUCK ON ATTACK]"};

int avatarVariationIndex(const std::string& tag) {
    for (int index = 0; index <= 46; ++index) {
        if (tag == kAvatarVariationTags[index]) {
            return index;
        }
    }
    return -1;
}

int growTypeFromText(const std::string& text) {
    if (text == "ranger" || text == "nenmaster" ||
        text == "soulbringer" || text == "rogue_dagger" ||
        text == "elementalmaster" || text == "crusader_buff" ||
        text == "elementalbomber") {
        return 0;
    }
    if (text == "mechanic" || text == "striker" ||
        text == "berserker" || text == "rogue_dual" ||
        text == "summoner" || text == "crusader_battle" ||
        text == "glacialmaster") {
        return 1;
    }
    if (text == "launcher" || text == "streetfighter" ||
        text == "asura" || text == "necromencer" || text == "witch" ||
        text == "infighter") {
        return 2;
    }
    if (text == "spitfire_physical" || text == "grappler" ||
        text == "weaponmaster" || text == "battlemage_physical" ||
        text == "exorcist_physical") {
        return 3;
    }
    if (text == "spitfire_magical" || text == "battlemage_magical" ||
        text == "exorcist_magical" || text == "demonic swordman") {
        return 4;
    }
    if (text == "avenger" || text == "creator mage") {
        return 5;
    }
    return -1;
}

bool scanAvatarIndexSet(tagRecommendNewbieOption& option, int grade) {
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/avatar index set]") {
            return true;
        }
        if (tag != "[avatar index set]") {
            continue;
        }

        for (int slot = 0; slot <= 8; ++slot) {
            std::string value;
            if (ScanStr(&value) != 1) {
                return false;
            }

            const int variation = avatarVariationIndex(value);
            if (variation >= 0 && grade >= 1 && grade <= 3) {
                option.avatar_variation_index[slot][grade - 1] = variation;
            }

            // The old parser consumes an extra number only for the two
            // absolute-defense avatar fields at slots four and five.
            if ((slot == 4 || slot == 5) && variation == 43) {
                bool scanned = false;
                const int value_number = ScanInt(&scanned);
                if (!scanned) {
                    return false;
                }
                if (grade >= 1 && grade <= 3) {
                    option.avatar_variation_value[slot][grade - 1] =
                        value_number;
                }
            }
        }
    }
    return true;
}

enum GrowTypeScanResult {
    kGrowTypeStopCharacter,
    kGrowTypeContinueCharacter,
    kGrowTypeCompleteRow,
    kGrowTypeFailure
};

GrowTypeScanResult scanGrowType(tagRecommendNewbieOption& option) {
    std::string grow_type;
    if (ScanStr(&grow_type) != 1) {
        return kGrowTypeStopCharacter;
    }
    option.clear();
    option.grow_type = growTypeFromText(grow_type);

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/grow type]") {
            return kGrowTypeCompleteRow;
        }
        if (tag != "[grade]") {
            continue;
        }

        bool scanned = false;
        const int grade = ScanInt(&scanned);
        if (!scanned || !scanAvatarIndexSet(option, grade)) {
            return kGrowTypeFailure;
        }
    }
    return kGrowTypeContinueCharacter;
}

bool scanCharacterJob(tagRecommendNewbieOptionTable& table) {
    std::string job_text;
    if (ScanStr(&job_text) != 1) {
        return true;
    }

    const int job = getCharacterJob(job_text.c_str());
    if (job < 0 || job >= oldsrv::config::kCharacterJobCount) {
        return true;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        tagRecommendNewbieOption option;
        if (tag == "[/character job]") {
            return true;
        }
        if (tag == "[grow type]") {
            const GrowTypeScanResult result = scanGrowType(option);
            if (result == kGrowTypeFailure) {
                return false;
            }
            if (result == kGrowTypeStopCharacter) {
                return true;
            }
            if (result == kGrowTypeCompleteRow) {
                table[job].push_back(option);
            }
        }
    }
    return true;
}

}  // namespace

// IDA_EA: 0x891AD16
// IDA_NAME: _ZN24tagRecommendNewbieOptionC2Ev
// DEMANGLED: tagRecommendNewbieOption::tagRecommendNewbieOption(void)
tagRecommendNewbieOption::tagRecommendNewbieOption() { clear(); }

// IDA_EA: 0x893908C
// IDA_NAME: _ZN24tagRecommendNewbieOptionD2Ev
// DEMANGLED: tagRecommendNewbieOption::~tagRecommendNewbieOption()
tagRecommendNewbieOption::~tagRecommendNewbieOption() {}

// IDA_EA: 0x891AD2A
// IDA_NAME: _ZN24tagRecommendNewbieOption5clearEv
// DEMANGLED: tagRecommendNewbieOption::clear(void)
bool tagRecommendNewbieOption::clear() {
    grow_type = -1;
    for (int slot = 0; slot <= 8; ++slot) {
        for (int grade = 0; grade <= 2; ++grade) {
            avatar_variation_value[slot][grade] = -1;
        }
    }
    return false;
}

// IDA_EA: 0x891AD8C
// IDA_NAME: _ZN29tagRecommendNewbieOptionTable5clearEv
// DEMANGLED: tagRecommendNewbieOptionTable::clear(void)
bool tagRecommendNewbieOptionTable::clear() {
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        push_back(std::vector<tagRecommendNewbieOption>());
    }
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        (*this)[job].clear();
        (*this)[job].reserve(6u);
    }
    return false;
}

// IDA_EA: 0x890457C
// IDA_NAME: _Z33importRecommendNewbieOptionScriptPKcR29tagRecommendNewbieOptionTable
// DEMANGLED: importRecommendNewbieOptionScript(char const*,tagRecommendNewbieOptionTable &)
int importRecommendNewbieOptionScript(const char* filename,
                                      tagRecommendNewbieOptionTable& table) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    tagRecommendNewbieOptionTable parsed;
    parsed.clear();

    bool found_start = false;
    bool found_end = false;
    bool malformed = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[newbie recommend option start]") {
            if (found_start) {
                malformed = true;
                break;
            }
            found_start = true;
            continue;
        }
        if (tag == "[newbie recommend option end]") {
            found_end = found_start;
            break;
        }
        if (tag == "[character job]") {
            if (!found_start || !scanCharacterJob(parsed)) {
                malformed = true;
                break;
            }
        }
    }
    unloadRDARScriptFile();
    if (!found_start || !found_end || malformed) {
        return 0;
    }
    table.swap(parsed);
    return 1;
}
