#include "old_game/social/party/cparty_tod_clear_reward_runtime.h"

#include <utility>
#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const TIMER_MESSAGE kTimerTodClearReward =
    static_cast<TIMER_MESSAGE>(44);
const TIMER_MESSAGE kTimerDungeonClear =
    static_cast<TIMER_MESSAGE>(21);
const TIMER_MESSAGE kTimerTodFollowupKey =
    static_cast<TIMER_MESSAGE>(36);

}  // namespace

// IDA_EA: 0x85BA906
// IDA_NAME: _ZN6CParty16OnTODRewardTimerE13TIMER_MESSAGE
// DEMANGLED: CParty::OnTODRewardTimer(TIMER_MESSAGE)
int CParty::OnTODRewardTimer(TIMER_MESSAGE timer_message) {
    if (timer_message == kTimerTodClearReward) {
        _Send_TOD_ClearReward();
        SetEPLPState(2);

        const int dungeon_clear_key = gen_timer_key(kTimerDungeonClear);
        G_TimerQueue()->InsertTimer(1, GetPartyIndex(), kTimerDungeonClear,
                                    60, dungeon_clear_key, 0);
        set_state(1);

        const int followup_key = gen_timer_key(kTimerTodFollowupKey);
        G_TimerQueue()->InsertTimer(1, GetPartyIndex(),
                                    kTimerTodClearReward, 8, followup_key,
                                    kTimerTodFollowupKey);
    }
    return 1;
}

// IDA_EA: 0x85BAA06
// IDA_NAME: _ZN6CParty21_Send_TOD_ClearRewardEv
// DEMANGLED: CParty::_Send_TOD_ClearReward(void)
int CParty::_Send_TOD_ClearReward() {
    PacketGuard packet;
    packet.put_header(0, 347);
    packet.put_int(battle_data_.tod_clear_reward_packet_value());

    TOD_UserState* const tod_state = static_cast<TOD_UserState*>(
        getManager()->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
    packet.put_short(tod_state->getLastEnterLayer().GetLayer());
    packet.put_byte(10);

    const CBattle_Field::CTowerOfDespairRewardData& reward =
        battle_field_.tower_reward();
    for (std::vector<std::pair<int, int> >::const_iterator it =
             reward.today_reward_items.begin();
         it != reward.today_reward_items.end();
         ++it) {
        packet.put_int(it->first);
        packet.put_int(it->second);
    }
    for (size_t slot = 0;
         slot < 9u - reward.today_reward_items.size();
         ++slot) {
        packet.put_int(-1);
        packet.put_int(0);
    }

    packet.put_int(reward.period_reward.first);
    packet.put_int(reward.period_reward.second);
    packet.finalize(true);
    send_to_party(packet);
    return 1;
}

// IDA_EA: 0x863CFC2
// IDA_NAME: _ZN26Timer_TOD_Send_ClearRewardC2Ev
// DEMANGLED: Timer_TOD_Send_ClearReward::Timer_TOD_Send_ClearReward(void)
Timer_TOD_Send_ClearReward::Timer_TOD_Send_ClearReward() : TimerSig() {}

// IDA_EA: 0x863E880 (D2), 0x863E8B0 (D0)
// IDA_NAME: _ZN26Timer_TOD_Send_ClearRewardD2Ev
// IDA_NAME: _ZN26Timer_TOD_Send_ClearRewardD0Ev
// DEMANGLED: Timer_TOD_Send_ClearReward::~Timer_TOD_Send_ClearReward()
Timer_TOD_Send_ClearReward::~Timer_TOD_Send_ClearReward() {}

// IDA_EA: 0x863A89E
// IDA_NAME: _ZN26Timer_TOD_Send_ClearReward12dispatch_sigEiij
// DEMANGLED: Timer_TOD_Send_ClearReward::dispatch_sig(int,int,unsigned int)
int Timer_TOD_Send_ClearReward::dispatch_sig(int party_index,
                                             int timer_key,
                                             unsigned int timer_message) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            static_cast<TIMER_MESSAGE>(timer_message), timer_key)) {
        return 0;
    }
    return party->OnTODRewardTimer(
               static_cast<TIMER_MESSAGE>(timer_message)) == 1;
}
