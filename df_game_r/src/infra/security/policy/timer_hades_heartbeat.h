#ifndef OLDSRV_GLOBAL_TIMER_HADES_HEARTBEAT_H_
#define OLDSRV_GLOBAL_TIMER_HADES_HEARTBEAT_H_

#include "infra/concurrent/timer/timer_sig.h"

class Timer_HadesHeartBeat : public TimerSig {
public:
    // IDA_EA: 0x863CFA6
    // IDA_NAME: _ZN20Timer_HadesHeartBeatC2Ev
    Timer_HadesHeartBeat();

    // IDA_EA: 0x863E91C (D2), 0x863E94C (D0)
    virtual ~Timer_HadesHeartBeat();

    // IDA_EA: 0x863A760
    // IDA_NAME: _ZN20Timer_HadesHeartBeat12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int ignored_param,
                             unsigned int ignored_value);

    // IDA_EA: 0x863A7C2
    // IDA_NAME: _ZN20Timer_HadesHeartBeat15registNextTimerEv
    static int registNextTimer();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Timer_HadesHeartBeat) == 0x04u,
              "Timer_HadesHeartBeat must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_TIMER_HADES_HEARTBEAT_H_
