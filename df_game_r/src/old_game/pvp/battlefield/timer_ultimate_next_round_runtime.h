#ifndef OLDSRV_OLD_GAME_PVP_BATTLEFIELD_TIMER_ULTIMATE_NEXT_ROUND_RUNTIME_H_
#define OLDSRV_OLD_GAME_PVP_BATTLEFIELD_TIMER_ULTIMATE_NEXT_ROUND_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// The official 0627 dispatcher owns one vptr-only callback in timer slot 38.
// It is the 11007-only bridge from a finished round to the next PVF-defined
// ultimate difficulty/round sequence.
class TimerUltimateNextRoundStart : public TimerSig {
public:
    TimerUltimateNextRoundStart();
    virtual ~TimerUltimateNextRoundStart();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

static_assert(sizeof(TimerUltimateNextRoundStart) == sizeof(void*),
              "ultimate next-round timer must remain vptr-only");

#endif  // OLDSRV_OLD_GAME_PVP_BATTLEFIELD_TIMER_ULTIMATE_NEXT_ROUND_RUNTIME_H_
