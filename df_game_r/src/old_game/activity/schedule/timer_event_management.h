#ifndef OLDSRV_OLD_GAME_EVENT_TIMER_EVENT_MANAGEMENT_H_
#define OLDSRV_OLD_GAME_EVENT_TIMER_EVENT_MANAGEMENT_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 11_event_runtime/Timer_EventManagement.  The TimerDispatcher
// owns the sole vptr-only handler in TIMER_MESSAGE 156.
class Timer_EventManagement : public TimerSig {
public:
    // IDA_EA: 0x863AB92
    // IDA_NAME: _ZN21Timer_EventManagement12dispatch_sigEiij
    // DEMANGLED: Timer_EventManagement::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x863ABD4
    // IDA_NAME: _ZN21Timer_EventManagement15RegistNextTimerEi
    // DEMANGLED: Timer_EventManagement::RegistNextTimer(int)
    static int RegistNextTimer(int delay_seconds);

    // IDA_EA: 0x863D032 (C2), 0x863E748 (D2), 0x863E778 (D0)
    Timer_EventManagement();
    virtual ~Timer_EventManagement();
};

// IDA unit: 11_event_runtime/Timer_StayTimeEvent.  The callback has no
// instance fields; it obtains the live CEventStayTime (repeat-event 159) and
// fan-outs its recovered reward operation to entered users.
class Timer_StayTimeEvent : public TimerSig {
public:
    // IDA_EA: 0x863B928
    // IDA_NAME: _ZN19Timer_StayTimeEvent12dispatch_sigEiij
    // DEMANGLED: Timer_StayTimeEvent::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x863D166 (C2), 0x863E43C (D2), 0x863E46C (D0)
    Timer_StayTimeEvent();
    virtual ~Timer_StayTimeEvent();
};

// IDA unit: 11_event_runtime/TimerEventCoinRefill.  The callback is the
// vptr-only message-101 consumer of CCoinRefillEvent's next-refill timer.
class TimerEventCoinRefill : public TimerSig {
public:
    // IDA_EA: 0x86364C4
    // IDA_NAME: _ZN20TimerEventCoinRefill12dispatch_sigEiij
    // DEMANGLED: TimerEventCoinRefill::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x863C816 (C2), 0x863FDE0 (D2), 0x863FE10 (D0)
    TimerEventCoinRefill();
    virtual ~TimerEventCoinRefill();
};

// IDA unit: 11_event_runtime/Timer_TournamentDugeonClearStateNotiReward.
// Its state is owned by the party selected by TimerEntry::object_id, not by
// the callback object itself.
class Timer_TournamentDugeonClearStateNotiReward : public TimerSig {
public:
    // IDA_EA: 0x863B2FE
    // IDA_NAME: _ZN42Timer_TournamentDugeonClearStateNotiReward12dispatch_sigEiij
    // DEMANGLED: Timer_TournamentDugeonClearStateNotiReward::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int unused_param2);

    // IDA_EA: 0x863D0BE (C2), 0x863E5C2 (D2), 0x863E5F2 (D0)
    Timer_TournamentDugeonClearStateNotiReward();
    virtual ~Timer_TournamentDugeonClearStateNotiReward();
};

#if defined(__i386__)
static_assert(sizeof(Timer_EventManagement) == 0x04u,
              "Timer_EventManagement must remain TimerSig-only on i386");
static_assert(sizeof(Timer_StayTimeEvent) == 0x04u,
              "Timer_StayTimeEvent must remain TimerSig-only on i386");
static_assert(sizeof(TimerEventCoinRefill) == 0x04u,
              "TimerEventCoinRefill must remain TimerSig-only on i386");
static_assert(sizeof(Timer_TournamentDugeonClearStateNotiReward) == 0x04u,
              "Timer_TournamentDugeonClearStateNotiReward must remain TimerSig-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_TIMER_EVENT_MANAGEMENT_H_
