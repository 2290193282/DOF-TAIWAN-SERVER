#ifndef OLDSRV_OLD_GAME_PLAYER_CORE_TIMER_PREMIUM_SERVICE_RUNTIME_H_
#define OLDSRV_OLD_GAME_PLAYER_CORE_TIMER_PREMIUM_SERVICE_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA TimerDispatcher::TimerDispatcher @0x863120A installs these three
// one-vptr handlers in message slots 4, 5 and 6 respectively.
class TimerPremiumServiceOver : public TimerSig {
public:
    // IDA_EA: 0x863485A
    virtual int dispatch_sig(int user_uid,
                             int premium_type,
                             unsigned int account_id);
};

class TimerPremiumServiceUpdate : public TimerSig {
public:
    // IDA_EA: 0x86348D4
    virtual int dispatch_sig(int user_uid,
                             int premium_type,
                             unsigned int account_id);
};

class TimerPremiumServiceStart : public TimerSig {
public:
    // IDA_EA: 0x8634A30
    virtual int dispatch_sig(int user_uid,
                             int premium_type,
                             unsigned int account_id);
};

#if defined(__i386__)
static_assert(sizeof(TimerPremiumServiceOver) == 0x04u,
              "TimerPremiumServiceOver must remain vptr-only");
static_assert(sizeof(TimerPremiumServiceUpdate) == 0x04u,
              "TimerPremiumServiceUpdate must remain vptr-only");
static_assert(sizeof(TimerPremiumServiceStart) == 0x04u,
              "TimerPremiumServiceStart must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_PLAYER_CORE_TIMER_PREMIUM_SERVICE_RUNTIME_H_
