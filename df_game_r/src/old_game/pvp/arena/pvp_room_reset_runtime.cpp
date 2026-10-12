#include "old_game/pvp/arena/pvp_room.h"

#include <cstring>

#include "old_game/world/region/game_world.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"

// IDA_EA: 0x85D62C8
// IDA_NAME: _ZN8PvP_Room5resetEv
// DEMANGLED: PvP_Room::reset(void)
int PvP_Room::reset() {
    channel_zombie_matching_authority::OnRoomReset(this);
    zombie_mode::ResetRoom(this);
    battle_reset();
    lock();

    member_count_ = 0;
    battle_state_ = 0;
    battle_mode_ = static_cast<PVP_BATTLE_MODE>(2);
    room_title_mode_ = 0;
    std::memset(room_title_, 0, sizeof(room_title_));
    manager_ = 0;
    manager_seat_ = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        users_[seat] = 0;
        teams_[seat] = 255;
        ready_flags_[seat] = 0;
    }

    if (G_GameWorld()->GetChannelType() == 6) {
        guild_war_log_.Clear();
    }
    relay_battle_mgr_.Reset();
    relay_battle_mgr_.SetParent(this);
    death_match_battle_mgr_.Reset();
    death_match_battle_mgr_.SetParent(this);
    timed_out_ = 0;
    pvp_rank_requested_ = 0;
    recv_pvp_rank_timer_inserted_ = 0;
    pvp_battle_ended_ = 0;

    unlock();
    return 0;
}
