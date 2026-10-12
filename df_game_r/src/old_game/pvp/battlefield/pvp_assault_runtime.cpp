#include "old_game/pvp/battlefield/pvp_assault.h"
#include "common/manager/global_data.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

#include <map>
#include <stdint.h>

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// The original CAssaultMgr lookup overloads throw ENUM_ERROR values 21
// (unknown assault place) and 8 (authentication mismatch).  This enum's
// underlying ABI is int in the original i386 binary.
enum ENUM_ERROR : int {};

namespace pvp_assault {


// IDA_EA: 0x82E6662
// IDA_NAME: _ZN11pvp_assault10CAssaulter15GetUserUniqueIdEv
// DEMANGLED: pvp_assault::CAssaulter::GetUserUniqueId(void)
int CAssaulter::GetUserUniqueId() {
    if (user) {
        return user->get_unique_id();
    }
    return 0;
}

// IDA_EA: 0x82F0542
// IDA_NAME: _ZN11pvp_assault10CAssaulter7GetUserEv
// DEMANGLED: pvp_assault::CAssaulter::GetUser(void)
CUser* CAssaulter::GetUser() {
    return user;
}

// IDA_EA: 0x82F054C
// IDA_NAME: _ZN11pvp_assault10CAssaulter12IsAttachAbleEv
// DEMANGLED: pvp_assault::CAssaulter::IsAttachAble(void)
bool CAssaulter::IsAttachAble() {
    return user == 0;
}

// IDA_EA: 0x82F055C
// IDA_NAME: _ZN11pvp_assault10CAssaulter5EmptyEv
// DEMANGLED: pvp_assault::CAssaulter::Empty(void)
bool CAssaulter::Empty() {
    return IsAttachAble();
}

// IDA_EA: 0x82F0570
// IDA_NAME: _ZN11pvp_assault10CAssaulter7GetTeamEv
// DEMANGLED: pvp_assault::CAssaulter::GetTeam(void)
int CAssaulter::GetTeam() {
    return team;
}

// IDA_EA: 0x82F058A
// IDA_NAME: _ZN11pvp_assault10CAssaulter8GetStateEv
// DEMANGLED: pvp_assault::CAssaulter::GetState(void)
int CAssaulter::GetState() {
    return state;
}

// IDA_EA: 0x82E7232
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_GetTeamCountEPi
// DEMANGLED: pvp_assault::CAssaultPlace::_GetTeamCount(int *)
bool CAssaultPlace::_GetTeamCount(int* team_count) {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        CAssaulter* assaulter = &assaulters[index];
        if (!assaulter->Empty()) {
            const int team = assaulter->GetTeam();
            ++team_count[team];
        }
    }
    return result;
}

