#include "old_game/pvp/match/league_match_static_state_runtime.h"

#include <algorithm>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/season/pvp_rating_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/match/league_match_room_list_runtime.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/pvp/match/timer_start_matching.h"

// IDA_EA: 0x855D1C0
// IDA_NAME: _ZN12CLeagueMatch11OnLeaveRoomEP5CUserP8PvP_Roomii
// DEMANGLED: CLeagueMatch::OnLeaveRoom(CUser *,PvP_Room *,int,int)
void CLeagueMatch::OnLeaveRoom(CUser* user,
                               PvP_Room* room,
                               int team,
                               int seat) {
    if (state_ == 4) {
        _SendMatchStatePacket(4, 2);
        state_ = 0;
        _ReturnTeam(room, room->get_manager_team(), user);
    } else if (state_ == 5) {
        // IDA 0x855D239 leaves both stack floats uninitialised until the
        // matching PlayerInfo slot is found.  The n3 <= 3 guard below means
        // neither value is observed on the no-match path.
        float own_rating;
        float opposing_rating;
        int player_index = 0;
        for (; player_index <= 3; ++player_index) {
            if (team == 1) {
                if (teams_[0].players[player_index].user == user) {
                    own_rating = teams_[0].players[player_index].rating;
                    opposing_rating =
                        teams_[1].players[player_index].rating;
                    break;
                }
            } else if (teams_[1].players[player_index].user == user) {
                own_rating = teams_[1].players[player_index].rating;
                opposing_rating = teams_[0].players[player_index].rating;
                break;
            }
        }
        if (player_index <= 3) {
            CEloRating::calc(
                own_rating,
                opposing_rating,
                static_cast<ENUM_ELO_RATING_SYSTEM>(0),
                PvPGlobalEnvironmentVariable::school_match_elo_k_);
            user->UpdateRating(own_rating);
        }
    } else {
        state_ = 0;
    }

    if (team == 1) {
        teams_[0].del(user);
    } else {
        teams_[1].del(user);
    }
    pWaitingRoomList_->DelRoom(this);
    (void)seat;
}

// IDA_EA: 0x855D3D2
// IDA_NAME: _ZN12CLeagueMatch12_PullOutRoomEP5CUserP8PvP_Roomi
// DEMANGLED: CLeagueMatch::_PullOutRoom(CUser *,PvP_Room *,int)
void CLeagueMatch::_PullOutRoom(CUser* user, PvP_Room* room, int seat) {
    if (static_cast<uint8_t>(room->OnSetPVPSeatState(
            user, seat, static_cast<ENUM_SEAT_STATE>(254),
            static_cast<ENUM_WALKOUT_TYPE>(2))) != 1u) {
        room->reset();
        G_CGameManager()->PutPvp(room);
    }
}

// IDA_EA: 0x855D42A
// IDA_NAME: _ZN12CLeagueMatch10_PutInRoomEP5CUserP8PvP_Roomb
// DEMANGLED: CLeagueMatch::_PutInRoom(CUser *,PvP_Room *,bool)
int CLeagueMatch::_PutInRoom(CUser* user, PvP_Room* room, bool move_room) {
    PacketGuard packet;
    int seat = 0;
    const int result = room->join_room(user, seat, move_room);
    if (result <= 0) {
        room->make_seat_info(reinterpret_cast<char*>(&packet), seat);
        G_GameWorld()->send_all(packet);
        G_GameWorld()->goto_pvp(user);
    } else {
        packet.put_header(1, ENUM_CMDPACKET_ENTER_PVP_ROOM);
        packet.put_byte(0);
        packet.put_byte(result);
        packet.finalize(true);
        user->Send(packet);
    }
    return seat;
}

// IDA_EA: 0x855D554
// IDA_NAME: _ZN12CLeagueMatch15_BackupRoomInfoEP8PvP_Room
// DEMANGLED: CLeagueMatch::_BackupRoomInfo(PvP_Room *)
void CLeagueMatch::_BackupRoomInfo(PvP_Room* room) {
    room->get_room_info(&room_info_);
}

