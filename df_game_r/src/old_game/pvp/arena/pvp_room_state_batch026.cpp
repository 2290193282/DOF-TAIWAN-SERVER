#include "old_game/pvp/arena/pvp_room.h"

#include <string.h>

#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x85D65B4
// IDA_NAME: _ZN8PvP_Room15check_authorityEP5CUseri
// DEMANGLED: PvP_Room::check_authority(CUser *,int)
bool PvP_Room::check_authority(CUser* user, int seat) {
    return manager_ == user || users_[seat] == user;
}

// IDA_EA: 0x85D65E8
// IDA_NAME: _ZN8PvP_Room17check_start_stateEv
// DEMANGLED: PvP_Room::check_start_state(void)
int PvP_Room::check_start_state() {
    lock();
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] && IsPvpObserver(seat) != 1 &&
            ready_flags_[seat] != 1) {
            unlock();
            return 0;
        }
    }
    unlock();
    return 1;
}

// IDA_EA: 0x85D6680
// IDA_NAME: _ZN8PvP_Room14check_team_pvpEv
// DEMANGLED: PvP_Room::check_team_pvp(void)
bool PvP_Room::check_team_pvp() {
    const int mode = static_cast<int>(battle_mode_);
    if (mode != 2 && mode != 3 && mode != 5)
        return true;
    if (G_GameWorld()->GetChannelType() == 7)
        return true;

    int team_one = 0;
    int team_two = 0;
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] && IsPvpObserver(seat) != 1) {
            const int team = teams_[seat];
            if (team == 1)
                ++team_one;
            else if (team == 2)
                ++team_two;
        }
    }
    return team_one == team_two;
}

// IDA_EA: 0x85D7D9C
// IDA_NAME: _ZN8PvP_Room11is_last_winEi
// DEMANGLED: PvP_Room::is_last_win(int)
bool PvP_Room::is_last_win(int seat) {
    CUser* user = users_[seat];
    if (!user)
        return false;
    return user_table_.GetWinTeam() == GetTeamIndex(user);
}

