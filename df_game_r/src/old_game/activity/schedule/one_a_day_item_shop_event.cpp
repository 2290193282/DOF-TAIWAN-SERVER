#include "old_game/activity/schedule/one_a_day_item_shop_event.h"

#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/itemshop/item_shop_script.h"
#include "old_data/db/event/db_event_item_and_levelup_dispatchers.h"
#include "old_game/activity/schedule/event_messages.h"
#include "old_game/progression/quest/quest_delivery_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"

namespace {

// IDA data: correctQuestIds @0x9370168, read by 0x81BA1F4.
const int kCorrectQuestIds[3] = {0x1567, 0x1568, 0x1569};

}  // namespace

// IDA_EA: 0x81B9FB4
// IDA_NAME: _ZN20OneADayItemShopEventC2Ev
// DEMANGLED: OneADayItemShopEvent::OneADayItemShopEvent(void)
OneADayItemShopEvent::OneADayItemShopEvent()
    : CEventBase() {}

// IDA_EA: 0x81B9FD0 (D2), 0x81BA000 (D0)
// IDA_NAME: _ZN20OneADayItemShopEventD2Ev /
//           _ZN20OneADayItemShopEventD0Ev
// DEMANGLED: OneADayItemShopEvent::~OneADayItemShopEvent()
OneADayItemShopEvent::~OneADayItemShopEvent() {}

// IDA_EA: 0x81BA01E
// IDA_NAME: _ZN20OneADayItemShopEvent10StartEventEv
// DEMANGLED: OneADayItemShopEvent::StartEvent(void)
void OneADayItemShopEvent::StartEvent() {
    SetEventFlag(true);
}

// IDA_EA: 0x81BA03A
// IDA_NAME: _ZN20OneADayItemShopEvent10StartEventE10Word_Param
// DEMANGLED: OneADayItemShopEvent::StartEvent(Word_Param)
void OneADayItemShopEvent::StartEvent(Word_Param param) {
    // 0x81BA03A calls virtual slot 2 and logs the existing CEventBase add-info;
    // it does not decode or store the incoming Word_Param itself.
    (void)param;
    StartEvent();
    cMyTrace trace("virtual void OneADayItemShopEvent::StartEvent(Word_Param)",
                   32,
                   0);
    trace("OneADayItemShopEvent(AddInfo:%d,%d) Event Start!",
          add_info_.word_00,
          add_info_.word_02);
}

// IDA_EA: 0x81BA0AE
// IDA_NAME: _ZN20OneADayItemShopEvent8EndEventEv
// DEMANGLED: OneADayItemShopEvent::EndEvent(void)
void OneADayItemShopEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void OneADayItemShopEvent::EndEvent()", 45, 0);
    trace("OneADayItemShopEvent Event End!");
}

// IDA_EA: 0x81BA100
// IDA_NAME: _ZN20OneADayItemShopEvent11StartActionEv
// DEMANGLED: OneADayItemShopEvent::StartAction(void)
void OneADayItemShopEvent::StartAction() {
    (void)G_GameWorld()->ResetOneADayItemShop();
}

// IDA_EA: 0x81BA116
// IDA_NAME: _ZN20OneADayItemShopEvent8deliveryER11DeliveryMsg
// DEMANGLED: OneADayItemShopEvent::delivery(DeliveryMsg &)
void OneADayItemShopEvent::delivery(DeliveryMsg& message) {
    if (message.delivery_type == 1) {
        questClearLog(message);
    } else if (message.delivery_type == 2) {
        buyItemLog(message);
    }
}

// IDA_EA: 0x81BA156
// IDA_NAME: _ZN20OneADayItemShopEvent13deliveryStartEP9CDelivery
// DEMANGLED: OneADayItemShopEvent::deliveryStart(CDelivery *)
void OneADayItemShopEvent::deliveryStart(CDelivery* delivery) {
    if (delivery) {
        delivery->attach(1, this);
        delivery->attach(2, this);
    }
}

// IDA_EA: 0x81BA19C
// IDA_NAME: _ZN20OneADayItemShopEvent11deliveryEndEP9CDelivery
// DEMANGLED: OneADayItemShopEvent::deliveryEnd(CDelivery *)
void OneADayItemShopEvent::deliveryEnd(CDelivery* delivery) {
    if (delivery) {
        delivery->detach(1, this);
        delivery->detach(2, this);
    }
}

// IDA_EA: 0x81BA1E2
// IDA_NAME: _ZN20OneADayItemShopEvent9isQuestIdEi
// DEMANGLED: OneADayItemShopEvent::isQuestId(int)
int OneADayItemShopEvent::isQuestId(int quest_id) {
    for (int index = 0; index != 3; ++index) {
        if (kCorrectQuestIds[index] == quest_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x81BA21E
// IDA_NAME: _ZN20OneADayItemShopEvent13questClearLogER11DeliveryMsg
// DEMANGLED: OneADayItemShopEvent::questClearLog(DeliveryMsg &)
void OneADayItemShopEvent::questClearLog(DeliveryMsg& message) {
    const QuestClearDeliveryMsg& quest_message =
        static_cast<const QuestClearDeliveryMsg&>(message);
    if (isQuestId(quest_message.quest_index) != 1) {
        return;
    }

    SIG_EVENT_STAGE_LOG event_log;
    event_log.event_id = 109;
    event_log.account_id = message.account_id;
    event_log.charac_no = message.charac_no;
    event_log.server_info = message.delivery_state;
    event_log.stage = 1;

    std::string stage_result;
    if (quest_message.items && !quest_message.items->empty()) {
        for (std::vector<std::pair<int, int> >::const_iterator item =
                 quest_message.items->begin();
             item != quest_message.items->end();
             ++item) {
            std::ostringstream value;
            value << item->first;
            stage_result += value.str();
            stage_result += '/';
        }
        if (stage_result.size() <= sizeof(event_log.stage_result) - 1u) {
            std::memcpy(event_log.stage_result,
                        stage_result.c_str(),
                        stage_result.size());
        }
    }

    DB_InsertEventStageLog::makeRequest(event_log);
}

// IDA_EA: 0x81BA5E8
// IDA_NAME: _ZN20OneADayItemShopEvent10buyItemLogER11DeliveryMsg
// DEMANGLED: OneADayItemShopEvent::buyItemLog(DeliveryMsg &)
void OneADayItemShopEvent::buyItemLog(DeliveryMsg& message) {
    const BuyItemDeliveryMsg& purchase =
        static_cast<const BuyItemDeliveryMsg&>(message);
    CItemShop* const item_shop = G_CDataManager()->get_item_shop();
    if (item_shop->IsOneADayShop(purchase.item_id) != 1) {
        return;
    }

    SIG_EVENT_STAGE_LOG event_log;
    event_log.event_id = 109;
    event_log.account_id = message.account_id;
    event_log.charac_no = message.charac_no;
    event_log.server_info = message.delivery_state;
    event_log.stage = 9;

    if (purchase.item_count != 0) {
        std::ostringstream value;
        value << static_cast<unsigned long>(purchase.item_count) << '/';
        const std::string stage_result = value.str();
        if (stage_result.size() <= sizeof(event_log.stage_result) - 1u) {
            std::memcpy(event_log.stage_result,
                        stage_result.c_str(),
                        stage_result.size());
        }
    }

    DB_InsertEventStageLog::makeRequest(event_log);
}
