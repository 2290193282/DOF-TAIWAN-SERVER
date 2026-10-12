#ifndef OLDSRV_RUNTIME_TIMER_START_MATCHING_H_
#define OLDSRV_RUNTIME_TIMER_START_MATCHING_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 99_misc_global/TimerStartMatching.  The message dispatcher owns
// this TimerSig-only object; registNextTimer is a separate static scheduling
// entry point.  The long first argument is the delay in seconds, not an
// object pointer.
class TimerStartMatching : public TimerSig {
public:
    // IDA_EA: 0x863CB5E
    // IDA_NAME: _ZN18TimerStartMatchingC2Ev
    TimerStartMatching();

    // IDA_EA: 0x863F4BC (D2), 0x863F4EC (D0)
    // IDA_NAME: _ZN18TimerStartMatchingD2Ev / _ZN18TimerStartMatchingD0Ev
    virtual ~TimerStartMatching();

    // IDA_EA: 0x8637D92
    // IDA_NAME: _ZN18TimerStartMatching12dispatch_sigEiij
    // DEMANGLED: TimerStartMatching::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int room_index, int auth_value,
                             unsigned int message);

    // IDA_EA: 0x8637E14
    // IDA_NAME: _ZN18TimerStartMatching15registNextTimerElii
    // DEMANGLED: TimerStartMatching::registNextTimer(long,int,int)
    static int registNextTimer(long delay_seconds,
                               int room_index,
                               int auth_value);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerStartMatching) == 0x04u,
              "TimerStartMatching must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_RUNTIME_TIMER_START_MATCHING_H_
