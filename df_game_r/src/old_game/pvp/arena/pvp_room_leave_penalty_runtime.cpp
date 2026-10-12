#include "old_game/pvp/arena/pvp_room.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/battlefield/integrated_match_pvp_score_runtime.h"
#include "old_game/pvp/match/packet_server_match_data.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <stdint.h>


// IDA_EA: 0x85DF954
// IDA_NAME: _ZN24Packet_Server_Match_dataC2Ev
// DEMANGLED: Packet_Server_Match_data::Packet_Server_Match_data(void)
Packet_Server_Match_data::Packet_Server_Match_data()
    : PacketHeader(0x1b67u, 0x0du), server_group(0), win_flag(0),
      lose_or_leave_flag(0) {}

// IDA_EA: 0x85DCAF2
// IDA_NAME: _ZN8PvP_Room16GiveOuterPenaltyEP5CUser
// DEMANGLED: PvP_Room::GiveOuterPenalty(CUser *)
void PvP_Room::GiveOuterPenalty(CUser* user) {
    const int legacy_wins = user && user->getPVPResultR()
                                ? user->getPVPResultR()->values[0]
                                : 0;
    PvpResultType empty_result;
    empty_result.Clear();
    user->update_pvp_rank(empty_result);

    const int team = GetTeamIndex(user);
    if (!user_table_.IsValidTeam(team)) {
        cMyTrace trace("void PvP_Room::GiveOuterPenalty(CUser*)", 4344, 5);
        trace("PVP invalid team(%d)", team);
        return;
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        int team_one_count = 0;
        int team_two_count = 0;
        for (int seat = 0; seat < 8; ++seat) {
            if (!users_[seat] || IsPvpObserver(seat)) {
                continue;
            }
            if (battle_teams_[seat] == 1) {
                ++team_one_count;
            } else if (battle_teams_[seat] == 2) {
                ++team_two_count;
            }
        }
        const integrated_match_pvp_score::MatchMode match_mode =
            integrated_match_pvp_score::ResolveMatchMode(
                static_cast<int>(battle_mode_),
                team_one_count,
                team_two_count);
        if (match_mode ==
            integrated_match_pvp_score::MATCH_MODE_ONE_ON_ONE) {
            // The departing user is removed before pvp_reward(); record the
            // 1v1 loss and streak reset while its battle identity is intact.
            (void)integrated_match_pvp_score::Record(
                user,
                match_mode,
                false,
                true,
                0u,
                legacy_wins > 0
                    ? static_cast<uint32_t>(legacy_wins)
                    : 0u);
        }
    }

    bool has_integrated_match_event = false;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        CEventBase* const event =
            GlobalData::s_event_manager->GetRepeatEvent(56);
        if (event->IsEventing(0)) {
            has_integrated_match_event = true;
        }
    }

    if (has_integrated_match_event && user->GetServerGroup()) {
        Packet_Server_Match_data match_data;
        match_data.server_group =
            static_cast<uint8_t>(user->GetServerGroup());
        match_data.lose_or_leave_flag = 1;
        CStatisticServerProxy* const proxy =
            GlobalData::s_statistic_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendPacket(reinterpret_cast<char*>(&match_data),
                          sizeof(match_data));
    }

    if (G_GameWorld()->IsPVPChannel()) {
        const int channel_type = G_GameWorld()->GetChannelType();
        if (channel_type == 4 || channel_type == 8) {
            user->update_pvp_point(user_table_.GetLeavePenalty(team));
        } else if (channel_type == 6) {
            user->add_guild_pvp_result(-1);
            return;
        }
    }

    if (G_GameWorld()->IsWinPointPvPChannel()) {
        user->add_pvp_result(false, 0);
    }
    current_match_->OnGiveup(user);
}
