#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_ITEM_DROP_RATE_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_ITEM_DROP_RATE_RUNTIME_H_

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

namespace QuickParty {

// IDA unit: 08_quest_skill_growth/QuickParty/QuestItemDropRate.
// The old record reuses IRandomwReward's data pointer and adds only the
// random-buff selector at +0x08.
struct QuestItemDropRate : public IRandomwReward {
    // IDA_EA: 0x826D398
    // IDA_NAME: _ZN10QuickParty17QuestItemDropRate7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    // DEMANGLED: QuickParty::QuestItemDropRate::process(
    //     QuickParty::RewardData_Param, QuickParty::RewardData_Result &)
    virtual int process(RewardData_Param param, RewardData_Result& result);

    int random_buff_type;
};

#if defined(__i386__)
static_assert(sizeof(QuestItemDropRate) == 0x0cu,
              "QuestItemDropRate must retain the old 12-byte ABI");
#endif

}  // namespace QuickParty

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_ITEM_DROP_RATE_RUNTIME_H_
