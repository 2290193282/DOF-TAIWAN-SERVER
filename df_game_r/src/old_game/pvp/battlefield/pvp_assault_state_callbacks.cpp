#include "old_game/pvp/battlefield/pvp_assault.h"

#include <cstdio>
#include <cstring>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/pvp/battlefield/assault_power_war_packet_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/disconnect_detecter_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/etc/power_parameter_script.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "old_game/pvp/battlefield/timer_assault_lifecycle.h"

enum ENUM_ERROR : int {};

extern "C" void Area_send_area_users_Noti24(Area* area, CUser* user);

namespace {


}  // namespace

namespace pvp_assault {

// IDA_EA: 0x82E6E2C
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace8_DelUserEib
// DEMANGLED: pvp_assault::CAssaultPlace::_DelUser(int,bool)
int CAssaultPlace::_DelUser(int index, bool unregister_disconnect) {
    CAssaulter& assaulter = assaulters[index];
    if (unregister_disconnect && assaulter.GetState() > 4) {
        disconnect_detecter::GetInstanceDisconnectDetecter()
            ->UnregisterUser(assaulter.GetUser());
    }

    const int team = assaulter.GetTeam();
    CUser* const user = assaulter.GetUser();
    assaulter.DetachUser();
    --assaulter_count;

    if (!user || !user->getCurCharacR()) {
        return 0;
    }

    if (!CPowerManager::IsPowerWarEventOn() ||
        static_cast<unsigned char>(user->getCurCharacVill()) != 7u) {
        user->history_log().EndPvP(
            static_cast<ENUM_PVP_HISTORY_REASON>(5),
            team_result[team] != 0,
            static_cast<unsigned int>(
                GlobalData::system_time()->getCurSec() -
                static_cast<int>(assault_start_second)));
    }

    Village* const village = G_GameWorld()->getUserVillage(user);
    if (village) {
        Area* const area = village->getArea(user->get_area(false));
        if (area) {
            Area_send_area_users_Noti24(area, user);
            GetInstanceAssaultMgr()->OnMoveArea(user);
        }
    }

    GlobalData::s_villageObjectMgr->on_move_area(user);
    GlobalData::s_villageMonsterMgr->OnMoveArea(user);
    return user->SetAssaultPlace(0);
}

// IDA_EA: 0x82ED752
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace8ChangeHpEP5CUseri
// DEMANGLED: pvp_assault::CAssaultPlace::ChangeHp(CUser *,int)
int CAssaultPlace::ChangeHp(CUser* user, int hp) {
    const int index = _GetUserIndex(user, true);
    if (assaulters[index].ChangeHp(hp)) {
        PacketGuard packet;
        _MakeChangeHp(&packet, index, hp);
        if (CPowerManager::IsPowerWarEventOn() &&
            static_cast<unsigned char>(user->getCurCharacVill()) == 7u) {
            CParty* const party = user->GetParty();
            if (party) {
                party->send_to_party(packet);
            } else {
                user->Send(packet);
            }
        } else {
            _SendPacketToArea(packet);
        }
    }

    if (state == 5u || state == 6u) {
        assaulters[index].SetState(static_cast<ENUM_ASSAULT_STATE>(8));
        bool received_all = true;
        for (int seat = 0; seat <= 7; ++seat) {
            if (!assaulters[seat].Empty() &&
                assaulters[seat].GetState() != 8) {
                received_all = false;
            }
        }
        if (received_all) {
            if (party_info[0].Get()) {
                party_info[0].Get()->SetAssaultState(7);
            }
            if (party_info[1].Get()) {
                party_info[1].Get()->SetAssaultState(7);
            }
            state = 7u;
            return CloseAssault();
        }
    }
    return 1;
}

// IDA_EA: 0x82EB5E2
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace9AbuseUserEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::AbuseUser(CUser *)
int CAssaultPlace::AbuseUser(CUser* user) {
    const int elapsed = GlobalData::system_time()->getCurSec() -
                        static_cast<int>(assault_start_second);
    if (elapsed <= 19) {
        cMyTrace trace("void pvp_assault::CAssaultPlace::AbuseUser(CUser*)",
                       3107,
                       0);
        trace("CAssaultPlace::AbuseUser() charac_no(%d), timeout sec(%d)",
              user->getCurCharacNo(),
              elapsed);
    }

    if (state != 4u) {
        return static_cast<int>(state);
    }

    for (int seat = 0; seat <= 7; ++seat) {
        CAssaulter& assaulter = assaulters[seat];
        if (!assaulter.Empty()) {
            CUser* const member = assaulter.GetUser();
            member->getHackAnalyzer()->addServerHackCnt(
                member,
                static_cast<WongWork::ENUM_HACKTYPE>(0x32e),
                1u,
                0u,
                0u);
        }
    }

    state = 6u;
    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(6);
    }
    if (party_info[1].Get()) {
        party_info[1].Get()->SetAssaultState(6);
    }
    return TimeoutCloseAssault();
}

// IDA_EA: 0x82E91FA
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace12DoConnectP2PEv
// DEMANGLED: pvp_assault::CAssaultPlace::DoConnectP2P(void)
int CAssaultPlace::DoConnectP2P() {
    state = 3u;
    for (int index = 0; index <= 7; ++index) {
        if (!assaulters[index].Empty()) {
            assaulters[index].SetState(
                static_cast<ENUM_ASSAULT_STATE>(3));
        }
    }
    return TimerConnectP2PAssaultTimeout::registNextTimer(
        15,
        static_cast<int>(assault_place_id),
        static_cast<int>(authen_value));
}