// IDA_EA: 0x82E9142
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14_CheckPlayableEv
// DEMANGLED: pvp_assault::CAssaultPlace::_CheckPlayable(void)
int CAssaultPlace::_CheckPlayable() {
    int team_count[2] = {0, 0};
    _GetTeamCount(team_count);
    for (int index = 0; index <= 1; ++index) {
        if (!team_count[index]) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x82E849A
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_GetUserIndexEP5CUserb
// DEMANGLED: pvp_assault::CAssaultPlace::_GetUserIndex(CUser *,bool)
int CAssaultPlace::_GetUserIndex(CUser* user, bool throw_on_missing) {
    for (int index = 0; index <= 7; ++index) {
        CAssaulter* assaulter = &assaulters[index];
        if (!assaulter->Empty() && assaulter->GetUser() == user) {
            return index;
        }
    }

    if (throw_on_missing) {
        throw 21;
    }
    return -1;
}

// IDA_EA: 0x82E8366
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace9ExistUserEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::ExistUser(CUser *)
int CAssaultPlace::ExistUser(CUser* user) {
    for (int index = 0; index <= 7; ++index) {
        CAssaulter* assaulter = &assaulters[index];
        if (!assaulter->Empty() && assaulter->GetUser() == user) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x82E89C0
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace28MakeViewAssaultIconOnePlayerEP11PacketGuardb
// DEMANGLED: pvp_assault::CAssaultPlace::MakeViewAssaultIconOnePlayer(PacketGuard *,bool)
int CAssaultPlace::MakeViewAssaultIconOnePlayer(PacketGuard* packet,
                                                bool visible) {
    packet->put_int(assault_place_id);
    int result = packet->put_byte(visible);
    if (visible) {
        packet->put_short(pos_x);
        packet->put_short(pos_y);
        int visible_count = 0;
        int count_index = packet->get_index();
        packet->put_short(0);
        for (int index = 0; index <= 7; ++index) {
            CAssaulter* assaulter = &assaulters[index];
            if (!assaulter->Empty()) {
                packet->put_short(assaulter->GetUserUniqueId());
                packet->put_byte(GetDirectionByTeam(
                    static_cast<ENUM_ASSAULT_TEAM>(assaulter->GetTeam())));
                ++visible_count;
            }
        }
        result = packet->put_short(count_index, visible_count);
    }
    return result;
}

// IDA_EA: 0x82EB450
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17LeaveAssaultPlaceEP5CUserb
// DEMANGLED: pvp_assault::CAssaultPlace::LeaveAssaultPlace(CUser *,bool)
int CAssaultPlace::LeaveAssaultPlace(CUser* user, bool force) {
    const int index = _GetUserIndex(user, true);
    return LeaveAssaultPlace(index, force, false);
}

// IDA_EA: 0x82EB49C
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17EvadeAssaultPlaceEP5CUser
// DEMANGLED: pvp_assault::CAssaultPlace::EvadeAssaultPlace(CUser *)
int CAssaultPlace::EvadeAssaultPlace(CUser* user) {
    if ((state - 2u > 1u) &&
        GlobalData::system_time()->getCurSec() -
                static_cast<int>(assault_start_second) >
            5) {
        throw static_cast<ENUM_ERROR>(19);
    }
    if (request_user == user) {
        throw static_cast<ENUM_ERROR>(19);
    }

    const int user_index = _GetUserIndex(user, true);
    if (assaulters[user_index].GetTeam() !=
        static_cast<int>(winner_team)) {
        throw static_cast<ENUM_ERROR>(19);
    }
    return LeaveAssaultPlace(_GetUserIndex(user, true), true, true);
}

// IDA_EA: 0x82F06AE
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace6IsAreaEii
// DEMANGLED: pvp_assault::CAssaultPlace::IsArea(int,int)
int CAssaultPlace::IsArea(int village_id, int area_id) {
    return village == static_cast<uint32_t>(village_id) &&
           area == static_cast<uint32_t>(area_id);
}

// IDA_EA: 0x82F06EA
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace13CheckPlayableEv
// DEMANGLED: pvp_assault::CAssaultPlace::CheckPlayable(void)
int CAssaultPlace::CheckPlayable() {
    return _CheckPlayable();
}

// IDA_EA: 0x82F06FE
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace8GetStateEv
// DEMANGLED: pvp_assault::CAssaultPlace::GetState(void)
int CAssaultPlace::GetState() {
    return state;
}

// IDA_EA: 0x82F0680
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17GetAssaultPlaceIdEv
// DEMANGLED: pvp_assault::CAssaultPlace::GetAssaultPlaceId(void)
int CAssaultPlace::GetAssaultPlaceId() {
    return assault_place_id;
}

// IDA_EA: 0x82F068E
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17SetAssaultPlaceIdEj
// DEMANGLED: pvp_assault::CAssaultPlace::SetAssaultPlaceId(unsigned int)
CAssaultPlace* CAssaultPlace::SetAssaultPlaceId(unsigned int id) {
    assault_place_id = id;
    return this;
}

// IDA_EA: 0x82F06A0
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace14GetAuthenValueEv
// DEMANGLED: pvp_assault::CAssaultPlace::GetAuthenValue(void)
int CAssaultPlace::GetAuthenValue() {
    return authen_value;
}

// IDA_EA: 0x82F06DC
// IDA_NAME: _ZN11pvp_assault13CAssaultPlace17GetAssaulterCountEv
// DEMANGLED: pvp_assault::CAssaultPlace::GetAssaulterCount(void)
int CAssaultPlace::GetAssaulterCount() {
    return assaulter_count;
}

// IDA_EA: 0x823506E
// IDA_NAME: _ZN11pvp_assault11CAssaultMgrD2Ev
// DEMANGLED: pvp_assault::CAssaultMgr::~CAssaultMgr()
CAssaultMgr::~CAssaultMgr() {}

// IDA_EA: 0x8235084
// IDA_NAME: _ZN11pvp_assault11CAssaultMgrC2Ev
// DEMANGLED: pvp_assault::CAssaultMgr::CAssaultMgr(void)
CAssaultMgr::CAssaultMgr() : assault_places() {
    assault_channel = 0;
}

// IDA_EA: 0x82ED5F4
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr19FrocedCloseAssaultsEv
// DEMANGLED: pvp_assault::CAssaultMgr::FrocedCloseAssaults(void)
int CAssaultMgr::FrocedCloseAssaults() {
    for (std::map<unsigned int, CAssaultPlace*>::iterator it =
             assault_places.begin();
         it != assault_places.end(); ++it) {
        CAssaultPlace* const place = it->second;
        if (place) {
            place->EndAssault();
        }
    }
    return 0;
}

// IDA_EA: 0x82ED966
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr5ResetEv
// DEMANGLED: pvp_assault::CAssaultMgr::Reset(void)
CAssaultMgr* CAssaultMgr::Reset() {
    next_assault_place_id = 0;
    return this;
}

// IDA_EA: 0x82350A0
// IDA_NAME: _ZN11pvp_assault21GetInstanceAssaultMgrEv
// DEMANGLED: pvp_assault::GetInstanceAssaultMgr(void)
CAssaultMgr* GetInstanceAssaultMgr() {
    static CAssaultMgr instance;
    return &instance;
}

// IDA_EA: 0x82EF1E2
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr14OnCompleteLoadEP5CUser
// DEMANGLED: pvp_assault::CAssaultMgr::OnCompleteLoad(CUser *)
int CAssaultMgr::OnCompleteLoad(CUser* user) {
    CAssaultPlace* const place = _GetAssaultPlace(
        static_cast<unsigned int>(user->GetAssaultPlace()));
    place->CompleteLoad(user);
    return 1;
}

// IDA_EA: 0x82EF464
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr13OnRevivalUserEP5CUser
// DEMANGLED: pvp_assault::CAssaultMgr::OnRevivalUser(CUser *)
int CAssaultMgr::OnRevivalUser(CUser* user) {
    int remaining_seconds = 0;
    CAssaultPlace::RevivalUser(user, remaining_seconds);
    return 1;
}

// IDA_EA: 0x82ED9FE
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr16_GetAssaultPlaceEj
// DEMANGLED: pvp_assault::CAssaultMgr::_GetAssaultPlace(unsigned int)
CAssaultPlace* CAssaultMgr::_GetAssaultPlace(unsigned int assault_place_id) {
    std::map<unsigned int, CAssaultPlace*>::iterator it =
        assault_places.find(assault_place_id);
    if (it == assault_places.end()) {
        throw static_cast<ENUM_ERROR>(21);
    }
    return it->second;
}

// IDA_EA: 0x82EFB54
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr10SendPacketEP5CUserbP11PacketGuard
// DEMANGLED: pvp_assault::CAssaultMgr::SendPacket(CUser *,bool,PacketGuard *)
int CAssaultMgr::SendPacket(CUser* user,
                            bool team_only,
                            PacketGuard* packet) {
    try {
        CAssaultPlace* const place = _GetAssaultPlace(
            static_cast<unsigned int>(user->GetAssaultPlace()));
        if (team_only) {
            ENUM_ASSAULT_TEAM team;
            if (place->GetTeam(user, team)) {
                place->SendPacket(*packet, team);
            }
        } else {
            place->SendPacket(*packet);
        }
    } catch (ENUM_ERROR) {
    }
    return 1;
}

// IDA_EA: 0x82EDA94
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr16_GetAssaultPlaceEji
// DEMANGLED: pvp_assault::CAssaultMgr::_GetAssaultPlace(unsigned int,int)
CAssaultPlace* CAssaultMgr::_GetAssaultPlace(unsigned int assault_place_id,
                                              int authen_value) {
    CAssaultPlace* const place = _GetAssaultPlace(assault_place_id);
    if (place->GetAuthenValue() != authen_value) {
        throw static_cast<ENUM_ERROR>(8);
    }
    return place;
}

// IDA_EA: 0x82EF320
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr24OnConnectP2PAfterAssaultEP5CUserPci
// DEMANGLED: pvp_assault::CAssaultMgr::OnConnectP2PAfterAssault(CUser *,char *,int)
int CAssaultMgr::OnConnectP2PAfterAssault(CUser* user,
                                          char* states,
                                          int count) {
    CParty* const party = user->GetParty();
    if (party) {
        party->ConnectP2P(user, states, count);
    }
    return 1;
}

// IDA_EA: 0x82EF176
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr19OnLeaveAssaultPlaceEP5CUserb
// DEMANGLED: pvp_assault::CAssaultMgr::OnLeaveAssaultPlace(CUser *,bool)
int CAssaultMgr::OnLeaveAssaultPlace(CUser* user, bool force) {
    try {
        CAssaultPlace* place = _GetAssaultPlace(
            static_cast<unsigned int>(user->GetAssaultPlace()));
        place->LeaveAssaultPlace(user, force);
    } catch (ENUM_ERROR) {
    }
    return 1;
}

// IDA_EA: 0x82EF610
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr10OnMoveAreaEP5CUser
// DEMANGLED: pvp_assault::CAssaultMgr::OnMoveArea(CUser *)
int CAssaultMgr::OnMoveArea(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 134);
    int visible_count = 0;
    int count_index = packet.get_index();
    packet.put_short(visible_count);

    for (std::map<unsigned int, CAssaultPlace*>::iterator it =
             assault_places.begin();
         it != assault_places.end();
         ++it) {
        CAssaultPlace* place = it->second;
        if (!place) {
            continue;
        }
        const int area_id = user->get_area(false);
        const int village_id = static_cast<unsigned char>(
            user->getCurCharacVill());
        if (static_cast<unsigned char>(place->IsArea(village_id, area_id)) &&
            static_cast<unsigned char>(place->ExistUser(user)) != 1u &&
            static_cast<unsigned char>(place->CheckPlayable()) &&
            place->GetState() == 4) {
            place->MakeViewAssaultIconOnePlayer(&packet, true);
            ++visible_count;
        }
    }

    if (visible_count) {
        packet.put_short(count_index, visible_count);
        packet.finalize(true);
        user->Send(packet);
    }
    return 1;
}

// IDA_EA: 0x8376048
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr17SetAssaultChannelEb
// DEMANGLED: pvp_assault::CAssaultMgr::SetAssaultChannel(bool)
CAssaultMgr* CAssaultMgr::SetAssaultChannel(bool enabled) {
    assault_channel = enabled ? 1u : 0u;
    return this;
}

// IDA_EA: 0x82EFBF6
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr19OnEvadeAssaultPlaceEP5CUser
// DEMANGLED: pvp_assault::CAssaultMgr::OnEvadeAssaultPlace(CUser *)
int CAssaultMgr::OnEvadeAssaultPlace(CUser* user) {
    CAssaultPlace* const place = _GetAssaultPlace(
        static_cast<unsigned int>(user->GetAssaultPlace()));
    place->EvadeAssaultPlace(user);
    return 1;
}

// IDA_EA: 0x82EFE14
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr25check_party_teleport_userEcP5CUserS2_b
// DEMANGLED: pvp_assault::CAssaultMgr::check_party_teleport_user(char,CUser *,CUser *,bool)
int CAssaultMgr::check_party_teleport_user(char request_type,
                                           CUser* requester,
                                           CUser* target,
                                           bool send_error) {
    int error = 0;

    CParty* requester_party = 0;
    if (requester->CheckInParty()) {
        requester_party = requester->GetParty();
    }
    if (requester_party) {
        const int state = requester_party->GetPartyTelePort()->get_teleport_state();
        if ((static_cast<unsigned int>(state) & 0x80u) == 0u &&
            static_cast<int8_t>(state) <= 3) {
            error = 156;
        }
    }

    CParty* target_party = 0;
    if (target->CheckInParty()) {
        target_party = target->GetParty();
    }
    if (target_party) {
        const int state = target_party->GetPartyTelePort()->get_teleport_state();
        if ((static_cast<unsigned int>(state) & 0x80u) == 0u &&
            static_cast<int8_t>(state) <= 3) {
            error = 157;
        }
    }

    if (send_error && error) {
        // CLIENT_IDA_EVIDENCE: 0627 0x82EFE14 calls this as
        // SendCmdPeerErrorPacket(requester, 10, request_type, error).
        // 0828 registers its matching response reader at CMD10 (0x57EA5C).
        requester->SendCmdPeerErrorPacket(
            ENUM_CMDPACKET_REQUEST_PEER,
            static_cast<ENUM_PEER_REQUEST_TYPE>(request_type),
            static_cast<unsigned char>(error));
    }
    return error;
}

}  // namespace pvp_assault
