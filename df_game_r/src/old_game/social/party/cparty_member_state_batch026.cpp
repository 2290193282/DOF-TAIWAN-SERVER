#include "old_game/social/party/party.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/system_time.h"

// The original P2P wait path throws this fixed-width named game error to its
// enclosing packet-dispatch boundary.
enum ENUM_ERROR : int;

// IDA_EA: 0x859A48C
// IDA_NAME: _ZN6CParty10ConnectP2PEP5CUserPci
// DEMANGLED: CParty::ConnectP2P(CUser *,char *,int)
char CParty::ConnectP2P(CUser* user, char* states, int count) {
    if (static_cast<unsigned char>(get_state()) != 1u) {
        throw static_cast<ENUM_ERROR>(19);
    }

    bool received_from_all_members = true;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u) {
            unsigned char& received = members_[seat].p2p_received;
            if (members_[seat].user == user) {
                if (received == 1u) {
                    throw static_cast<ENUM_ERROR>(18);
                }
                received = 1u;
                std::memcpy(members_[seat].p2p_connectivity,
                            states,
                            std::min(count, 4));
            }
            if (received != 1u) {
                received_from_all_members = false;
            }
        }
    }

    if (received_from_all_members) {
        return ClearBadP2PUser();
    }
    return 0;
}

// IDA_EA: 0x822D8AC
// IDA_NAME: _ZN6CParty23set_dungeon_clear_stateEc
// DEMANGLED: CParty::set_dungeon_clear_state(char)
CParty* CParty::set_dungeon_clear_state(char state) {
    if (state <= 6) {
        dungeon_clear_state_ = static_cast<uint8_t>(state);
        return this;
    }
    return reinterpret_cast<CParty*>(static_cast<intptr_t>(state));
}

// IDA_EA: 0x859A286
// IDA_NAME: _ZN6CParty22is_limit_minimum_levelEiRi
// DEMANGLED: CParty::is_limit_minimum_level(int,int &)
int CParty::is_limit_minimum_level(int minimum_level, int& unique_id) {
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat) &&
            members_[seat].user->get_charac_level() < minimum_level) {
            unique_id = static_cast<uint16_t>(members_[seat].user->get_unique_id());
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x859A610
// IDA_NAME: _ZN6CParty19_IsCannotConnectP2PEii
// DEMANGLED: CParty::_IsCannotConnectP2P(int,int)
bool CParty::_IsCannotConnectP2P(int seat, int peer_seat) {
    return !members_[seat].p2p_connectivity[peer_seat] ||
           _checkValidUser(peer_seat) == 1;
}

// IDA_EA: 0x859A664
// IDA_NAME: _ZN6CParty24_ClearConnectP2PAbsoluteEi
// DEMANGLED: CParty::_ClearConnectP2PAbsolute(int)
int CParty::_ClearConnectP2PAbsolute(int seat) {
    for (int peer = 0; peer < 4; ++peer) {
        if (peer != seat && _IsCannotConnectP2P(seat, peer))
            leave_user(members_[peer].user,
                       static_cast<ENUM_PARTY_INFO_TYPE>(2));
    }
    return 1;
}

// IDA_EA: 0x859A6DC
// IDA_NAME: _ZN6CParty24_ClearConnectP2PRelativeEi
// DEMANGLED: CParty::_ClearConnectP2PRelative(int)
int CParty::_ClearConnectP2PRelative(int connection_count) {
    for (int seat = 2; seat < 4; ++seat) {
        if (_GetConnectP2PPoint(seat) == connection_count)
            leave_user(members_[seat].user,
                       static_cast<ENUM_PARTY_INFO_TYPE>(2));
    }
    return 1;
}

// IDA_EA: 0x859A74C
// IDA_NAME: _ZN6CParty19_GetConnectP2PPointEi
// DEMANGLED: CParty::_GetConnectP2PPoint(int)
int CParty::_GetConnectP2PPoint(int seat) {
    int point = 0;
    for (int peer = 0; peer < 4; ++peer)
        if (seat != peer && !_IsCannotConnectP2P(seat, peer))
            ++point;
    return point;
}

// IDA_EA: 0x859A7A2
// IDA_NAME: _ZN6CParty25_GetConnectP2PMaxMinPointERiS0_
// DEMANGLED: CParty::_GetConnectP2PMaxMinPoint(int &,int &)
bool CParty::_GetConnectP2PMaxMinPoint(int& maximum, int& minimum) {
    maximum = 0;
    minimum = 4;
    int seat = 0;
    for (; seat < 4; ++seat) {
        if (_checkValidUser(seat) == 1) {
            const int point = _GetConnectP2PPoint(seat);
            minimum = std::min(minimum, point);
            maximum = std::max(maximum, point);
        }
    }
    return seat <= 3;
}

// IDA_EA: 0x859B1BE
// IDA_NAME: _ZN6CParty12create_partyEP5CUser
// DEMANGLED: CParty::create_party(CUser *)
int CParty::create_party(CUser* manager) {
    members_[0].user = manager;
    members_[0].user_uid = manager->GetUID();
    manager_ = manager;
    member_count_ = 1;
    state_ = 1;
    manager->SetPartyIndex(static_cast<short>(party_index_));
    manager->resetStraightVictories();
    party_teleport_.reset_teleport_data();
    return 1;
}

// IDA_EA: 0x859C978
// IDA_NAME: _ZN6CParty14change_managerEP5CUseri
// DEMANGLED: CParty::change_manager(CUser *,int)
int CParty::change_manager(CUser* requester, int seat) {
    if (static_cast<unsigned int>(seat) >= 4u || !members_[seat].user)
        return 21;
    if (manager_ != requester)
        return 8;
    if (members_[seat].user == manager_)
        return 18;
    manager_ = members_[seat].user;
    set_host();
    send_host_info();
    G_GameWorld()->send_party_info_to_all(this, 2);
    return 0;
}

// IDA_EA: 0x859D656
// IDA_NAME: _ZN6CParty15CheckMemberAreaEv
// DEMANGLED: CParty::CheckMemberArea(void)
int CParty::CheckMemberArea() {
    int area = -1;
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat) == 1) {
            const int member_area = members_[seat].user->get_area(false);
            if (area == -1)
                area = member_area;
            else if (member_area != area)
                return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x85A2140
// IDA_NAME: _ZN6CParty16FindUserByCharIdEj
// DEMANGLED: CParty::FindUserByCharId(unsigned int)
CUser* CParty::FindUserByCharId(unsigned int charac_no) {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1 &&
            static_cast<unsigned int>(members_[seat].user->get_charac_no(-1)) ==
                charac_no)
            return members_[seat].user;
    return 0;
}