// IDA_EA: 0x82EB774
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace12CompleteLoadEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::CompleteLoad(CUser *)
char CAssaultPlace::CompleteLoad(CUser* user) {
    if (state != 2u) {
        throw static_cast<ENUM_ERROR>(19);
    }

    bool all_loaded = true;
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) {
            continue;
        }
        if (assaulter.GetUser() == user) {
            if (assaulter.GetState() != 1) {
                throw static_cast<ENUM_ERROR>(18);
            }
            assaulter.SetState(static_cast<ENUM_ASSAULT_STATE>(2));
            PacketGuard packet;
            _MakeCompleteLoad(&packet, index);
            SendPacket(packet);
        }
        if (assaulter.GetState() != 2) {
            all_loaded = false;
        }
    }

    if (!all_loaded) {
        return 0;
    }

    DoConnectP2P();
    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(3);
    }
    CParty* const second_party = party_info[1].Get();
    if (!second_party) {
        return 0;
    }
    return static_cast<char>(reinterpret_cast<uintptr_t>(
        second_party->SetAssaultState(3)));
}

// IDA_EA: 0x82E927E
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace19_IsCannotConnectP2PEii
// DEMANGLED: pvp_assault::CAssaultPlace::_IsCannotConnectP2P(int,int)
int CAssaultPlace::_IsCannotConnectP2P(int source_index,
                                       int target_index) {
    return !assaulters[source_index].connect_p2p[target_index] ||
           assaulters[target_index].Empty();
}

// IDA_EA: 0x82E92C6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace24_ClearConnectP2PAbsoluteEi
// DEMANGLED: pvp_assault::CAssaultPlace::_ClearConnectP2PAbsolute(int)
int CAssaultPlace::_ClearConnectP2PAbsolute(int index) {
    for (int target = 0; target <= 7; ++target) {
        if (index != target && _IsCannotConnectP2P(index, target)) {
            if (CPowerManager::IsPowerWarEventOn()) {
                GlobalData::s_power_manager->GetPowerWarLog()
                    ->IncP2PBadCountSecede();
            }
            LeaveAssaultPlace(target, true, false);
        }
    }
    return 1;
}

// IDA_EA: 0x82E9358
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace24_ClearConnectP2PRelativeEi
// DEMANGLED: pvp_assault::CAssaultPlace::_ClearConnectP2PRelative(int)
int CAssaultPlace::_ClearConnectP2PRelative(int point) {
    for (int index = 2; index <= 7; ++index) {
        if (_GetConnectP2PPoint(index) == point) {
            LeaveAssaultPlace(index, true, false);
        }
    }
    return 1;
}

// IDA_EA: 0x82E93BC
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace19_GetConnectP2PPointEi
// DEMANGLED: pvp_assault::CAssaultPlace::_GetConnectP2PPoint(int)
int CAssaultPlace::_GetConnectP2PPoint(int index) {
    int point = 0;
    for (int target = 0; target <= 7; ++target) {
        if (index != target &&
            !_IsCannotConnectP2P(index, target)) {
            ++point;
        }
    }
    return point;
}

// IDA_EA: 0x82E9412
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace25_GetConnectP2PMaxMinPointERiS1_
// DEMANGLED: pvp_assault::CAssaultPlace::_GetConnectP2PMaxMinPoint(int &,int &)
bool CAssaultPlace::_GetConnectP2PMaxMinPoint(int& maximum,
                                              int& minimum) {
    maximum = 0;
    minimum = 8;
    int index = 0;
    for (; index <= 7; ++index) {
        if (!assaulters[index].Empty()) {
            const int point = _GetConnectP2PPoint(index);
            if (minimum > point) {
                minimum = point;
            }
            if (maximum < point) {
                maximum = point;
            }
        }
    }
    return index <= 7;
}

// IDA_EA: 0x82E949C
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace15ClearBadP2PUserEv
// DEMANGLED: pvp_assault::CAssaultPlace::ClearBadP2PUser(void)
int CAssaultPlace::ClearBadP2PUser() {
    int maximum = 0;
    int minimum = 0;
    int result = 0;
    for (int pass = 0;; ++pass) {
        result = pass <= 7;
        if (pass > 7) {
            break;
        }
        _GetConnectP2PMaxMinPoint(maximum, minimum);
        result = minimum;
        if (maximum == minimum) {
            break;
        }
        _ClearConnectP2PRelative(minimum);
    }
    return result;
}

// IDA_EA: 0x82E9502
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace12StartAssaultEv
// DEMANGLED: pvp_assault::CAssaultPlace::StartAssault(void)
int CAssaultPlace::StartAssault() {
    ClearBadP2PUser();
    if (assaulter_count == 0u) {
        if (party_info[0].Get()) {
            party_info[0].Get()->SetAssaultState(7);
        }
        if (party_info[1].Get()) {
            party_info[1].Get()->SetAssaultState(7);
        }
        if (CPowerManager::IsPowerWarEventOn()) {
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncStartFailureCount();
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncP2PBadCountFailure();
        }
        state = 7u;
        CloseAssault();
        return 0;
    }

    if (static_cast<unsigned char>(_CheckPlayable()) != 1u) {
        state = 6u;
        if (party_info[0].Get()) {
            party_info[0].Get()->SetAssaultState(6);
        }
        if (party_info[1].Get()) {
            party_info[1].Get()->SetAssaultState(6);
        }
        if (CPowerManager::IsPowerWarEventOn()) {
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncStartFailureCount();
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncP2PBadCountFailure();
        }
        EndAssault();
        return 0;
    }

    state = 4u;
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (!assaulter.Empty()) {
            assaulter.SetState(static_cast<ENUM_ASSAULT_STATE>(5));
            disconnect_detecter::GetInstanceDisconnectDetecter()
                ->RegisterUser(assaulter.GetUser());
        }
    }
    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(4);
    }
    if (party_info[1].Get()) {
        party_info[1].Get()->SetAssaultState(4);
    }

    PacketGuard packet;
    _MakeStartAssault(&packet);
    SendPacket(packet);
    packet.clear();
    _MakeViewAssaultIconPlayer(&packet, true);
    _SendPacketToArea(packet);
    _SendUserState();
    TimerEndAssault::registNextTimer(
        180,
        static_cast<int>(assault_place_id),
        static_cast<int>(authen_value));

    if (CPowerManager::IsPowerWarEventOn()) {
        GlobalData::s_power_manager->GetPowerWarLog()
            ->IncStartSuccessCount();
        cMyTrace trace(
            "bool pvp_assault::CAssaultPlace::StartAssault()",
            2199,
            0);
        trace("POWER WAR START SUCCESS");
        for (int index = 0; index <= 7; ++index) {
            CAssaulter& assaulter = assaulters[index];
            if (assaulter.Empty()) {
                continue;
            }
            if (assaulter.GetTeam()) {
                cMyTrace member_trace(
                    "bool pvp_assault::CAssaultPlace::StartAssault()",
                    2209,
                    0);
                member_trace("RED TEAM : %s",
                             assaulter.GetUser()->getCurCharacName());
            } else {
                cMyTrace member_trace(
                    "bool pvp_assault::CAssaultPlace::StartAssault()",
                    2207,
                    0);
                member_trace("BLUE TEAM : %s",
                             assaulter.GetUser()->getCurCharacName());
            }
        }
    }

    assault_start_second =
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec());
    _PowerWarAssaultCheckIn();
    return 1;
}

