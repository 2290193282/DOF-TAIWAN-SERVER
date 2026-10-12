#ifndef OLDSRV_RUNTIME_TIMER_QUERY_COUNTER_H_
#define OLDSRV_RUNTIME_TIMER_QUERY_COUNTER_H_

#include "infra/concurrent/timer/timer_sig.h"


class TimerQueryCounter : public TimerSig {
public:
    // IDA_EA: 0x863C68E
    // IDA_NAME: _ZN17TimerQueryCounterC2Ev
    // DEMANGLED: TimerQueryCounter::TimerQueryCounter(void)
    TimerQueryCounter();

    // IDA_EA: 0x864013A (D2), 0x864016A (D0)
    // IDA_NAME: _ZN17TimerQueryCounterD2Ev / _ZN17TimerQueryCounterD0Ev
    // DEMANGLED: TimerQueryCounter::~TimerQueryCounter()
    virtual ~TimerQueryCounter();

    // IDA_EA: 0x8635A60
    // IDA_NAME: _ZN17TimerQueryCounter12dispatch_sigEiij
    // DEMANGLED: TimerQueryCounter::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1,
                             unsigned int param2);

    // IDA_EA: 0x8635BDC
    // IDA_NAME: _ZN17TimerQueryCounter15RegistNextTimerEv
    // DEMANGLED: TimerQueryCounter::RegistNextTimer(void)
    int RegistNextTimer();
};


#endif  // OLDSRV_RUNTIME_TIMER_QUERY_COUNTER_H_
