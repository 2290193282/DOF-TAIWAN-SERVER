#ifndef OLDSRV_OLD_GAME_EVENT_PCROOM_PLAY_TIME_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_PCROOM_PLAY_TIME_EVENT_H_

#include "old_game/activity/reward/event_base.h"

// IDA unit: 99_misc_global/CPCRoomPlayTimeEvent.
// The original C2 at 0x811C55A stores no state after CEventBase; StartAction
// is the class-specific vtable slot at 0x8B52048 + 0x14.
class CPCRoomPlayTimeEvent : public CEventBase {
public:
    // IDA_EA: 0x811C55A
    // IDA_NAME: _ZN20CPCRoomPlayTimeEventC2Ev
    CPCRoomPlayTimeEvent();

    // IDA_EA: 0x811C576 (D2), 0x811C5A6 (D0)
    // IDA_NAME: _ZN20CPCRoomPlayTimeEventD2Ev
    // IDA_NAME: _ZN20CPCRoomPlayTimeEventD0Ev
    virtual ~CPCRoomPlayTimeEvent();

    // IDA_EA: 0x811C5C4 / 0x811C630 / 0x811C616 / 0x811C754
    // IDA_NAME: _ZN20CPCRoomPlayTimeEvent10StartEventEv
    // IDA_NAME: _ZN20CPCRoomPlayTimeEvent8EndEventEv
    // IDA_NAME: _ZN20CPCRoomPlayTimeEvent10StartEventE10Word_Param
    // IDA_NAME: _ZN20CPCRoomPlayTimeEvent11StartActionEv
    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void StartAction();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CPCRoomPlayTimeEvent) == 0x0cu,
              "CPCRoomPlayTimeEvent IDA size must stay 0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PCROOM_PLAY_TIME_EVENT_H_