// IDA_EA: 0x82E99A2
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace10EndAssaultEv
// DEMANGLED: pvp_assault::CAssaultPlace::EndAssault(void)
int CAssaultPlace::EndAssault() {
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) {
            continue;
        }

        assaulter.SetState(static_cast<ENUM_ASSAULT_STATE>(7));
        PacketGuard packet;
        _MakeEndAssault(&packet);
        assaulter.GetUser()->Send(packet);
    }

    int alive_team_count[2] = {0, 0};
    _GetTeamCountAliveUser(alive_team_count);
    if (!alive_team_count[0]) {
        team_result[1] = 1;
    }
    if (!alive_team_count[1]) {
        team_result[0] = 1;
    }
    const int timer_result = TimerCloseAssault::registNextTimer(
        5,
        static_cast<int>(assault_place_id),
        static_cast<int>(authen_value));
    LogManager::logFormat(
        1,
        "PvpAssault.cpp",
        "pvp_assault::CAssaultPlace::EndAssault",
        0,
        "[ASSAULT_DIAG][END] place=%u members=%u alive0=%d alive1=%d "
        "result0=%u result1=%u winner=%u timer_result=%d",
        assault_place_id,
        static_cast<unsigned int>(assaulter_count),
        alive_team_count[0],
        alive_team_count[1],
        static_cast<unsigned int>(team_result[0]),
        static_cast<unsigned int>(team_result[1]),
        winner_team,
        timer_result);
    return timer_result;
}

// IDA_EA: 0x82E9AD6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace12CloseAssaultEv
// DEMANGLED: pvp_assault::CAssaultPlace::CloseAssault(void)
int CAssaultPlace::CloseAssault() {
    LogManager::logFormat(
        1,
        "PvpAssault.cpp",
        "pvp_assault::CAssaultPlace::CloseAssault",
        0,
        "[ASSAULT_DIAG][CLOSE_ENTER] place=%u members=%u result0=%u "
        "result1=%u winner=%u state=%u",
        assault_place_id,
        static_cast<unsigned int>(assaulter_count),
        static_cast<unsigned int>(team_result[0]),
        static_cast<unsigned int>(team_result[1]),
        winner_team,
        static_cast<unsigned int>(state));
    int last_village = 0;
    if (assaulter_count > 0u) {
        for (int index = 0; index <= 7; ++index) {
            CAssaulter& assaulter = assaulters[index];
            if (assaulter.Empty()) {
                continue;
            }

            CUser* const user = assaulter.GetUser();
            if (user) {
                if (CPowerManager::IsPowerWarEventOn() &&
                    static_cast<unsigned char>(user->getCurCharacVill()) ==
                        7u) {
                    if (static_cast<unsigned char>(
                            assaulter.IsPowerWarWinner()) != 1u) {
                        ForcedMoveAtPowerWarPlace(user);
                    }
                    _SendCloseAssault(user, user->getPowerWarHP());
                } else if (user->getCurCharacR()) {
                    const int hp = user->IsCurCharacGhost()
                                       ? 100
                                       : assaulter.GetHp();
                    user->setCurCharacStamina(
                        static_cast<uint8_t>(_SendCloseAssault(user, hp)));
                }
                last_village =
                    static_cast<char>(user->getCurCharacVill());
            }

            user->SetAssaultPlace(0);
            if (assaulter.IsPowerWarWinner() &&
                static_cast<unsigned short>(user->getPowerWarHP()) <= 29u) {
                user->setPowerWarHP(30u);
            }
            if (assaulter.GetTeam() == static_cast<int>(winner_team) &&
                static_cast<unsigned char>(
                    CPowerManager::IsPowerWarEventOn()) != 1u &&
                static_cast<unsigned char>(user->getCurCharacVill()) != 7u) {
                user->SetCurCharacChaosKillTime(
                    GlobalData::system_time()->getCurSec());
            }

            int ghost_time = 0;
            if (CPowerManager::IsPowerWarEventOn() ||
                static_cast<unsigned char>(user->getCurCharacVill()) == 7u) {
                const long double response_penalty =
                    GlobalData::s_power_manager->GetPowerWarResponPenalty(
                        user);
                ghost_time =
                    GlobalData::s_power_manager->GetPowerWarGhostTime(
                        user,
                        static_cast<float>(response_penalty));
            } else {
                const ServerParameterScript* const server_parameter =
                    G_CDataManager()->get_server_parameter_script();
                ghost_time = server_parameter->getAssaultGhostTime(
                    user->get_charac_level(),
                    user->GetCurCharacChaosPoint());
            }

            if (user->IsCurCharacGhost() && ghost_time) {
                const int current_second =
                    GlobalData::system_time()->getCurSec();
                user->SetCurCharacChaosDieTime(current_second);
                user->SetCurCharacChaosResponTime(current_second +
                                                  ghost_time);
                user->character_view().enableSaveCharacView();
                CAssaultMgr::NotifyGhostTime(
                    user, ghost_time, ghost_time);
            }
        }
        _SendUserState();
    }

    PacketGuard packet;
    _MakeViewAssaultIconPlayer(&packet, false);
    _SendPacketToArea(packet);

    if (CPowerManager::IsPowerWarEventOn() && last_village == 7) {
        CParty* const first_party = party_info[0].Get();
        if (first_party) {
            const int timer_key = first_party->gen_timer_key(
                static_cast<TIMER_MESSAGE>(42));
            TimerCompleteLoadAfterAssaultTimeout::registNextTimer(
                60,
                first_party->GetPartyIndex(),
                timer_key);
        }
        CParty* const second_party = party_info[1].Get();
        if (second_party) {
            const int timer_key = second_party->gen_timer_key(
                static_cast<TIMER_MESSAGE>(42));
            TimerCompleteLoadAfterAssaultTimeout::registNextTimer(
                60,
                second_party->GetPartyIndex(),
                timer_key);
        }
    }

    for (int index = 0; index <= 7; ++index) {
        if (!assaulters[index].Empty()) {
            _DelUser(index, true);
        }
    }
    LogManager::logFormat(
        1,
        "PvpAssault.cpp",
        "pvp_assault::CAssaultPlace::CloseAssault",
        0,
        "[ASSAULT_DIAG][CLOSE_DONE] place=%u last_village=%d",
        assault_place_id,
        last_village);
    GetInstanceAssaultMgr()->FreeAssaultPlace(
        static_cast<int>(assault_place_id), this);
    return 1;
}

