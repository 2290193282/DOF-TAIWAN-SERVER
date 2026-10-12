#include "old_game/social/party/timer_dungeon_clear_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"

// IDA_EA: 0x863C5CA
// IDA_NAME: _ZN28TimerBloodDungeonClearRewardC2Ev
// DEMANGLED: TimerBloodDungeonClearReward::TimerBloodDungeonClearReward(void)
TimerBloodDungeonClearReward::TimerBloodDungeonClearReward()
    : TimerSig() {}

// IDA_EA: 0x863F06C (D2), 0x863F09C (D0)
// IDA_NAME: _ZN28TimerBloodDungeonClearRewardD2Ev /
//           _ZN28TimerBloodDungeonClearRewardD0Ev
// DEMANGLED: TimerBloodDungeonClearReward::~TimerBloodDungeonClearReward()
TimerBloodDungeonClearReward::~TimerBloodDungeonClearReward() {}

// IDA_EA: 0x8638CB4
// IDA_NAME: _ZN28TimerBloodDungeonClearReward12dispatch_sigEiij
// DEMANGLED: TimerBloodDungeonClearReward::dispatch_sig(int,int,unsigned int)
//
// The original dispatcher passes CParty's timer key as param1 and the
// TIMER_MESSAGE value as param2. Only a matching key reaches the party's
// recovered blood-reward state machine.
int TimerBloodDungeonClearReward::dispatch_sig(int party_index,
                                               int timer_key,
                                               unsigned int timer_message) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party || !party->check_timer_key(
                      static_cast<TIMER_MESSAGE>(timer_message), timer_key)) {
        return 0;
    }

    return party->onBloodRewardTimer(
               static_cast<TIMER_MESSAGE>(timer_message)) == 1;
}