// IDA_EA: 0x855D572
// IDA_NAME: _ZN12CLeagueMatch9_MakeRoomEP5CUser
// DEMANGLED: CLeagueMatch::_MakeRoom(CUser *)
PvP_Room* CLeagueMatch::_MakeRoom(CUser* user) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_MAKE_PVP_ROOM);

    PvP_Room* const room = G_CGameManager()->GetPvp();
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
        static_cast<int>(room_info_.password.length()),
        static_cast<PVP_BATTLE_MODE>(2));
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
    LogManager::logFormat(1,
                          "MatchingSystem.cpp",
                          "PvP_Room* CLeagueMatch::_MakeRoom(CUser*)",
                          0x1fe,
                          "create_room fail: PVP",
                          room->get_index());
    G_CGameManager()->PutPvp(room);
    return 0;
}

// IDA_EA: 0x855D884
// IDA_NAME: _ZN12CLeagueMatch20_RegisterWaitingRoomEP8PvP_Room
// DEMANGLED: CLeagueMatch::_RegisterWaitingRoom(PvP_Room *)
int CLeagueMatch::_RegisterWaitingRoom(PvP_Room* room) {
    bool have_active_player = false;
    float lowest_rating = 0.0f;
    for (int index = 0; index <= 3; ++index) {
        const PlayerInfo& player = teams_[0].players[index];
        if (player.active) {
            if (!have_active_player) {
                lowest_rating = player.rating;
                have_active_player = true;
            }
            if (lowest_rating > player.rating) {
                lowest_rating = player.rating;
            }
        }
    }

    const float observer_rating = lowest_rating - 1.0f;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            CUser* const observer = room->get_user_seat(seat);
            for (int index = 0; index <= 3; ++index) {
                PlayerInfo& player = teams_[0].players[index];
                if (player.user == observer && observer) {
                    player.rating = observer_rating;
                }
            }
        }
    }

    std::sort(teams_[0].players, teams_[0].players + 4, CompareRating);
    return pWaitingRoomList_->AddRoom(this);
}

