#ifndef OLDSRV_GLOBAL_TIMER_DAILY_SCHEDULE_H_
#define OLDSRV_GLOBAL_TIMER_DAILY_SCHEDULE_H_

#include "infra/concurrent/timer/timer_sig.h"


// IDA unit: 99_misc_global/Timer_DailySchedule.  This vptr-only callback is
// owned by TimerDispatcher slot 145 and refreshes the imported daily schedule
// states once per minute.
class Timer_DailySchedule : public TimerSig {
public:
    // IDA_EA: 0x812C920
    // IDA_NAME: _ZN19Timer_DailySchedule12dispatch_sigEiij
    // DEMANGLED: Timer_DailySchedule::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);

    // IDA_EA: 0x812C940
    // IDA_NAME: _ZN19Timer_DailySchedule15RegistNextTimerEv
    // DEMANGLED: Timer_DailySchedule::RegistNextTimer(void)
    static int RegistNextTimer();

    // IDA_EA: 0x863CEC6 (C2), 0x812CA26 (D2), 0x812CA56 (D0)
    Timer_DailySchedule();
    virtual ~Timer_DailySchedule();
};

#if defined(__i386__)
static_assert(sizeof(Timer_DailySchedule) == 0x04u,
              "Timer_DailySchedule must remain TimerSig-only on i386");
#endif

#endif  // OLDSRV_GLOBAL_TIMER_DAILY_SCHEDULE_H_
