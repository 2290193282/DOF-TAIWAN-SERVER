#include "old_game/pvp/match/fair_pvp_match_runtime.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/season/pvp_rating_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/pvp/arena/fair_pvp_waiting_room_runtime.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/pvp/match/timer_start_matching.h"

namespace fair_pvp {

CMultiMatchRoomList* CFairMatch::pWaitingRoomList_ = 0;
int CFairMatch::nWaitingRoomListRefCount_ = 0;
int CFairMatch::nextAuthValue_ = 0;
bool CFairMatch::bMatchable_ = true;

// IDA_EA: 0x8562834
// IDA_NAME: _ZN8fair_pvp10CFairMatch22GetPlayCountPerOneTeamEv
// DEMANGLED: fair_pvp::CFairMatch::GetPlayCountPerOneTeam(void)
int CFairMatch::GetPlayCountPerOneTeam() {
    return G_CDataManager()->GetPvPTeamCount();
}

// IDA_EA: 0x856284A
// IDA_NAME: _ZN8fair_pvp10CFairMatchC2Ev
// DEMANGLED: fair_pvp::CFairMatch::CFairMatch(void)
CFairMatch::CFairMatch() {
    if (!pWaitingRoomList_) {
        pWaitingRoomList_ = new CMultiMatchRoomList();
    }
    ++nWaitingRoomListRefCount_;
    next_match_ = 0;
}

// IDA_EA: 0x856292E
// IDA_NAME: _ZN8fair_pvp10CFairMatchD2Ev
// DEMANGLED: fair_pvp::CFairMatch::~CFairMatch()
// IDA_EA: 0x85629CA
// IDA_NAME: _ZN8fair_pvp10CFairMatchD0Ev
// DEMANGLED: fair_pvp::CFairMatch::~CFairMatch()
CFairMatch::~CFairMatch() {
    --nWaitingRoomListRefCount_;
    if (pWaitingRoomList_ && !nWaitingRoomListRefCount_) {
        // The old deleting path calls operator delete directly and leaves the
        // static pointer value unchanged.
        ::operator delete(pWaitingRoomList_);
    }
    next_match_ = 0;
}

// IDA_EA: 0x85629E8
// IDA_NAME: _ZN8fair_pvp10CFairMatch14_CheckSameTeamEP8PvP_RoomP5CUser
// DEMANGLED: fair_pvp::CFairMatch::_CheckSameTeam(PvP_Room *,CUser *)
int CFairMatch::_CheckSameTeam(PvP_Room*, CUser*) {
    return 1;
}

// IDA_EA: 0x85629F2
// IDA_NAME: _ZN8fair_pvp10CFairMatch14_CheckObserverEP8PvP_RoomP5CUser
// DEMANGLED: fair_pvp::CFairMatch::_CheckObserver(PvP_Room *,CUser *)
bool CFairMatch::_CheckObserver(PvP_Room* room, CUser*) {
    int observer_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            ++observer_count;
        }
    }
    return teams_[0].size() - observer_count <
           static_cast<uint8_t>(GetPlayCountPerOneTeam());
}

// IDA_EA: 0x8562A6E
// IDA_NAME: _ZN8fair_pvp10CFairMatch13CheckJoinableEP8PvP_RoomP5CUser
// DEMANGLED: fair_pvp::CFairMatch::CheckJoinable(PvP_Room *,CUser *)
int CFairMatch::CheckJoinable(PvP_Room* room, CUser* user) {
    if (CheckUnusableEquipmentInUser(user) != 1) {
        return 1;
    }

    const uint8_t play_count =
        static_cast<uint8_t>(GetPlayCountPerOneTeam());
    if (room->get_player_count() >= 2 * play_count) {
        return 4;
    }
    if (teams_[0].size() + teams_[1].size() > 2 * play_count) {
        return 4;
    }

    switch (state_) {
    case 0:
        if (_CheckSameTeam(room, user) == 1) {
            return _CheckObserver(room, user) ? 0 : 4;
        }
        return 4;
    case 3:
        return 0;
    case 1:
    case 4:
    case 5:
    default:
        return 4;
    }
}

// IDA_EA: 0x8562B78
// IDA_NAME: _ZN8fair_pvp10CFairMatch9CheckSeatEi
// DEMANGLED: fair_pvp::CFairMatch::CheckSeat(int)
int CFairMatch::CheckSeat(int seat) {
    return seat != 2;
}

// IDA_EA: 0x8562B90
// IDA_NAME: _ZN8fair_pvp10CFairMatch22SetPlayCountPerOneTeamEj
// DEMANGLED: fair_pvp::CFairMatch::SetPlayCountPerOneTeam(unsigned int)
void CFairMatch::SetPlayCountPerOneTeam(uint32_t) {
    // Exact old body: ret.
}

// IDA_EA: 0x8562B96
// IDA_NAME: _ZN8fair_pvp10CFairMatch12SetMatchAbleEb
// DEMANGLED: fair_pvp::CFairMatch::SetMatchAble(bool)
void CFairMatch::SetMatchAble(bool matchable) {
    bMatchable_ = matchable;
}

// IDA_EA: 0x8562BAE
// IDA_NAME: _ZN8fair_pvp10CFairMatch10RequestMapERsR15PVP_BATTLE_MODE
// DEMANGLED: fair_pvp::CFairMatch::RequestMap(short &,PVP_BATTLE_MODE &)
void CFairMatch::RequestMap(int16_t& map_no, PVP_BATTLE_MODE& battle_mode) {
    map_no = 0;
    if (static_cast<int>(battle_mode) != 6) {
        battle_mode = static_cast<PVP_BATTLE_MODE>(G_CDataManager()->GetPvPMode());
    }
}

// IDA_EA: 0x8562CFE
// IDA_NAME: _ZN8fair_pvp10CFairMatch28CheckUnusableEquipmentInUserEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::CheckUnusableEquipmentInUser(CUser *)
int CFairMatch::CheckUnusableEquipmentInUser(CUser*) {
    return 1;
}

