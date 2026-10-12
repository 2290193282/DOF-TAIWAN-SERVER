#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TIMER_VENDING_MACHINE_ITEM_NOTIFY_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TIMER_VENDING_MACHINE_ITEM_NOTIFY_H_

#include "infra/concurrent/timer/timer_sig.h"

#include <stdint.h>

// IDA unit: 99_misc_global/TimerVendingMachineItemNotify.  The original
// notification timer has only the TimerSig vptr; registration is a static
// ABI entry because the first explicit argument is the target user's UID.
class TimerVendingMachineItemNotify : public TimerSig {
public:
    // IDA_EA: 0x863CBB2
    // IDA_NAME: _ZN29TimerVendingMachineItemNotifyC2Ev
    // DEMANGLED: TimerVendingMachineItemNotify::TimerVendingMachineItemNotify(void)
    TimerVendingMachineItemNotify();

    // IDA_EA: 0x863F3D2
    // IDA_NAME: _ZN29TimerVendingMachineItemNotifyD2Ev
    // DEMANGLED: TimerVendingMachineItemNotify::~TimerVendingMachineItemNotify()
    // IDA_EA: 0x863F402
    // IDA_NAME: _ZN29TimerVendingMachineItemNotifyD0Ev
    // DEMANGLED: TimerVendingMachineItemNotify::~TimerVendingMachineItemNotify() deleting destructor
    virtual ~TimerVendingMachineItemNotify();

    // IDA_EA: 0x8637F40
    // IDA_NAME: _ZN29TimerVendingMachineItemNotify12dispatch_sigEiij
    // DEMANGLED: TimerVendingMachineItemNotify::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid, int item_id, unsigned int item_count);

    // IDA_EA: 0x8638074
    // IDA_NAME: _ZN29TimerVendingMachineItemNotify11registTimerEimjj
    // DEMANGLED: TimerVendingMachineItemNotify::registTimer(int,unsigned long,unsigned int,unsigned int)
    static int registTimer(int user_uid,
                           unsigned long item_index,
                           unsigned int item_count,
                           unsigned int delay_seconds);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerVendingMachineItemNotify) == 0x04u,
              "TimerVendingMachineItemNotify must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TIMER_VENDING_MACHINE_ITEM_NOTIFY_H_