// IDA_EA: 0x82EA004
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17TimeoutEndAssaultEv
// DEMANGLED: pvp_assault::CAssaultPlace::TimeoutEndAssault(void)
int CAssaultPlace::TimeoutEndAssault() {
    cMyTrace trace(
        "void pvp_assault::CAssaultPlace::TimeoutEndAssault()",
        2510,
        0);
    trace("TimeoutEndAssault() state(%d) %s,%s,%s,%s,%s,%s,%s,%s",
          static_cast<int>(state),
          assaulters[0].GetCharacName(),
          assaulters[1].GetCharacName(),
          assaulters[2].GetCharacName(),
          assaulters[3].GetCharacName(),
          assaulters[4].GetCharacName(),
          assaulters[5].GetCharacName(),
          assaulters[6].GetCharacName(),
          assaulters[7].GetCharacName());

    if (state != 4u) {
        return static_cast<int>(state);
    }

    state = 5u;
    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(5);
    }
    if (party_info[1].Get()) {
        party_info[1].Get()->SetAssaultState(5);
    }
    return EndAssault();
}

// IDA_EA: 0x82EA188
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace19TimeoutCloseAssaultEv
// DEMANGLED: pvp_assault::CAssaultPlace::TimeoutCloseAssault(void)
int CAssaultPlace::TimeoutCloseAssault() {
    cMyTrace trace(
        "void pvp_assault::CAssaultPlace::TimeoutCloseAssault()",
        2538,
        0);
    trace("TimeoutCloseAssault() state(%d) %s,%s,%s,%s,%s,%s,%s,%s",
          static_cast<int>(state),
          assaulters[0].GetCharacName(),
          assaulters[1].GetCharacName(),
          assaulters[2].GetCharacName(),
          assaulters[3].GetCharacName(),
          assaulters[4].GetCharacName(),
          assaulters[5].GetCharacName(),
          assaulters[6].GetCharacName(),
          assaulters[7].GetCharacName());

    if (state != 5u && state != 6u) {
        return static_cast<int>(state);
    }

    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(7);
    }
    if (party_info[1].Get()) {
        party_info[1].Get()->SetAssaultState(7);
    }
    state = 7u;
    return CloseAssault();
}

// IDA_EA: 0x82EA31A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace19TimeoutCompleteLoadEv
// DEMANGLED: pvp_assault::CAssaultPlace::TimeoutCompleteLoad(void)
int CAssaultPlace::TimeoutCompleteLoad() {
    if (state != 2u) {
        throw static_cast<ENUM_ERROR>(19);
    }

    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty() || assaulter.GetState() == 2) {
            continue;
        }

        if (CPowerManager::IsPowerWarEventOn()) {
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncLoadingTimeOutCountSecede();
            cMyTrace trace(
                "bool pvp_assault::CAssaultPlace::TimeoutCompleteLoad()",
                2584,
                0);
            trace("POWER WAR LOADING SECEDE");
            for (int member_index = 0; member_index <= 7; ++member_index) {
                CAssaulter& member = assaulters[member_index];
                if (member.Empty()) {
                    continue;
                }
                if (member.GetTeam()) {
                    cMyTrace member_trace(
                        "bool pvp_assault::CAssaultPlace::TimeoutCompleteLoad()",
                        2594,
                        0);
                    member_trace("RED TEAM : %s",
                                 member.GetUser()->getCurCharacName());
                } else {
                    cMyTrace member_trace(
                        "bool pvp_assault::CAssaultPlace::TimeoutCompleteLoad()",
                        2592,
                        0);
                    member_trace("BLUE TEAM : %s",
                                 member.GetUser()->getCurCharacName());
                }
            }
            cMyTrace secede_trace(
                "bool pvp_assault::CAssaultPlace::TimeoutCompleteLoad()",
                2597,
                0);
            secede_trace("SECEDE : %s", assaulter.GetUser());
        }

        CUser* const user = assaulter.GetUser();
        if (CPowerManager::IsPowerWarEventOn() &&
            static_cast<unsigned char>(user->getCurCharacVill()) == 7u) {
            PacketGuard packet;
            packet.put_header(0, 230);
            packet.put_byte(1);
            packet.finalize(true);
            user->Send(packet);
        }
    }

    for (int index = 0; index <= 7; ++index) {
        if (!assaulters[index].Empty() &&
            assaulters[index].GetState() != 2) {
            LeaveAssaultPlace(index, true, false);
            if (state == 6u) {
                return 0;
            }
        }
    }

    DoConnectP2P();
    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(3);
    }
    if (party_info[1].Get()) {
        party_info[1].Get()->SetAssaultState(3);
    }
    return 1;
}

