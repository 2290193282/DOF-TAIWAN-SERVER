#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_BOSS_TOWER_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_BOSS_TOWER_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/Timer_TowerOnTime. The callback owns no
// state beyond TimerSig; the tower object is looked up from the static pool.
class Timer_TowerOnTime : public TimerSig {
public:
    // IDA_EA: 0x863D0F6
    // IDA_NAME: _ZN17Timer_TowerOnTimeC2Ev
    // DEMANGLED: Timer_TowerOnTime::Timer_TowerOnTime(void)
    Timer_TowerOnTime();

    // IDA_EA: 0x863E4D8 (D2), 0x863E508 (D0)
    // IDA_NAME: _ZN17Timer_TowerOnTimeD2Ev
    // IDA_NAME: _ZN17Timer_TowerOnTimeD0Ev
    // DEMANGLED: Timer_TowerOnTime::~Timer_TowerOnTime()
    virtual ~Timer_TowerOnTime();

    // IDA_EA: 0x863B8C4
    // IDA_NAME: _ZN17Timer_TowerOnTime12dispatch_sigEiij
    // DEMANGLED: Timer_TowerOnTime::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int tower_index,
                             int timer_key,
                             unsigned int timer_message);
};

#if defined(__i386__)
static_assert(sizeof(Timer_TowerOnTime) == 4u,
              "Timer_TowerOnTime must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_BOSS_TOWER_RUNTIME_H_
