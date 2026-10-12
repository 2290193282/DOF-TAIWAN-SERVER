#include "infra/concurrent/timer/timer_death_tower.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x863CAEE
// IDA_NAME: _ZN15TimerDeathTowerC2Ev
// DEMANGLED: TimerDeathTower::TimerDeathTower(void)
TimerDeathTower::TimerDeathTower() : TimerSig() {}

// IDA_EA: 0x863F5F4 (D2), 0x863F624 (D0)
// IDA_NAME: _ZN15TimerDeathTowerD2Ev / _ZN15TimerDeathTowerD0Ev
// DEMANGLED: TimerDeathTower::~TimerDeathTower()
TimerDeathTower::~TimerDeathTower() {}

// IDA_EA: 0x86377CC
// IDA_NAME: _ZN15TimerDeathTower12dispatch_sigEiij
// DEMANGLED: TimerDeathTower::dispatch_sig(int,int,unsigned int)
int TimerDeathTower::dispatch_sig(int object_id,
                                  int timer_key,
                                  unsigned int timer_message) {
    WongWork::CDeathTower* const tower =
        G_CGameManager()->getDeathTower(object_id);
    if (!tower) {
        return 0;
    }

    (void)tower->onTimer(static_cast<TIMER_MESSAGE>(timer_message),
                         static_cast<unsigned int>(timer_key));
    return 1;
}
