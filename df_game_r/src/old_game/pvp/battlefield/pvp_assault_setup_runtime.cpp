#include "old_game/pvp/battlefield/pvp_assault.h"

#include "common/manager/global_data.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <vector>

enum ENUM_ERROR : int {};

namespace pvp_assault {

namespace {

const int kAssaultTeamCount = 2;
const int kAssaultUserCount = 8;
const int kPartyMemberCount = 4;

}  // namespace

// IDA_EA: 0x82E7018
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace8_AddUserEP5CUser17ENUM_ASSAULT_TEAM
int CAssaultPlace::_AddUser(CUser* user, ENUM_ASSAULT_TEAM team) {
    for (int index = 0; index < kAssaultUserCount; ++index) {
        if (!assaulters[index].IsAttachAble()) continue;

        assaulters[index].AttachUser(user, team);
        user->SetAssaultPlace(static_cast<int>(assault_place_id));
        ++assaulter_count;
        return 1;
    }
    throw static_cast<ENUM_ERROR>(4);
}

// IDA_EA: 0x82E70C8
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace8_AddTeamEP5CUser17ENUM_ASSAULT_TEAM
int CAssaultPlace::_AddTeam(CUser* user, ENUM_ASSAULT_TEAM team) {
    CParty* const party = user->CheckInParty() ? user->GetParty() : 0;
    if (!party) return _AddUser(user, team);

    for (int seat = 0; seat < kPartyMemberCount; ++seat) {
        CUser* const member = party->get_user(seat);
        if (member) _AddUser(member, team);
    }
    return 1;
}

// IDA_EA: 0x82E75CA
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_AvailableAssaultEP5CUserRi
// DEMANGLED: pvp_assault::CAssaultPlace::_AvailableAssault(CUser *,int &)
char CAssaultPlace::_AvailableAssault(CUser* user, int& unique_id) {
    CParty* const party = user->CheckInParty() ? user->GetParty() : 0;
    const int minimum_age = CPowerManager::IsPowerWarEventOn() &&
                                    user->getCurCharacVill() == 7
                                ? 12
                                : 15;

    if (user->CheckLimitMiniumAge(minimum_age, unique_id) != 1) {
        throw static_cast<ENUM_ERROR>(95);
    }
    if (user->get_charac_level() <= 29) {
        unique_id = static_cast<uint16_t>(user->get_unique_id());
        throw static_cast<ENUM_ERROR>(request_user == user ? 13 : 14);
    }
    if (!party) {
        return 0;
    }

    if (party->is_limit_minimum_age(minimum_age, unique_id) != 1) {
        throw static_cast<ENUM_ERROR>(95);
    }
    if (party->is_limit_minimum_level(30, unique_id) != 1) {
        throw static_cast<ENUM_ERROR>(request_user == user ? 254 : 255);
    }
    return 0;
}

// IDA_EA: 0x82E782A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace9_MoveUserEPNS_12CDispositionEii
int CAssaultPlace::_MoveUser(CDisposition* disposition,
                             int village_id,
                             int area_id) {
    int point_index[kAssaultTeamCount] = {0, 0};
    for (int index = 0; index < kAssaultUserCount; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) continue;

        const int team = assaulter.GetTeam();
        CForcedMoveCharacter character = {};
        character.team = team;
        character.point_index = point_index[team];
        character.user = assaulter.GetUser();
        character.village = village_id;
        character.area = area_id;
        character.direction = GetDirectionByTeam(
            static_cast<ENUM_ASSAULT_TEAM>(team));
        disposition->MoveUser(&character);
        ++point_index[team];
    }
    return 1;
}

// IDA_EA: 0x82E8B66
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_MakeUserInfoEP11PacketGuard13ENUM_USERINFO
// DEMANGLED: pvp_assault::CAssaultPlace::_MakeUserInfo(PacketGuard *,ENUM_USERINFO)
int CAssaultPlace::_MakeUserInfo(PacketGuard* packet,
                                 ENUM_USERINFO user_info_type) {
    packet->put_header(0, 2);
    packet->put_byte(user_info_type);

    int count = 0;
    int count_index = packet->get_index();
    packet->put_short(0);
    for (int index = 0; index < kAssaultUserCount; ++index) {
        CUser* const user = assaulters[index].GetUser();
        if (!user) continue;
        user->make_basic_info(*packet, static_cast<char>(user_info_type));
        ++count;
    }
    packet->put_short(count_index, count);
    return packet->finalize(true);
}