// IDA_EA: 0x82EA716
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17TimeoutConnectP2PEv
// DEMANGLED: pvp_assault::CAssaultPlace::TimeoutConnectP2P(void)
int CAssaultPlace::TimeoutConnectP2P() {
    if (state != 3u) {
        throw static_cast<ENUM_ERROR>(19);
    }

    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty() || assaulter.GetState() == 4) {
            continue;
        }

        if (CPowerManager::IsPowerWarEventOn()) {
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncP2PTimeOutCountSecede();
            cMyTrace trace(
                "bool pvp_assault::CAssaultPlace::TimeoutConnectP2P()",
                2688,
                0);
            trace("POWER WAR P2P SECEDE");
            for (int member_index = 0; member_index <= 7; ++member_index) {
                CAssaulter& member = assaulters[member_index];
                if (member.Empty()) {
                    continue;
                }
                if (member.GetTeam()) {
                    cMyTrace member_trace(
                        "bool pvp_assault::CAssaultPlace::TimeoutConnectP2P()",
                        2698,
                        0);
                    member_trace("RED TEAM : %s",
                                 member.GetUser()->getCurCharacName());
                } else {
                    cMyTrace member_trace(
                        "bool pvp_assault::CAssaultPlace::TimeoutConnectP2P()",
                        2696,
                        0);
                    member_trace("BLUE TEAM : %s",
                                 member.GetUser()->getCurCharacName());
                }
            }
            cMyTrace secede_trace(
                "bool pvp_assault::CAssaultPlace::TimeoutConnectP2P()",
                2701,
                0);
            secede_trace("SECEDE : %s", assaulter.GetUser());
        }

        CUser* const user = assaulter.GetUser();
        if (CPowerManager::IsPowerWarEventOn() &&
            static_cast<unsigned char>(user->getCurCharacVill()) == 7u) {
            PacketGuard packet;
            packet.put_header(0, 230);
            packet.put_byte(2);
            packet.finalize(true);
            user->Send(packet);
        }
    }

    for (int index = 0; index <= 7; ++index) {
        if (!assaulters[index].Empty() &&
            assaulters[index].GetState() != 4) {
            LeaveAssaultPlace(index, true, false);
            if (state == 6u) {
                return 0;
            }
        }
    }

    const int started = StartAssault();
    if (started) {
        if (party_info[0].Get()) {
            party_info[0].Get()->SetAssaultState(4);
        }
        if (party_info[1].Get()) {
            party_info[1].Get()->SetAssaultState(4);
        }
    }
    return started;
}

// IDA_EA: 0x82EAB1A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace19_IsBroadcastRunawayEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::_IsBroadcastRunaway(CUser *)
int CAssaultPlace::_IsBroadcastRunaway(CUser* user) {
    return (!CPowerManager::IsPowerWarEventOn() ||
            static_cast<unsigned char>(user->getCurCharacVill()) != 7u) &&
           user->getCurCharacR();
}

// IDA_EA: 0x82EAB7A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_BroadcastRunawayEP5CUseri
// DEMANGLED: pvp_assault::CAssaultPlace::_BroadcastRunaway(CUser *,int)
int CAssaultPlace::_BroadcastRunaway(CUser* user, int index) {
    char message[1024] = {};
    std::sprintf(
        message,
        G_CDataManager()->get_server_parameter_script()
            ->getAssaultUserRunawayNotice(),
        user->getCurCharacName(),
        GetAnotherTeamCharacName(index));

    Packet_Broadcast_Msg packet;
    packet.raw_0a = 0;
    packet.text_length =
        static_cast<unsigned char>(std::strlen(message));
    std::memset(packet.text, 0, sizeof(packet.text));
    std::strncpy(packet.text, message, packet.text_length);
    packet.size =
        static_cast<uint16_t>(packet.text_length + 15u);

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    return proxy->SendPacket(
        reinterpret_cast<char*>(&packet),
        static_cast<int>(packet.size));
}

// IDA_EA: 0x82EACF4
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace16_IsGiveupPanaltyEb
// DEMANGLED: pvp_assault::CAssaultPlace::_IsGiveupPanalty(bool)
int CAssaultPlace::_IsGiveupPanalty(bool evade) {
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(32);
    if (event->IsEventing(0)) {
        return 0;
    }
    return state > 1u && state <= 4u && !evade;
}

// IDA_EA: 0x82EC558
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace22_SendStraightVictoriesEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::_SendStraightVictories(CUser *)
void CAssaultPlace::_SendStraightVictories(CUser* user) {
    PacketGuard packet;
    uint16_t straight_victories = 0;
    const int broadcast_threshold =
        G_CDataManager()->get_power_war_straight_victories();
    if (!user) {
        return;
    }

    CParty* const party = user->GetParty();
    if (party) {
        party->incStraightVictories();
        straight_victories =
            static_cast<uint16_t>(party->getStraightVictories());
        if (straight_victories < broadcast_threshold) {
            return;
        }

        packet.put_header(0, 224);
        packet.put_byte(party->get_member_count());
        for (int index = 0; index <= 3; ++index) {
            CUser* const member = party->get_user(index);
            if (member) {
                const char* const name = member->getCurCharacName();
                const int length = static_cast<int>(std::strlen(name));
                packet.put_int(length);
                packet.put_str(name, length);
            }
        }
    } else {
        user->incStraightVictories();
        straight_victories =
            static_cast<uint16_t>(user->getStraightVictories());
        if (straight_victories < broadcast_threshold) {
            return;
        }

        packet.put_header(0, 224);
        packet.put_byte(1);
        const char* const name = user->getCurCharacName();
        const int length = static_cast<int>(std::strlen(name));
        packet.put_int(length);
        packet.put_str(name, length);
    }

    packet.put_int(straight_victories);
    packet.put_byte(user->getPowerSide());
    packet.finalize(true);
    G_GameWorld()->send_to_vill(7, packet);
}

