#include "old_game/pvp/battlefield/relay_battle_timer_runtime.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/arena/pvp_room.h"

// IDA_EA: 0x863C7DE
// IDA_NAME: _ZN21TimerStartRelayBattleC2Ev
// DEMANGLED: TimerStartRelayBattle::TimerStartRelayBattle(void)
TimerStartRelayBattle::TimerStartRelayBattle() : TimerSig() {}

// IDA_EA: 0x863FE7C (D2), 0x863FEAC (D0)
// IDA_NAME: _ZN21TimerStartRelayBattleD2Ev
// IDA_NAME: _ZN21TimerStartRelayBattleD0Ev
// DEMANGLED: TimerStartRelayBattle::~TimerStartRelayBattle()
TimerStartRelayBattle::~TimerStartRelayBattle() {}

// IDA_EA: 0x863640C
// IDA_NAME: _ZN21TimerStartRelayBattle12dispatch_sigEiij
// DEMANGLED: TimerStartRelayBattle::dispatch_sig(int,int,unsigned int)
int TimerStartRelayBattle::dispatch_sig(int pvp_index, int timer_key,
                                        unsigned int) {
    PvP_Room* const room = G_CGameManager()->GetPvp(pvp_index, 0, 0);
    if (!room || room->relay_battle_mgr_.GetTimerKey() != timer_key) {
        return 0;
    }

    room->relay_battle_mgr_.OnFight();
    return 1;
}
