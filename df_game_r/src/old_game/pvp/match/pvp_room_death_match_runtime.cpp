#include "old_game/pvp/arena/pvp_room.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "infra/log/format/trace.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x85DF55F
// IDA_NAME: _Z27funDeathMatchKillCntCompareRKSt4pairIiiES2_
// DEMANGLED: funDeathMatchKillCntCompare(std::pair<int,int> const&,
//                                        std::pair<int,int> const&)
bool funDeathMatchKillCntCompare(const std::pair<int, int>& left,
                                 const std::pair<int, int>& right) {
    return left.second > right.second;
}

// IDA_EA: 0x85DEE66
// IDA_NAME: _ZN20CDeathMatchBattleMgr12OnCreateRoomEi
void CDeathMatchBattleMgr::OnCreateRoom(int) {}

// IDA_EA: 0x85DEE78
// IDA_NAME: _ZN20CDeathMatchBattleMgr10OnJoinRoomEi
void CDeathMatchBattleMgr::OnJoinRoom(int) {}

// IDA_EA: 0x85DEE8A
// IDA_NAME: _ZN20CDeathMatchBattleMgr11OnLeaveRoomEi
void CDeathMatchBattleMgr::OnLeaveRoom(int) {}

// IDA_EA: 0x85DEE9C
// IDA_NAME: _ZN20CDeathMatchBattleMgr12OnChangeTeamEii
void CDeathMatchBattleMgr::OnChangeTeam(int, int) {}

// IDA_EA: 0x85DEEAE
// IDA_NAME: _ZN20CDeathMatchBattleMgr7OnReadyEib
int CDeathMatchBattleMgr::OnReady(int, bool ready) {
    return ready;
}

// IDA_EA: 0x85DEECA
// IDA_NAME: _ZN20CDeathMatchBattleMgr7OnStartEv
CDeathMatchBattleMgr* CDeathMatchBattleMgr::OnStart() {
    std::memset(kill_count_, 0, sizeof(kill_count_));
    std::memset(death_count_, 0, sizeof(death_count_));
    return this;
}

// IDA_EA: 0x85DEF0E
// IDA_NAME: _ZN20CDeathMatchBattleMgr17IncreaseKillCountEi
unsigned int CDeathMatchBattleMgr::IncreaseKillCount(unsigned int seat) {
    if (seat < 8u) {
        ++kill_count_[seat];
    }
    return seat;
}

// IDA_EA: 0x85DEF34
// IDA_NAME: _ZN20CDeathMatchBattleMgr18IncreaseDeathCountEi
unsigned int CDeathMatchBattleMgr::IncreaseDeathCount(unsigned int seat) {
    if (seat < 8u) {
        ++death_count_[seat];
    }
    return seat + 8u;
}

// IDA_EA: 0x85DEF60
// IDA_NAME: _ZN20CDeathMatchBattleMgr30InsertTimerKilledUserForReviveEii
int CDeathMatchBattleMgr::InsertTimerKilledUserForRevive(int room_index,
                                                         int seat) {
    const int key = parent_->gen_timer_key(
        static_cast<TIMER_MESSAGE>(seat + 51));
    return G_TimerQueue()->InsertTimer(5, room_index, seat + 51, 3, key, 0);
}

// IDA_EA: 0x85DEFC6
// IDA_NAME: _ZN20CDeathMatchBattleMgr7OnFightEv
void CDeathMatchBattleMgr::OnFight() {}

// IDA_EA: 0x85DEFCC
// IDA_NAME: _ZN20CDeathMatchBattleMgr15getMaxKillCountEv
int CDeathMatchBattleMgr::getMaxKillCount() {
    int maximum = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (kill_count_[seat] > maximum) {
            maximum = kill_count_[seat];
        }
    }
    return maximum;
}

// IDA_EA: 0x85DF012
// IDA_NAME: _ZN20CDeathMatchBattleMgr26checkGameOverForSinglePlayEi
bool CDeathMatchBattleMgr::checkGameOverForSinglePlay(int player_count) {
    if (!player_count) {
        return true;
    }
    switch (player_count) {
    case 2:
    case 3:
        return getMaxKillCount() > 1;
    case 4:
        return getMaxKillCount() > 2;
    case 5:
        return getMaxKillCount() > 3;
    case 6:
        return getMaxKillCount() > 4;
    case 7:
        return getMaxKillCount() > 5;
    case 8:
        return getMaxKillCount() > 6;
    default:
        return true;
    }
}

// IDA_EA: 0x85DF126
// IDA_NAME: _ZN20CDeathMatchBattleMgr24checkGameOverForTeamPlayEPP5CUseriPh
int CDeathMatchBattleMgr::checkGameOverForTeamPlay(CUser** users,
                                                   int team_count,
                                                   unsigned char* teams) {
    if (team_count == -1) {
        return 1;
    }

    int team_one_kills = 0;
    int team_two_kills = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (users[seat] || kill_count_[seat]) {
            if (teams[seat] == 1) {
                team_one_kills += kill_count_[seat];
            } else if (teams[seat] == 2) {
                team_two_kills += kill_count_[seat];
            } else {
                LogManager::logFormat(
                    1,
                    "pvp.cpp",
                    "bool CDeathMatchBattleMgr::checkGameOverForTeamPlay(CUser**, int, unsigned char*)",
                    0x157F,
                    "CDeathMatchBattleMgr::checkWinnerForTeamPlay , ERROR, bTeamState[%d] = %d",
                    seat,
                    teams[seat]);
            }
        }
    }

    const int maximum = team_one_kills > team_two_kills
                            ? team_one_kills
                            : team_two_kills;
    if (team_count == 1) {
        return maximum > 1;
    }
    if (team_count == 2) {
        return maximum > 4;
    }
    if (team_count == 3) {
        return maximum > 7;
    }
    if (team_count == 4) {
        return maximum > 9;
    }
    return 1;
}

