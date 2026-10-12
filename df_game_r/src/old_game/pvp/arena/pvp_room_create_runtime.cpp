#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/arena/zombie_owner_snapshot.h"

#include <cstring>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/pvp/match/pvp_matching_system_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x855C8F4
// IDA_NAME: _Z15GetMatchingTypev
// DEMANGLED: GetMatchingType(void)
int GetMatchingType() {
    switch (G_GameWorld()->GetChannelType()) {
    case 10: return ENUM_PVP_MATCHING_TOURNAMENT;
    case 9: return ENUM_PVP_MATCHING_LEAGUE;
    case 14: return ENUM_PVP_MATCHING_ONLINE_PRELIMINARY;
    case 15: return ENUM_PVP_MATCHING_FAIR_PVP;
    default: return ENUM_PVP_MATCHING_NORMAL;
    }
}

namespace {

typedef void (*RequestMapFn)(IMatch*,
                             int16_t*,
                             PVP_BATTLE_MODE*,
                             const char*,
                             int,
                             int);

}  // namespace

// IDA_EA: 0x85D6908
// IDA_NAME: _ZN8PvP_Room11create_roomEP5CUsercPcsbPKci15PVP_BATTLE_MODE
// DEMANGLED: PvP_Room::create_room(CUser *,char,char *,short,bool,char const*,int,PVP_BATTLE_MODE)
int PvP_Room::create_room(CUser* user,
                          char room_title_mode,
                          char* room_title,
                          int16_t requested_map_no,
                          bool password_enabled,
                          const char* password,
                          int password_length,
                          PVP_BATTLE_MODE battle_mode) {
    const int room_index = index_;
    const char* const charac_name = user->getCurCharacName();

    if (!user->getCurCharacR()) return 19;

    current_match_ = matching_system_->GetMatchingSystem(
        static_cast<ENUM_PVP_MATCHING_TYPE>(GetMatchingType()));
    void** const vtable = *reinterpret_cast<void***>(current_match_);
    reinterpret_cast<RequestMapFn>(vtable[6])(
        current_match_, &requested_map_no_, &battle_mode_, charac_name,
        room_index, static_cast<int>(battle_mode));

    if (battle_state_) return 19;

    battle_state_ = 1;
    manager_ = user;
    manager_seat_ = 0;
    users_[0] = user;
    teams_[0] = 1;
    relay_battle_mgr_.OnChangeTeam(0, 1);
    room_title_mode_ = static_cast<uint8_t>(room_title_mode);
    if (!room_title_mode_) {
        std::strncpy(room_title_, room_title, std::strlen(room_title));
    }
    user->SetPvpIndex(static_cast<int16_t>(index_));
    member_count_ = 1;
    requested_map_no_ = requested_map_no;
    selected_map_no_ = requested_map_no_;
    password_exists_ = password_enabled;
    if (password_exists_) {
        password_length_ = password_length;
        std::memset(password_, 0, sizeof(password_));
        std::memcpy(password_, password, password_length);
    }
    relay_battle_mgr_.OnCreateRoom(0);
    death_match_battle_mgr_.OnCreateRoom(0);
    battle_mode_ = battle_mode;
    return current_match_->OnCreateRoom(user);
}

