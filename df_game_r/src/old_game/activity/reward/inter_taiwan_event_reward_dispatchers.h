#ifndef OLDSRV_OLD_GAME_EVENT_INTER_TAIWAN_EVENT_REWARD_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_TAIWAN_EVENT_REWARD_DISPATCHERS_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"

class CUser;

// Taiwan inner code 5.  The raw class has no fields beyond the old
// TaiwanInterfaceInterDispatcher [vptr, code] base.
class InterCheckStayTimeEvent : public TaiwanInterfaceInterDispatcher {
public:
    // IDA_EA: 0x8161FA8
    // IDA_NAME: _ZN23InterCheckStayTimeEventC2Ev
    InterCheckStayTimeEvent();

    // IDA_EA: 0x8161FCC (D2), 0x8161FFC (D0)
    // IDA_NAME: _ZN23InterCheckStayTimeEventD2Ev / D0Ev
    virtual ~InterCheckStayTimeEvent();

    // IDA_EA: 0x8160A42
    // IDA_NAME: _ZN23InterCheckStayTimeEvent12dispatch_sigEP5CUserPci
    // DEMANGLED: InterCheckStayTimeEvent::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// Taiwan inner code 16.  It has the same exact two-word base-only layout.
class InterInsertGiveBoxEventReward : public TaiwanInterfaceInterDispatcher {
public:
    // IDA_EA: 0x8162254
    // IDA_NAME: _ZN29InterInsertGiveBoxEventRewardC2Ev
    InterInsertGiveBoxEventReward();

    // IDA_EA: 0x8162278 (D2), 0x81622A8 (D0)
    // IDA_NAME: _ZN29InterInsertGiveBoxEventRewardD2Ev / D0Ev
    virtual ~InterInsertGiveBoxEventReward();

    // IDA_EA: 0x8161A8A
    // IDA_NAME: _ZN29InterInsertGiveBoxEventReward12dispatch_sigEP5CUserPci
    // DEMANGLED: InterInsertGiveBoxEventReward::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(InterCheckStayTimeEvent) == 0x08u,
              "InterCheckStayTimeEvent must retain the Taiwan ABI");
static_assert(sizeof(InterInsertGiveBoxEventReward) == 0x08u,
              "InterInsertGiveBoxEventReward must retain the Taiwan ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_TAIWAN_EVENT_REWARD_DISPATCHERS_H_
