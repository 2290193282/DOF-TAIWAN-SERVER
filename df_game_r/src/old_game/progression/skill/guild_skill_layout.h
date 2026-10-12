#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_GUILD_SKILL_LAYOUT_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_GUILD_SKILL_LAYOUT_H_

#include <stdint.h>

// IDA unit: 08_quest_skill_growth/STGuildSkill.
//
// This is a five-byte packed DB record.  It is embedded sixteen times in
// STGuildDBInfoOnly at +0x45 with a five-byte stride; it is not an
// eight-byte naturally aligned gameplay value.
#pragma pack(push, 1)
struct STGuildSkill {
    // IDA_EA: 0x8693F40
    // IDA_NAME: _ZN12STGuildSkillC2Ev
    // DEMANGLED: STGuildSkill::STGuildSkill(void)
    STGuildSkill();

    int skill_id;       // IDA +0x00
    int8_t level;       // IDA +0x04
};
#pragma pack(pop)

static_assert(sizeof(STGuildSkill) == 0x05u,
              "STGuildSkill IDA ABI size must stay 0x05");

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_GUILD_SKILL_LAYOUT_H_