// IDA_EA: 0x82EC808
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace32_SendPowerWarAssaultResultToUserEP5CUseri
// DEMANGLED: pvp_assault::CAssaultPlace::_SendPowerWarAssaultResultToUser(CUser *,int)
void CAssaultPlace::_SendPowerWarAssaultResultToUser(CUser* user,
                                                      int point) {
    PacketGuard packet;
    packet.put_header(0, 200);
    packet.put_byte(point);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x82ECAEE
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace16_SendHoldVictoryEP5CUserS2_i
// DEMANGLED: pvp_assault::CAssaultPlace::_SendHoldVictory(CUser *,CUser *,int)
void CAssaultPlace::_SendHoldVictory(CUser* winner,
                                     CUser* loser,
                                     int victories) {
    PacketGuard packet;
    packet.put_header(0, 265);
    packet.put_byte(winner->getPowerSide());
    packet.put_int(victories + 1);

    CParty* const winner_party = winner->GetParty();
    if (winner_party) {
        packet.put_byte(winner_party->get_member_count());
        for (int index = 0; index <= 3; ++index) {
            CUser* const member = winner_party->get_user(index);
            if (member) {
                const char* const name = member->getCurCharacName();
                const int length = static_cast<int>(std::strlen(name));
                packet.put_int(length);
                packet.put_str(name, length);
            }
        }
    } else {
        packet.put_byte(1);
        const char* const name = winner->getCurCharacName();
        const int length = static_cast<int>(std::strlen(name));
        packet.put_int(length);
        packet.put_str(name, length);
    }

    CParty* const loser_party = loser->GetParty();
    if (loser_party) {
        packet.put_byte(loser_party->get_member_count());
        for (int index = 0; index <= 3; ++index) {
            CUser* const member = loser_party->get_user(index);
            if (member) {
                const char* const name = member->getCurCharacName();
                const int length = static_cast<int>(std::strlen(name));
                packet.put_int(length);
                packet.put_str(name, length);
            }
        }
    } else {
        packet.put_byte(1);
        const char* const name = loser->getCurCharacName();
        const int length = static_cast<int>(std::strlen(name));
        packet.put_int(length);
        packet.put_str(name, length);
    }

    packet.finalize(true);
    G_GameWorld()->send_to_vill(7, packet);
}

// IDA_EA: 0x82ECDBC
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendVictoryBonusEP5CUseriiii
// DEMANGLED: pvp_assault::CAssaultPlace::_SendVictoryBonus(CUser *,int,int,int,int)
void CAssaultPlace::_SendVictoryBonus(CUser* user,
                                      int victories,
                                      int penalty,
                                      int broken_victories,
                                      int break_bonus) {
    PacketGuard packet;
    packet.put_header(0, 264);
    packet.put_int(victories + 1);
    packet.put_int(penalty);
    packet.put_int(broken_victories);
    packet.put_int(break_bonus);
    packet.finalize(true);

    CParty* const party = user->GetParty();
    if (party) {
        party->send_to_party(packet);
    } else {
        user->Send(packet);
    }
}

// IDA_EA: 0x82EC89C
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_VictoryBonusEP5CUserc
// DEMANGLED: pvp_assault::CAssaultPlace::_VictoryBonus(CUser *,char)
int CAssaultPlace::_VictoryBonus(CUser* user, char winning_power_side) {
    CUser* opponent = 0;
    for (int index = 0; index <= 7; ++index) {
        CUser* const member = assaulters[index].GetUser();
        if (member &&
            user->getPowerSide() != member->getPowerSide()) {
            opponent = member;
            break;
        }
    }

    int penalty = 0;
    int broken_victories = 0;
    int break_bonus = 0;
    if (user->getPowerSide() == winning_power_side) {
        const int victories = user->GetPowerWarPartyVictory();
        penalty =
            G_CDataManager()
                ->get_power_war_victory_penalty(victories)
                .value_0c;
        if (opponent) {
            broken_victories = opponent->GetPowerWarPartyVictory();
            if (broken_victories > 4) {
                break_bonus =
                    G_CDataManager()->get_power_war_break_bonus(
                        broken_victories);
                _SendHoldVictory(
                    user, opponent, broken_victories);
            }
        }
        _SendVictoryBonus(user,
                          victories,
                          penalty,
                          broken_victories,
                          break_bonus);
    } else if (opponent) {
        const int victories = opponent->GetPowerWarPartyVictory();
        penalty =
            G_CDataManager()
                ->get_power_war_victory_penalty(victories)
                .value_0c;
        broken_victories = user->GetPowerWarPartyVictory();
        if (broken_victories > 4) {
            break_bonus =
                G_CDataManager()->get_power_war_break_bonus(
                    broken_victories);
            _SendHoldVictory(
                opponent, user, broken_victories);
        }
        _SendVictoryBonus(opponent,
                          victories,
                          penalty,
                          broken_victories,
                          break_bonus);
    }
    return penalty + break_bonus;
}

// IDA_EA: 0x82ECEB2
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace31_SendPowerWarPointToGuildServerEP5CUser30ENUM_POWER_WAR_WINNER_OR_LOSERiii
// DEMANGLED: pvp_assault::CAssaultPlace::_SendPowerWarPointToGuildServer(CUser *,ENUM_POWER_WAR_WINNER_OR_LOSER,int,int,int)
int CAssaultPlace::_SendPowerWarPointToGuildServer(
    CUser* user,
    ENUM_POWER_WAR_WINNER_OR_LOSER result,
    int winner_count,
    int loser_count,
    int live_count) {
    const char* const function_name =
        "void pvp_assault::CAssaultPlace::"
        "_SendPowerWarPointToGuildServer(CUser*, "
        "ENUM_POWER_WAR_WINNER_OR_LOSER, int, int, int)";
    if (!user) {
        LogManager::logFormat(
            1,
            "Assault.cpp",
            function_name,
            3916,
            "Invalid User pointer income (%d, %d, %d, %d)",
            static_cast<int>(result),
            winner_count,
            loser_count,
            live_count);
        return 0;
    }

    Packet_Guild_Change_Power_War_Point packet;
    char winning_power_side = 0;
    if (static_cast<int>(result) == 1) {
        winning_power_side =
            static_cast<char>(user->getPowerSide());
    } else {
        winning_power_side =
            user->getPowerSide() == 1 ? 2 : 1;
    }
    packet.power_side =
        static_cast<uint8_t>(winning_power_side);

    if (winner_count - live_count < 0) {
        LogManager::logFormat(
            1,
            "Assault.cpp",
            function_name,
            3932,
            "Power War Point Computer Error( winCnt : %d, "
            "winAliveCnt : %d )",
            winner_count,
            live_count);
        live_count = winner_count;
    }

    const int multiplier =
        G_CDataManager()->get_power_war_point_multiplier();
    const int loser_point =
        (winner_count - live_count) * multiplier;
    if (loser_point <= 255) {
        packet.loser_point =
            static_cast<uint8_t>(loser_point);
    } else {
        LogManager::logFormat(
            1,
            "Assault.cpp",
            function_name,
            3938,
            "Power War Point Computer Error( winCnt : %d, "
            "winAliveCnt : %d , multiple n : %d)",
            winner_count,
            live_count,
            multiplier);
        packet.loser_point = 0xffu;
    }

    const int winner_point = loser_count * multiplier;
    if (winner_point <= 255) {
        packet.winner_point =
            static_cast<uint8_t>(winner_point);
    } else {
        LogManager::logFormat(
            1,
            "Assault.cpp",
            function_name,
            3946,
            "Power War Point Computer Error( winCnt : %d, "
            "winAliveCnt : %d , multiple n : %d )",
            winner_count,
            live_count,
            multiplier);
        packet.winner_point = 0xffu;
    }

    packet.winner_point = static_cast<uint8_t>(
        packet.winner_point +
        _VictoryBonus(user, winning_power_side));

    int winner_index = 0;
    int loser_index = 0;
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) {
            continue;
        }
        CUser* const member = assaulter.GetUser();
        if (!member) {
            continue;
        }

        member->history_log().EndPvP(
            static_cast<ENUM_PVP_HISTORY_REASON>(1),
            member->getPowerSide() == winning_power_side,
            static_cast<unsigned int>(
                GlobalData::system_time()->getCurSec() -
                static_cast<int>(assault_start_second)));
        member->IncPowerWarAssaultCount();
        if (member->getPowerSide() == winning_power_side) {
            member->IncPowerWarAssaultVictoryCount();
            member->IncreasePowerWarPoint(packet.winner_point);
            if (winner_index <= 3) {
                packet.winner_charac_nos[winner_index] =
                    static_cast<uint32_t>(
                        member->get_charac_no(-1));
                ++winner_index;
                _SendPowerWarAssaultResultToUser(
                    member, packet.winner_point);
            } else {
                LogManager::logFormat(
                    1,
                    "Assault.cpp",
                    function_name,
                    3988,
                    "Power War Point Computer Error( winner index : "
                    "%d, loser index : %d )",
                    winner_index,
                    loser_index);
            }
        } else {
            member->IncreasePowerWarPoint(packet.loser_point);
            if (loser_index <= 3) {
                packet.loser_charac_nos[loser_index] =
                    static_cast<uint32_t>(
                        member->get_charac_no(-1));
                ++loser_index;
                _SendPowerWarAssaultResultToUser(
                    member, packet.loser_point);
            } else {
                LogManager::logFormat(
                    1,
                    "Assault.cpp",
                    function_name,
                    4004,
                    "Power War Point Computer Error( winner index : "
                    "%d, loser index : %d )",
                    winner_index,
                    loser_index);
            }
        }
    }

    cMyTrace trace(function_name, 4013, 0);
    trace("win side(%d), win pp(%d, %d), lose pp(%d, %d), "
          "winAliveCnt(%d)",
          winning_power_side,
          packet.winner_point,
          winner_count,
          packet.loser_point,
          loser_count,
          live_count);

    GlobalData::s_guild_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(0))
        ->SendPowerWarPoint(packet);
    GlobalData::s_power_manager->IncreaePowerWarPointInChannel(
        winning_power_side, packet.winner_point);
    const char losing_power_side =
        winning_power_side == 1 ? 2 : 1;
    CPowerManager* const update_result =
        GlobalData::s_power_manager->IncreaePowerWarPointInChannel(
            losing_power_side, packet.loser_point);
    return static_cast<int>(
        reinterpret_cast<intptr_t>(update_result));
}