// IDA_EA: 0x8562D08
// IDA_NAME: _ZN8fair_pvp10CFairMatch12OnCreateRoomEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::OnCreateRoom(CUser *)
int CFairMatch::OnCreateRoom(CUser* user) {
    if (CheckUnusableEquipmentInUser(user) != 1) {
        return 19;
    }
    auth_value_ = ++nextAuthValue_;
    state_ = 0;
    teams_[1].reset();
    teams_[0].reset();
    teams_[0].add(user);
    p2p_ready_count_ = 0;
    rewarded_round_count_ = 0;
    enter_charac_no_ = 0;
    team_leader_seat_[0] = 0;
    team_leader_seat_[1] = 0;
    next_match_ = 0;
    return 0;
}

// IDA_EA: 0x8562DC6
// IDA_NAME: _ZN8fair_pvp10CFairMatch13OnDestroyRoomEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::OnDestroyRoom(CUser *)
void CFairMatch::OnDestroyRoom(CUser*) {
    state_ = 0;
    pWaitingRoomList_->DelRoom(this);
}

// IDA_EA: 0x8562DF6
// IDA_NAME: _ZN8fair_pvp10CFairMatch10OnJoinRoomEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::OnJoinRoom(CUser *)
void CFairMatch::OnJoinRoom(CUser* user) {
    if (state_ != 3) {
        teams_[0].add(user);
    }
}

// IDA_EA: 0x8562E1E
// IDA_NAME: _ZN8fair_pvp10CFairMatch13_UpdateRatingEi22ENUM_ELO_RATING_SYSTEM
// DEMANGLED: fair_pvp::CFairMatch::_UpdateRating(int,ENUM_ELO_RATING_SYSTEM)
bool CFairMatch::_UpdateRating(int team,
                               ENUM_ELO_RATING_SYSTEM rating_system) {
    // The old branches use the selected team's average as the opponent
    // rating, then update the other TeamInfoThird.  Keep those two roles
    // distinct now that both old subobjects have typed owners.
    TeamInfoThird& opposing_team = team == 1 ? teams_[1] : teams_[0];
    TeamInfoThird& updated_team = team == 1 ? teams_[0] : teams_[1];
    float opposing_team_rating = opposing_team.GetTeamRating();
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoThird& player = updated_team.players[index];
        if (player.present && player.user) {
            float rating = player.rating;
            CEloRating::calc(rating, opposing_team_rating, rating_system,
                             PvPGlobalEnvironmentVariable::school_match_elo_k_);
            player.user->UpdateRating(rating);
        }
    }
    // Hex-Rays exposes the loop-condition byte as the result; at loop exit
    // it is false.  Preserve that machine-level result rather than inventing
    // a success value.
    return false;
}

// IDA_EA: 0x8562FBC
// IDA_NAME: _ZN8fair_pvp10CFairMatch11OnLeaveRoomEP5CUserP8PvP_Roomii
// DEMANGLED: fair_pvp::CFairMatch::OnLeaveRoom(CUser *,PvP_Room *,int,int)
void CFairMatch::OnLeaveRoom(CUser* user,
                             PvP_Room* room,
                             int team,
                             int seat) {
    if (state_ == 4) {
        _SendMatchStatePacket(4, 2);
        state_ = 0;
        _PullOutRoomTeam(room, ENUM_SCHOOL_MATCH_TEAM_1);
    } else if (state_ == 5) {
        if (room->GetTeamCount(team) == 1) {
            _UpdateRating(team, ENUM_ELO_RATING_SYSTEM_0);
        }
        const int user_team = _FindTeam(user);
        if (user_team == 2) {
            return;
        }
        const int other_team =
            _GetOtherTeam(static_cast<ENUM_SCHOOL_MATCH_TEAM>(user_team));
        if (other_team == 2) {
            return;
        }
        if (teams_[other_team].rematch_requested) {
            PacketGuard packet;
            packet.put_header(0, 314);
            packet.finalize(true);
            teams_[other_team].SendToTeam(packet);
            teams_[other_team].rematch_state = 1;
        }
    } else {
        state_ = 0;
    }

    if (team == 1) {
        teams_[0].del(user);
        _ChangeTeamLeader(room, ENUM_SCHOOL_MATCH_TEAM_0, seat);
    } else {
        teams_[1].del(user);
        _ChangeTeamLeader(room, ENUM_SCHOOL_MATCH_TEAM_1, seat);
    }
    pWaitingRoomList_->DelRoom(this);
    _SendMatchStatePacket(5, 1);
}

// IDA_EA: 0x8563272
// IDA_NAME: _ZN8fair_pvp10CFairMatch17_ChangeTeamLeaderEP8PvP_Room22ENUM_SCHOOL_MATCH_TEAMi
// DEMANGLED: fair_pvp::CFairMatch::_ChangeTeamLeader(PvP_Room *,ENUM_SCHOOL_MATCH_TEAM,int)
void CFairMatch::_ChangeTeamLeader(PvP_Room* room,
                                   ENUM_SCHOOL_MATCH_TEAM team,
                                   int departed_seat) {
    const int team_index = static_cast<int>(team);
    CUser* new_leader = 0;
    if (team_leader_seat_[team_index] == departed_seat) {
        for (int index = 0; index <= 3; ++index) {
            if (teams_[team_index].players[index].present) {
                new_leader = teams_[team_index].players[index].user;
                break;
            }
        }
    }
    if (new_leader) {
        team_leader_seat_[team_index] = room->get_user_seat(new_leader);
        PacketGuard packet;
        packet.put_header(0, 317);
        packet.put_short(team_leader_seat_[team_index]);
        packet.finalize(true);
        teams_[team_index].SendToTeam(packet);
    }
}

// IDA_EA: 0x85633CE
// IDA_NAME: _ZN8fair_pvp10CFairMatch12_PullOutRoomEP5CUserP8PvP_Roomi
// DEMANGLED: fair_pvp::CFairMatch::_PullOutRoom(CUser *,PvP_Room *,int)
void CFairMatch::_PullOutRoom(CUser* user, PvP_Room* room, int seat) {
    if (room->OnSetPVPSeatState(
            user, seat, static_cast<ENUM_SEAT_STATE>(254),
            static_cast<ENUM_WALKOUT_TYPE>(2)) != 1) {
        room->reset();
        G_CGameManager()->PutPvp(room);
    }
}

