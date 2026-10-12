#include "old_game/pvp/battlefield/pvp_assault.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

enum ENUM_ERROR : int {};

namespace pvp_assault {

// IDA_EA: 0x82F0614
// IDA_NAME: _ZN11pvp_assault13CAssaultPlaceC2Ev
// DEMANGLED: pvp_assault::CAssaultPlace::CAssaultPlace(void)
CAssaultPlace::CAssaultPlace()
    : assaulters(),
      party_info() {}

// IDA_EA: 0x82E6AB8
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace5ResetEv
// DEMANGLED: pvp_assault::CAssaultPlace::Reset(void)
CAssaultPlace* CAssaultPlace::Reset() {
    for (int index = 0; index != 8; ++index) {
        CAssaulter& assaulter = assaulters[index];
        CUser* const user = assaulter.GetUser();
        if (user) {
            user->SetAssaultPlace(0);
        }
        assaulter.Reset();
    }

    assaulter_count = 0;
    assault_place_id = 0;
    authen_value = 0;
    state = 0;
    pos_x = 0;
    pos_y = 0;
    village = 0;
    area = 0;
    team_result[0] = 0;
    team_result[1] = 0;
    winner_team = 0;
    assaulted_user = 0;
    request_user = 0;
    assault_start_second = 0;
    return this;
}

// IDA_EA: 0x82E6BF6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace10SendPacketER11PacketGuard
// DEMANGLED: pvp_assault::CAssaultPlace::SendPacket(PacketGuard &)
int CAssaultPlace::SendPacket(PacketGuard& packet) {
    int result = 1;
    for (int index = 0; index != 8; ++index) {
        CUser* const user = assaulters[index].GetUser();
        if (user && user->Send(packet) != 1) {
            result = 0;
        }
    }
    return result;
}

// IDA_EA: 0x82E6C56
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace10SendPacketER11PacketGuard17ENUM_ASSAULT_TEAM
// DEMANGLED: pvp_assault::CAssaultPlace::SendPacket(PacketGuard &,ENUM_ASSAULT_TEAM)
int CAssaultPlace::SendPacket(PacketGuard& packet, ENUM_ASSAULT_TEAM team) {
    int result = 1;
    for (int index = 0; index != 8; ++index) {
        CAssaulter& assaulter = assaulters[index];
        CUser* const user = assaulter.GetUser();
        if (user && assaulter.GetTeam() == static_cast<int>(team) &&
            user->Send(packet) != 1) {
            result = 0;
        }
    }
    return result;
}

// IDA_EA: 0x82E6CDC
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace7GetTeamEP5CUserR17ENUM_ASSAULT_TEAM
// DEMANGLED: pvp_assault::CAssaultPlace::GetTeam(CUser *,ENUM_ASSAULT_TEAM &)
int CAssaultPlace::GetTeam(CUser* user, ENUM_ASSAULT_TEAM& team) {
    for (int index = 0; index != 8; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.user == user) {
            team = static_cast<ENUM_ASSAULT_TEAM>(assaulter.team);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x82E6D3A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendPacketToAreaER11PacketGuard
// DEMANGLED: pvp_assault::CAssaultPlace::_SendPacketToArea(PacketGuard &)
int CAssaultPlace::_SendPacketToArea(PacketGuard& packet) {
    if (!village && !area) {
        return 0;
    }
    G_GameWorld()->send_to_area(village, area, packet);
    return 1;
}

// IDA_EA: 0x82E6DCE
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendPacketToAreaER11PacketGuardRSt6vectorIP5CUserSaIS5_EE
// DEMANGLED: pvp_assault::CAssaultPlace::_SendPacketToArea(
//     PacketGuard &,std::vector<CUser *,std::allocator<CUser *>> &)
int CAssaultPlace::_SendPacketToArea(PacketGuard& packet,
                                     std::vector<CUser*>& users) {
    if (!village && !area) {
        return 0;
    }
    return G_GameWorld()->send_to_area(
        static_cast<int>(village), static_cast<int>(area), packet, users);
}

// IDA_EA: 0x82E6D92
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendPacketToVillER11PacketGuard
// DEMANGLED: pvp_assault::CAssaultPlace::_SendPacketToVill(PacketGuard &)
int CAssaultPlace::_SendPacketToVill(PacketGuard& packet) {
    if (!village) {
        return 0;
    }
    G_GameWorld()->send_to_vill(village, packet);
    return 1;
}

// IDA_EA: 0x82E7162
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_GetTeamCountE17ENUM_ASSAULT_TEAM
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCount(ENUM_ASSAULT_TEAM)
int CAssaultPlace::_GetTeamCount(ENUM_ASSAULT_TEAM team) {
    int count = 0;
    for (int index = 0; index != 8; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (!assaulter.Empty() && assaulter.team == static_cast<int>(team)) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x82E71C4
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace22_GetTeamCountAliveUserEPi
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCountAliveUser(int *)
bool CAssaultPlace::_GetTeamCountAliveUser(int* team_count) {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        CUser* const user = assaulters[index].user;
        if (user && !user->IsCurCharacGhost()) {
            ++team_count[assaulters[index].team];
        }
    }
    return result;
}

// IDA_EA: 0x82E7288
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_GetTeamCountEPiNS_18ENUM_ASSAULT_STATEE
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCount(int *,pvp_assault::ENUM_ASSAULT_STATE)
bool CAssaultPlace::_GetTeamCount(int* team_count,
                                  ENUM_ASSAULT_STATE assault_state) {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        CAssaulter& assaulter = assaulters[index];
        if (!assaulter.Empty() &&
            assaulter.state == static_cast<int>(assault_state)) {
            ++team_count[assaulter.team];
        }
    }
    return result;
}

// IDA_EA: 0x82E8810
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_MakeCompleteLoadEP11PacketGuardi
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeCompleteLoad(PacketGuard *,int)
int CAssaultPlace::_MakeCompleteLoad(PacketGuard* packet, int index) {
    packet->put_header(0, 129);
    packet->put_short(assaulters[index].GetUserUniqueId());
    return packet->finalize(true);
}

// IDA_EA: 0x82E886A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_MakeStartAssaultEP11PacketGuard
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeStartAssault(PacketGuard *)
int CAssaultPlace::_MakeStartAssault(PacketGuard* packet) {
    packet->put_header(0, 130);
    return packet->finalize(true);
}

// IDA_EA: 0x82E88A0
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace15_MakeEndAssaultEP11PacketGuard
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeEndAssault(PacketGuard *)
int CAssaultPlace::_MakeEndAssault(PacketGuard* packet) {
    packet->put_header(0, 132);
    return packet->finalize(true);
}

// IDA_EA: 0x82E88D6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_MakeCloseAssaultEP11PacketGuardi
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeCloseAssault(PacketGuard *,int)
int CAssaultPlace::_MakeCloseAssault(PacketGuard* packet, int hp) {
    packet->put_header(0, 137);
    packet->put_byte(hp);
    return packet->finalize(true);
}

// IDA_EA: 0x82E891E
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace21_MakeDieAssaultPlayerEP11PacketGuardi
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeDieAssaultPlayer(PacketGuard *,int)
int CAssaultPlace::_MakeDieAssaultPlayer(PacketGuard* packet, int index) {
    packet->put_header(0, 131);
    packet->put_short(assaulters[index].GetUserUniqueId());
    return packet->finalize(true);
}

// IDA_EA: 0x82E8978
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace25_MakeRevivalAssaultPlayerEP11PacketGuardi
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeRevivalAssaultPlayer(PacketGuard *,int)
int CAssaultPlace::_MakeRevivalAssaultPlayer(PacketGuard* packet, int unique_id) {
    packet->put_header(0, 133);
    packet->put_short(unique_id);
    return packet->finalize(true);
}

// IDA_EA: 0x82ED43E
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace11RevivalUserEP5CUserRi
// DEMANGLED: pvp_assault::CAssaultPlace::RevivalUser(CUser *,int &)
void CAssaultPlace::RevivalUser(CUser* user, int& remaining_seconds) {
    if (user->IsCurCharacGhost() != 1) {
        throw static_cast<ENUM_ERROR>(18);
    }

    const uint32_t current_second = GlobalData::s_systemTime_.getCurSec();
    if (CPowerManager::IsPowerWarEventOn() || user->getCurCharacVill() == 7) {
        if (GlobalData::s_power_manager->IsAbleToRevival(
                user, current_second, remaining_seconds) != 1) {
            throw static_cast<ENUM_ERROR>(22);
        }
    } else {
        const int elapsed_seconds =
            static_cast<int>(current_second) - user->GetCurCharacChaosDieTime();
        if (elapsed_seconds <= 599) {
            remaining_seconds = 600 - elapsed_seconds;
            throw static_cast<ENUM_ERROR>(22);
        }
    }

    PacketGuard packet;
    _MakeRevivalAssaultPlayer(&packet, user->get_unique_id());
    G_GameWorld()->send_all(packet);
    user->SetCurCharacGhost(false);
}

// IDA_EA: 0x82E8AFC
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace26_MakeViewAssaultIconPlayerEP11PacketGuardb
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeViewAssaultIconPlayer(PacketGuard *,bool)
int CAssaultPlace::_MakeViewAssaultIconPlayer(PacketGuard* packet,
                                               bool visible) {
    packet->put_header(0, 134);
    packet->put_short(1);
    MakeViewAssaultIconOnePlayer(packet, visible);
    return packet->finalize(true);
}

// IDA_EA: 0x82E8C5A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_MakeUserStateEP11PacketGuard
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeUserState(PacketGuard *)
int CAssaultPlace::_MakeUserState(PacketGuard* packet) {
    packet->put_header(0, 3);
    int count = 0;
    int count_index = packet->get_index();
    packet->put_byte(0);
    for (int index = 0; index != 8; ++index) {
        CUser* const user = assaulters[index].GetUser();
        if (!user) {
            continue;
        }
        packet->put_short(user->get_unique_id());
        packet->put_byte(user->GetUserState());
        ++count;
    }
    packet->put_byte(count_index, count);
    return packet->finalize(true);
}

// IDA_EA: 0x82E8D60
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_MakeUserStateEP11PacketGuardP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeUserState(PacketGuard *,CUser *)
int CAssaultPlace::_MakeUserState(PacketGuard* packet, CUser* user) {
    packet->put_header(0, 3);
    packet->put_byte(1);
    packet->put_short(user->get_unique_id());
    packet->put_byte(user->GetUserState());
    return packet->finalize(true);
}

// IDA_EA: 0x82E9046
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_MakeLeaveUserEP11PacketGuardi
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeLeaveUser(PacketGuard *,int)
int CAssaultPlace::_MakeLeaveUser(PacketGuard* packet, int index) {
    packet->put_header(0, 135);
    packet->put_short(assaulters[index].GetUserUniqueId());
    return packet->finalize(true);
}

// IDA_EA: 0x82E90A0
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_MakeChangeHpEP11PacketGuardii
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeChangeHp(PacketGuard *,int,int)
int CAssaultPlace::_MakeChangeHp(PacketGuard* packet, int index, int hp) {
    packet->put_header(0, 136);
    packet->put_short(assaulters[index].GetUserUniqueId());
    packet->put_int(hp);
    return packet->finalize(true);
}

// IDA_EA: 0x82E79F6
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_SendUserStateEv
// DEMANGLED: pvp_assault::CAssaultPlace::_SendUserState(void)
void CAssaultPlace::_SendUserState() {
    PacketGuard packet;
    _MakeUserState(&packet);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x82E83C4
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13SendUserStateEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::SendUserState(CUser *)
void CAssaultPlace::SendUserState(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 3);
    packet.put_byte(1);
    packet.put_short(user->get_unique_id());
    packet.put_byte(user->GetUserState());
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
}

// IDA_EA: 0x82E7B2A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendCloseAssaultEP5CUseri
// DEMANGLED: pvp_assault::CAssaultPlace::_SendCloseAssault(CUser *,int)
int CAssaultPlace::_SendCloseAssault(CUser* user, int hp) {
    if (user->getCurCharacR()) {
        if (hp <= 29) {
            hp = 30;
        }
        PacketGuard packet;
        _MakeCloseAssault(&packet, hp);
        user->Send(packet);
    }
    return hp;
}

}  // namespace pvp_assault
