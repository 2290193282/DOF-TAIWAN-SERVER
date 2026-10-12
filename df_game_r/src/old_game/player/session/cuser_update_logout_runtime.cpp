#include "old_game/player/core/cuser.h"

#include <cstring>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hades_public_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/dungeon/db_update_revenge_data.h"
#include "old_data/db/event/db_update_ontime_last_recv_idx.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/logout_payload.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x86554AA
// IDA_NAME: _ZN5CUser12UpdateLogoutEb
// DEMANGLED: CUser::UpdateLogout(bool)
void CUser::UpdateLogout(bool send_complete_packet) {
    UpdateCharacView();

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("user.cpp", 8800), true);
    *guard << 44;
    *guard << GetUID();
    SIG_UPDATE_LOGOUT* const logout = guard.GetInBuffer<SIG_UPDATE_LOGOUT>();
    std::memset(logout, 0, sizeof(*logout));

    logout->m_id = static_cast<uint32_t>(get_acc_id());
    GameWorld* const world = G_GameWorld();
    if (world->IsIntegratedPvPBaseChannel()) {
        logout->channel_no =
            G_CEnvironment()->get_gc_no_hardcode(GetServerGroup());
    } else {
        logout->channel_no = G_CEnvironment()->get_gc_no();
    }

    if (getCurCharacR()) {
        logout->charac_no = static_cast<uint32_t>(getCurCharacNo());
        logout->fatigue = static_cast<uint16_t>(GetFatigue());
    }
    getHades()->Send_Logout();

    const int current_second = GlobalData::system_time()->getCurSec();
    if (get_state() > 1) {
        logout->write_member_play_info = 1;
        logout->play_time = current_second - static_cast<int>(login_time_);
        logout->trade_count = static_cast<int32_t>(trade_count_);
        logout->exp = static_cast<uint32_t>(play_experience_);
        logout->used_fatigue = account_used_fatigue_;
        logout->ting_flag = ting_flag_;
        logout->pcbang_flag = static_cast<uint8_t>(
            isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12)));
        network_.GetPeerIP2(logout->peer_ip, sizeof(logout->peer_ip));
        if (!logout->peer_ip[0] && login_session_peer_ip_[0]) {
            std::strncpy(logout->peer_ip,
                         login_session_peer_ip_,
                         sizeof(logout->peer_ip));
        }
        const char* const dot = std::strrchr(logout->peer_ip, '.');
        if (dot) {
            const size_t prefix_length =
                static_cast<size_t>(dot - logout->peer_ip);
            const size_t suffix_length = std::strlen(dot + 1);
            if (prefix_length <= sizeof(logout->ip) &&
                suffix_length <= sizeof(logout->end_ip)) {
                std::strncpy(logout->ip, logout->peer_ip, prefix_length);
                std::strncpy(logout->end_ip, dot + 1, suffix_length);
                logout->last_play_time = login_session_connected_at_;
            }
        }
        std::strncpy(logout->mac_addr,
                     network_.get_mac_addr(),
                     sizeof(logout->mac_addr));
        logout->save_member_bonus = static_cast<uint8_t>(
            IsEnableSaveMemberBonusFatigue());
        DisableSaveMemberBonusFatigue();
        logout->member_bonus_fatigue = static_cast<int16_t>(
            getAccountMemberBonusFatigue());
        resetAccountMemberBonusFatigue();
    }

    login_time_ = static_cast<uint32_t>(current_second);
    if (account_id_ ||
        world->IsIntegratedPvPBaseChannel() || world->GetChannelType() == 7) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }

    if (*network_.get_str_ip() && getCurCharacName() &&
        GetCharacStartPlayTick() > 0) {
        const int play_time = GlobalData::system_time()->getCurSec() -
                              GetCharacStartPlayTick();
        history_log().Logout(getWebAddress(),
                             network_.get_str_ip(),
                             network_.get_port(),
                             G_CEnvironment()->getProcessSequence(),
                             play_time,
                             static_cast<unsigned char>(GetLogInOutState()));
    }

    DBUpdateDBLogItem();
    ResetDBLogItem();
    if (static_cast<unsigned char>(IsVillageAttackDBUpdate())) {
        DB_UpdateRevengeData::makeRequest(
            static_cast<unsigned int>(getCurCharacNo()),
            static_cast<unsigned char>(GetCurVillageAttackCount()),
            static_cast<unsigned char>(GetCurRevengeDungeonCount()));
    }
    if (is_update_ontime_last_recv_idx()) {
        DB_UpdateOntimeLastRecvIdx::makeRequest(
            GetUID(),
            static_cast<unsigned int>(get_acc_id()),
            get_ontime_last_recv_idx());
        reset_update_ontime_last_recv_idx();
    }

    EventClassify::CEventScriptMng* const event_manager =
        G_CDataManager()->get_event_script_mng();
    if (event_manager && event_manager->is_eventing(0x56) &&
        get_server_fatigue_day_size()) {
        DB_UpdateFatigueAttendance::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            *get_server_fatigue_day_data());
        reset_server_fatigue_data();
    }
    reset_ingame_event_history();

    if (!send_complete_packet) {
        PacketGuard complete;
        complete.put_header(1, 3);
        complete.put_byte(1);
        complete.finalize(true);
        Send(complete);
    }
}