// IDA_EA: 0x85D7DF0
// IDA_NAME: _ZN8PvP_Room24change_manager_to_winnerEv
// DEMANGLED: PvP_Room::change_manager_to_winner(void)
int PvP_Room::change_manager_to_winner() {
    if (!manager_)
        return 0;
    GameWorld* world = G_GameWorld();
    if (world->GetChannelType() == 15 || world->GetChannelType() == 7 ||
        world->IsSchoolPvPChannel())
        return 1;
    if (is_last_win(manager_seat_))
        return 1;
    for (int seat = 0; seat < 8; ++seat) {
        CUser* user = users_[seat];
        if (user && is_last_win(seat)) {
            manager_ = user;
            manager_seat_ = seat;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85D7EF4
// IDA_NAME: _ZN8PvP_Room18select_new_managerEv
// DEMANGLED: PvP_Room::select_new_manager(void)
int PvP_Room::select_new_manager() {
    for (int seat = 0; seat < 8; ++seat) {
        CUser* user = users_[seat];
        if (user && user != manager_) {
            manager_ = user;
            manager_seat_ = seat;
            ready_flags_[seat] = 0;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85D9296
// IDA_NAME: _ZN8PvP_Room13get_room_infoEP13PVP_ROOM_INFO
// DEMANGLED: PvP_Room::get_room_info(PVP_ROOM_INFO *)
void PvP_Room::get_room_info(PVP_ROOM_INFO* room_info) {
    room_info->room_title = room_title_;
    room_info->room_title_mode = room_title_mode_;
    room_info->requested_map_no = requested_map_no_;
    room_info->password_enabled = password_exists_;
    room_info->password = password_;
}

// IDA_EA: 0x85D92FC
// IDA_NAME: _ZN8PvP_Room7end_pvpEv
// DEMANGLED: PvP_Room::end_pvp(void)
int PvP_Room::end_pvp() {
    void* handler = current_match_;
    typedef int (*Function)(void*, PvP_Room*);
    void** vtable = *reinterpret_cast<void***>(handler);
    return reinterpret_cast<Function>(vtable[16])(handler, this);
}

// IDA_EA: 0x85D932A
// IDA_NAME: _ZN8PvP_Room13get_user_listEPv
// DEMANGLED: PvP_Room::get_user_list(void *)
int PvP_Room::get_user_list(void* user_list) {
    void* handler = current_match_;
    typedef int (*Function)(void*, void*);
    void** vtable = *reinterpret_cast<void***>(handler);
    return reinterpret_cast<Function>(vtable[5])(handler, user_list);
}

// IDA_EA: 0x85D9358
// IDA_NAME: _ZN8PvP_Room19get_extra_win_pointEi
// DEMANGLED: PvP_Room::get_extra_win_point(int)
int PvP_Room::get_extra_win_point(int win_point) {
    void* handler = current_match_;
    typedef int (*Function)(void*, int);
    void** vtable = *reinterpret_cast<void***>(handler);
    return reinterpret_cast<Function>(vtable[7])(handler, win_point);
}

// IDA_EA: 0x85DA1D8
// IDA_NAME: _ZN8PvP_Room31CalculateVictoryPointOfTeamPlayEibi
// DEMANGLED: PvP_Room::CalculateVictoryPointOfTeamPlay(int,bool,int)
int PvP_Room::CalculateVictoryPointOfTeamPlay(int rank,
                                               bool won,
                                               int point) {
    if (won && rank == 0)
        return point + 2;
    if (won)
        return point;
    return 1;
}

// IDA_EA: 0x85DA210
// IDA_NAME: _ZN8PvP_Room20CalcuateVictoryPointEPii
// DEMANGLED: PvP_Room::CalcuateVictoryPoint(int *,int)
void PvP_Room::CalcuateVictoryPoint(int* ranking, int ranking_count) {
    if (battle_mode_ == 1 || battle_mode_ == 4) {
        for (int seat = 0; seat <= 7; ++seat) {
            CUser* const user = users_[seat];
            if (!user || IsPvpObserver(seat)) {
                continue;
            }

            int rank = 0;
            while (rank < ranking_count && ranking[rank] != seat) {
                ++rank;
            }
            if (rank == ranking_count) {
                cMyTrace trace(
                    "void PvP_Room::CalcuateVictoryPoint(int*, int)",
                    2940,
                    5);
                trace("1 PvP_Room::CalcuateVictoryPoint Error, "
                      "No Rank User Found i<%d> RankCount<%d>",
                      seat,
                      ranking_count);
                return;
            }

            const bool won = user_table_.GetWinTeam() == seat;
            user->pending_pvp_win_point_ =
                CalculateVictoryPointOfSinglePlay(
                    rank, won, battle_player_count_);
        }
        return;
    }

    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = users_[seat];
        if (!user || IsPvpObserver(seat)) {
            continue;
        }

        const int team = GetTeamIndex(user);
        int rank = 0;
        while (rank < ranking_count && ranking[rank] != seat) {
            ++rank;
        }
        if (rank == ranking_count) {
            cMyTrace trace(
                "void PvP_Room::CalcuateVictoryPoint(int*, int)",
                2967,
                5);
            trace("1 PvP_Room::CalcuateVictoryPoint Error, "
                  "No Rank User Found i<%d> RankCount<%d>",
                  seat,
                  ranking_count);
            return;
        }

        int adjusted_rank = rank;
        for (int previous = rank - 1; previous >= 0; --previous) {
            const int previous_seat = ranking[previous];
            CUser* const previous_user = users_[previous_seat];
            if (!previous_user || IsPvpObserver(previous_seat)) {
                continue;
            }
            if (GetTeamIndex(previous_user) != team) {
                --adjusted_rank;
            }
        }

        const bool won = user_table_.GetWinTeam() == team;
        user->pending_pvp_win_point_ =
            CalculateVictoryPointOfTeamPlay(
                adjusted_rank, won, battle_player_count_ / 2);
    }
}

// IDA_EA: 0x85DA58A
// IDA_NAME: _ZN8PvP_Room18IsAllMoreThanGradeEi
// DEMANGLED: PvP_Room::IsAllMoreThanGrade(int)
int PvP_Room::IsAllMoreThanGrade(int grade) {
    if (static_cast<unsigned int>(grade) >= 0x26u)
        return 0;
    for (int seat = 0; seat < 8; ++seat) {
        CUser* user = users_[seat];
        if (user && user->get_pvp_grade() < grade)
            return 0;
    }
    return 1;
}

// IDA_EA: 0x85DA5F4
// IDA_NAME: _ZN8PvP_Room22IsThereSameGuildMemberEv
// DEMANGLED: PvP_Room::IsThereSameGuildMember(void)
int PvP_Room::IsThereSameGuildMember() {
    const int mode = static_cast<int>(battle_mode_);
    if (mode == 2 || mode == 3) {
        int guilds_one[4] = {0, 0, 0, 0};
        int guilds_two[4] = {0, 0, 0, 0};
        int count_one = 0;
        int count_two = 0;
        for (int seat = 0; seat < 8; ++seat) {
            CUser* user = users_[seat];
            if (!user)
                continue;
            const int team = teams_[seat];
            if (team == 1)
                guilds_one[count_one++] = user->get_charac_guildkey();
            else if (team == 2)
                guilds_two[count_two++] = user->get_charac_guildkey();
        }
        if (!count_one || !count_two)
            return 0;
        for (int one = 0; one < count_one; ++one)
            for (int two = 0; two < count_two; ++two)
                if (guilds_one[one] == guilds_two[two])
                    return 1;
        return 0;
    }

    int previous_guild = 0;
    for (int seat = 0; seat < 8; ++seat) {
        CUser* user = users_[seat];
        if (!user)
            continue;
        const int guild = user->get_charac_guildkey();
        if (guild) {
            if (previous_guild == guild)
                return 1;
            previous_guild = guild;
        }
    }
    return 0;
}

// IDA_EA: 0x85DA7CC
// IDA_NAME: _ZN8PvP_Room25check_acquire_guild_pointEv
// DEMANGLED: PvP_Room::check_acquire_guild_point(void)
bool PvP_Room::check_acquire_guild_point() {
    GameWorld* world = G_GameWorld();
    if (!world->IsPVPChannel() || world->IsFreePvPChannel())
        return false;
    return IsAllMoreThanGrade(10) == 1 && IsThereSameGuildMember() == 0;
}

// IDA_EA: 0x85DA840
// IDA_NAME: _ZN8PvP_Room14count_pvp_userEv
// DEMANGLED: PvP_Room::count_pvp_user(void)
int PvP_Room::count_pvp_user() {
    int count = 0;
    for (int seat = 0; seat < 8; ++seat)
        if (users_[seat])
            ++count;
    return count;
}

// IDA_EA: 0x85DBF82
// IDA_NAME: _ZN8PvP_Room22set_recv_pvp_rank_flagEP5CUser
// DEMANGLED: PvP_Room::set_recv_pvp_rank_flag(CUser *)
int PvP_Room::set_recv_pvp_rank_flag(CUser* user) {
    const int seat = get_user_seat(user);
    if (seat < 0)
        return 0;
    if (recv_pvp_rank_.IsRecv(seat))
        return 0;
    recv_pvp_rank_.SetRecvFlag(seat);
    return 1;
}

// IDA_EA: 0x85DBFEC
// IDA_NAME: _ZN8PvP_Room28set_recv_pvp_end_result_flagEP5CUser
// DEMANGLED: PvP_Room::set_recv_pvp_end_result_flag(CUser *)
int PvP_Room::set_recv_pvp_end_result_flag(CUser* user) {
    const int seat = get_user_seat(user);
    if (seat < 0)
        return 0;
    recv_pvp_end_result_.SetRecvFlag(seat);
    return 1;
}

// IDA_EA: 0x85DC034
// IDA_NAME: _ZN8PvP_Room12SetPVPResultEiRK13PvpResultType
// DEMANGLED: PvP_Room::SetPVPResult(int,PvpResultType const&)
void PvP_Room::SetPVPResult(int seat, const PvpResultType& result) {
    if (static_cast<unsigned int>(seat) < 8u) {
        battle_scores_[seat] = result.GetTotalScore();
        battle_kill_counts_[seat] = result.values[6];
    }
}

// IDA_EA: 0x85DC07E
// IDA_NAME: _ZNK8PvP_Room23recv_pvp_end_result_allEv
// DEMANGLED: PvP_Room::recv_pvp_end_result_all(void)const
int PvP_Room::recv_pvp_end_result_all() const {
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] && teams_[seat] != 255 &&
            recv_pvp_end_result_.IsRecv(seat) != 1)
            return 0;
    }
    return 1;
}

// IDA_EA: 0x85DC0F0
// IDA_NAME: _ZNK8PvP_Room24recv_pvp_end_result_teamEP5CUser
// DEMANGLED: PvP_Room::recv_pvp_end_result_team(CUser *)const
int PvP_Room::recv_pvp_end_result_team(CUser* user) const {
    int team = 255;
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] == user) {
            team = teams_[seat];
            break;
        }
    }
    if (team == 255)
        return 0;
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] && teams_[seat] == team &&
            recv_pvp_end_result_.IsRecv(seat) != 1)
            return 0;
    }
    return 1;
}