// IDA_EA: 0x85DF296
// IDA_NAME: _ZN20CDeathMatchBattleMgr22checkWinnerForTeamPlayEPP5CUserPbPhR12PvpUserTable
bool CDeathMatchBattleMgr::checkWinnerForTeamPlay(CUser** users,
                                                  bool* live,
                                                  unsigned char* teams,
                                                  PvpUserTable& table) {
    int team_one_kills = 0;
    int team_two_kills = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (users[seat]) {
            if (teams[seat] == 1) {
                team_one_kills += kill_count_[seat];
            } else if (teams[seat] == 2) {
                team_two_kills += kill_count_[seat];
            } else {
                LogManager::logFormat(
                    1,
                    "pvp.cpp",
                    "void CDeathMatchBattleMgr::checkWinnerForTeamPlay(CUser**, bool*, unsigned char*, PvpUserTable&)",
                    0x1617,
                    "CDeathMatchBattleMgr::checkWinnerForTeamPlay , ERROR, bTeamState[%d] = %d",
                    seat,
                    teams[seat]);
            }
        }
    }

    int team_one_live = 0;
    int team_two_live = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (live[seat] && users[seat]) {
            if (teams[seat] == 1) {
                ++team_one_live;
            } else if (teams[seat] == 2) {
                ++team_two_live;
            }
        }
    }

    unsigned char winning_team;
    if (team_one_kills > team_two_kills) {
        winning_team = 1;
    } else if (team_one_kills < team_two_kills) {
        winning_team = 2;
    } else if (team_one_live > team_two_live) {
        winning_team = 1;
    } else if (team_one_live < team_two_live) {
        winning_team = 2;
    } else {
        winning_team = (std::rand() & 1) ? 2 : 1;
    }

    std::memset(live, 0, 8u);
    for (int seat = 0; seat <= 7; ++seat) {
        if (teams[seat] == winning_team) {
            live[seat] = true;
        }
    }
    return table.SetWinTeam(winning_team);
}

// IDA_EA: 0x85DF48A
// IDA_NAME: _ZN20CDeathMatchBattleMgr24checkWinnerForSinglePlayEPP5CUserPbR12PvpUserTable
bool CDeathMatchBattleMgr::checkWinnerForSinglePlay(CUser** users,
                                                    bool* live,
                                                    PvpUserTable& table) {
    for (int seat = 0; seat <= 7; ++seat) {
        if (!users[seat]) {
            kill_count_[seat] = 0;
        }
        live[seat] = false;
    }

    const unsigned int winner = static_cast<unsigned int>(
        std::max_element(kill_count_, kill_count_ + 8) - kill_count_);
    if (winner <= 7u) {
        live[winner] = true;
    }
    return table.SetWinTeam(static_cast<int>(winner));
}

// IDA_EA: 0x85DF576
// IDA_NAME: _ZNK20CDeathMatchBattleMgr10getRankingEPP5CUserPi
// DEMANGLED: CDeathMatchBattleMgr::getRanking(CUser **,int *)const
int CDeathMatchBattleMgr::getRanking(CUser** users, int* ranking) const {
    std::pair<int, int> ranked_seats[8];
    std::memset(ranking, 0, sizeof(int) * 8u);

    int user_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (users[seat]) {
            ranked_seats[seat] =
                std::make_pair(seat, kill_count_[seat]);
            ++user_count;
        } else {
            ranked_seats[seat] = std::make_pair(seat, -1);
        }
    }

    std::sort(ranked_seats,
              ranked_seats + 8,
              funDeathMatchKillCntCompare);
    for (int rank = 0; rank < user_count; ++rank) {
        ranking[rank] = ranked_seats[rank].first;
        cMyTrace trace(
            "int CDeathMatchBattleMgr::getRanking(CUser**, int*) const",
            5781,
            0);
        trace("CDeathMatchBattleMgr::getRanking => "
              "Room(%d), Idx(%d), Rank(%d)",
              parent_->get_index(),
              rank,
              ranking[rank]);
    }
    return user_count;
}

// IDA_EA: 0x85DF524
// IDA_NAME: _ZN20CDeathMatchBattleMgr12getKillCountEi
int CDeathMatchBattleMgr::getKillCount(int seat) {
    return seat > 7 ? 0 : kill_count_[seat];
}

// IDA_EA: 0x85DF540
// IDA_NAME: _ZN20CDeathMatchBattleMgr13getDeathCountEi
int CDeathMatchBattleMgr::getDeathCount(int seat) {
    return seat > 7 ? 0 : death_count_[seat];
}
