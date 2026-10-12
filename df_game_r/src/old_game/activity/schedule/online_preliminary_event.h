#ifndef OLDSRV_OLD_GAME_EVENT_ONLINE_PRELIMINARY_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_ONLINE_PRELIMINARY_EVENT_H_

#include "old_game/activity/reward/event_base.h"

// IDA unit: 99_misc_global/COnlinePreliminaryEvent.
// The original C2 at 0x81BB490 allocates no state after CEventBase; its
// vtable is at 0x8BBF968.
class COnlinePreliminaryEvent : public CEventBase {
public:
    // IDA_EA: 0x81BB490
    // IDA_NAME: _ZN23COnlinePreliminaryEventC2Ev
    COnlinePreliminaryEvent();

    // IDA_EA: 0x81BB4AC (D2), 0x81BB4DC (D0)
    // IDA_NAME: _ZN23COnlinePreliminaryEventD2Ev
    // IDA_NAME: _ZN23COnlinePreliminaryEventD0Ev
    virtual ~COnlinePreliminaryEvent();

    // IDA_EA: 0x81BB4FA / 0x81BB5E2 / 0x81BB56E
    // IDA_NAME: _ZN23COnlinePreliminaryEvent10StartEventEv
    // IDA_NAME: _ZN23COnlinePreliminaryEvent8EndEventEv
    // IDA_NAME: _ZN23COnlinePreliminaryEvent10StartEventE10Word_Param
    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(COnlinePreliminaryEvent) == 0x0cu,
              "COnlinePreliminaryEvent IDA size must stay 0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_ONLINE_PRELIMINARY_EVENT_H_