// IDA_EA: 0x85DC1C8
// IDA_NAME: _ZNK8PvP_Room29get_recv_pvp_end_result_countEv
// DEMANGLED: PvP_Room::get_recv_pvp_end_result_count(void)const
int PvP_Room::get_recv_pvp_end_result_count() const {
    int count = 0;
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] && teams_[seat] != 255 &&
            recv_pvp_end_result_.IsRecv(seat))
            ++count;
    }
    return count;
}

// IDA_EA: 0x85DC4BA
// IDA_NAME: _ZNK8PvP_Room17recv_pvp_rank_allEv
// DEMANGLED: PvP_Room::recv_pvp_rank_all(void)const
int PvP_Room::recv_pvp_rank_all() const {
    for (int seat = 0; seat < 8; ++seat) {
        if (users_[seat] && teams_[seat] != 255 &&
            recv_pvp_rank_.IsRecv(seat) != 1)
            return 0;
    }
    return 1;
}

// IDA_EA: 0x85DC52C
// IDA_NAME: _ZNK8PvP_Room10GetRankingEPi
// DEMANGLED: PvP_Room::GetRanking(int *)const
int PvP_Room::GetRanking(int* ranking) const {
    if (battle_mode_ == 4 || battle_mode_ == 5) {
        return death_match_battle_mgr_.getRanking(
            const_cast<CUser**>(users_), ranking);
    }

    int sorted_scores[8];
    int ranking_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (!users_[seat]) {
            continue;
        }

        const int score = battle_scores_[seat];
        int insert_at = 0;
        while (insert_at < ranking_count &&
               sorted_scores[insert_at] >= score) {
            ++insert_at;
        }
        for (int move = ranking_count; move > insert_at; --move) {
            ranking[move] = ranking[move - 1];
            sorted_scores[move] = sorted_scores[move - 1];
        }
        ranking[insert_at] = seat;
        sorted_scores[insert_at] = score;
        ++ranking_count;
    }
    return ranking_count;
}

