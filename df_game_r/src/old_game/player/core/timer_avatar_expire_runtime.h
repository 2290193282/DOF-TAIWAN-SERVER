#ifndef OLDSRV_OLD_GAME_USERINFO_TIMER_AVATAR_EXPIRE_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_TIMER_AVATAR_EXPIRE_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

class CUser;

// IDA unit: 06_userinfo_stat_inventory/TimerAvatarExpire.  It is a vptr-only
// TimerSig derivative.  TimerDispatcher owns the message-table registration;
// this C-owned unit restores only the original gameplay callbacks.
class TimerAvatarExpire : public TimerSig {
public:
    // IDA_EA: 0x8632A78
    // IDA_NAME: _ZN17TimerAvatarExpire12dispatch_sigEiij
    // DEMANGLED: TimerAvatarExpire::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);

    // IDA_EA: 0x8632A82
    // IDA_NAME: _ZN17TimerAvatarExpire18CheckExpiredAvatarEP5CUser
    // DEMANGLED: TimerAvatarExpire::CheckExpiredAvatar(CUser *)
    int CheckExpiredAvatar(CUser* user);

    // IDA_EA: 0x8632B32
    // IDA_NAME: _ZN17TimerAvatarExpire11RegistTimerEP5CUser
    // DEMANGLED: TimerAvatarExpire::RegistTimer(CUser *)
    int RegistTimer(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(TimerAvatarExpire) == 0x04u,
              "TimerAvatarExpire must retain its one-vptr TimerSig ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_TIMER_AVATAR_EXPIRE_RUNTIME_H_
