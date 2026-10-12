#ifndef OLDSRV_OLD_GAME_EVENT_ONE_A_DAY_ITEM_SHOP_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_ONE_A_DAY_ITEM_SHOP_EVENT_H_

#include "old_game/activity/reward/event_base.h"

// IDA unit: 07_npc_shop_dialog/OneADayItemShopEvent.
// C2 @0x81B9FB4 allocates no derived storage: this is the exact 0x0c
// CEventBase object with vtable 0x8BBEC48.
class OneADayItemShopEvent : public CEventBase {
public:
    // IDA_EA: 0x81B9FB4
    // IDA_NAME: _ZN20OneADayItemShopEventC2Ev
    // DEMANGLED: OneADayItemShopEvent::OneADayItemShopEvent(void)
    OneADayItemShopEvent();

    // IDA_EA: 0x81B9FD0 (D2), 0x81BA000 (D0)
    // IDA_NAME: _ZN20OneADayItemShopEventD2Ev /
    //           _ZN20OneADayItemShopEventD0Ev
    virtual ~OneADayItemShopEvent();

    // vtable slots 2..5 at 0x8BBEC48.
    // IDA_EA: 0x81BA01E / 0x81BA0AE / 0x81BA03A / 0x81BA100
    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void StartAction();

    // vtable slots 10..12 at 0x8BBEC48.
    // IDA_EA: 0x81BA156 / 0x81BA19C / 0x81BA116
    virtual void deliveryStart(CDelivery* delivery);
    virtual void deliveryEnd(CDelivery* delivery);
    virtual void delivery(DeliveryMsg& message);

    // IDA_EA: 0x81BA1E2 / 0x81BA21E / 0x81BA5E8
    // IDA_NAME: _ZN20OneADayItemShopEvent9isQuestIdEi /
    //           _ZN20OneADayItemShopEvent13questClearLogER11DeliveryMsg /
    //           _ZN20OneADayItemShopEvent10buyItemLogER11DeliveryMsg
    int isQuestId(int quest_id);
    void questClearLog(DeliveryMsg& message);
    void buyItemLog(DeliveryMsg& message);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(OneADayItemShopEvent) == 0x0cu,
              "OneADayItemShopEvent IDA size must stay 0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_ONE_A_DAY_ITEM_SHOP_EVENT_H_
