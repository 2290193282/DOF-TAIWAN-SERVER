#include "old_game/economy/inventory/item_lock_timer.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"


// IDA_EA: 0x863CDCA
// IDA_NAME: _ZN22TimerItemUnlockWaitingC2Ev
// DEMANGLED: TimerItemUnlockWaiting::TimerItemUnlockWaiting(void)
TimerItemUnlockWaiting::TimerItemUnlockWaiting()
    : TimerSig() {}

// IDA_EA: 0x863EDFC
// IDA_NAME: _ZN22TimerItemUnlockWaitingD2Ev
// DEMANGLED: TimerItemUnlockWaiting::~TimerItemUnlockWaiting()
// IDA_EA: 0x863EE2C
// IDA_NAME: _ZN22TimerItemUnlockWaitingD0Ev
// DEMANGLED: TimerItemUnlockWaiting::~TimerItemUnlockWaiting()
TimerItemUnlockWaiting::~TimerItemUnlockWaiting() {}

// IDA_EA: 0x86390EA
// IDA_NAME: _ZN22TimerItemUnlockWaiting12dispatch_sigEiij
// DEMANGLED: TimerItemUnlockWaiting::dispatch_sig(int,int,unsigned int)
int TimerItemUnlockWaiting::dispatch_sig(int object_id,
                                          int param1,
                                          unsigned int charac_no) {
    CUser* const user = G_CGameManager()->getUser(object_id, 0);
    if (!user) {
        return 0;
    }
    if (!user->getCurCharacR()) {
        return 0;
    }
    if (static_cast<unsigned int>(user->getCurCharacNo()) != charac_no) {
        return 0;
    }
    user->OnItemUnlockWaitTimeout(param1, static_cast<int>(charac_no));
    return 1;
}
