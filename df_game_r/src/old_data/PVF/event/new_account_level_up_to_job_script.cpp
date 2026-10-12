#include "old_data/PVF/event/level_reward_scripts.h"

#include "old_data/PVF/common/rdar_script.h"
#include "common/manager/global_tools.h"

#include <utility>
#include <vector>

// IDA_EA: 0x837DED8
// IDA_NAME: _ZN30stNewAccountLevelUpToJobScriptC2Ev
// DEMANGLED: stNewAccountLevelUpToJobScript::stNewAccountLevelUpToJobScript(void)
stNewAccountLevelUpToJobScript::stNewAccountLevelUpToJobScript()
    : stackable_rewards(),
      equip_rewards(),
      avatar_rewards() {}

// IDA_EA: 0x837DF44
// IDA_NAME: _ZN30stNewAccountLevelUpToJobScriptD2Ev
// DEMANGLED: stNewAccountLevelUpToJobScript::~stNewAccountLevelUpToJobScript()
stNewAccountLevelUpToJobScript::~stNewAccountLevelUpToJobScript() {}

// IDA_EA: 0x89179EB
// IDA_NAME: _Z35ImportNewAccountLeventUpToJobScriptPKcR30stNewAccountLevelUpToJobScript
// DEMANGLED: ImportNewAccountLeventUpToJobScript(char const*,stNewAccountLevelUpToJobScript &)
int ImportNewAccountLeventUpToJobScript(
    const char* filename,
    stNewAccountLevelUpToJobScript& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 13865;
    }

    std::string tag;
    int result = 0;
    script.stackable_rewards.clear();

    while (ScanType(tag, true) == 1) {
        if (tag == "[reward stackable item]") {
            result = LoadRewardDataByNoType(
                script.stackable_rewards,
                std::string("[/reward stackable item]"));
            if (result) {
                return result;
            }
        } else if (tag == "[reward equip item]") {
            result = LoadRewardDataByGrowType(
                script.equip_rewards,
                std::string("[/reward equip item]"));
            if (result) {
                return result;
            }
        } else if (tag == "[reward avatar item]") {
            result = LoadRewardDataByGrowType(
                script.avatar_rewards,
                std::string("[/reward avatar item]"));
            if (result) {
                return result;
            }
        }
    }

    return 0;
}

// IDA_EA: 0x8917C99
// IDA_NAME: _Z22LoadRewardDataByNoTypeR31stStepRewardStackableItemScriptSs
// DEMANGLED: LoadRewardDataByNoType(stStepRewardStackableItemScript &,std::string)
int LoadRewardDataByNoType(stStepRewardStackableItemScript& script,
                           std::string end_tag) {
    // The original ABI carries this string, but 0x8917C99 does not inspect it.
    (void)end_tag;

    bool success = false;
    StackableItemData item_data;
    std::vector<StackableItemData> unused_items;
    (void)unused_items;

    while (true) {
        int step_index = 0;
        item_data.clear();
        unused_items.clear();

        step_index = ScanInt(&success);
        if (!success) {
            break;
        }

        item_data.item_index = ScanInt(&success);
        if (!success) {
            return 13922;
        }

        item_data.item_count = ScanInt(&success);
        if (!success) {
            return 13926;
        }

        script.set_data(step_index, item_data);
    }

    return 0;
}

// IDA_EA: 0x8917DB3
// IDA_NAME: _Z22deleteTagBracketStringRSs
// DEMANGLED: deleteTagBracketString(std::string &)
void deleteTagBracketString(std::string& text) {
    std::string bracket_text("[]");
    std::string output;
    bool changed = false;

    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == bracket_text[0] ||
            text[index] == bracket_text[1]) {
            changed = true;
        } else {
            output.push_back(text[index]);
        }
    }

    if (changed) {
        text = output;
    }
}

// IDA_EA: 0x8917F44
// IDA_NAME: _Z24LoadRewardDataByGrowTypeR25stLevelUpRewardItemScriptSs
// DEMANGLED: LoadRewardDataByGrowType(stLevelUpRewardItemScript &,std::string)
int LoadRewardDataByGrowType(stLevelUpRewardItemScript& script,
                             std::string end_tag) {
    bool success = false;
    std::string tag;
    LevelUpRewardItemTableKey key;
    LevelUpRewardItemData data;

    while (ScanType(tag, true) == 1) {
        if (tag == end_tag) {
            return 0;
        }

        deleteTagBracketString(tag);
        const int charac_job = ::getCharacterJobToEnumValue(tag);
        if (charac_job == -1) {
            return 13971;
        }

        key.charac_job = static_cast<uint8_t>(charac_job);
        key.first_grow_type = static_cast<uint8_t>(ScanInt(&success));
        if (!success) {
            break;
        }

        while (true) {
            key.charac_level = static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                break;
            }
            data.item_id = ScanInt(&success);
            if (!success) {
                return 13987;
            }
            data.count = ScanInt(&success);
            if (!success) {
                return 13991;
            }
            script.reward_items.insert(std::make_pair(key, data));
        }
    }

    return 0;
}
