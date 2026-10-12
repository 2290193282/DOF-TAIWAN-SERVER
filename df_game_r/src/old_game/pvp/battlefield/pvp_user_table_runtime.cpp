#include "old_game/pvp/arena/pvp_room.h"

#include <algorithm>
#include <cstring>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x85D48F0
// IDA_NAME: _ZN12PvpUserTableC2Ev
// DEMANGLED: PvpUserTable::PvpUserTable(void)
PvpUserTable::PvpUserTable() {
    Clear();
}

// IDA_EA: 0x85D4904
// IDA_NAME: _ZN12PvpUserTableD2Ev
// DEMANGLED: PvpUserTable::~PvpUserTable()
PvpUserTable::~PvpUserTable() {
}

// IDA_EA: 0x85D490A
// IDA_NAME: _ZN12PvpUserTable5ClearEv
// DEMANGLED: PvpUserTable::Clear(void)
PvpUserTable* PvpUserTable::Clear() {
    for (int team = 0; team <= 7; ++team) {
        for (int slot = 0; slot <= 7; ++slot) {
            users[team][slot].user = 0;
            users[team][slot].won = false;
        }
    }
    win_team = -1;
    return this;
}

// IDA_EA: 0x85D4980
// IDA_NAME: _ZN12PvpUserTable3SetEiP5CUser
// DEMANGLED: PvpUserTable::Set(int,CUser *)
int PvpUserTable::Set(int team, CUser* user) {
    if (!IsValidTeam(team) || !user) {
        return 0;
    }

    const int slot = GetTeamUserCount(team);
    if (slot > 7) {
        return 0;
    }

    users[team][slot].user = user;
    users[team][slot].won = false;
    return 1;
}

// IDA_EA: 0x85D4A14
// IDA_NAME: _ZN12PvpUserTable11IsValidTeamEi
// DEMANGLED: PvpUserTable::IsValidTeam(int)
bool PvpUserTable::IsValidTeam(int team) {
    return team >= 0 && team < 8;
}

// IDA_EA: 0x85D4A32
// IDA_NAME: _ZN12PvpUserTable21IsValidConvertedGradeEi
// DEMANGLED: PvpUserTable::IsValidConvertedGrade(int)
bool PvpUserTable::IsValidConvertedGrade(int grade) {
    return grade >= 0 && grade <= 20;
}

// IDA_EA: 0x85D4A50
// IDA_NAME: _ZN12PvpUserTable16GetTeamUserCountEi
// DEMANGLED: PvpUserTable::GetTeamUserCount(int)
int PvpUserTable::GetTeamUserCount(int team) {
    if (!IsValidTeam(team)) {
        return -1;
    }

    int user_count = 0;
    for (int slot = 0; slot <= 7; ++slot) {
        if (users[team][slot].user) {
            ++user_count;
        }
    }
    return user_count;
}

// IDA_EA: 0x85D4AB8
// IDA_NAME: _ZN12PvpUserTable12GetExpectValEiiP5CUser
// DEMANGLED: PvpUserTable::GetExpectVal(int,int,CUser *)
long double PvpUserTable::GetExpectVal(int grade_a,
                                       int grade_b,
                                       CUser* comparison_user) {
    int converted_grade_a = GradeFilter(grade_a);
    int converted_grade_b = GradeFilter(grade_b);
    bool use_original_grade = false;

    if (((converted_grade_a == 1 && !converted_grade_b) ||
         (!converted_grade_a && converted_grade_b == 1)) &&
        (!comparison_user || comparison_user->get_pvp_grade() > 9)) {
        converted_grade_a = grade_a;
        converted_grade_b = grade_b;
        use_original_grade = true;
    }

    if (!IsValidConvertedGrade(converted_grade_a) ||
        !IsValidConvertedGrade(converted_grade_b)) {
        return -1.0L;
    }

    if (converted_grade_a == converted_grade_b || use_original_grade) {
        return static_cast<long double>(converted_grade_a - converted_grade_b) *
                   0.024L +
               0.5L;
    }
    if (!converted_grade_a) {
        return 0.0L;
    }
    if (converted_grade_b) {
        return static_cast<long double>(converted_grade_a - converted_grade_b) *
                   0.024L +
               0.5L;
    }
    return 1.0L;
}

