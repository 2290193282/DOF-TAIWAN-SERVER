#include "old_game/pvp/match/league_match_static_state_runtime.h"

#include <cmath>
#include <cstring>
#include <new>

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/match/league_match_room_list_runtime.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

// IDA_EA: 0x8CA2A40
// IDA_NAME: _ZL13g_RankingIncr_8
// CLeagueMatch::GetGap indexes this four-float local table directly.  All
// four recovered IEEE-754 values are 1.0f; keeping the table still preserves
// the original indexed read and its out-of-range contract.
const float g_RankingIncr[4] = {1.0f, 1.0f, 1.0f, 1.0f};

}  // namespace

// IDA_EA: 0x855CD74
// IDA_NAME: _ZN12CLeagueMatchC2Ev
// DEMANGLED: CLeagueMatch::CLeagueMatch(void)
CLeagueMatch::CLeagueMatch()
    : IMatch(),
      room_info_() {
    // The old constructor deliberately leaves state_, auth_value_, and both
    // embedded TeamInfo instances untouched.  They are initialized by
    // OnCreateRoom, not by object construction.
    if (!pWaitingRoomList_) {
        pWaitingRoomList_ = new CLeagueMatchRoomList();
    }
    ++nWaitingRoomListRefCount_;
}

// IDA_EA: 0x855CE4C
// IDA_NAME: _ZN12CLeagueMatchD2Ev
// DEMANGLED: CLeagueMatch::~CLeagueMatch()
//
// The virtual destructor declaration causes the GCC i386 ABI deleting
// destructor (_ZN12CLeagueMatchD0Ev, IDA 0x855CEDA) to be emitted as the
// normal D0 wrapper around this complete destructor.
CLeagueMatch::~CLeagueMatch() {
    --nWaitingRoomListRefCount_;
    if (pWaitingRoomList_ && !nWaitingRoomListRefCount_) {
        // The target D2 invokes operator delete directly.  It does not call
        // CLeagueMatchRoomList::~CLeagueMatchRoomList and does not clear the
        // static pointer afterward.
        ::operator delete(pWaitingRoomList_);
    }
}

// IDA_EA: 0x855CEF8
// IDA_NAME: _ZN12CLeagueMatch12_CheckSchoolEP8PvP_RoomP5CUser
// DEMANGLED: CLeagueMatch::_CheckSchool(PvP_Room *,CUser *)
int CLeagueMatch::_CheckSchool(PvP_Room* room, CUser* user) {
    if (!user->GetSchoolNo()) {
        return 0;
    }

    int waiting_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            ++waiting_count;
        }
    }

    return teams_[0].size() - waiting_count <
           static_cast<int>(nPlayerCountPerOneTeam_);
}

// IDA_EA: 0x855DA56
// IDA_NAME: _ZNK12CLeagueMatch15_CheckMatchableEP8PvP_Room
// DEMANGLED: CLeagueMatch::_CheckMatchable(PvP_Room *)const
int CLeagueMatch::_CheckMatchable(PvP_Room* room) const {
    int waiting_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            ++waiting_count;
            continue;
        }

        CUser* const user = room->get_user_seat(seat);
        if (user && !user->GetSchoolNo()) {
            const int school_no = user->GetSchoolNo();
            LogManager::logFormat(
                1,
                "MatchingSystem.cpp",
                "int CLeagueMatch::_CheckMatchable(PvP_Room*) const",
                0x257,
                "(%d) found a different school no(%d)",
                0,
                school_no);
            return 22;
        }
    }

    return teams_[0].size() - waiting_count ==
                   static_cast<int>(nPlayerCountPerOneTeam_)
               ? 0
               : 22;
}

// IDA_EA: 0x855CF8C
// IDA_NAME: _ZN12CLeagueMatch13CheckJoinableEP8PvP_RoomP5CUser
// DEMANGLED: CLeagueMatch::CheckJoinable(PvP_Room *,CUser *)
int CLeagueMatch::CheckJoinable(PvP_Room* room, CUser* user) {
    const int max_player_count =
        2 * static_cast<int>(nPlayerCountPerOneTeam_);
    if (room->get_player_count() >= max_player_count) {
        return 4;
    }

    const int first_team_size = teams_[0].size();
    if (first_team_size + teams_[1].size() > max_player_count) {
        return 4;
    }

    switch (state_) {
    case 0:
        return _CheckSchool(room, user) ? 0 : 4;
    case 1:
    case 4:
    case 5:
        return 4;
    case 3:
        return 0;
    default:
        return 4;
    }
}

