#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_VALUE_TYPES_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_VALUE_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include "common/manager/striker_public_abi.h"
#include "old_game/progression/skill/guild_skill_layout.h"

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "old_data/PVF/n_quest/quest_parameter_script.h"
#include "old_data/PVF/n_quest/training_quest_script.h"

// IDA unit: 08_quest_skill_growth/STGrowthType.
struct STGrowthType {
    STGrowthType();

    int8_t growth_type;
};

// STGuildSkill is a canonical old_game/quest_skill packed DB layout.  This
// legacy header intentionally imports that one declaration while migration
// removes all legacy IDA bodies.

// IDA unit: 08_quest_skill_growth/comoboSkillData.
struct comoboSkillData {
    comoboSkillData();

    uint16_t skill_id;
    uint16_t command[6];
};

// IDA unit: 08_quest_skill_growth/buySkillInfo.
struct buySkillInfo {
    buySkillInfo();

    uint8_t job;
    uint8_t tree;
    uint8_t pad_02[2];
    int skill_id;
    int skill_level;
};

// IDA_EA: CUser::master_new_skill @0x866AFA2.
// IDA_NAME: _ZN5CUser16master_new_skillER14stBuySkillInfob
// This is the exact three-byte command object written by
// CEventCharacterHandler::_MasterNewSkill before it passes it by reference:
// selected tree, skill index, then signed level delta.  It is intentionally
// distinct from the CMD41 buySkillInfo response workspace above.
struct stBuySkillInfo {
    uint8_t skill_tree_index;
    uint8_t skill_index;
    int8_t skill_level_delta;
};

static_assert(sizeof(stBuySkillInfo) == 3u,
              "stBuySkillInfo IDA command ABI must stay three bytes");

// IDA unit: 08_quest_skill_growth/LevelUpSkill.
struct LevelUpSkill {
    LevelUpSkill();
    ~LevelUpSkill();

    LevelUpSkill* clear();

    int skill_type;
    int skill_id;
    int skill_level;
};

// IDA unit: 08_quest_skill_growth/STSkillCommandData.
struct STSkillCommandData {
    struct STCommand {
        STCommand();

        uint8_t raw[4];
    };

    STSkillCommandData();

    uint16_t word_00;
    uint8_t byte_02;
    uint8_t raw_03[5];
    uint8_t command_storage[4];
};

// IDA unit: 08_quest_skill_growth/SKILL_USING_VALIDATE_INFO.
struct SKILL_USING_VALIDATE_INFO {
    SKILL_USING_VALIDATE_INFO(int charac_job,
                              uint8_t level,
                              char first_grow_type,
                              char second_grow_type);

    int charac_job;
    uint8_t level;
    int8_t first_grow_type;
    int8_t second_grow_type;
    uint8_t pad_07;
};

// IDA unit: 08_quest_skill_growth/SkillUseItem.
struct SkillUseItem {
    SkillUseItem();
    ~SkillUseItem();

    bool clear();

    int item_index;
    int value_04;
    int value_08;
};

// IDA unit: 08_quest_skill_growth/stQuestTriggerState_t.
struct stQuestTriggerState_t {
    stQuestTriggerState_t();

    stQuestTriggerState_t* setTrigger(int16_t trigger_0,
                                      int16_t trigger_1,
                                      int16_t trigger_2);
    int getTrigger();
    int operator=(const int& packed_trigger);
    uint32_t incTirggerState(char flags);
    stQuestTriggerState_t* subTriggerState(char flags);

    uint16_t trigger_words[3];
    uint16_t pad_06;
};

static_assert(offsetof(stQuestTriggerState_t, trigger_words) == 0x00u &&
                  sizeof(stQuestTriggerState_t) == 0x08u,
              "stQuestTriggerState_t named trigger layout changed");

namespace WongWork {

// IDA unit: 08_quest_skill_growth/WongWork_STClearedQuest.
struct STClearedQuest {
    STClearedQuest();

    uint8_t clear_flag;
};

}  // namespace WongWork


#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_VALUE_TYPES_H_
