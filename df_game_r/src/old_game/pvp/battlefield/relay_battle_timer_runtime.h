#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_RELAY_BATTLE_TIMER_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_RELAY_BATTLE_TIMER_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 99_misc_global/TimerStartRelayBattle.  The object has no
// storage beyond TimerSig; it is the timer-queue callback that changes a
// relay room from its start delay into the fight state.
class TimerStartRelayBattle : public TimerSig {
public:
    // IDA_EA: 0x863C7DE
    // IDA_NAME: _ZN21TimerStartRelayBattleC2Ev
    // DEMANGLED: TimerStartRelayBattle::TimerStartRelayBattle(void)
    TimerStartRelayBattle();

    // IDA_EA: 0x863FE7C (D2), 0x863FEAC (D0)
    // IDA_NAME: _ZN21TimerStartRelayBattleD2Ev
    // IDA_NAME: _ZN21TimerStartRelayBattleD0Ev
    // DEMANGLED: TimerStartRelayBattle::~TimerStartRelayBattle()
    virtual ~TimerStartRelayBattle();

    // IDA_EA: 0x863640C
    // IDA_NAME: _ZN21TimerStartRelayBattle12dispatch_sigEiij
    // DEMANGLED: TimerStartRelayBattle::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

#if defined(__i386__)
static_assert(sizeof(TimerStartRelayBattle) == 4u,
              "TimerStartRelayBattle must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_RELAY_BATTLE_TIMER_RUNTIME_H_