// IDA_EA: 0x85AD084
// IDA_NAME: _ZN6CParty17CalcCreatureBonusEP5CUser
// DEMANGLED: CParty::CalcCreatureBonus(CUser *)
long double CParty::CalcCreatureBonus(CUser* user) {
    const CInventory* inventory = user->getCurCharacInvenR();
    const user_creature::CCreatureMgr* creatures = inventory->GetCreatureMgrR();
    return creatures->IsEquippedCreature() ? 0.050000001L : 0.0L;
}

// IDA_EA: 0x85B357C
// IDA_NAME: _ZN6CParty12GetCoinCountEP5CUser
// DEMANGLED: CParty::GetCoinCount(CUser *)
int CParty::GetCoinCount(CUser* user) {
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat) == 1 && members_[seat].user == user) {
            return 0x7fffffff -
                   battle_data_.member_coin_limit_remaining_[seat];
        }
    }
    return 0;
}

// IDA_EA: 0x85B3F34
// IDA_NAME: _ZN6CParty26set_recv_party_packet_flagEP5CUser
// DEMANGLED: CParty::set_recv_party_packet_flag(CUser *)
int CParty::set_recv_party_packet_flag(CUser* user) {
    const int seat = GetMemberSlotNo(user);
    return seat >= 0 ? result_recv_flag_.SetRecvFlag(seat) : seat;
}

// IDA_EA: 0x85B3F72
// IDA_NAME: _ZN6CParty21recv_party_packet_allEv
// DEMANGLED: CParty::recv_party_packet_all(void)
int CParty::recv_party_packet_all() {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1 &&
            result_recv_flag_.IsRecv(seat) != 1)
            return 0;
    return 1;
}

// IDA_EA: 0x85B3FD8
// IDA_NAME: _ZN6CParty22CheckRecvFromAllMemberEP5CUser
// DEMANGLED: CParty::CheckRecvFromAllMember(CUser *)
int CParty::CheckRecvFromAllMember(CUser* user) {
    set_recv_party_packet_flag(user);
    if (recv_party_packet_all() != 1)
        return 0;
    reset_recv_flag();
    return 1;
}

// IDA_EA: 0x85B49D0
// IDA_NAME: _ZN6CParty12isGuildPartyEv
// DEMANGLED: CParty::isGuildParty(void)
int CParty::isGuildParty() {
    uint8_t guild_party = 0;
    int previous_guild = -1;
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat) == 1) {
            const int guild = members_[seat].user->get_charac_guildkey();
            if (!guild)
                return 0;
            if (guild == previous_guild)
                guild_party = 1;
            else {
                if (previous_guild != -1)
                    return 0;
                guild_party = 1;
            }
            previous_guild = guild;
        }
    }
    return guild_party;
}

