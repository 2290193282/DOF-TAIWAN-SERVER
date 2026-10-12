#include "old_game/pvp/arena/pvp_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/common_statistics_manager_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/disconnect_detecter_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/statistics_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x85D8C3A
// IDA_NAME: _ZN8PvP_Room9start_pvpEv
// DEMANGLED: PvP_Room::start_pvp(void)
void PvP_Room::start_pvp() {
    if (get_waiter_count() <= 1) {
        return;
    }

    current_match_->RequestMap(requested_map_no_, battle_mode_);

    CMapList* const map_list = G_CDataManager()->get_map_list();
    const int map_count = map_list->GetPVPMapCount();
    if (requested_map_no_ < 0 || requested_map_no_ > map_count) {
        cMyTrace trace("void PvP_Room::start_pvp()", 2176, 0);
        trace("START_PVP request_map_index(%d)", requested_map_no_);
        requested_map_no_ = 0;
    }

    if (requested_map_no_ != 0) {
        selected_map_no_ = requested_map_no_;
    } else {
        selected_map_no_ = static_cast<int16_t>(SelectRandomMap());
        if (selected_map_no_ < 0 || selected_map_no_ >= map_count) {
            cMyTrace trace("void PvP_Room::start_pvp()", 2187, 0);
            trace("START_PVP select_map_index(%d)", selected_map_no_);
        }
    }

    if (G_GameWorld()->GetChannelType() == 6) {
        guild_war_log_.SetpvpStartTime(
            GlobalData::s_systemTime_.getCurSec());
    }

    int player_count = 0;
    PacketGuard packet;
    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = users_[seat];
        if (!user) {
            continue;
        }

        ++player_count;
        battle_live_flags_[seat] = 1;
        battle_teams_[seat] = static_cast<uint8_t>(teams_[seat]);
        relay_battle_mgr_.OnChangeTeam(seat, battle_teams_[seat]);
        user->reset_pvp_masterid_walkingout_me();

        packet.clear();
        packet.put_header(0, 45);
        packet.put_byte(selected_map_no_);
        packet.put_byte(battle_mode_);
        packet.finalize(true);
        user->Send(packet);
        user->set_state(7);
        user->getHackAnalyzer()->beginCollectHackInfo(user);
        disconnect_detecter::GetInstanceDisconnectDetecter()->RegisterUser(
            user);
    }

    Statistics::PVPPlay2(selected_map_no_, battle_mode_, player_count);
    calculate_pvp_point();
    battle_player_count_ = get_waiter_count();
    battle_state_ = 2;

    if (battle_mode_ == static_cast<PVP_BATTLE_MODE>(3)) {
        relay_battle_mgr_.OnStart();
    }
    if (battle_mode_ == static_cast<PVP_BATTLE_MODE>(4) ||
        battle_mode_ == static_cast<PVP_BATTLE_MODE>(5)) {
        death_match_battle_mgr_.OnStart();
    }

    battle_start_time_sec_ = GlobalData::s_systemTime_.getCurSec();

    int player_rate_key = 0;
    switch (static_cast<int>(battle_mode_)) {
    case 1:
    case 4:
        player_rate_key = player_count > 2 ? 1 : 0;
        break;
    case 2:
    case 5:
        player_rate_key = player_count > 2 ? 3 : 2;
        break;
    case 3:
        player_rate_key = player_count > 2 ? 5 : 4;
        break;
    default:
        break;
    }

    CCommonStatisticsMgr* const common_statistics =
        GetInstanceCommonStatisticsMgr();
    common_statistics->IncreaseQuantity(
        CCommonStatisticsMgr::ENUM_PVP_PLAYER_RATE,
        player_rate_key);

    if (selected_map_no_ >= 0 && selected_map_no_ < map_count) {
        common_statistics->IncreaseQuantity(
            CCommonStatisticsMgr::ENUM_PVP_MAP_COUNT,
            selected_map_no_);
    } else {
        cMyTrace trace("void PvP_Room::start_pvp()", 2370, 0);
        trace("START_PVP filter select_map_index(%d)", selected_map_no_);
    }

    current_match_->OnStartFight();
}
