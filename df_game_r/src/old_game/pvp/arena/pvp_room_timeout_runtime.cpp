#include "old_game/pvp/arena/pvp_room.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "old_game/pvp/battlefield/pvp_reference_public_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/system_time.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"

// IDA_EA: 0x85D9B0E
// IDA_NAME: _ZN8PvP_Room8time_outEPi
// DEMANGLED: PvP_Room::time_out(int *)
int PvP_Room::time_out(int* scores) {
    if (timed_out_) {
        return 0;
    }
    timed_out_ = 1;

    const bool tournament_channel = G_GameWorld()->GetChannelType() == 7;
    const int play_time =
        GlobalData::s_systemTime_.getCurSec() - battle_start_time_sec_;
    int end_time = 0;
    if (battle_mode_ >= 1 && battle_mode_ <= 2) {
        end_time = PVP_NORMAL_BATTLE_TIME[tournament_channel] / 1000;
    } else if (battle_mode_ >= 4 && battle_mode_ <= 5) {
        end_time = PVP_DM_BATTLE_TIME[tournament_channel] / 1000;
    }
    if (end_time - play_time > 0) {
        cMyTrace trace("int PvP_Room::time_out(int*)", 2714, 0);
        trace("PVP_TIMEOUT_CHECK end_time(%d), play_time(%d)",
              end_time,
              play_time);
    }

    if (battle_state_ != 2) {
        return 19;
    }

    if (zombie_mode::FinishByTimeout(this)) {
        pvp_request_pvp_rank();
        return 0;
    }

    bool all_equal = true;
    for (int seat = 0; seat <= 6; ++seat) {
        if (scores[seat] != scores[seat + 1]) {
            all_equal = false;
            break;
        }
    }

    switch (battle_mode_) {
    case 2:
    case 3: {
        lock();
        if (all_equal) {
            for (int seat = 0; seat <= 7; ++seat) {
                battle_live_flags_[seat] = 0;
            }
        } else {
            int team_member_count[5] = {0, 0, 0, 0, 0};
            int team_score[5] = {0, 0, 0, 0, 0};
            for (int seat = 0; seat <= 7; ++seat) {
                const unsigned char team = battle_teams_[seat];
                if (users_[seat] && team && team <= 0xfd) {
                    ++team_member_count[team];
                    team_score[team] += scores[seat];
                }
            }

            int winning_team = 0;
            int minimum_score = 0x7fffffff;
            for (int team = 1; team <= 4; ++team) {
                if (team_member_count[team] > 0 &&
                    team_score[team] < minimum_score) {
                    winning_team = team;
                    minimum_score = team_score[team];
                }
            }
            for (int seat = 0; seat <= 7; ++seat) {
                battle_live_flags_[seat] =
                    battle_teams_[seat] == winning_team;
            }
            user_table_.SetWinTeam(winning_team);
        }
        unlock();
        break;
    }
    case 5:
        death_match_battle_mgr_.checkWinnerForTeamPlay(
            users_,
            reinterpret_cast<bool*>(battle_live_flags_),
            battle_teams_,
            user_table_);
        break;
    case 4:
        death_match_battle_mgr_.checkWinnerForSinglePlay(
            users_,
            reinterpret_cast<bool*>(battle_live_flags_),
            user_table_);
        break;
    default: {
        lock();
        if (all_equal) {
            for (int seat = 0; seat <= 7; ++seat) {
                battle_live_flags_[seat] = 0;
            }
        } else {
            int winning_seat = 0;
            int minimum_score = 0x7fffffff;
            for (int seat = 0; seat <= 7; ++seat) {
                if (users_[seat] && scores[seat] < minimum_score) {
                    winning_seat = seat;
                    minimum_score = scores[seat];
                }
            }
            for (int seat = 0; seat <= 7; ++seat) {
                battle_live_flags_[seat] = winning_seat == seat;
            }
            user_table_.SetWinTeam(winning_seat);
        }
        unlock();
        break;
    }
    }

    pvp_request_pvp_rank();
    return 0;
}