// IDA_EA: 0x85D4BD0
// IDA_NAME: _ZN12PvpUserTable9CalculateEv
// DEMANGLED: PvpUserTable::Calculate(void)
int PvpUserTable::Calculate() {
    std::memset(team_vs_team_point, 0, sizeof(team_vs_team_point));
    std::memset(expected_value_sum, 0, sizeof(expected_value_sum));
    CheckDan();

    for (int winning_team = 0; winning_team <= 7; ++winning_team) {
        SetWinTeam(winning_team);
        for (int opposing_team = 0; opposing_team <= 7; ++opposing_team) {
            if (winning_team != opposing_team) {
                GetTeamVsTeamPoint(winning_team, opposing_team);
            }
        }
    }

    for (int team = 0; team <= 7; ++team) {
        leave_penalty[team] = 0.0f;
        for (int opposing_team = 0; opposing_team <= 7; ++opposing_team) {
            leave_penalty[team] = std::min(
                leave_penalty[team], team_vs_team_point[opposing_team][team]);
        }
    }

    for (int team = 0; team <= 7; ++team) {
        for (int slot = 0; slot <= 7; ++slot) {
            users[team][slot].user = 0;
        }
    }
    return 1;
}

// IDA_EA: 0x85D4D5E
// IDA_NAME: _ZN12PvpUserTable18GetTeamVsTeamPointEii
// DEMANGLED: PvpUserTable::GetTeamVsTeamPoint(int,int)
int PvpUserTable::GetTeamVsTeamPoint(int team, int opposing_team) {
    const int team_user_count = GetTeamUserCount(team);
    const int opposing_user_count = GetTeamUserCount(opposing_team);
    if (!team_user_count || !opposing_user_count) {
        return 0;
    }
    if (!users[team][0].won) {
        return 0;
    }

    expected_value_sum[team] = 0.0f;
    expected_value_sum[opposing_team] = 0.0f;
    for (int slot = 0; slot <= 7; ++slot) {
        CUser* comparison_user = users[team][slot].user;
        GetUserVsTeamPoint(slot, team, opposing_team, comparison_user);
        GetUserVsTeamPoint(slot, opposing_team, team, comparison_user);
    }

    if (team_user_count > opposing_user_count) {
        const int ghost_count = team_user_count - opposing_user_count;
        for (int ghost = 0; ghost < ghost_count; ++ghost) {
            GetGhostVsTeamPoint(team, opposing_team);
        }
    }

    const float ratio_sum = expected_value_sum[team] +
                            expected_value_sum[opposing_team];
    if (ratio_sum == 0.0f) {
        cMyTrace trace("bool PvpUserTable::GetTeamVsTeamPoint(int, int)",
                       354,
                       5);
        trace("[FPE][PvpUserTable::GetTeamVsTeamPoint] RatioSum: %f", ratio_sum);
        return 0;
    }

    float adjusted_ratio = expected_value_sum[opposing_team] / ratio_sum;
    adjusted_ratio = static_cast<float>(team_user_count + opposing_user_count) *
                     adjusted_ratio;
    team_vs_team_point[team][team] += adjusted_ratio / team_user_count;
    team_vs_team_point[team][opposing_team] -=
        adjusted_ratio / opposing_user_count;
    return 1;
}

// IDA_EA: 0x85D4FCA
// IDA_NAME: _ZN12PvpUserTable19GetGhostVsTeamPointEii
// DEMANGLED: PvpUserTable::GetGhostVsTeamPoint(int,int)
int PvpUserTable::GetGhostVsTeamPoint(int team, int /* opposing_team */) {
    for (int slot = 0; slot <= 7; ++slot) {
        CUser* user = users[team][slot].user;
        if (user) {
            expected_value_sum[team] +=
                static_cast<float>(GetExpectVal(user->get_pvp_grade(), 0, 0));
        }
    }
    return 1;
}