// IDA_EA: 0x85B4A70
// IDA_NAME: _ZN6CParty15hasPCRoomPlayerEv
// DEMANGLED: CParty::hasPCRoomPlayer(void)
int CParty::hasPCRoomPlayer() {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1 &&
            members_[seat].user->isAffectedPremium(
                static_cast<ENUM_PREMIUM_TYPE>(12)))
            return 1;
    return 0;
}

// IDA_EA: 0x85B603E
// IDA_NAME: _ZN6CParty17getMemberMinLevelEv
// DEMANGLED: CParty::getMemberMinLevel(void)
int CParty::getMemberMinLevel() {
    int level = 0x7fffffff;
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1)
            level = std::min(level, members_[seat].user->get_charac_level());
    return level;
}

// IDA_EA: 0x85B60BE
// IDA_NAME: _ZN6CParty17getMemberMaxLevelEv
// DEMANGLED: CParty::getMemberMaxLevel(void)
int CParty::getMemberMaxLevel() {
    int level = 0;
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1)
            level = std::max(level, members_[seat].user->get_charac_level());
    return level;
}

// IDA_EA: 0x85B613E
// IDA_NAME: _ZN6CParty30getMemberMaxLevelBeforeDungeonEv
// DEMANGLED: CParty::getMemberMaxLevelBeforeDungeon(void)
int CParty::getMemberMaxLevelBeforeDungeon() {
    int level = 0;
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1)
            level = std::max(
                level,
                members_[seat].user->get_charac_level_before_dungeon());
    return level;
}

// IDA_EA: 0x85B66FA
// IDA_NAME: _ZN6CParty10isMyMemberEj
// DEMANGLED: CParty::isMyMember(unsigned int)
int CParty::isMyMember(unsigned int unique_id) {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) &&
            static_cast<uint16_t>(members_[seat].user->get_unique_id()) ==
                unique_id)
            return 1;
    return 0;
}

// IDA_EA: 0x85B67CA
// IDA_NAME: _ZN6CParty19existSamePCRoomUserEj
// DEMANGLED: CParty::existSamePCRoomUser(unsigned int)
int CParty::existSamePCRoomUser(unsigned int pc_room_no) {
    int count = 0;
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat)) {
            const int member_pc_room = members_[seat].user->GetPCRoomNo();
            if (static_cast<unsigned int>(member_pc_room) == pc_room_no &&
                member_pc_room && ++count > 1)
                return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85B687C
// IDA_NAME: _ZN6CParty17isSamePCRoomPartyEv
// DEMANGLED: CParty::isSamePCRoomParty(void)
int CParty::isSamePCRoomParty() {
    int pc_room_no = 0;
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat)) {
            const int member_pc_room = members_[seat].user->GetPCRoomNo();
            if (!member_pc_room)
                return 0;
            if (pc_room_no && pc_room_no != member_pc_room)
                return 0;
            if (!pc_room_no)
                pc_room_no = member_pc_room;
        }
    }
    return 1;
}

// IDA_EA: 0x85B8F92
// IDA_NAME: _ZN6CParty18finishBloodDungeonEv
// DEMANGLED: CParty::finishBloodDungeon(void)
void CParty::finishBloodDungeon() {
    ReturnToVillage();
}

// IDA_EA: 0x85B8FA6
// IDA_NAME: _ZN6CParty19IsThereGhostInPartyEv
// DEMANGLED: CParty::IsThereGhostInParty(void)
int CParty::IsThereGhostInParty() {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) && members_[seat].user->IsCurCharacGhost())
            return 1;
    return 0;
}

// IDA_EA: 0x85B900C
// IDA_NAME: _ZN6CParty18getMemberPassPartyEP5CUser
// DEMANGLED: CParty::getMemberPassParty(CUser *)
bool CParty::getMemberPassParty(CUser* user) {
    int maximum_level = 0;
    for (int seat = 0; seat < 4; ++seat) {
        if (_checkValidUser(seat) == 1) {
            CUser* member = members_[seat].user;
            if (user->get_connect_upper_member() != member->get_charac_no(-1) &&
                user->get_charac_no(-1) != member->get_charac_no(-1))
                maximum_level =
                    std::max(maximum_level, member->get_charac_level());
        }
    }
    if (maximum_level &&
        maximum_level - user->get_charac_level_before_dungeon() <= 6)
        return false;
    return maximum_level != 0;
}

