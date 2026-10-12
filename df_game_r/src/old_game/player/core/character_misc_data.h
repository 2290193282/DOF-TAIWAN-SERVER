#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_MISC_DATA_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_MISC_DATA_H_

#include <stdint.h>

#include <vector>

#include "common/manager/play_info_public_abi.h"
#include "old_game/economy/trade/trade_slot.h"


struct CUser;

// IDA unit: 04_character_select_create/Character.
struct Character : public IData, public PlayInfo {
    Character();
    ~Character();

    Character& operator+=(const Character& rhs);
    Character* Reset();

    uint32_t dword_3c;
};

static_assert(sizeof(Character) == 0x40u,
              "Character IDA ABI size must stay 0x40");
static_assert(offsetof(Character, dword_3c) == 0x3cu,
              "Character update count must stay at +0x3c");

// IDA unit: 04_character_select_create/CharacterEtcStats.
struct CharacterEtcStats {
    CharacterEtcStats();
    ~CharacterEtcStats();

    bool clear();

    std::vector<int> values;
    int raw_0c[4];  // IDA offset follows the i386 std::vector<int>.
};

// IDA unit: 04_character_select_create/stCharacLinkSystemData.
struct stCharacLinkSystemData {
    stCharacLinkSystemData();
    ~stCharacLinkSystemData();

    // One four-int row read by importCharacLinkSystemScript.  The 0627
    // characlinksystem.etc rows use [min level, max level, rate, value].
    int minimum_character_level;
    int maximum_character_level;
    int bonus_rate_percent;
    int bonus_value;
};

// IDA unit: 04_character_select_create/stCharacLinkSystem_t.
struct stCharacLinkSystem_t {
    stCharacLinkSystem_t();
    ~stCharacLinkSystem_t();

    // Dispatcher_SelectStriker @0x81E4066 reads this first dword through
    // CDataManager+0x629c as the minimum level for striker selection.
    int striker_skill_minimum_level;
    std::vector<stCharacLinkSystemData> low_link_character_exp_bonus_table;
    std::vector<stCharacLinkSystemData> low_link_character_gold_bonus_table;
    std::vector<stCharacLinkSystemData> high_link_character_exp_bonus_table;
    std::vector<stCharacLinkSystemData> high_link_character_gold_bonus_table;
};

// IDA unit: 04_character_select_create/stQuestTargetCharacter_t.
struct stQuestTargetCharacter_t {
    stQuestTargetCharacter_t(int character_job_value, int grow_type_value,
                             int level_value);
    ~stQuestTargetCharacter_t();

    bool operator==(const stQuestTargetCharacter_t& other) const;

    int character_job;
    int grow_type;
    int level;
};


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_MISC_DATA_H_