// IDA_EA: 0x85634BC
// IDA_NAME: _ZN8fair_pvp10CFairMatch10_PutInRoomEP5CUserP8PvP_Roomb
// DEMANGLED: fair_pvp::CFairMatch::_PutInRoom(CUser *,PvP_Room *,bool)
int CFairMatch::_PutInRoom(CUser* user, PvP_Room* room, bool move_room) {
    PacketGuard packet;
    int seat = 0;
    const int join_result = room->join_room(user, seat, move_room);
    if (join_result <= 0) {
        room->make_seat_info(reinterpret_cast<char*>(&packet), seat);
        G_GameWorld()->send_all(packet);
        G_GameWorld()->goto_pvp(user);
    } else {
        packet.put_header(1, ENUM_CMDPACKET_ENTER_PVP_ROOM);
        packet.put_byte(0);
        packet.put_byte(join_result);
        packet.finalize(true);
        user->Send(packet);
    }
    return seat;
}

// IDA_EA: 0x8563674
// IDA_NAME: _ZN8fair_pvp10CFairMatch15_BackupRoomInfoEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::_BackupRoomInfo(PvP_Room *)
void CFairMatch::_BackupRoomInfo(PvP_Room* room) {
    room->get_room_info(&room_info_);
}

// IDA_EA: 0x8563692
// IDA_NAME: _ZN8fair_pvp10CFairMatch9_MakeRoomEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::_MakeRoom(CUser *)
PvP_Room* CFairMatch::_MakeRoom(CUser* user) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_MAKE_PVP_ROOM);

    PvP_Room* room = G_CGameManager()->GetPvp();
    if (!room) {
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const int create_result = room->create_room(
        user, static_cast<char>(room_info_.room_title_mode),
        const_cast<char*>(room_info_.room_title.c_str()),
        static_cast<int16_t>(room_info_.requested_map_no),
        room_info_.password_enabled != 0, room_info_.password.c_str(),
        static_cast<int>(room_info_.password.length()), PVP_BATTLE_MODE_2);
    if (create_result <= 0) {
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(1);
        packet.put_short(1);
        user->make_basic_info(packet, 1);
        packet.finalize(true);
        user->Send(packet);

        packet.clear();
        packet.put_header(0, 41);
        packet.put_short(1);
        room->make_room_info(reinterpret_cast<char*>(&packet));
        packet.finalize(true);
        G_GameWorld()->send_all(packet);
        G_GameWorld()->goto_pvp(user);
        return room;
    }

    packet.put_byte(0);
    packet.put_byte(create_result);
    packet.finalize(true);
    user->Send(packet);
    G_CGameManager()->PutPvp(room);
    return 0;
}

// IDA_EA: 0x8563A32
// IDA_NAME: _ZN8fair_pvp10CFairMatch20_RegisterWaitingRoomEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::_RegisterWaitingRoom(PvP_Room *)
int CFairMatch::_RegisterWaitingRoom(PvP_Room*) {
    return pWaitingRoomList_->AddRoom(this);
}

// IDA_EA: 0x8563A54
// IDA_NAME: _ZNK8fair_pvp10CFairMatch15_CheckMatchableEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::_CheckMatchable(PvP_Room *)const
int CFairMatch::_CheckMatchable(PvP_Room* room) const {
    int observer_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            ++observer_count;
        }
        CUser* const user = room->get_user_seat(seat);
        if (user && G_CDataManager()->GetMaxGradePvPChannel() <
                        user->get_pvp_grade()) {
            return 13;
        }
    }
    if (teams_[0].size() - observer_count <=
        static_cast<uint8_t>(GetPlayCountPerOneTeam())) {
        return 0;
    }
    return 22;
}

// IDA_EA: 0x8563B16
// IDA_NAME: _ZNK8fair_pvp10CFairMatch17_CheckRematchableEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::_CheckRematchable(PvP_Room *)const
bool CFairMatch::_CheckRematchable(PvP_Room* room) const {
    int team_count[3] = {0, 0, 0};
    for (int seat = 0; seat <= 7; ++seat) {
        const int team = room->get_team(seat);
        if (team == 1 || team == 2) {
            ++team_count[team];
        }
    }
    const uint8_t play_count =
        static_cast<uint8_t>(GetPlayCountPerOneTeam());
    return team_count[1] == play_count && team_count[2] == play_count;
}

// IDA_EA: 0x8563BBA
// IDA_NAME: _ZN8fair_pvp10CFairMatch15OnRegisterMatchEP5CUserP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnRegisterMatch(CUser *,PvP_Room *)
int CFairMatch::OnRegisterMatch(CUser* user, PvP_Room* room) {
    if (state_ == 1) {
        return 1;
    }
    if (!bMatchable_) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_MATCH, 19);
        return 0;
    }

    const int match_error = _CheckMatchable(room);
    if (match_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_MATCH,
                                 static_cast<unsigned char>(match_error));
        return 0;
    }

    if (state_ != 2) {
        _RegisterWaitingRoom(room);
    }
    state_ = 1;
    TimerStartMatching::registNextTimer(10, room->get_index(), auth_value_);
    _SendMatchStatePacket(0, 1);
    return 1;
}

// IDA_EA: 0x8563CC6
// IDA_NAME: _ZN8fair_pvp10CFairMatch17OnUnregisterMatchEP5CUserP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnUnregisterMatch(CUser *,PvP_Room *)
int CFairMatch::OnUnregisterMatch(CUser*, PvP_Room*) {
    if (state_ != 1 && state_ != 2) {
        return 0;
    }
    state_ = 0;
    pWaitingRoomList_->DelRoom(this);
    _SendMatchStatePacket(5, 1);
    return 1;
}