// IDA_EA: 0x855DB58
// IDA_NAME: _ZN12CLeagueMatch15OnRegisterMatchEP5CUserP8PvP_Room
// DEMANGLED: CLeagueMatch::OnRegisterMatch(CUser *,PvP_Room *)
int CLeagueMatch::OnRegisterMatch(CUser* user, PvP_Room* room) {
    if (state_ == 1) {
        return 1;
    }
    if (bMatchable_ != true) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_MATCH, 19);
        return 0;
    }

    const int match_error = _CheckMatchable(room);
    if (match_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_MATCH,
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

// IDA_EA: 0x855DC64
// IDA_NAME: _ZN12CLeagueMatch11OnMatchRoomEP8PvP_Roomi
// DEMANGLED: CLeagueMatch::OnMatchRoom(PvP_Room *,int)
int CLeagueMatch::OnMatchRoom(PvP_Room* room, int auth_value) {
    static const char kFunction[] =
        "virtual bool CLeagueMatch::OnMatchRoom(PvP_Room*, int)";

    if (auth_value_ != auth_value) {
        cMyTrace trace(kFunction, 0x295, 0);
        trace("OnMatchRoom timeout");
        return 0;
    }
    if (state_ != 1) {
        return 0;
    }

    IMatch* matched = 0;
    pWaitingRoomList_->Match(this, &matched);
    if (!matched) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    CUser* const matched_user = matched->GetUser(0);
    if (!matched_user) {
        cMyTrace trace(kFunction, 0x2b0, 0);
        trace("error file(%s) line(%d)", "MatchingSystem.cpp", 0x2b0);
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    PvP_Room* const matched_room = matched_user->GetPVPRoom();
    if (!matched_room) {
        cMyTrace trace(kFunction, 0x2b9, 0);
        trace("error file(%s) line(%d)", "MatchingSystem.cpp", 0x2b9);
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    const int player_count =
        static_cast<int>(nPlayerCountPerOneTeam_);
    if (room->get_player_count() > player_count) {
        cMyTrace trace(kFunction, 0x2c2, 0);
        trace("error file(%s) line(%d)", "MatchingSystem.cpp", 0x2c2);
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }
    if (teams_[0].size() > player_count ||
        matched_room->get_player_count() > player_count) {
        cMyTrace trace(kFunction, 0x2ca, 0);
        trace("error file(%s) line(%d)", "MatchingSystem.cpp", 0x2ca);
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    _BackupRoomInfo(matched_room);
    teams_[1].reset();
    matched_room->get_user_list(&teams_[1]);
    _SendMatchStatePacket(3, 2);
    state_ = 3;

    for (int source_seat = 0; source_seat <= 7; ++source_seat) {
        CUser* const user = matched_room->get_user_seat(source_seat);
        if (!user) {
            continue;
        }
        const int previous_team = matched_room->get_team(source_seat);
        _PullOutRoom(user, matched_room, source_seat);
        const int destination_seat = _PutInRoom(user, room, false);
        if (previous_team == 3) {
            room->OnSetPVPSeatState(
                user,
                destination_seat,
                static_cast<ENUM_SEAT_STATE>(3),
                static_cast<ENUM_WALKOUT_TYPE>(2));
        }
    }

    pWaitingRoomList_->DelRoom(this);
    state_ = 4;
    _SendMatchStatePacket(2, 2);
    return 1;
}

// IDA_EA: 0x855E378
// IDA_NAME: _ZN12CLeagueMatch11_ReturnTeamEP8PvP_RoomiP5CUser
// DEMANGLED: CLeagueMatch::_ReturnTeam(PvP_Room *,int,CUser *)
CLeagueMatch* CLeagueMatch::_ReturnTeam(PvP_Room* room,
                                        int retained_team,
                                        CUser* retained_user) {
    PvP_Room* new_room = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = room->get_user_seat(seat);
        if (!user) {
            continue;
        }

        if (retained_team == room->get_team(seat)) {
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
    return this;
}

// IDA_EA: 0x855E4C4
// IDA_NAME: _ZN12CLeagueMatch9OnWalkOutEP8PvP_Roomi
// DEMANGLED: CLeagueMatch::OnWalkOut(PvP_Room *,int)
int CLeagueMatch::OnWalkOut(PvP_Room* room, int seat) {
    OnLeaveRoom(room->get_user_seat(seat),
                room,
                room->get_team(seat),
                seat);
    return 1;
}

// IDA_EA: 0x855E55A
// IDA_NAME: _ZN12CLeagueMatch10OnEndFightEP8PvP_Room
// DEMANGLED: CLeagueMatch::OnEndFight(PvP_Room *)
void CLeagueMatch::OnEndFight(PvP_Room* room) {
    const int manager_team = room->get_manager_team();
    state_ = 0;
    _ReturnTeam(room, manager_team, 0);
    if (manager_team != 1) {
        teams_[0] = teams_[1];
    }
    teams_[0].refresh();
    teams_[1].reset();
}

// IDA_EA: 0x855E664
// IDA_NAME: _ZN12CLeagueMatch21_SendMatchStatePacketEii
// DEMANGLED: CLeagueMatch::_SendMatchStatePacket(int,int)
void CLeagueMatch::_SendMatchStatePacket(int state, int team_count) {
    PacketGuard packet;
    packet.put_header(0, 175);
    packet.put_byte(state);
    packet.finalize(true);

    for (int team = 0; team < team_count; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfo& player = teams_[team].players[index];
            if (player.active && player.user) {
                player.user->Send(packet);
            }
        }
    }
}