// IDA_EA: 0x82EAD6A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17LeaveAssaultPlaceEibb
// DEMANGLED: pvp_assault::CAssaultPlace::LeaveAssaultPlace(int,bool,bool)
int CAssaultPlace::LeaveAssaultPlace(int index, bool force, bool evade) {
    if (static_cast<unsigned int>(index) >= 8u) {
        return evade;
    }

    CAssaulter& assaulter = assaulters[index];
    CUser* const user = assaulter.GetUser();
    if (user) {
        if (_IsGiveupPanalty(evade)) {
            assaulter.GiveupPanalty();
            if (state > 1u && state <= 3u) {
                user->SetCurCharacLeaveTimeout(true);
            } else if (state > 3u && state <= 6u) {
                PacketGuard user_state_packet;
                _MakeUserState(&user_state_packet, user);
                G_GameWorld()->send_all(user_state_packet);
            }
            if (_IsBroadcastRunaway(user)) {
                _BroadcastRunaway(user, index);
            }
        }

        PacketGuard leave_packet;
        _MakeLeaveUser(&leave_packet, index);
        SendPacket(leave_packet);

        const bool power_war_user =
            CPowerManager::IsPowerWarEventOn() &&
            static_cast<unsigned char>(user->getCurCharacVill()) == 7u;
        if (power_war_user) {
            GlobalData::s_power_manager->GetPowerWarLog()
                ->IncTotalLeaveAssaultUser();
            _SendCloseAssault(user, user->getPowerWarHP());
        } else {
            _SendCloseAssault(user, assaulter.GetHp());
        }

        if (!assaulter.Empty()) {
            _DelUser(index, force);
            if (!user->IsCurCharacGhost()) {
                PacketGuard user_state_packet;
                _MakeUserState(&user_state_packet, user);
                G_GameWorld()->send_all(user_state_packet);
            }
        }
    }

    const bool normal_leave =
        user &&
        (!CPowerManager::IsPowerWarEventOn() ||
         static_cast<unsigned char>(user->getCurCharacVill()) != 7u);
    if ((normal_leave && state <= 3u &&
         static_cast<unsigned int>(index) <= 1u) ||
        assaulter_count == 0u) {
        if (party_info[0].Get()) {
            party_info[0].Get()->SetAssaultState(7);
        }
        if (party_info[1].Get()) {
            party_info[1].Get()->SetAssaultState(7);
        }
        state = 7u;
        return CloseAssault();
    }

    if (static_cast<unsigned char>(_CheckPlayable()) == 1u) {
        return 0;
    }

    const bool power_war_user =
        CPowerManager::IsPowerWarEventOn() && user &&
        static_cast<unsigned char>(user->getCurCharacVill()) == 7u;
    if (power_war_user && state == 4u) {
        int winner_count = 0;
        int loser_count = 0;
        int live_count = 0;
        if (_CheckPowerWarInPlayer(
                static_cast<ENUM_POWER_SIDE_TYPE>(user->getPowerSide()),
                static_cast<ENUM_POWER_WAR_WINNER_OR_LOSER>(2),
                winner_count,
                loser_count,
                live_count)) {
            _SendPowerWarPointToGuildServer(
                user,
                static_cast<ENUM_POWER_WAR_WINNER_OR_LOSER>(2),
                winner_count,
                loser_count,
                live_count);
        }

        CParty* const party = user->GetParty();
        if (party) {
            party->resetStraightVictories();
        } else {
            user->resetStraightVictories();
        }

        for (int other_index = 0; other_index <= 7; ++other_index) {
            CAssaulter& other = assaulters[other_index];
            if (other.Empty()) {
                continue;
            }
            CUser* const other_user = other.GetUser();
            if (other_user->getPowerSide() != user->getPowerSide()) {
                _SendStraightVictories(other_user);
                break;
            }
        }
    }

    if (party_info[0].Get()) {
        party_info[0].Get()->SetAssaultState(6);
    }
    if (party_info[1].Get()) {
        party_info[1].Get()->SetAssaultState(6);
    }
    state = 6u;
    return EndAssault();
}