// IDA_EA: 0x8563D32
// IDA_NAME: _ZN8fair_pvp10CFairMatch9_MoveRoomEP8PvP_RoomS2_b
// DEMANGLED: fair_pvp::CFairMatch::_MoveRoom(PvP_Room *,PvP_Room *,bool)
bool CFairMatch::_MoveRoom(PvP_Room* destination,
                           PvP_Room* source,
                           bool primary) {
    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = source->get_user_seat(seat);
        if (!user) {
            continue;
        }

        const int team = source->get_team(seat);
        PacketGuard packet;
        packet.put_header(0, 41);
        packet.put_short(1);
        destination->make_room_info(reinterpret_cast<char*>(&packet));
        packet.finalize(true);
        source->send_to_pvp(packet);
        _PullOutRoom(user, source, seat);
        _SetEnterCharacNo(user->getCurCharacNo());
        const int destination_seat = _PutInRoom(user, destination, primary);
        if (team == 3) {
            destination->OnSetPVPSeatState(
                user, destination_seat, static_cast<ENUM_SEAT_STATE>(3),
                static_cast<ENUM_WALKOUT_TYPE>(2));
        }
    }
    // The original assigns the loop predicate to its result; it is false
    // after the complete 0..7 walk.
    return false;
}

// IDA_EA: 0x8563ED8
// IDA_NAME: _ZN8fair_pvp10CFairMatch17_CheckMovableRoomEP6IMatchP8PvP_RoomPS4_
// DEMANGLED: fair_pvp::CFairMatch::_CheckMovableRoom(IMatch *,PvP_Room *,PvP_Room **)
int CFairMatch::_CheckMovableRoom(IMatch* match,
                                  PvP_Room*,
                                  PvP_Room** result_room) {
    if (!match) {
        return 0;
    }
    CUser* user = 0;
    for (int index = 0; index <= 3; ++index) {
        user = match->GetUser(index);
        if (user) {
            break;
        }
    }
    if (!user) {
        return 0;
    }
    *result_room = user->GetPVPRoom();
    return *result_room ? 1 : 0;
}

// IDA_EA: 0x8564150
// IDA_NAME: _ZN8fair_pvp10CFairMatch11OnMatchRoomEP8PvP_Roomi
// DEMANGLED: fair_pvp::CFairMatch::OnMatchRoom(PvP_Room *,int)
int CFairMatch::OnMatchRoom(PvP_Room* room, int auth_value) {
    if (auth_value_ != auth_value || state_ != 1) {
        return 0;
    }

    room->setReamtchable(GetPlayCountPerOneTeam() == 1);
    team_leader_seat_[0] = room->GetManagerSeat();

    IMatch* matched = 0;
    if (!pWaitingRoomList_->Match(this, &matched)) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    PvP_Room* first_source = 0;
    if (!_CheckMovableRoom(matched, room, &first_source) ||
        room->get_index() == first_source->get_index()) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    teams_[1].reset();
    state_ = 3;
    int destination_team = 0;
    CUser* second_team_manager = 0;
    while (matched) {
        PvP_Room* source = 0;
        if (!_CheckMovableRoom(matched, room, &source)) {
            state_ = 2;
            _SendMatchStatePacket(1, 1);
            return 0;
        }

        if (GetPlayerCount() ==
            static_cast<uint8_t>(GetPlayCountPerOneTeam())) {
            destination_team = 1;
            second_team_manager = source->GetManager();
            _BackupRoomInfo(source);
        }

        for (int index = 0; index <= 3; ++index) {
            CUser* const user = matched->GetUser(index);
            if (user) {
                teams_[destination_team].add(user);
            }
        }
        _SendMatchStatePacket(3, 2);
        _MoveRoom(room, source, destination_team == 0);
        matched = matched->GetNextMatch();
    }

    team_leader_seat_[1] = room->get_user_seat(second_team_manager);
    pWaitingRoomList_->DelRoom(this);
    state_ = 4;
    _SendMatchStatePacket(2, 2);
    p2p_ready_count_ = 0;
    const int timer_key =
        room->gen_timer_key(static_cast<TIMER_MESSAGE>(59));
    G_TimerQueue()->InsertTimer(
        5, room->get_index(), 59,
        GlobalData::s_ref_pvp_grade->getPvPTimeout(), timer_key, 0);

    PacketGuard packet;
    room->make_seat_info(reinterpret_cast<char*>(&packet), 0xffff);
    G_GameWorld()->send_all(packet);
    for (int team = 0; team <= 1; ++team) {
        packet.clear();
        packet.put_header(0, 317);
        packet.put_short(team_leader_seat_[team]);
        packet.finalize(true);
        teams_[team].SendToTeam(packet);
    }
    return 1;
}

// IDA_EA: 0x856491C
// IDA_NAME: _ZN8fair_pvp10CFairMatch6GetGapEP6IMatch
// DEMANGLED: fair_pvp::CFairMatch::GetGap(IMatch *)
float CFairMatch::GetGap(IMatch* match) {
    return std::fabs(_GetRating(ENUM_SCHOOL_MATCH_TEAM_0) -
                     static_cast<CFairMatch*>(match)->_GetRating(
                         ENUM_SCHOOL_MATCH_TEAM_0));
}

// IDA_EA: 0x8564952
// IDA_NAME: _ZN8fair_pvp10CFairMatch12OnStartFightEv
// DEMANGLED: fair_pvp::CFairMatch::OnStartFight(void)
void CFairMatch::OnStartFight() {
    state_ = 5;
}

// IDA_EA: 0x8564962
// IDA_NAME: _ZN8fair_pvp10CFairMatch14OnCompleteLoadEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::OnCompleteLoad(CUser *)
void CFairMatch::OnCompleteLoad(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 278);
    packet.put_short(user->get_unique_id());
    packet.finalize(true);
    for (int team = 0; team <= 1; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfoThird& player = teams_[team].players[index];
            if (player.present && player.user) {
                player.user->Send(packet);
            }
        }
    }
}

// IDA_EA: 0x8564A8E
// IDA_NAME: _ZN8fair_pvp10CFairMatch12OnConnectP2PEP5CUserP8PvP_RoomPci
// DEMANGLED: fair_pvp::CFairMatch::OnConnectP2P(CUser *,PvP_Room *,char *,int)
void CFairMatch::OnConnectP2P(CUser*, PvP_Room* room,
                              char* result, int count) {
    if (state_ == 4) {
        for (int seat = 0; seat <= 7 && seat < count; ++seat) {
            if (!result[seat]) {
                _SendMatchStatePacket(4, 2);
                OnConnectP2PTimeout(room);
                return;
            }
        }

        ++p2p_ready_count_;
        if (p2p_ready_count_ == 2 * GetPlayCountPerOneTeam()) {
            PacketGuard packet;
            packet.put_header(0, 279);
            packet.finalize(true);
            for (int team = 0; team <= 1; ++team) {
                for (int index = 0; index <= 3; ++index) {
                    PlayerInfoThird& player = teams_[team].players[index];
                    if (player.present && player.user) {
                        player.user->Send(packet);
                    }
                }
            }
            packet.clear();

            room->battle_reset();
            for (int seat = 0; seat <= 7; ++seat) {
                if (room->get_user_seat(seat)) {
                    room->relay_battle_mgr_.OnReady(seat, true);
                }
            }
            room->start_pvp();
            room->make_state_info(reinterpret_cast<char*>(&packet));
            G_GameWorld()->send_all(packet);
        }
        return;
    }

    _SendMatchStatePacket(4, 2);
    state_ = 0;
    _ReturnTeam(room, room->get_manager_team(), 0);
}

