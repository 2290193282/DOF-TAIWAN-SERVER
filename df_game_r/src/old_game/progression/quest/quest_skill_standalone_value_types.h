#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_STANDALONE_VALUE_TYPES_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_STANDALONE_VALUE_TYPES_H_

#include <stdint.h>

#include <vector>

#include "common/manager/play_info_public_abi.h"
#include "old_data/PVF/etc/quest_movie_info.h"

// IDA unit: 08_quest_skill_growth/CNRepeatQuestList.
// This object is the vector itself, with no wrapper fields before or after it.
struct CNRepeatQuestList {
    CNRepeatQuestList();
    ~CNRepeatQuestList();

    void clear();

    std::vector<int> quest_indices;  // +0x00
};

// IDA unit: 08_quest_skill_growth/Skill.
// IData is the original empty marker base.  The four scalar words are proven
// by both Reset and operator+=; the fourth word is incremented on every merge.
struct Skill : public IData {
    Skill();
    ~Skill();

    Skill* Reset();
    Skill& operator+=(const Skill& other);

    uint32_t value_00;       // +0x00
    uint32_t value_04;       // +0x04
    uint32_t value_08;       // +0x08
    uint32_t merge_count_0c; // +0x0c
};

#if defined(__i386__)
static_assert(sizeof(std::vector<int>) == 0x0cu,
              "old-server i386 std::vector ABI must stay twelve bytes");
static_assert(sizeof(CNRepeatQuestList) == 0x0cu,
              "CNRepeatQuestList must be its vector child only");
static_assert(sizeof(Skill) == 0x10u,
              "Skill must preserve its four-word i386 layout");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_STANDALONE_VALUE_TYPES_H_