// IDA_EA: 0x85D6CF2
// IDA_NAME: _ZN8PvP_Room9join_roomEP5CUserRib
// DEMANGLED: PvP_Room::join_room(CUser *,int &,bool)
int PvP_Room::join_room(CUser* user, int& seat, bool same_team) {
    const int waiter_count_before_join = get_waiter_count();
    const int logged_battle_mode = static_cast<int>(battle_mode_);
    const int room_index = index_;
    const char* const character_name = user->getCurCharacName();
    const char* const account_name = user->get_acc_name();
    CSwitchLog trace("int PvP_Room::join_room(CUser*, int&, bool)",
                     1192, 0, 0);
    trace("pvp@log %s,%s,room(%d),%d,%d,%d",
          account_name,
          character_name,
          room_index,
          logged_battle_mode,
          static_cast<int>(same_team),
          waiter_count_before_join);

    if (G_GameWorld()->GetChannelType() == 15 ||
        G_GameWorld()->GetChannelType() == 16) {
        if (G_CDataManager()->GetMaxGradePvPChannel() <
            user->get_pvp_grade()) {
            return 13;
        }

        fair_pvp::CFairPvPScore* const fair_score =
            static_cast<fair_pvp::CFairPvPScore*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
        if (fair_score->GetGiveItem() <=
            G_CDataManager()
                ->get_server_parameter_script()
                ->get_pvp_item_give(user->get_pvp_grade())) {
            return 7;
        }
    }

    if (!user->getCurCharacR()) {
        return 19;
    }
    if (battle_state_ == 2) {
        return 19;
    }

    const int waiter_count = get_waiter_count();
    PacketGuard basic_info_packet;
    PacketGuard peer_info_packet;

    basic_info_packet.put_header(0, 2);
    peer_info_packet.put_header(0, 11);
    basic_info_packet.put_byte(1);
    basic_info_packet.put_short(waiter_count);
    peer_info_packet.put_byte(waiter_count);

    for (int index = 0; index <= 7; ++index) {
        CUser* const occupant = users_[index];
        if (!occupant) {
            continue;
        }

        occupant->make_basic_info(basic_info_packet, 1);
        CNetwork4096_450000& network = occupant->network_;
        peer_info_packet.put_short(
            static_cast<uint16_t>(occupant->get_unique_id()));
        peer_info_packet.put_int(
            static_cast<int>(network.get_inner_ip()));
        peer_info_packet.put_int(static_cast<int>(network.get_ip()));
        peer_info_packet.put_short(
            static_cast<uint16_t>(network.get_port()));
        peer_info_packet.put_int(occupant->get_acc_id());
        peer_info_packet.put_byte(network.get_nat_type());
        peer_info_packet.put_int(static_cast<int>(network.get_mtu()));
        if (occupant->getCurCharacR()) {
            peer_info_packet.put_byte(occupant->getCurCharacStamina());
        } else {
            peer_info_packet.put_byte(0);
        }
    }

    unlock();
    basic_info_packet.finalize(true);
    peer_info_packet.finalize(true);
    lock();

    const int joinable_result = current_match_->CheckJoinable(this, user);
    if (joinable_result) {
        unlock();
        return joinable_result;
    }

    int joined_seat = -1;
    for (int index = 0; index <= 7; ++index) {
        if (!users_[index] && teams_[index] == 255) {
            joined_seat = index;
            users_[index] = user;
            user->SetPvpIndex(static_cast<int16_t>(index_));
            seat = index;
            break;
        }
    }

    if (joined_seat == -1) {
        unlock();
        return 4;
    }

    user->Send(basic_info_packet);
    user->Send(peer_info_packet);

    if (battle_mode_ == static_cast<PVP_BATTLE_MODE>(1)) {
        teams_[joined_seat] = 0;
        relay_battle_mgr_.OnChangeTeam(joined_seat, 0);
    } else if (battle_mode_ == static_cast<PVP_BATTLE_MODE>(4)) {
        teams_[joined_seat] = 0;
    } else if (same_team) {
        teams_[joined_seat] = 1;
        relay_battle_mgr_.OnChangeTeam(joined_seat, 1);
    } else {
        int team_counts[3] = {0, 0, 0};
        for (int index = 0; index <= 7; ++index) {
            const int team = teams_[index];
            if (team > 0 && team <= 2) {
                ++team_counts[team];
            }
        }

        int minimum_count = 8;
        int selected_team = 1;
        for (int team = 1; team <= 2; ++team) {
            if (team_counts[team] <= minimum_count) {
                selected_team = team;
                minimum_count = team_counts[team];
            }
        }
        teams_[joined_seat] = selected_team;
        relay_battle_mgr_.OnChangeTeam(joined_seat, selected_team);
    }
    unlock();

    basic_info_packet.clear();
    basic_info_packet.put_header(0, 2);
    basic_info_packet.put_byte(1);
    basic_info_packet.put_short(1);
    user->make_basic_info(basic_info_packet, 1);
    basic_info_packet.finalize(true);
    send_to_pvp(basic_info_packet);

    peer_info_packet.clear();
    peer_info_packet.put_header(0, 11);
    peer_info_packet.put_byte(1);
    CNetwork4096_450000& network = user->network_;
    peer_info_packet.put_short(static_cast<uint16_t>(user->get_unique_id()));
    peer_info_packet.put_int(static_cast<int>(network.get_inner_ip()));
    peer_info_packet.put_int(static_cast<int>(network.get_ip()));
    peer_info_packet.put_short(static_cast<uint16_t>(network.get_port()));
    peer_info_packet.put_int(user->get_acc_id());
    peer_info_packet.put_byte(network.get_nat_type());
    peer_info_packet.put_int(static_cast<int>(network.get_mtu()));
    if (user->getCurCharacR()) {
        peer_info_packet.put_byte(user->getCurCharacStamina());
    } else {
        peer_info_packet.put_byte(0);
    }
    peer_info_packet.finalize(true);
    send_to_pvp(peer_info_packet);

    basic_info_packet.clear();
    basic_info_packet.put_header(1, ENUM_CMDPACKET_ENTER_PVP_ROOM);
    basic_info_packet.put_byte(1);
    lock();
    for (int index = 0; index <= 7; ++index) {
        basic_info_packet.put_byte(ready_flags_[index]);
    }
    unlock();
    basic_info_packet.finalize(true);
    user->Send(basic_info_packet);

    user->pending_pvp_win_point_ = 0;
    ++member_count_;
    relay_battle_mgr_.OnJoinRoom(seat);
    current_match_->OnJoinRoom(user);
    return 0;
}

