#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_DELIVERY_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_DELIVERY_RUNTIME_H_

#include "old_game/economy/mall/delivery_runtime.h"

#include <utility>
#include <vector>

// IDA unit: 08_quest_skill_growth/QuestClearDeliveryMsg.
// IDA_EA: C2 @0x8697914; the base DeliveryMsg is +0x00 and the five
// explicitly zeroed fields are +0x10 through +0x20.
struct QuestClearDeliveryMsg : public DeliveryMsg {
    QuestClearDeliveryMsg();

    int quest_index;                               // +0x10
    int reward_exp;                                // +0x14
    int reward_gold;                               // +0x18
    int reward_quest_point;                        // +0x1c
    std::vector<std::pair<int, int> >* items;      // +0x20
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(QuestClearDeliveryMsg) == 0x24u,
              "QuestClearDeliveryMsg IDA ABI size must stay 0x24");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_QUEST_DELIVERY_RUNTIME_H_
