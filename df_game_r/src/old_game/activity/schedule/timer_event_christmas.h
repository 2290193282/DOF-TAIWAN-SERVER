#ifndef OLDSRV_OLD_GAME_EVENT_TIMER_EVENT_CHRISTMAS_H_
#define OLDSRV_OLD_GAME_EVENT_TIMER_EVENT_CHRISTMAS_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 11_event_runtime/TimerEventChristmas.  TimerDispatcher owns the
// sole vptr-only handler at TIMER_MESSAGE 83.
class TimerEventChristmas : public TimerSig {
public:
    // IDA_EA: 0x863570A
    // IDA_NAME: _ZN19TimerEventChristmas12dispatch_sigEiij
    // DEMANGLED: TimerEventChristmas::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id,
                             int exp_rate_delta,
                             unsigned int event_id);

    // IDA_EA: 0x863C602 / 0x8640272 / 0x86402A2
    TimerEventChristmas();
    virtual ~TimerEventChristmas();
};

#if defined(__i386__)
static_assert(sizeof(TimerEventChristmas) == 0x04u,
              "TimerEventChristmas must remain TimerSig-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_TIMER_EVENT_CHRISTMAS_H_
