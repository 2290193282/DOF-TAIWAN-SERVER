#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_CHECK_PAID_PCROOM_TERM_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_CHECK_PAID_PCROOM_TERM_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// TimerDispatcher owns the single stateless callback at the old paid-PC-room
// expiry slot.  The queued object id is a CUser UID and param2 is the account
// id that prevents a recycled user slot from receiving the expiration event.
class TimerCheckPaidPCRoomTerm : public TimerSig {
public:
    // IDA_EA: 0x863C362
    // IDA_NAME: _ZN24TimerCheckPaidPCRoomTermC2Ev
    TimerCheckPaidPCRoomTerm();

    // IDA_EA: 0x8640926 / 0x8640956
    // IDA_NAME: _ZN24TimerCheckPaidPCRoomTermD2Ev
    //           _ZN24TimerCheckPaidPCRoomTermD0Ev
    virtual ~TimerCheckPaidPCRoomTerm();

    // IDA_EA: 0x8634B60
    // IDA_NAME: _ZN24TimerCheckPaidPCRoomTerm12dispatch_sigEiij
    virtual int dispatch_sig(int user_uid, int premium_type,
                             unsigned int account_id);
};

#if defined(__i386__)
static_assert(sizeof(TimerCheckPaidPCRoomTerm) == 4u,
              "TimerCheckPaidPCRoomTerm must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_CHECK_PAID_PCROOM_TERM_RUNTIME_H_