// IDA_EA: 0x82E8DE2
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace18_MakeUserUdpIpPortEP11PacketGuard
int CAssaultPlace::_MakeUserUdpIpPort(PacketGuard* packet) {
    packet->put_header(0, 11);

    int count = 0;
    int count_index = packet->get_index();
    packet->put_byte(0);
    for (int index = 0; index < kAssaultUserCount; ++index) {
        CUser* const user = assaulters[index].GetUser();
        if (!user) continue;

        CNetwork4096_450000& network = user->network_;
        packet->put_short(user->get_unique_id());
        packet->put_int(network.get_inner_ip());
        packet->put_int(network.get_ip());
        packet->put_short(network.get_port());
        packet->put_int(user->get_acc_id());
        packet->put_byte(network.get_nat_type());
        packet->put_int(network.get_mtu());
        const _Charac_info* const current_character = user->getCurCharacR();
        packet->put_byte(current_character
                             ? current_character->fields_.stamina_
                             : 0);
        ++count;
    }
    packet->put_byte(count_index, count);
    return packet->finalize(true);
}

// IDA_EA: 0x82E910C
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_MakeGiveAlarmEP11PacketGuard
int CAssaultPlace::_MakeGiveAlarm(PacketGuard* packet) {
    packet->put_header(0, 139);
    return packet->finalize(true);
}

// IDA_EA: 0x82E852E
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace21_MakeMoveAssaultPlaceEP11PacketGuard
int CAssaultPlace::_MakeMoveAssaultPlace(PacketGuard* packet) {
    packet->put_header(0, 128);
    packet->put_int(assault_place_id);
    packet->put_byte(village);
    packet->put_byte(area);
    packet->put_short(request_user ? request_user->get_unique_id() : 0);
    packet->put_short(assaulted_user ? assaulted_user->get_unique_id() : 0);

    int assault_user_count = 0;
    int assault_user_count_index = packet->get_index();
    packet->put_byte(0);
    for (int index = 0; index < kAssaultUserCount; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) continue;
        packet->put_byte(index);
        packet->put_byte(assaulter.GetTeam() + 1);
        packet->put_short(assaulter.GetUserUniqueId());
        ++assault_user_count;
    }
    packet->put_byte(assault_user_count_index, assault_user_count);

    std::vector<uint16_t> area_user_ids;
    G_GameWorld()->get_user_id_list(static_cast<int>(village),
                                    static_cast<int>(area),
                                    area_user_ids);
    int viewer_count = 0;
    int viewer_count_index = packet->get_index();
    packet->put_byte(0);
    for (size_t index = 0; index < area_user_ids.size() && index <= 9u;
         ++index) {
        packet->put_short(area_user_ids[index]);
        ++viewer_count;
    }
    packet->put_byte(viewer_count_index, viewer_count);
    return packet->finalize(true);
}

// IDA_EA: 0x82E7918
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_SendUserInfoEv
// DEMANGLED: pvp_assault::CAssaultPlace::_SendUserInfo(void)
void CAssaultPlace::_SendUserInfo() {
    PacketGuard packet;
    _MakeUserInfo(&packet, static_cast<ENUM_USERINFO>(1));
    SendPacket(packet);
}