// IDA_EA: 0x855D058
// IDA_NAME: _ZN12CLeagueMatch9CheckSeatEi
// DEMANGLED: CLeagueMatch::CheckSeat(int)
int CLeagueMatch::CheckSeat(int seat) {
    return seat != 2;
}

// IDA_EA: 0x855D096
// IDA_NAME: _ZN12CLeagueMatch10RequestMapERsR15PVP_BATTLE_MODE
// DEMANGLED: CLeagueMatch::RequestMap(short &,PVP_BATTLE_MODE &)
void CLeagueMatch::RequestMap(int16_t& map_no, PVP_BATTLE_MODE& battle_mode) {
    map_no = 3;
    if (static_cast<int>(battle_mode) != 6) {
        battle_mode = static_cast<PVP_BATTLE_MODE>(nBattleMode_);
    }
}

// IDA_EA: 0x855D0BC
// IDA_NAME: _ZN12CLeagueMatch16GetExtraWinPointEi
// DEMANGLED: CLeagueMatch::GetExtraWinPoint(int)
int CLeagueMatch::GetExtraWinPoint(int value) {
    if (GlobalData::s_systemTime_.getCurSec() > 1225897200 &&
        GlobalData::s_systemTime_.getCurSec() <= 1227106799) {
        return value;
    }
    return 0;
}

// IDA_EA: 0x855E21C
// IDA_NAME: _ZN12CLeagueMatch12OnStartFightEv
// DEMANGLED: CLeagueMatch::OnStartFight(void)
void CLeagueMatch::OnStartFight() {
    // The original leaf only transitions the per-match state.  It has no
    // timer, room, packet, or reward-side effect.
    state_ = 5;
}

// IDA_EA: 0x855E52A
// IDA_NAME: _ZN12CLeagueMatch7OnReadyEP5CUserb
// DEMANGLED: CLeagueMatch::OnReady(CUser *,bool)
bool CLeagueMatch::OnReady(CUser*, bool ready) {
    // The CUser argument is intentionally unused in the original logic.
    // A ready notification is denied only while the match is in state 4.
    return ready || state_ != 4;
}

// IDA_EA: 0x855E22C
// IDA_NAME: _ZN12CLeagueMatch14OnCompleteLoadEP5CUser
// DEMANGLED: CLeagueMatch::OnCompleteLoad(CUser *)
void CLeagueMatch::OnCompleteLoad(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 129);
    packet.put_short(user->get_unique_id());
    packet.finalize(true);

    for (int team = 0; team <= 1; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfo& player = teams_[team].players[index];
            if (player.active && player.user) {
                player.user->Send(packet);
            }
        }
    }
}

// IDA_EA: 0x855D104
// IDA_NAME: _ZN12CLeagueMatch12OnCreateRoomEP5CUser
// DEMANGLED: CLeagueMatch::OnCreateRoom(CUser *)
int CLeagueMatch::OnCreateRoom(CUser* user) {
    auth_value_ = ++nextAuthValue_;
    state_ = 0;
    teams_[1].reset();
    teams_[0].reset();
    teams_[0].add(user);
    return 0;
}

// IDA_EA: 0x855D168
// IDA_NAME: _ZN12CLeagueMatch13OnDestroyRoomEP5CUser
// DEMANGLED: CLeagueMatch::OnDestroyRoom(CUser *)
void CLeagueMatch::OnDestroyRoom(CUser*) {
    state_ = 0;
    pWaitingRoomList_->DelRoom(this);
}

// IDA_EA: 0x855D198
// IDA_NAME: _ZN12CLeagueMatch10OnJoinRoomEP5CUser
// DEMANGLED: CLeagueMatch::OnJoinRoom(CUser *)
void CLeagueMatch::OnJoinRoom(CUser* user) {
    if (state_ != 3) {
        teams_[0].add(user);
    }
}