// IDA_EA: 0x85DC64C
// IDA_NAME: _ZNK8PvP_Room6GetAceEv
// DEMANGLED: PvP_Room::GetAce(void)const
int PvP_Room::GetAce() const {
    int ace = -1;
    int highest_kill_count = -1;
    for (int seat = 0; seat < 8; ++seat) {
        if (!users_[seat])
            continue;
        const int kills = battle_kill_counts_[seat];
        if (kills > highest_kill_count ||
            (kills == highest_kill_count &&
             battle_scores_[ace] < battle_scores_[seat])) {
            highest_kill_count = kills;
            ace = seat;
        }
    }
    return ace;
}

// IDA_EA: 0x85DC6EE
// IDA_NAME: _ZN8PvP_Room12GetTeamIndexEP5CUser
// DEMANGLED: PvP_Room::GetTeamIndex(CUser *)
int PvP_Room::GetTeamIndex(CUser* user) {
    int seat = 0;
    while (seat < 8 && users_[seat] != user)
        ++seat;
    if (seat == 8)
        return -1;
    const int mode = static_cast<int>(battle_mode_);
    if (mode == 1 || mode == 4)
        return seat;
    return battle_teams_[seat];
}

// IDA_EA: 0x85DC778
// IDA_NAME: _ZN8PvP_Room19calculate_pvp_pointEv
// DEMANGLED: PvP_Room::calculate_pvp_point(void)
int PvP_Room::calculate_pvp_point() {
    user_table_.Clear();
    const int mode = static_cast<int>(battle_mode_);
    for (int seat = 0; seat < 8; ++seat) {
        CUser* user = users_[seat];
        if (user && !IsPvpObserver(seat)) {
            const int team =
                (mode == 1 || mode == 4)
                    ? seat
                    : static_cast<int>(battle_teams_[seat]);
            user_table_.Set(team, user);
        }
    }
    return user_table_.Calculate();
}

