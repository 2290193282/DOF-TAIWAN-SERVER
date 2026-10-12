#ifndef OLDSRV_GLOBAL_TIMER_DISPATCHER_ABI_H_
#define OLDSRV_GLOBAL_TIMER_DISPATCHER_ABI_H_

#include "infra/concurrent/timer/timer_queue.h"
#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 13_database_dispatch/TimerDispatcher.  The old table is exactly
// 175 TimerSig pointers.  This provider restores the detector and assault
// registrations at their original fixed message indexes.
class TimerDispatcher {
public:
    // IDA_EA: 0x863120A
    // IDA_NAME: _ZN15TimerDispatcherC2Ev
    TimerDispatcher();
    // IDA_EA: 0x863298E
    // IDA_NAME: _ZN15TimerDispatcherD2Ev
    ~TimerDispatcher();

    // IDA_EA: 0x86329F6
    // IDA_NAME: _ZN15TimerDispatcher14get_dispatcherEi
    TimerSig* get_dispatcher(int message);
    // IDA_EA: 0x8632A18
    // IDA_NAME: _ZN15TimerDispatcher8dispatchER10TimerEntry
    int dispatch(TimerEntry& entry);

private:
    TimerSig* dispatchers_[175];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerDispatcher) == 0x2bcu,
              "TimerDispatcher IDA ABI size must stay 0x2bc");
#endif

#endif  // OLDSRV_GLOBAL_TIMER_DISPATCHER_ABI_H_