// IDA_EA: 0x8564D3A
// IDA_NAME: _ZN8fair_pvp10CFairMatch19OnConnectP2PTimeoutEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnConnectP2PTimeout(PvP_Room *)
void CFairMatch::OnConnectP2PTimeout(PvP_Room* room) {
    if (state_ != 4) {
        return;
    }
    _SendMatchStatePacket(4, 2);
    const int manager_team = room->get_manager_team();
    _ReturnTeam(room, manager_team, 0);
    if (manager_team != 1) {
        teams_[0] = teams_[1];
    }
    teams_[0].refresh();
    teams_[1].reset();
}

// IDA_EA: 0x8564DE0
// IDA_NAME: _ZN8fair_pvp10CFairMatch8OnGiveupEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::OnGiveup(CUser *)
int CFairMatch::OnGiveup(CUser*) {
    return 1;
}

// IDA_EA: 0x8564DEA
// IDA_NAME: _ZN8fair_pvp10CFairMatch11_ReturnTeamEP8PvP_RoomiP5CUser
// DEMANGLED: fair_pvp::CFairMatch::_ReturnTeam(PvP_Room *,int,CUser *)
PvP_Room* CFairMatch::_ReturnTeam(PvP_Room* room,
                                  int retained_team,
                                  CUser* retained_user) {
    PvP_Room* new_room = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = room->get_user_seat(seat);
        if (!user) {
            continue;
        }
        const int user_team = room->get_team(seat);
        if (retained_team == user_team) {
            if (retained_team == 2) {
                room->OnSetPVPSeatState(
                    user, seat, static_cast<ENUM_SEAT_STATE>(1),
                    static_cast<ENUM_WALKOUT_TYPE>(2));
            }
        } else if (!retained_user || user != retained_user) {
            _PullOutRoom(user, room, seat);
            if (new_room) {
                _PutInRoom(user, new_room, true);
            } else {
                new_room = _MakeRoom(user);
            }
        }
    }
    state_ = 0;
    return new_room;
}

// IDA_EA: 0x8564F8C
// IDA_NAME: _ZN8fair_pvp10CFairMatch16_GetOppositeTeamEi
// DEMANGLED: fair_pvp::CFairMatch::_GetOppositeTeam(int)
int CFairMatch::_GetOppositeTeam(int team) {
    if (team == 1) {
        return 2;
    }
    if (team == 2) {
        return 1;
    }
    return team;
}

// IDA_EA: 0x8564FB2
// IDA_NAME: _ZN8fair_pvp10CFairMatch9OnWalkOutEP8PvP_Roomi
// DEMANGLED: fair_pvp::CFairMatch::OnWalkOut(PvP_Room *,int)
int CFairMatch::OnWalkOut(PvP_Room* room, int seat) {
    OnLeaveRoom(room->get_user_seat(seat), room, room->get_team(seat), seat);
    return 1;
}

// IDA_EA: 0x8565018
// IDA_NAME: _ZN8fair_pvp10CFairMatch7OnReadyEP5CUserb
// DEMANGLED: fair_pvp::CFairMatch::OnReady(CUser *,bool)
bool CFairMatch::OnReady(CUser*, bool ready) {
    return ready || state_ != 4;
}

// IDA_EA: 0x8565048
// IDA_NAME: _ZN8fair_pvp10CFairMatch10OnEndFightEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnEndFight(PvP_Room *)
void CFairMatch::OnEndFight(PvP_Room* room) {
    const int manager_team = room->get_manager_team();
    state_ = 0;
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1) {
        const int timer_key =
            room->gen_timer_key(static_cast<TIMER_MESSAGE>(62));
        G_TimerQueue()->InsertTimerInMilisecond(
            1, room->get_index(), 62, 0, timer_key, 0);
        return;
    }
    _ReturnTeam(room, manager_team, 0);
    if (manager_team != 1) {
        teams_[0] = teams_[1];
    }
    teams_[0].refresh();
    teams_[1].reset();
}

// IDA_EA: 0x8565140
// IDA_NAME: _ZN8fair_pvp10CFairMatch7GetUserEj
// DEMANGLED: fair_pvp::CFairMatch::GetUser(unsigned int)
CUser* CFairMatch::GetUser(uint32_t index) {
    if (index > 3 || !teams_[0].players[index].present) {
        return 0;
    }
    return teams_[0].players[index].user;
}

// IDA_EA: 0x8565184
// IDA_NAME: _ZN8fair_pvp10CFairMatch11GetUserListEPv
// DEMANGLED: fair_pvp::CFairMatch::GetUserList(void *)
void CFairMatch::GetUserList(void* destination) {
    // The original emits one TeamInfoThird-sized record (0x48) through the
    // inherited output-buffer ABI.  EAX=18 is the rep-movsd count, not an
    // int return value.
    std::memcpy(destination, &teams_[0], sizeof(TeamInfoThird));
}

// IDA_EA: 0x85651A6
// IDA_NAME: _ZN8fair_pvp10CFairMatch21_SendMatchStatePacketEii
// DEMANGLED: fair_pvp::CFairMatch::_SendMatchStatePacket(int,int)
void CFairMatch::_SendMatchStatePacket(int state, int team_count) {
    PacketGuard packet;
    packet.put_header(0, 175);
    packet.put_byte(state);
    packet.finalize(true);
    for (int team = 0; team < team_count; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfoThird& player = teams_[team].players[index];
            if (player.present && player.user) {
                player.user->Send(packet);
            }
        }
    }
}

