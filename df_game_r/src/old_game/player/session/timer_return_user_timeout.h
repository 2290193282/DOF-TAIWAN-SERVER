#ifndef OLDSRV_RUNTIME_TIMER_RETURN_USER_TIMEOUT_H_
#define OLDSRV_RUNTIME_TIMER_RETURN_USER_TIMEOUT_H_

#include <stdint.h>

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 99_misc_global/Timer_ReturnUserTimeout.
//
// The original C++ member spelling is retained even though registNextTimer
// treats its first value as a CUser UID rather than an allocated timer object.
// It queues TIMER_MESSAGE 149; TimerDispatcher owns the one real handler
// instance at that index.
class Timer_ReturnUserTimeout : public TimerSig {
public:
    // IDA_EA: 0x863CF8A
    // IDA_NAME: _ZN23Timer_ReturnUserTimeoutC2Ev
    Timer_ReturnUserTimeout();

    // IDA_EA: 0x863E96A (D2), 0x863E99A (D0)
    // IDA_NAME: _ZN23Timer_ReturnUserTimeoutD2Ev /
    //           _ZN23Timer_ReturnUserTimeoutD0Ev
    virtual ~Timer_ReturnUserTimeout();

    // IDA_EA: 0x863A4C8
    // IDA_NAME: _ZN23Timer_ReturnUserTimeout12dispatch_sigEiij
    // DEMANGLED: Timer_ReturnUserTimeout::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid,
                             int unique_id,
                             unsigned int unused_param2);

    // IDA_EA: 0x863A706
    // IDA_NAME: _ZN23Timer_ReturnUserTimeout15registNextTimerEiij
    // DEMANGLED: Timer_ReturnUserTimeout::registNextTimer(int,int,unsigned int)
    static int registNextTimer(int user_uid,
                               int unique_id,
                               unsigned int expire_time);
};

#if defined(__i386__)
static_assert(sizeof(Timer_ReturnUserTimeout) == 0x4u,
              "Timer_ReturnUserTimeout is a one-vptr TimerSig derivative");
#endif

#endif  // OLDSRV_RUNTIME_TIMER_RETURN_USER_TIMEOUT_H_