// IDA_EA: 0x85DCA48
// IDA_NAME: _ZN8PvP_Room14end_pvp_resultEv
// DEMANGLED: PvP_Room::end_pvp_result(void)
int PvP_Room::end_pvp_result() {
    change_manager_to_winner();
    battle_state_ = 1;
    for (int seat = 0; seat < 8; ++seat) {
        CUser* user = users_[seat];
        if (user)
            user->set_state(6);
    }
    return SkipLastRankResultTimer();
}

// IDA_EA: 0x85DCABA
// IDA_NAME: _ZN8PvP_Room23SkipLastRankResultTimerEv
// DEMANGLED: PvP_Room::SkipLastRankResultTimer(void)
int PvP_Room::SkipLastRankResultTimer() {
    return gen_timer_key(static_cast<TIMER_MESSAGE>(50));
}

// IDA_EA: 0x85DCAD6
// IDA_NAME: _ZN8PvP_Room14CancelAllTimerEv
// DEMANGLED: PvP_Room::CancelAllTimer(void)
int PvP_Room::CancelAllTimer() {
    return gen_timer_key(static_cast<TIMER_MESSAGE>(50));
}

// IDA_EA: 0x85DDEAE
// IDA_NAME: _ZN8PvP_Room13CheckPasswordEPKci
// DEMANGLED: PvP_Room::CheckPassword(char const*,int)
bool PvP_Room::CheckPassword(const char* password, int length) {
    return password_length_ == length &&
           memcmp(password_, password, length) == 0;
}

// IDA_EA: 0x85DDEF6
// IDA_NAME: _ZN8PvP_Room22OnConnectP2PPvPTimeoutEv
// DEMANGLED: PvP_Room::OnConnectP2PPvPTimeout(void)
int PvP_Room::OnConnectP2PPvPTimeout() {
    const int lacks_ready_user = check_start_state() ^ 1;
    if (lacks_ready_user) {
        void* handler = current_match_;
        typedef int (*Function)(void*, PvP_Room*);
        void** vtable = *reinterpret_cast<void***>(handler);
        return reinterpret_cast<Function>(vtable[22])(handler, this);
    }
    return lacks_ready_user;
}