// IDA_EA: 0x85652C8
// IDA_NAME: _ZN8fair_pvp10CFairMatch16GiveTrainingSealEP5CUserb
// DEMANGLED: fair_pvp::CFairMatch::GiveTrainingSeal(CUser *,bool)
void CFairMatch::GiveTrainingSeal(CUser* user, bool won) {
    if (!user->getCurCharacR()) {
        return;
    }

    ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    CFairPvPScore* const score = static_cast<CFairPvPScore*>(
        user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));

    const int remaining_bonus =
        static_cast<int>(parameters->get_training_seal_bonus_count()) -
        score->GetDailyPlayCount();
    const bool bonus_available = remaining_bonus > 0;
    score->SetRemainBonusCount(
        bonus_available ? remaining_bonus : 0);

    const bool relay_battle = GetPlayCountPerOneTeam() != 1;
    int seal_count = parameters->get_training_seal_gain_count(
        relay_battle, won, false);
    int bonus_count = 0;
    if (bonus_available) {
        bonus_count = parameters->get_training_seal_gain_count(
            relay_battle, won, true);
        seal_count += bonus_count;
    }

    score->SetLastTrainingSealBonusCount(bonus_count);
    score->SetLastTrainingSealCount(seal_count);
    if (seal_count <= 0) {
        return;
    }

    std::vector<std::pair<int, int> > rewards;
    std::vector<int> inserted_slots;
    rewards.push_back(std::make_pair(3312, seal_count));
    const int inserted = user->getCurCharacInvenW()->insert_event_items(
        rewards,
        inserted_slots,
        static_cast<eMoneyAddReason>(3),
        static_cast<eItemAddReason>(8),
        true,
        "game_server_msg_171",
        "game_server_msg_172");
    if (inserted > 0 && inserted_slots.size() > 2u) {
        user->SendUpdateItem(
            static_cast<CUser::eSendTarget>(1),
            static_cast<ENUM_ITEMSPACE>(0),
            inserted_slots[0]);
    }
}

// IDA_EA: 0x8565700
// IDA_NAME: _ZN8fair_pvp10CFairMatch11UpdateScoreEP8PvP_RoomRNS_15PlayerInfoThirdE22ENUM_SCHOOL_MATCH_TEAMiii
// DEMANGLED: fair_pvp::CFairMatch::UpdateScore(PvP_Room *,fair_pvp::PlayerInfoThird &,ENUM_SCHOOL_MATCH_TEAM,int,int,int)
char CFairMatch::UpdateScore(PvP_Room* room, PlayerInfoThird& player,
                             ENUM_SCHOOL_MATCH_TEAM team, int won,
                             int lost, int draw) {
    const int team_index = static_cast<int>(team);
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoThird& scored_player = teams_[team_index].players[index];
        CUser* const user = scored_player.user;
        if (!scored_player.present || !user) {
            continue;
        }

        const int seat = room->get_user_seat(user);
        if (seat < 0 || seat > 7) {
            return static_cast<char>(seat);
        }

        const int battle_mode = room->get_pvp_battle_mode();
        const int room_index = room->get_index();
        const char* const character_name = user->getCurCharacName();
        const char* const account_name = user->get_acc_name();
        CSwitchLog trace(
            "void fair_pvp::CFairMatch::UpdateScore(PvP_Room*, "
            "fair_pvp::PlayerInfoThird&, ENUM_SCHOOL_MATCH_TEAM, int, "
            "int, int)",
            3813, 0, 0);
        trace("pvp@log %s,%s,room(%d),%d,%d,%d", account_name,
              character_name, room_index, battle_mode, won, lost);

        const int relay_kill_count =
            room->relay_battle_mgr_.GetKillCount(seat);
        CFairPvPScore* const score = static_cast<CFairPvPScore*>(
            user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
        score->UpdateScore(
            static_cast<int>(player.character_kind.charac_job),
            static_cast<int>(player.character_kind.first_grow_type & 0x0fu),
            static_cast<uint8_t>(GetPlayCountPerOneTeam()), won, lost, draw,
            relay_kill_count);

        CMissionList_Charac* const missions =
            static_cast<CMissionList_Charac*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
        MissionClearCondition_Parameter parameter(
            GetPlayCountPerOneTeam() == 1 ? PVP_BATTLE_MODE_2
                                          : PVP_BATTLE_MODE_3);
        parameter.kill_count = room->relay_battle_mgr_.GetKillCount(seat);
        missions->Update_GameOver_event(*user, parameter);
        GiveTrainingSeal(user, won != 0);
    }
    return 0;
}

// IDA_EA: 0x8565966
// IDA_NAME: _ZN8fair_pvp10CFairMatch8OnRewardEP8PvP_Roomib
// DEMANGLED: fair_pvp::CFairMatch::OnReward(PvP_Room *,int,bool)
void CFairMatch::OnReward(PvP_Room* room, int team, bool draw) {
    if (draw) {
        _UpdateRating(1, ENUM_ELO_RATING_SYSTEM_1);
        _UpdateRating(2, ENUM_ELO_RATING_SYSTEM_1);
        UpdateScore(room, teams_[1].players[0], ENUM_SCHOOL_MATCH_TEAM_0,
                    0, 0, 1);
        UpdateScore(room, teams_[0].players[0], ENUM_SCHOOL_MATCH_TEAM_1,
                    0, 0, 1);
    } else if (team == 1) {
        _UpdateRating(1, ENUM_ELO_RATING_SYSTEM_2);
        _UpdateRating(2, ENUM_ELO_RATING_SYSTEM_0);
        UpdateScore(room, teams_[1].players[0], ENUM_SCHOOL_MATCH_TEAM_0,
                    1, 0, 0);
        UpdateScore(room, teams_[0].players[0], ENUM_SCHOOL_MATCH_TEAM_1,
                    0, 1, 0);
    } else {
        _UpdateRating(2, ENUM_ELO_RATING_SYSTEM_2);
        _UpdateRating(1, ENUM_ELO_RATING_SYSTEM_0);
        UpdateScore(room, teams_[1].players[0], ENUM_SCHOOL_MATCH_TEAM_0,
                    0, 1, 0);
        UpdateScore(room, teams_[0].players[0], ENUM_SCHOOL_MATCH_TEAM_1,
                    1, 0, 0);
    }
    ++rewarded_round_count_;
    PacketGuard packet;
    packet.put_header(0, 316);
    packet.put_int(
        GlobalData::s_ref_pvp_grade->getPvPRematch() -
        rewarded_round_count_);
    packet.finalize(true);
    room->send_to_pvp(packet);
}

