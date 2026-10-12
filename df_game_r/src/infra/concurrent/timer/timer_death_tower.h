#ifndef OLDSRV_GLOBAL_TIMER_DEATH_TOWER_H_
#define OLDSRV_GLOBAL_TIMER_DEATH_TOWER_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/TimerDeathTower.  This callback carries no
// state beyond TimerSig; the TimerEntry object id selects the active
// CDeathTower from CGameManager's pool-backed map.
class TimerDeathTower : public TimerSig {
public:
    // IDA_EA: 0x863CAEE
    // IDA_NAME: _ZN15TimerDeathTowerC2Ev
    // DEMANGLED: TimerDeathTower::TimerDeathTower(void)
    TimerDeathTower();

    // IDA_EA: 0x863F5F4 (D2), 0x863F624 (D0)
    // IDA_NAME: _ZN15TimerDeathTowerD2Ev / _ZN15TimerDeathTowerD0Ev
    // DEMANGLED: TimerDeathTower::~TimerDeathTower()
    virtual ~TimerDeathTower();

    // IDA_EA: 0x86377CC
    // IDA_NAME: _ZN15TimerDeathTower12dispatch_sigEiij
    // DEMANGLED: TimerDeathTower::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id,
                             int timer_key,
                             unsigned int timer_message);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerDeathTower) == 0x04u,
              "TimerDeathTower must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_TIMER_DEATH_TOWER_H_
