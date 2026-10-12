#include "old_game/pvp/battlefield/pvp_assault.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace pvp_assault {

// These three raw leaves are canonically emitted by
// pvp_assault_lifecycle_packets.cpp.  Keep this PowerWar unit focused on its
// non-overlapping state transitions so that both manifest objects can link.
#if 0
// IDA_EA: 0x82E7162
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_GetTeamCountE17ENUM_ASSAULT_TEAM
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCount(ENUM_ASSAULT_TEAM)
int CAssaultPlace::_GetTeamCount(ENUM_ASSAULT_TEAM team) {
    int count = 0;
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (!assaulter.Empty() && assaulter.GetTeam() == static_cast<int>(team)) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x82E71C4
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace22_GetTeamCountAliveUserEPi
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCountAliveUser(int &)
bool CAssaultPlace::_GetTeamCountAliveUser(int* team_count) {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        CAssaulter& assaulter = assaulters[index];
        CUser* const user = assaulter.GetUser();
        if (user && !user->IsCurCharacGhost()) {
            ++team_count[assaulter.GetTeam()];
        }
    }
    return result;
}

// IDA_EA: 0x82E7288
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_GetTeamCountEPiNS_18ENUM_ASSAULT_STATEE
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCount(int &,pvp_assault::ENUM_ASSAULT_STATE)
bool CAssaultPlace::_GetTeamCount(int* team_count,
                                  ENUM_ASSAULT_STATE required_state) {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        CAssaulter& assaulter = assaulters[index];
        if (!assaulter.Empty() &&
            assaulter.GetState() == static_cast<int>(required_state)) {
            ++team_count[assaulter.GetTeam()];
        }
    }
    return result;
}
#endif

// IDA_EA: 0x82E72FA
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace23_PowerWarAssaultCheckInEv
// DEMANGLED: pvp_assault::CAssaultPlace::_PowerWarAssaultCheckIn(void)
bool CAssaultPlace::_PowerWarAssaultCheckIn() {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        CAssaulter& assaulter = assaulters[index];
        if (!assaulter.Empty()) {
            CUser* const user = assaulter.GetUser();
            user->SetPowerWarLastPlayTime(
                GlobalData::system_time()->getCurSec());
        }
    }
    return result;
}

// IDA_EA: 0x82E7362
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace22_CheckPowerWarInPlayerE20ENUM_POWER_SIDE_TYPE30ENUM_POWER_WAR_WINNER_OR_LOSERRiS3_S3_
// DEMANGLED: pvp_assault::CAssaultPlace::_CheckPowerWarInPlayer(ENUM_POWER_SIDE_TYPE,ENUM_POWER_WAR_WINNER_OR_LOSER,int &,int &,int &)
int CAssaultPlace::_CheckPowerWarInPlayer(
    ENUM_POWER_SIDE_TYPE power_side,
    ENUM_POWER_WAR_WINNER_OR_LOSER winner_or_loser,
    int& winner_count,
    int& loser_count,
    int& alive_winner_count) {
    for (int index = 0; index <= 7; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) {
            continue;
        }

        CUser* const user = assaulter.GetUser();
        if (!user || !user->isJoinPowerWar()) {
            continue;
        }

        const bool same_side =
            user->getPowerSide() == static_cast<int>(power_side);
        const bool winner =
            (static_cast<int>(winner_or_loser) == 1) ? same_side : !same_side;
        if (winner) {
            if (assaulter.GetState() == 5) {
                ++alive_winner_count;
            }
            ++winner_count;
            assaulter.SetPowerWarWinner(true);
            user->SetCurCharacSuperState(true);
        } else {
            ++loser_count;
            assaulter.DieUser();
            user->setPowerWarHP(100u);
        }

        PacketGuard packet;
        _MakeChangeHp(&packet, index, user->getPowerWarHP());
        _SendPacketToVill(packet);
    }
    return 1;
}

// IDA_EA: 0x82E919A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_CheckAssaultableEv
// DEMANGLED: pvp_assault::CAssaultPlace::_CheckAssaultable(void)
int CAssaultPlace::_CheckAssaultable() {
    int team_count[2] = {0, 0};
    _GetTeamCount(team_count, static_cast<ENUM_ASSAULT_STATE>(5));
    for (int index = 0; index <= 1; ++index) {
        if (!team_count[index]) {
            return 0;
        }
    }
    return 1;
}

}  // namespace pvp_assault