// IDA_EA: 0x82E7986
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendUserMiniInfoEv
// DEMANGLED: pvp_assault::CAssaultPlace::_SendUserMiniInfo(void)
void CAssaultPlace::_SendUserMiniInfo() {
    PacketGuard packet;
    _MakeUserInfo(&packet, static_cast<ENUM_USERINFO>(0));
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x82E7A5E
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace18_SendUserUdpIpPortEv
// DEMANGLED: pvp_assault::CAssaultPlace::_SendUserUdpIpPort(void)
void CAssaultPlace::_SendUserUdpIpPort() {
    PacketGuard packet;
    _MakeUserUdpIpPort(&packet);
    SendPacket(packet);
}

// IDA_EA: 0x82E7AC4
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_SendGiveAlarmEv
// DEMANGLED: pvp_assault::CAssaultPlace::_SendGiveAlarm(void)
void CAssaultPlace::_SendGiveAlarm() {
    PacketGuard packet;
    _MakeGiveAlarm(&packet);
    SendPacket(packet);
}

// IDA_EA: 0x82E8300
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_MoveAssaultPlaceEv
void CAssaultPlace::_MoveAssaultPlace() {
    PacketGuard packet;
    _MakeMoveAssaultPlace(&packet);
    SendPacket(packet);
}

// IDA_EA: 0x82E7BBC
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace16_DispositionUserEPP5CUser
void CAssaultPlace::_DispositionUser(CUser** users) {
    int team_count[kAssaultTeamCount] = {0, 0};
    _GetTeamCount(team_count);

    CStaticDisposition disposition(users, team_count);
    if (G_GameWorld()->get_disposition(static_cast<int>(village),
                                       static_cast<int>(area),
                                       &disposition) != 1) {
        throw static_cast<ENUM_ERROR>(3);
    }

    int flag_x = 0;
    int flag_y = 0;
    disposition.GetFlagPoint(flag_x, flag_y);
    pos_x = static_cast<uint32_t>(flag_x);
    pos_y = static_cast<uint32_t>(flag_y);
    _SendGiveAlarm();
    _SendUserInfo();
    _SendUserUdpIpPort();
    _MoveUser(&disposition, static_cast<int>(village), static_cast<int>(area));
    _MoveAssaultPlace();
}

// IDA_EA: 0x82E804A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace16_BackupPartyInfoEP6CParty17ENUM_ASSAULT_TEAM
PartyInfo* CAssaultPlace::_BackupPartyInfo(CParty* party,
                                           ENUM_ASSAULT_TEAM team) {
    return party_info[static_cast<int>(team)].Set(party);
}

// IDA_EA: 0x82E8098
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_SeparateTeamEP5CUserS2_
int CAssaultPlace::_SeparateTeam(CUser* first_user, CUser* second_user) {
    std::vector<CUser*> team_users[kAssaultTeamCount];
    _BackupPartyInfo(first_user->GetUserListByParty(team_users[0]),
                     static_cast<ENUM_ASSAULT_TEAM>(0));
    _BackupPartyInfo(second_user->GetUserListByParty(team_users[1]),
                     static_cast<ENUM_ASSAULT_TEAM>(1));

    for (int index = 0; index < kAssaultUserCount; ++index) {
        for (int team = 0; team < kAssaultTeamCount; ++team) {
            if (static_cast<size_t>(index) < team_users[team].size()) {
                _AddUser(team_users[team][index],
                         static_cast<ENUM_ASSAULT_TEAM>(team));
            }
        }
    }

    CUser* users[kAssaultTeamCount] = {first_user, second_user};
    team_chaos_start[0] =
        first_user->GetCurCharacChaosPoint() > 0 ? 1u : 0u;
    team_chaos_start[1] =
        second_user->GetCurCharacChaosPoint() > 0 ? 1u : 0u;
    _DispositionUser(users);
    return 1;
}

// IDA_EA: 0x82E82A2
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_SetReadyStateEv
bool CAssaultPlace::_SetReadyState() {
    state = 2;
    for (int index = 0; index < kAssaultUserCount; ++index) {
        if (!assaulters[index].Empty()) {
            assaulters[index].SetState(static_cast<ENUM_ASSAULT_STATE>(1));
        }
    }
    return true;
}

// IDA_EA: 0x82E7D0A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace8MakeTeamEP5CUserS2_Ri
bool CAssaultPlace::MakeTeam(CUser* requester, CUser* target) {
    village = static_cast<unsigned char>(requester->getCurCharacVill());
    area = static_cast<uint32_t>(requester->get_area(false));
    if (!village && !area) throw static_cast<ENUM_ERROR>(21);

    request_user = requester;
    assaulted_user = target;
    if (requester->get_posX() >= target->get_posX()) {
        _SeparateTeam(target, requester);
        winner_team = 1;
    } else {
        _SeparateTeam(requester, target);
        winner_team = 0;
    }

    authen_value = static_cast<uint32_t>(GlobalData::system_time()->getCurSec());
    _SetReadyState();
    if (party_info[0].Get()) party_info[0].Get()->SetAssaultState(2);
    if (party_info[1].Get()) party_info[1].Get()->SetAssaultState(2);

    const char* team_names[kAssaultTeamCount][kPartyMemberCount] = {};
    int team_name_count[kAssaultTeamCount] = {0, 0};
    for (int index = 0; index < kAssaultUserCount; ++index) {
        CAssaulter& assaulter = assaulters[index];
        if (assaulter.Empty()) continue;
        const int team = assaulter.GetTeam();
        if (team_name_count[team] < kPartyMemberCount) {
            team_names[team][team_name_count[team]++] =
                assaulter.GetCharacName();
        }
    }
    for (int index = 0; index < kAssaultUserCount; ++index) {
        CUser* const user = assaulters[index].GetUser();
        if (!user) continue;
        const int reason = user->getCurCharacVill() == 7 ? 0 : 4;
        user->history_log().StartPvP(
            static_cast<ENUM_PVP_HISTORY_REASON>(reason),
            team_names[0][0], team_names[0][1], team_names[0][2],
            team_names[0][3], team_names[1][0], team_names[1][1],
            team_names[1][2], team_names[1][3]);
    }
    return true;
}

}  // namespace pvp_assault
