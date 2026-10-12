#ifndef OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_TIMER_H_
#define OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_TIMER_H_

#include "infra/concurrent/timer/timer_sig.h"


class TimerItemUnlockWaiting : public TimerSig {
public:
    // IDA_EA: 0x863CDCA
    // IDA_NAME: _ZN22TimerItemUnlockWaitingC2Ev
    // DEMANGLED: TimerItemUnlockWaiting::TimerItemUnlockWaiting(void)
    TimerItemUnlockWaiting();

    // IDA_EA: 0x86390EA
    // IDA_NAME: _ZN22TimerItemUnlockWaiting12dispatch_sigEiij
    // DEMANGLED: TimerItemUnlockWaiting::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1,
                             unsigned int charac_no);

    // IDA_EA: 0x863EDFC (D2), 0x863EE2C (D0)
    // IDA_NAME: _ZN22TimerItemUnlockWaitingD2Ev / _ZN22TimerItemUnlockWaitingD0Ev
    // DEMANGLED: TimerItemUnlockWaiting::~TimerItemUnlockWaiting()
    virtual ~TimerItemUnlockWaiting();
};


#if defined(__i386__)
static_assert(sizeof(TimerItemUnlockWaiting) == 0x04u,
              "TimerItemUnlockWaiting IDA ABI size must stay 0x04");
#endif


#endif  // OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_TIMER_H_