// IDA_EA: 0x85D5064
// IDA_NAME: _ZN12PvpUserTable9GetResultEibi
// DEMANGLED: PvpUserTable::GetResult(int,bool,int)
int PvpUserTable::GetResult(int team, bool special_result, int pvp_grade) {
    if (!IsValidTeam(team)) {
        return -1;
    }

    if (special_result) {
        if (team != 2 && team != 1) {
            // IDA calls cMyTrace with internal data label `format__13` here.
            // The export has no recoverable literal, so preserve the state
            // branch without inventing diagnostic text.
            return 0;
        }

        const int other_team = team == 1 ? 2 : 1;
        if (win_team == team && !has_dan[other_team] &&
            team_vs_team_point[win_team][team] != 0.0f) {
            return 0;
        }
    }

    const float point = team_vs_team_point[win_team][team];
    if (G_GameWorld()->IsCharacterPvPExpRevisionChannel()) {
        if (win_team == team) {
            const int result = static_cast<int>(point * 1000.0f * 120.0f / 100.0f);
            if (result < -100000 || result > 100000) {
                cMyTrace trace("int PvpUserTable::GetResult(int, bool, int)",
                               464,
                               5);
                trace("pvp_exp_error3 %d, %f", result, point);
            }
            return result;
        }

        if (pvp_grade > 14) {
            const int result = static_cast<int>(point * 1000.0f * 40.0f / 100.0f);
            if (result < -100000 || result > 100000) {
                cMyTrace trace("int PvpUserTable::GetResult(int, bool, int)",
                               452,
                               5);
                trace("pvp_exp_error2 %d, %f", result, point);
            }
            return result;
        }

        const float own_team_point = team_vs_team_point[team][team];
        const int result =
            static_cast<int>(own_team_point * 1000.0f * 40.0f / 100.0f);
        if (result < -100000 || result > 100000) {
            cMyTrace trace("int PvpUserTable::GetResult(int, bool, int)", 441, 5);
            trace("pvp_exp_error1 %d, %f", result, own_team_point);
        }
        return result;
    }

    const int result = static_cast<int>(point * 1000.0f);
    if (result < -100000 || result > 100000) {
        cMyTrace trace("int PvpUserTable::GetResult(int, bool, int)", 477, 5);
        trace("pvp_exp_error4 %d, %f", result, point);
    }
    return result;
}

// IDA_EA: 0x85D5730
// IDA_NAME: _ZN12PvpUserTable15GetLeavePenaltyEi
// DEMANGLED: PvpUserTable::GetLeavePenalty(int)
int PvpUserTable::GetLeavePenalty(int team) {
    if (!IsValidTeam(team)) {
        return -1;
    }
    return static_cast<int>(leave_penalty[team] * 1000.0f);
}

// IDA_EA: 0x85D578A
// IDA_NAME: _ZN12PvpUserTable11GradeFilterEi
// DEMANGLED: PvpUserTable::GradeFilter(int)
int PvpUserTable::GradeFilter(int grade) {
    if (static_cast<unsigned int>(grade) > 0x22u) {
        return -1;
    }
    if (grade > 20) {
        return 20;
    }
    if (grade > 9) {
        return grade - 9;
    }
    return 0;
}

// IDA_EA: 0x85D57C2
// IDA_NAME: _ZN12PvpUserTable8CheckDanEv
// DEMANGLED: PvpUserTable::CheckDan(void)
bool PvpUserTable::CheckDan() {
    std::memset(has_dan, 0, sizeof(has_dan));
    for (int team = 0; team <= 7; ++team) {
        for (int slot = 0; slot <= 7; ++slot) {
            CUser* user = users[team][slot].user;
            if (user && user->get_pvp_grade() > 9) {
                has_dan[team] = true;
                break;
            }
        }
    }

    // IDA leaves the terminating `team <= 7` comparison in AL, so this
    // member returns false after completing the state update.
    return false;
}

// IDA_EA: 0x85D585E
// IDA_NAME: _ZN12PvpUserTable10SetWinTeamEi
// DEMANGLED: PvpUserTable::SetWinTeam(int)
bool PvpUserTable::SetWinTeam(int team) {
    win_team = team;
    for (int current_team = 0; current_team <= 7; ++current_team) {
        for (int slot = 0; slot <= 7; ++slot) {
            users[current_team][slot].won = current_team == team;
        }
    }

    // As above, the original loop's terminal comparison is the return value.
    return false;
}

// IDA_EA: 0x85DF7A4
// IDA_NAME: _ZN12PvpUserTable10GetWinTeamEv
// DEMANGLED: PvpUserTable::GetWinTeam(void)
int PvpUserTable::GetWinTeam() {
    return win_team;
}

// IDA_EA: 0x85DFDDC
// IDA_NAME: _ZN12PvpUserTable18GetUserVsTeamPointEiiiP5CUser
// DEMANGLED: PvpUserTable::GetUserVsTeamPoint(int,int,int,CUser *)
int PvpUserTable::GetUserVsTeamPoint(int user_index,
                                     int team,
                                     int opposing_team,
                                     CUser* comparison_user) {
    CUser* user = users[team][user_index].user;
    if (!user) {
        return 0;
    }

    for (int opposing_slot = 0; opposing_slot <= 7; ++opposing_slot) {
        CUser* opposing_user = users[opposing_team][opposing_slot].user;
        if (opposing_user) {
            expected_value_sum[team] += static_cast<float>(GetExpectVal(
                user->get_pvp_grade(),
                opposing_user->get_pvp_grade(),
                comparison_user));
        }
    }
    return 1;
}
