#include "old_game/pvp/battlefield/pvp_assault.h"

#include <cstring>

#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"

enum ENUM_ERROR : int {};

namespace {

[[noreturn]] void ThrowDieAssaultError(int error) {
    throw static_cast<ENUM_ERROR>(error);
}

bool IsChaosModeDisabled() {
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(32);
    return event->IsEventing(0) == 0;
}

}  // namespace

namespace pvp_assault {

// IDA_EA: 0x82EBAEE
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_IsChangeDieStateEv
// DEMANGLED: pvp_assault::CAssaultPlace::_IsChangeDieState(void)
int CAssaultPlace::_IsChangeDieState() {
    return 1;
}

// IDA_EA: 0x82EBAF8
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace15_ChangeDieStateEi
// DEMANGLED: pvp_assault::CAssaultPlace::_ChangeDieState(int)
int CAssaultPlace::_ChangeDieState(int index) {
    CAssaulter& assaulter = assaulters[index];
    assaulter.SetState(static_cast<ENUM_ASSAULT_STATE>(6));

    if (!IsChaosModeDisabled()) {
        return 0;
    }

    CUser* const user = assaulter.GetUser();
    if (CPowerManager::IsPowerWarEventOn() && user &&
        user->getCurCharacVill() == 7) {
        return 0;
    }

    assaulter.DieUser();
    return 1;
}

// IDA_EA: 0x82EBD64
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace15_IsProcsssChaosEv
// DEMANGLED: pvp_assault::CAssaultPlace::_IsProcsssChaos(void)
int CAssaultPlace::_IsProcsssChaos() {
    return IsChaosModeDisabled() ? 1 : 0;
}

// IDA_EA: 0x82EBB9C
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_ProcessChaosEii
// DEMANGLED: pvp_assault::CAssaultPlace::_ProcessChaos(int,int)
void CAssaultPlace::_ProcessChaos(int dead_index, unsigned int killer_index) {
    if (killer_index >= 8u ||
        assaulters[killer_index].GetTeam() !=
            static_cast<int>(winner_team)) {
        return;
    }

    CUser* const dead_user = assaulters[dead_index].GetUser();
    CUser* const killer_user = assaulters[killer_index].GetUser();
    if (!killer_user) {
        return;
    }

    const int dead_team = assaulters[dead_index].GetTeam();
    if (team_chaos_start[dead_team] != 1u) {
        killer_user->AddCurCharacChaosPoint(10);
        if (killer_user->getCurCharacR()) {
            PacketGuard packet;
            packet.put_header(0, 2);
            packet.put_byte(0);
            packet.put_short(1);
            killer_user->make_basic_info(packet, 0);
            packet.finalize(true);
            G_GameWorld()->send_all(packet, killer_user);
        }
    }

    if (!dead_user) {
        return;
    }
    if (dead_user->GetCurCharacChaosPoint() <= 0) {
        killer_user->IncCurCharacChaosKillCount();
    } else {
        killer_user->IncCurCharacChaosModeCount();
    }
    dead_user->IncCurCharacChaosDieCount();
}

// IDA_EA: 0x82EBDA6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace7DieUserEiiR22MSG_DIE_ASSAULT_PLAYER
// DEMANGLED: pvp_assault::CAssaultPlace::DieUser(int,int,MSG_DIE_ASSAULT_PLAYER &)
int CAssaultPlace::DieUser(int dead_index,
                           unsigned int killer_index,
                           const MSG_DIE_ASSAULT_PLAYER& message) {
    if (state != 4u) {
        ThrowDieAssaultError(19);
    }

    CAssaulter& dead_assaulter = assaulters[dead_index];
    CUser* const dead_user = dead_assaulter.GetUser();
    if (dead_assaulter.GetState() != 5) {
        ThrowDieAssaultError(18);
    }

    CUser* killer_user = 0;
    char killer_name[21] = {};
    if (killer_index >= 8u) {
        cMyTrace trace(
            "void pvp_assault::CAssaultPlace::DieUser(int, int, "
            "MSG_DIE_ASSAULT_PLAYER&)",
            3412,
            0);
        trace("CAssaultPlace::DieUser invalid killer index income(%d,%d)",
              dead_index,
              killer_index);
    } else {
        CAssaulter& killer_assaulter = assaulters[killer_index];
        killer_user = killer_assaulter.GetUser();
        if (killer_assaulter.GetState() != 5) {
            cMyTrace trace(
                "void pvp_assault::CAssaultPlace::DieUser(int, int, "
                "MSG_DIE_ASSAULT_PLAYER&)",
                3383,
                0);
            trace("CAssaultPlace::DieUser => killer is already died!");
        }
        std::strncpy(killer_name,
                     killer_assaulter.GetCharacName(),
                     sizeof(killer_name) - 1u);

        if (killer_user && message.damage_value != 0u) {
            if (message.damage_value == 1u) {
                killer_user->getHackAnalyzer()->addServerHackCnt(
                    killer_user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x7d),
                    1,
                    0,
                    0);
            }
            if (message.damage_count / message.damage_value > 100000u) {
                killer_user->getHackAnalyzer()->addServerHackCnt(
                    killer_user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x7e),
                    1,
                    0,
                    0);
            }
        }
    }

    if (dead_user) {
        const ENUM_PVP_HISTORY_REASON reason =
            dead_user->getCurCharacVill() == 7 &&
                    CPowerManager::IsPowerWarEventOn()
                ? static_cast<ENUM_PVP_HISTORY_REASON>(2)
                : static_cast<ENUM_PVP_HISTORY_REASON>(6);
        dead_user->history_log().DiePvP(reason,
                                        message.damage_value,
                                        message.damage_count,
                                        message.attacker_stat0,
                                        message.attacker_stat1,
                                        message.attacker_stat2,
                                        message.attacker_stat3,
                                        killer_name);
    }

    if (_IsChangeDieState()) {
        _ChangeDieState(dead_index);
    }

    if (dead_user &&
        !(dead_user->getCurCharacVill() == 7 &&
          CPowerManager::IsPowerWarEventOn())) {
        PacketGuard packet;
        _MakeUserState(&packet, dead_user);
        G_GameWorld()->send_all(packet);
    }

    if (killer_user &&
        !(killer_user->getCurCharacVill() == 7 &&
          CPowerManager::IsPowerWarEventOn()) &&
        _IsProcsssChaos()) {
        _ProcessChaos(dead_index, killer_index);
    }

    if (_CheckAssaultable() == 0) {
        if (killer_user && dead_user && dead_user->getCurCharacVill() == 7 &&
            CPowerManager::IsPowerWarEventOn()) {
            const int power_side = killer_index >= 8u
                                       ? dead_user->getPowerSide()
                                       : killer_user->getPowerSide();
            const ENUM_POWER_WAR_WINNER_OR_LOSER result =
                killer_index >= 8u
                    ? static_cast<ENUM_POWER_WAR_WINNER_OR_LOSER>(2)
                    : static_cast<ENUM_POWER_WAR_WINNER_OR_LOSER>(1);
            int winner_count = 0;
            int loser_count = 0;
            int live_count = 0;
            if (_CheckPowerWarInPlayer(
                    static_cast<ENUM_POWER_SIDE_TYPE>(power_side),
                    result,
                    winner_count,
                    loser_count,
                    live_count)) {
                _SendPowerWarPointToGuildServer(killer_index >= 8u
                                                     ? dead_user
                                                     : killer_user,
                                                 result,
                                                 winner_count,
                                                 loser_count,
                                                 live_count);
                PacketGuard packet;
                _MakeUserState(&packet);
                G_GameWorld()->send_all(packet);
                _SendStraightVictories(killer_user);
            }
        }

        if (dead_user && dead_user->getCurCharacVill() == 7 &&
            CPowerManager::IsPowerWarEventOn()) {
            CParty* const party = dead_user->GetParty();
            if (party) {
                party->resetStraightVictories();
            } else {
                dead_user->resetStraightVictories();
            }
        }

        state = 5;
        if (party_info[0].Get()) party_info[0].Get()->SetAssaultState(5);
        if (party_info[1].Get()) party_info[1].Get()->SetAssaultState(5);
        return EndAssault();
    }

    return 0;
}

// IDA_EA: 0x82ED3FE
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace7DieUserEP5CUseriR22MSG_DIE_ASSAULT_PLAYER
// DEMANGLED: pvp_assault::CAssaultPlace::DieUser(CUser *,int,MSG_DIE_ASSAULT_PLAYER &)
int CAssaultPlace::DieUser(CUser* dead_user,
                           unsigned int killer_index,
                           const MSG_DIE_ASSAULT_PLAYER& message) {
    return DieUser(_GetUserIndex(dead_user, true), killer_index, message);
}

// IDA_EA: 0x82EF3BE
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr9OnDieUserEP5CUseriR22MSG_DIE_ASSAULT_PLAYER
// DEMANGLED: pvp_assault::CAssaultMgr::OnDieUser(CUser *,int,MSG_DIE_ASSAULT_PLAYER &)
int CAssaultMgr::OnDieUser(CUser* user,
                           int killer_index,
                           const MSG_DIE_ASSAULT_PLAYER& message) {
    CAssaultPlace* const place =
        _GetAssaultPlace(static_cast<unsigned int>(user->GetAssaultPlace()));
    place->DieUser(user, static_cast<unsigned int>(killer_index), message);
    return 1;
}

}  // namespace pvp_assault
