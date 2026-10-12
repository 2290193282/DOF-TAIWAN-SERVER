#ifndef OLDSRV_OLD_GAME_GROUP_CPARTY_TOD_CLEAR_REWARD_RUNTIME_H_
#define OLDSRV_OLD_GAME_GROUP_CPARTY_TOD_CLEAR_REWARD_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 11_event_runtime/Timer_TOD_Send_ClearReward.  The concrete work
// remains CParty-owned; this vptr-only adapter validates the queued key.
class Timer_TOD_Send_ClearReward : public TimerSig {
public:
    // IDA_EA: 0x863CFC2
    // IDA_NAME: _ZN26Timer_TOD_Send_ClearRewardC2Ev
    // DEMANGLED: Timer_TOD_Send_ClearReward::Timer_TOD_Send_ClearReward(void)
    Timer_TOD_Send_ClearReward();

    // IDA_EA: 0x863E880 (D2), 0x863E8B0 (D0)
    // IDA_NAME: _ZN26Timer_TOD_Send_ClearRewardD2Ev
    // IDA_NAME: _ZN26Timer_TOD_Send_ClearRewardD0Ev
    // DEMANGLED: Timer_TOD_Send_ClearReward::~Timer_TOD_Send_ClearReward()
    virtual ~Timer_TOD_Send_ClearReward();

    // IDA_EA: 0x863A89E
    // IDA_NAME: _ZN26Timer_TOD_Send_ClearReward12dispatch_sigEiij
    // DEMANGLED: Timer_TOD_Send_ClearReward::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int timer_message);
};

#if defined(__i386__)
static_assert(sizeof(Timer_TOD_Send_ClearReward) == 4u,
              "Timer_TOD_Send_ClearReward must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_CPARTY_TOD_CLEAR_REWARD_RUNTIME_H_
