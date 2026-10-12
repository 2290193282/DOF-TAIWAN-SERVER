#include "old_game/progression/quest/quest_delivery_runtime.h"

// IDA_EA: 0x8697914
// IDA_NAME: _ZN21QuestClearDeliveryMsgC2Ev
// DEMANGLED: QuestClearDeliveryMsg::QuestClearDeliveryMsg(void)
QuestClearDeliveryMsg::QuestClearDeliveryMsg()
    : DeliveryMsg(),
      quest_index(0),
      reward_exp(0),
      reward_gold(0),
      reward_quest_point(0),
      items(0) {}