// IDA_EA: 0x82EB3BA
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace24GetAnotherTeamCharacNameEi
// DEMANGLED: pvp_assault::CAssaultPlace::GetAnotherTeamCharacName(int)
const char* CAssaultPlace::GetAnotherTeamCharacName(int index) {
    static const char empty_name[] = "";
    if (!assaulters[index].Empty()) {
        for (int other_index = 0; other_index <= 7; ++other_index) {
            if (assaulters[other_index].GetTeam() !=
                assaulters[index].GetTeam()) {
                CUser* const user = assaulters[other_index].GetUser();
                if (user) {
                    return user->getCurCharacName();
                }
            }
        }
    }
    return empty_name;
}

// IDA_EA: 0x82EB956
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace10ConnectP2PEP5CUserPci
// DEMANGLED: pvp_assault::CAssaultPlace::ConnectP2P(CUser *,char *,int)
int CAssaultPlace::ConnectP2P(CUser* user, char* states, int count) {
    if (state != 3u) {
        throw static_cast<ENUM_ERROR>(19);
    }

    bool received_all = true;
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) {
            continue;
        }
        if (assaulter.GetUser() == user) {
            if (assaulter.GetState() != 3) {
                throw static_cast<ENUM_ERROR>(18);
            }
            assaulter.SetState(static_cast<ENUM_ASSAULT_STATE>(4));
            assaulter.SetUserConnectP2P(states, count);
        }
        if (assaulter.GetState() != 4) {
            received_all = false;
        }
    }

    if (received_all) {
        StartAssault();
        if (party_info[0].Get()) {
            party_info[0].Get()->SetAssaultState(4);
        }
        if (party_info[1].Get()) {
            party_info[1].Get()->SetAssaultState(4);
        }
    }
    return received_all ? 1 : 0;
}

// IDA_EA: 0x82EF27A
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr12OnConnectP2PEP5CUserPci
// DEMANGLED: pvp_assault::CAssaultMgr::OnConnectP2P(CUser *,char *,int)
int CAssaultMgr::OnConnectP2P(CUser* user, char* states, int count) {
    _GetAssaultPlace(
        static_cast<unsigned int>(user->GetAssaultPlace()))
        ->ConnectP2P(user, states, count);
    return 1;
}

// IDA_EA: 0x82EF570
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr10OnChangeHpEP5CUseri
// DEMANGLED: pvp_assault::CAssaultMgr::OnChangeHp(CUser *,int)
int CAssaultMgr::OnChangeHp(CUser* user, int hp) {
    _GetAssaultPlace(static_cast<unsigned int>(user->GetAssaultPlace()))
        ->ChangeHp(user, hp);
    return 1;
}

// IDA_EA: 0x82EFC54
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr11OnAbuseUserEP5CUser
// DEMANGLED: pvp_assault::CAssaultMgr::OnAbuseUser(CUser *)
int CAssaultMgr::OnAbuseUser(CUser* user) {
    _GetAssaultPlace(static_cast<unsigned int>(user->GetAssaultPlace()))
        ->AbuseUser(user);
    return 1;
}

}  // namespace pvp_assault