// IDA_EA: 0x8565C4E
// IDA_NAME: _ZN8fair_pvp10CFairMatch8OnEndPvPEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnEndPvP(PvP_Room *)
void CFairMatch::OnEndPvP(PvP_Room*) {
    // Exact old body: ret.
}

// IDA_EA: 0x8565C54
// IDA_NAME: _ZN8fair_pvp10CFairMatch14CheckMatchableEP6IMatch
// DEMANGLED: fair_pvp::CFairMatch::CheckMatchable(IMatch *)
bool CFairMatch::CheckMatchable(IMatch* match) {
    CUser* match_user = 0;
    for (int index = 0; index <= 3; ++index) {
        match_user = match->GetUser(index);
        if (match_user) {
            break;
        }
    }
    if (!match_user) {
        return false;
    }
    CUser* this_user = 0;
    for (int index = 0; index <= 3; ++index) {
        this_user = GetUser(index);
        if (this_user) {
            break;
        }
    }
    return this_user != 0;
}

// IDA_EA: 0x8565D04
// IDA_NAME: _ZN8fair_pvp10CFairMatch12CheckRematchEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::CheckRematch(PvP_Room *)
int CFairMatch::CheckRematch(PvP_Room* room) {
    if (rewarded_round_count_ >=
            GlobalData::s_ref_pvp_grade->getPvPRematch() ||
        !_CheckRematchable(room)) {
        return 0;
    }
    for (int team = 0; team <= 1; ++team) {
        if (teams_[team].rematch_requested != true) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8565D86
// IDA_NAME: _ZN8fair_pvp10CFairMatch16_PullOutRoomTeamEP8PvP_Room22ENUM_SCHOOL_MATCH_TEAM
// DEMANGLED: fair_pvp::CFairMatch::_PullOutRoomTeam(PvP_Room *,ENUM_SCHOOL_MATCH_TEAM)
int CFairMatch::_PullOutRoomTeam(PvP_Room* room,
                                 ENUM_SCHOOL_MATCH_TEAM team) {
    const int team_index = static_cast<int>(team);
    int last_room_team = 0;
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoThird& player = teams_[team_index].players[index];
        if (!player.present || !player.user) {
            continue;
        }
        const int seat = room->get_user_seat(player.user);
        if (seat != -1) {
            last_room_team = room->get_team(seat);
            _PullOutRoom(player.user, room, seat);
        }
    }
    if (last_room_team == 1) {
        for (int seat = 0; seat <= 7; ++seat) {
            CUser* const user = room->get_user_seat(seat);
            if (user && room->get_team(seat) == 2) {
                room->OnSetPVPSeatState(
                    user, seat, static_cast<ENUM_SEAT_STATE>(1),
                    static_cast<ENUM_WALKOUT_TYPE>(2));
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8565F92
// IDA_NAME: _ZN8fair_pvp10CFairMatch16OnRematchTimeoutEP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnRematchTimeout(PvP_Room *)
void CFairMatch::OnRematchTimeout(PvP_Room* room) {
    CheckRematch(room);
    if (CheckRematch(room) == 1) {
        teams_[0].refresh();
        teams_[1].refresh();

        PacketGuard packet;
        room->battle_reset();
        for (int seat = 0; seat <= 7; ++seat) {
            if (room->get_user_seat(seat)) {
                room->relay_battle_mgr_.OnReady(seat, true);
            }
        }
        room->start_pvp();
        room->make_state_info(reinterpret_cast<char*>(&packet));
        G_GameWorld()->send_all(packet);
        return;
    }

    if (teams_[1].size() && !teams_[1].rematch_requested) {
        _PullOutRoomTeam(room, ENUM_SCHOOL_MATCH_TEAM_1);
    }

    if (teams_[0].rematch_state != 0 && teams_[0].rematch_state <= 2) {
        const int manager_team = room->get_manager_team();
        PvP_Room* const returned_room = _ReturnTeam(room, manager_team, 0);
        PacketGuard packet;
        packet.put_header(0, 315);
        packet.finalize(true);
        if (returned_room) {
            returned_room->send_to_pvp(packet);
        }
        teams_[0].SendToTeam(packet);
        teams_[1].SendToTeam(packet);
        if (!teams_[0].size()) {
            teams_[0] = teams_[1];
        }
        teams_[0].refresh();
        teams_[1].reset();
        rewarded_round_count_ = 0;
        return;
    }

    _PullOutRoomTeam(room, ENUM_SCHOOL_MATCH_TEAM_0);
    if (teams_[1].size()) {
        PacketGuard packet;
        packet.put_header(0, 315);
        packet.finalize(true);
        teams_[1].SendToTeam(packet);
        teams_[0] = teams_[1];
        teams_[0].refresh();
        teams_[1].reset();
        rewarded_round_count_ = 0;
    }
}

// IDA_EA: 0x85663A8
// IDA_NAME: _ZN8fair_pvp10CFairMatch9_FindTeamEP5CUser
// DEMANGLED: fair_pvp::CFairMatch::_FindTeam(CUser *)
int CFairMatch::_FindTeam(CUser* user) {
    for (int team = 0; team <= 1; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfoThird& player = teams_[team].players[index];
            if (player.present && player.user && player.user == user) {
                return team;
            }
        }
    }
    return 2;
}

// IDA_EA: 0x8566476
// IDA_NAME: _ZN8fair_pvp10CFairMatch13_GetOtherTeamE22ENUM_SCHOOL_MATCH_TEAM
// DEMANGLED: fair_pvp::CFairMatch::_GetOtherTeam(ENUM_SCHOOL_MATCH_TEAM)
int CFairMatch::_GetOtherTeam(ENUM_SCHOOL_MATCH_TEAM team) {
    if (team == ENUM_SCHOOL_MATCH_TEAM_0) {
        return 1;
    }
    if (team == ENUM_SCHOOL_MATCH_TEAM_1) {
        return 0;
    }
    return 2;
}

// IDA_EA: 0x856649A
// IDA_NAME: _ZN8fair_pvp10CFairMatch13OnWaitRematchEP5CUserP8PvP_Room
// DEMANGLED: fair_pvp::CFairMatch::OnWaitRematch(CUser *,PvP_Room *)
void CFairMatch::OnWaitRematch(CUser*, PvP_Room*) {
    // Exact old body: ret.
}

// IDA_EA: 0x85664A0
// IDA_NAME: _ZN8fair_pvp10CFairMatch12OnAskRematchEP5CUseri
// DEMANGLED: fair_pvp::CFairMatch::OnAskRematch(CUser *,int)
void CFairMatch::OnAskRematch(CUser* user, int answer) {
    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        return;
    }

    const int team = _FindTeam(user);
    if (team == 2) {
        return;
    }
    const int other_team =
        _GetOtherTeam(static_cast<ENUM_SCHOOL_MATCH_TEAM>(team));
    if (other_team == 2) {
        return;
    }

    PacketGuard packet;
    if (answer >= 0) {
        if (answer <= 1) {
            packet.clear();
            packet.put_header(0, 314);
            packet.finalize(true);
            teams_[team].SendToTeam(packet);
            teams_[team].rematch_state = answer;
            if (teams_[other_team].rematch_requested) {
                teams_[other_team].SendToTeam(packet);
                room->setReamtchable(false);
            }
            room->revengeMission(false);
        } else if (answer == 2) {
            teams_[team].rematch_requested = true;
            if (teams_[other_team].rematch_state != 2 ||
                !teams_[other_team].size() || !_CheckRematchable(room)) {
                packet.clear();
                packet.put_header(0, 314);
                packet.finalize(true);
                teams_[team].SendToTeam(packet);
                teams_[team].rematch_state = 1;
            } else if (teams_[other_team].rematch_requested) {
                packet.clear();
                packet.put_header(0, 314);
                packet.finalize(true);
                room->send_to_pvp(packet);
                room->revengeMission(true);
            }
        }
    }

    packet.clear();
    packet.put_header(0, 311);
    packet.put_byte(answer);
    packet.put_short(room->get_user_seat(user));
    packet.finalize(true);
    room->send_to_pvp(packet);
}

// IDA_EA: 0x856694C
// IDA_NAME: _ZN8fair_pvp10CFairMatch14OnEndPvPResultEP8PvP_RoomP5CUser
// DEMANGLED: fair_pvp::CFairMatch::OnEndPvPResult(PvP_Room *,CUser *)
void CFairMatch::OnEndPvPResult(PvP_Room* room, CUser* user) {
    const int seat = room->get_user_seat(user);
    if (seat == -1) {
        return;
    }
    const int room_team = room->get_team(seat);
    if (room_team == 254) {
        return;
    }
    const int team = _FindTeam(user);
    if (team == 2 || teams_[team].rematch_state == 2) {
        return;
    }
    const int other_team =
        _GetOtherTeam(static_cast<ENUM_SCHOOL_MATCH_TEAM>(team));
    if (other_team == 2) {
        return;
    }

    room->set_state(1);
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoThird& player = teams_[team].players[index];
        if (player.present && player.user) {
            player.user->set_state(6);
        }
    }

    if (teams_[team].rematch_state == 1) {
        PvP_Room* const returned_room =
            _ReturnTeam(room, _GetOppositeTeam(room_team), 0);
        if (returned_room) {
            PacketGuard packet;
            packet.put_header(0, 315);
            packet.finalize(true);
            returned_room->send_to_pvp(packet);
        }
    } else if (teams_[team].rematch_state == 0) {
        _PullOutRoomTeam(room, static_cast<ENUM_SCHOOL_MATCH_TEAM>(team));
    }

    room->set_state(2);
    if (room_team == 1) {
        std::swap(teams_[0], teams_[1]);
    }
}

// IDA_EA: 0x8566D3E
// IDA_NAME: _ZN8fair_pvp10CFairMatch14GetPlayerCountEv
// DEMANGLED: fair_pvp::CFairMatch::GetPlayerCount(void)
int CFairMatch::GetPlayerCount() {
    int count = 0;
    for (int index = 0; index <= 3; ++index) {
        if (teams_[0].players[index].present) {
            ++count;
        }
        if (teams_[1].players[index].present) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x856892A
// IDA_NAME: _ZN8fair_pvp10CFairMatch22GetEndPvPResultTimeoutEv
// DEMANGLED: fair_pvp::CFairMatch::GetEndPvPResultTimeout(void)
int CFairMatch::GetEndPvPResultTimeout() {
    return 60000;
}

// IDA_EA: 0x8568934
// IDA_NAME: _ZN8fair_pvp10CFairMatch12GetNextMatchEv
// DEMANGLED: fair_pvp::CFairMatch::GetNextMatch(void)
IMatch* CFairMatch::GetNextMatch() {
    return next_match_;
}

// IDA_EA: 0x8568942
// IDA_NAME: _ZN8fair_pvp10CFairMatch12SetNextMatchEP6IMatch
// DEMANGLED: fair_pvp::CFairMatch::SetNextMatch(IMatch *)
void CFairMatch::SetNextMatch(IMatch* match) {
    next_match_ = match;
}

// IDA_EA: 0x8568954
// IDA_NAME: _ZN8fair_pvp10CFairMatch17_SetEnterCharacNoEi
// DEMANGLED: fair_pvp::CFairMatch::_SetEnterCharacNo(int)
void CFairMatch::_SetEnterCharacNo(int charac_no) {
    enter_charac_no_ = charac_no;
}

// IDA_EA: 0x8568966
// IDA_NAME: _ZN8fair_pvp10CFairMatch10_GetRatingE22ENUM_SCHOOL_MATCH_TEAM
// DEMANGLED: fair_pvp::CFairMatch::_GetRating(ENUM_SCHOOL_MATCH_TEAM)
float CFairMatch::_GetRating(ENUM_SCHOOL_MATCH_TEAM team) {
    return teams_[static_cast<int>(team)].GetTeamRating();
}

}  // namespace fair_pvp
