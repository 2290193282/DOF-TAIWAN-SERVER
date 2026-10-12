#include "old_game/pvp/arena/pvp_room.h"

#include <cstring>

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85D6430
// IDA_NAME: _ZN8PvP_Room12battle_resetEv
// DEMANGLED: PvP_Room::battle_reset(void)
int PvP_Room::battle_reset() {
    lock();
    for (int seat = 0; seat <= 7; ++seat) {
        battle_live_flags_[seat] = 0;
        battle_teams_[seat] = 0;
        ready_flags_[seat] = 0;
        if (users_[seat]) {
            users_[seat]->getHackAnalyzer()->reportHackInfo();
            users_[seat]->getHackAnalyzer()->resetHackInfo();
        }
    }

    std::memset(battle_runtime_flags_, 0, sizeof(battle_runtime_flags_));
    battle_runtime_seed_ = 0;
    battle_state_ = 1;
    recv_pvp_end_result_.Clear();
    recv_pvp_rank_.Clear();
    user_table_.Clear();
    state_flag_0c0_ = 0;
    relay_battle_mgr_.Reset();
    timed_out_ = 0;
    pvp_rank_requested_ = 0;
    recv_pvp_rank_timer_inserted_ = 0;
    pvp_battle_ended_ = 0;
    for (int message = 49; message <= 62; ++message) {
        if (message != 62) {
            gen_timer_key(static_cast<TIMER_MESSAGE>(message));
        }
    }
    unlock();
    return 0;
}