// IDA_EA: 0x85BA150
// IDA_NAME: _ZN6CParty21GetMemberRoutingStateEP5CUser
// DEMANGLED: CParty::GetMemberRoutingState(CUser *)
int CParty::GetMemberRoutingState(CUser* user) {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1 && members_[seat].user == user)
            return routing_data_.GetMemberRoutingState(seat);
    return -1;
}

// IDA_EA: 0x85BA294
// IDA_NAME: _ZN6CParty24GetRoutingDiceMemberListERSt6vectorISt4pairIiiESaIS2_EEP5CItem
// DEMANGLED: CParty::GetRoutingDiceMemberList(std::vector<std::pair<int,int>> &,CItem *)
char CParty::GetRoutingDiceMemberList(
    std::vector<std::pair<int, int> >& members,
    CItem* item) {
    if (item->IsRoutingItem()) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) != 1 ||
                routing_data_.GetMemberRoutingState(seat) != 1) {
                continue;
            }

            CUser* const user = members_[seat].user;
            if (item->IsPriorityJob(user->get_charac_job(),
                                    user->getCurCharacGrowType()) == 1) {
                members.push_back(std::make_pair(seat,
                                                 user->getCurCharacNo()));
            }
        }
    }

    if (members.empty()) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) != 1 ||
                routing_data_.GetMemberRoutingState(seat) != 1) {
                continue;
            }

            CUser* const user = members_[seat].user;
            if (item->check_job_type(
                    static_cast<char>(user->get_charac_job())) == 1) {
                members.push_back(std::make_pair(seat,
                                                 user->getCurCharacNo()));
            }
        }
    }

    if (members.empty()) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) == 1 &&
                routing_data_.GetMemberRoutingState(seat) == 1) {
                members.push_back(std::make_pair(
                    seat, members_[seat].user->getCurCharacNo()));
            }
        }
    }
    return members.empty() ? 1 : 0;
}

// IDA_EA: 0x85BA1C0
// IDA_NAME: _ZN6CParty21HasAllMemberSelectionEv
// DEMANGLED: CParty::HasAllMemberSelection(void)
int CParty::HasAllMemberSelection() {
    for (int seat = 0; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1 &&
            !routing_data_.GetMemberRoutingState(seat))
            return 0;
    return 1;
}

// IDA_EA: 0x85BA222
// IDA_NAME: _ZN6CParty19SetRoutingTimerOverEv
// DEMANGLED: CParty::SetRoutingTimerOver(void)
bool CParty::SetRoutingTimerOver() {
    int seat = 0;
    for (; seat < 4; ++seat)
        if (_checkValidUser(seat) == 1 &&
            !routing_data_.GetMemberRoutingState(seat))
            routing_data_.SetMemberRoutingState(seat, 2);
    return seat <= 3;
}

// IDA_EA: 0x85BA680
// IDA_NAME: _ZN6CParty13switchPostionEcc
// DEMANGLED: CParty::switchPostion(char,char)
int CParty::switchPostion(char first, char second) {
    const int first_index = first;
    const int second_index = second;
    const cMember temporary = members_[first_index];
    members_[first_index] = members_[second_index];
    members_[second_index] = temporary;
    return 1;
}

// IDA_EA: 0x85BCAB6
// IDA_NAME: _ZN6CParty29CheckMemberLastTryAssaultTimeEv
// DEMANGLED: CParty::CheckMemberLastTryAssaultTime(void)
int CParty::CheckMemberLastTryAssaultTime() {
    for (int seat = 0; seat < 4; ++seat) {
        if (checkValidUser(seat) == 1) {
            const int now = GlobalData::s_systemTime_.getCurSec();
            if (members_[seat].user->GetCurCharacChaosKillTime() + 600 > now)
                return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85BD6BC
// IDA_NAME: _ZN6CParty30SetTournamentDungeonClearStateEc
// DEMANGLED: CParty::SetTournamentDungeonClearState(char)
CParty* CParty::SetTournamentDungeonClearState(char state) {
    if (state <= 3) {
        tournament_dungeon_clear_state_ = static_cast<uint8_t>(state);
        return this;
    }
    return reinterpret_cast<CParty*>(static_cast<intptr_t>(state));
}

// IDA_EA: 0x85BF6E0
// IDA_NAME: _ZN6CParty15reset_recv_flagEv
// DEMANGLED: CParty::reset_recv_flag(void)
void* CParty::reset_recv_flag() {
    return result_recv_flag_.Clear();
}
