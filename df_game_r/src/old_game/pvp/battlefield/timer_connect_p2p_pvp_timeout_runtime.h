#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_CONNECT_P2P_PVP_TIMEOUT_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_CONNECT_P2P_PVP_TIMEOUT_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/Timer_ConnectP2PPvPTimeout.
// The callback has no state beyond TimerSig; the PvP room owns the timer key
// and the timeout transition.
class Timer_ConnectP2PPvPTimeout : public TimerSig {
public:
    // IDA_EA: 0x863CEE2
    // IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeoutC2Ev
    // DEMANGLED: Timer_ConnectP2PPvPTimeout::Timer_ConnectP2PPvPTimeout(void)
    Timer_ConnectP2PPvPTimeout();

    // IDA_EA: 0x863EB3E (D2), 0x863EB6E (D0)
    // IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeoutD2Ev
    // IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeoutD0Ev
    // DEMANGLED: Timer_ConnectP2PPvPTimeout::~Timer_ConnectP2PPvPTimeout()
    virtual ~Timer_ConnectP2PPvPTimeout();

    // IDA_EA: 0x8639834
    // IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeout12dispatch_sigEiij
    // DEMANGLED: Timer_ConnectP2PPvPTimeout::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int pvp_index,
                             int timer_key,
                             unsigned int payload);
};

#if defined(__i386__)
static_assert(sizeof(Timer_ConnectP2PPvPTimeout) == 4u,
              "Timer_ConnectP2PPvPTimeout must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_CONNECT_P2P_PVP_TIMEOUT_RUNTIME_H_