namespace {

const size_t kZombieOwnerMinimumMembers = 5u;
const size_t kZombieOwnerMaximumMembers = 6u;
const size_t kPvpRoomSeatCount = 8u;

// Validate a compact logical combat roster without assuming that logical
// seat N still occupies physical PvP_Room seat N.  The latter stops being
// true when an arbitrary loading-time dropout is removed and the remaining
// five combat seats are compressed.
bool ValidateZombieOwnerRoster(PvP_Room* room,
                               CUser* recipient,
                               CUser* const* members,
                               size_t member_count,
                               size_t* recipient_slot) {
    if (!room || !recipient || !members || !recipient_slot ||
        member_count < kZombieOwnerMinimumMembers ||
        member_count > kZombieOwnerMaximumMembers ||
        room->get_player_count() != static_cast<int>(member_count)) {
        return false;
    }

    bool physical_seat_seen[kPvpRoomSeatCount] = {
        false, false, false, false, false, false, false, false
    };
    *recipient_slot = kPvpRoomSeatCount;
    for (size_t logical_slot = 0u;
         logical_slot < member_count;
         ++logical_slot) {
        CUser* const member = members[logical_slot];
        if (!member) return false;
        const int physical_seat = room->get_user_seat(member);
        if (physical_seat < 0 ||
            static_cast<size_t>(physical_seat) >= kPvpRoomSeatCount ||
            physical_seat_seen[physical_seat] ||
            room->get_user_seat(physical_seat) != member) {
            return false;
        }
        physical_seat_seen[physical_seat] = true;
        if (member == recipient) *recipient_slot = logical_slot;
    }
    return *recipient_slot < member_count;
}

}  // namespace

bool SendZombieOwnerFullActorSnapshot(PvP_Room* room,
                                      CUser* recipient,
                                      CUser* const* members,
                                      size_t member_count) {
    size_t recipient_slot = 0u;
    if (!ValidateZombieOwnerRoster(
            room, recipient, members, member_count, &recipient_slot)) {
        return false;
    }
    (void)recipient_slot;

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(1);
    packet.put_short(static_cast<short>(member_count));
    for (size_t slot = 0u; slot < member_count; ++slot)
        members[slot]->make_basic_info(packet, 1);
    packet.finalize(true);
    return recipient->Send(packet) != 0;
}

bool SendZombieOwnerCombatSeatMapping(PvP_Room* room,
                                      CUser* recipient,
                                      CUser* const* members,
                                      size_t member_count,
                                      int recipient_party_index) {
    size_t recipient_slot = 0u;
    if (recipient_party_index < 0 ||
        !ValidateZombieOwnerRoster(
            room, recipient, members, member_count, &recipient_slot)) {
        return false;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_PARTY_INFO);
    packet.put_short(1);
    packet.put_short(static_cast<short>(recipient_party_index));
    packet.put_byte(2);
    for (size_t slot = 0u; slot < kPvpRoomSeatCount; ++slot) {
        if (slot < member_count) {
            packet.put_short(static_cast<uint16_t>(
                members[slot]->get_unique_id()));
            const int sex = members[slot]->getSex();
            packet.put_byte(static_cast<uint8_t>(sex < 0 ? 1 : sex));
        } else {
            packet.put_short(static_cast<uint16_t>(0xffffu));
            packet.put_byte(static_cast<uint8_t>(0xffu));
        }
    }
    // return/event/manager-seat/quick/event/tag-count
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(static_cast<uint8_t>(recipient_slot));
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.finalize(true);
    return recipient->Send(packet) != 0;
}

bool PvP_Room::send_zombie_owner_full_peer_snapshot(
    CUser* recipient,
    CUser* const* members,
    size_t member_count) {
    size_t recipient_slot = 0u;
    if (!ValidateZombieOwnerRoster(
            this, recipient, members, member_count, &recipient_slot)) {
        return false;
    }
    (void)recipient_slot;

    PacketGuard packet;
    packet.put_header(0, 11);
    packet.put_byte(static_cast<uint8_t>(member_count));
    for (size_t slot = 0u; slot < member_count; ++slot) {
        CUser* const member = members[slot];
        CNetwork4096_450000& network = member->network_;
        packet.put_short(static_cast<uint16_t>(member->get_unique_id()));
        packet.put_int(static_cast<int>(network.get_inner_ip()));
        packet.put_int(static_cast<int>(network.get_ip()));
        packet.put_short(static_cast<uint16_t>(network.get_port()));
        packet.put_int(member->get_acc_id());
        packet.put_byte(network.get_nat_type());
        packet.put_int(static_cast<int>(network.get_mtu()));
        packet.put_byte(member->getCurCharacR()
                            ? member->getCurCharacStamina()
                            : 0);
    }
    packet.finalize(true);
    return recipient->Send(packet) != 0;
}

bool SendZombieOwnerFullPeerSnapshot(PvP_Room* room,
                                     CUser* recipient,
                                     CUser* const* members,
                                     size_t member_count) {
    return room && room->send_zombie_owner_full_peer_snapshot(
                       recipient, members, member_count);
}

bool SendZombieOwnerSnapshot(PvP_Room* room,
                             CUser* recipient,
                             CUser* const* members,
                             size_t member_count,
                             int recipient_party_index) {
    return SendZombieOwnerFullActorSnapshot(
               room, recipient, members, member_count) &&
           SendZombieOwnerCombatSeatMapping(
               room,
               recipient,
               members,
               member_count,
               recipient_party_index);
}
