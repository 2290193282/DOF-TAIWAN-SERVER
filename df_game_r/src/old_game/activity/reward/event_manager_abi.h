#ifndef OLDSRV_GLOBAL_EVENT_MANAGER_ABI_H_
#define OLDSRV_GLOBAL_EVENT_MANAGER_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/activity/reward/event_base.h"
#include "old_game/economy/mall/delivery_runtime.h"
#include "old_game/activity/schedule/event_runtime.h"


class PacketGuard;

// A/public lifecycle boundary for the old process-wide event manager.
// IDA_EA: CEventManager ctor/dtor @0x8114CE4/0x8114D4E.
// C still owns the event table, repeat-event bodies, DB load, and runtime
// event dispatch functions.
class CEventManager {
public:
    CEventManager();
    ~CEventManager();

    bool InitEventManager();
    int AddEvent(int event_id, CEventBase* event);
    CEventBase* GetRepeatEvent(int event_id);
    bool LoadEventFromDB();
    bool TriggerEventStart(int event_id, Word_Param param);
    bool TriggerEventEnd(int event_id);
    bool MakeNotiEventInfo(PacketGuard& guard, CUser* user);
    void BroadcastEventInfo();
    bool IsExistOverlappedExpBonusEvent();
    bool changeCharacName(CUser* user, int charac_no, char* name, int length);
    bool dailyresetData();
    int eventDeliveryNotify(int address, DeliveryMsg& msg);

    CEventBase* repeat_events_[166]; // IDA +0x000
    CDelivery* delivery_;            // IDA +0x298
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CEventManager) == 0x29Cu,
              "CEventManager IDA ABI size must stay 0x29C");
static_assert(offsetof(CEventManager, delivery_) == 0x298u,
              "CEventManager delivery offset must stay 0x298");
#endif


#endif  // OLDSRV_GLOBAL_EVENT_MANAGER_ABI_H_