// IDA_EA: 0x855E12A
// IDA_NAME: _ZN12CLeagueMatch6GetGapEP6IMatch
// DEMANGLED: CLeagueMatch::GetGap(IMatch *)
float CLeagueMatch::GetGap(IMatch* match) {
    float gap = 0.0f;
    for (unsigned int index = 0;
         index < static_cast<unsigned int>(nPlayerCountPerOneTeam_);
         ++index) {
        PlayerInfo& player = teams_[0].players[index];
        CUser* const other_user = match->GetUser(index);
        if (player.active && player.user && other_user) {
            const float left_rating =
                player.user->GetRating() * g_RankingIncr[index];
            const float right_rating =
                other_user->GetRating() * g_RankingIncr[index];
            gap += std::fabs(left_rating - right_rating);
        }
    }
    return gap;
}

// IDA_EA: 0x855E5E4
// IDA_NAME: _ZN12CLeagueMatch7GetUserEj
// DEMANGLED: CLeagueMatch::GetUser(unsigned int)
CUser* CLeagueMatch::GetUser(uint32_t index) {
    if (index > 3 || teams_[0].players[index].active != 1u) {
        return 0;
    }
    return teams_[0].players[index].user;
}

// IDA_EA: 0x855E642
// IDA_NAME: _ZN12CLeagueMatch11GetUserListEPv
// DEMANGLED: CLeagueMatch::GetUserList(void *)
void CLeagueMatch::GetUserList(void* destination) {
    std::memcpy(destination, &teams_[0], sizeof(teams_[0]));
}

// IDA_EA: 0x855F9AA
// IDA_NAME: _ZN12CLeagueMatch11GetSchoolNoEv
// DEMANGLED: CLeagueMatch::GetSchoolNo(void)
int CLeagueMatch::GetSchoolNo() {
    for (int index = 0; index <= 3; ++index) {
        const PlayerInfo& player = teams_[0].players[index];
        if (player.active) {
            return static_cast<int>(player.school_no);
        }
    }
    return 0;
}

// IDA_EA: 0x855FA14
// IDA_NAME: _ZN12CLeagueMatch14CheckMatchableEP6IMatch
// DEMANGLED: CLeagueMatch::CheckMatchable(IMatch *)
bool CLeagueMatch::CheckMatchable(IMatch* match) {
    CLeagueMatch* const other = static_cast<CLeagueMatch*>(match);
    int own_highest_grade = 0;
    int other_highest_grade = 0;

    for (int index = 0; index <= 3; ++index) {
        const PlayerInfo& player = teams_[0].players[index];
        if (player.active && player.user) {
            if (player.user->getPVPResultR()->values[5] >=
                own_highest_grade) {
                own_highest_grade = player.user->getPVPResultR()->values[5];
            }
        }
    }

    for (int index = 0; index <= 3; ++index) {
        const PlayerInfo& player = other->teams_[0].players[index];
        if (player.active && player.user) {
            if (player.user->getPVPResultR()->values[5] >=
                other_highest_grade) {
                other_highest_grade = player.user->getPVPResultR()->values[5];
            }
        }
    }

    if (own_highest_grade > 11 && own_highest_grade <= 14 &&
        other_highest_grade <= 10) {
        return false;
    }
    if (other_highest_grade > 11 && other_highest_grade <= 14 &&
        own_highest_grade <= 10) {
        return false;
    }

    int grade_gap = own_highest_grade - other_highest_grade;
    if (grade_gap < 0) {
        grade_gap = -grade_gap;
    }
    if (grade_gap > 4) {
        return false;
    }

    for (int own_index = 0; own_index <= 3; ++own_index) {
        const PlayerInfo& own_player = teams_[0].players[own_index];
        for (int other_index = 0; other_index <= 3; ++other_index) {
            const PlayerInfo& other_player =
                other->teams_[0].players[other_index];
            if (own_player.active && other_player.active &&
                own_player.school_no == other_player.school_no) {
                return false;
            }
        }
    }
    return true;
}

// IDA_EA: 0x8568834
// IDA_NAME: _ZN12CLeagueMatch18GetIndentificationEv
// DEMANGLED: CLeagueMatch::GetIndentification(void)
int CLeagueMatch::GetIndentification() {
    return GetSchoolNo();
}
