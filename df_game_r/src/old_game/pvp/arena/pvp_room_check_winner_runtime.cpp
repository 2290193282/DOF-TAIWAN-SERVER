#include "old_game/pvp/arena/pvp_room.h"

#include <cstring>

#include "infra/log/format/trace.h"

// IDA_EA: 0x85D96EC
// IDA_NAME: _ZN8PvP_Room12check_winnerEv
// DEMANGLED: PvP_Room::check_winner(void)
int PvP_Room::check_winner() {
    if (state_flag_0c0_) {
        return 0;
    }

    switch (battle_mode_) {
    case 2:
    case 3: {
        lock();
        int team_live[3] = {0, 0, 0};
        int team_member[3] = {0, 0, 0};
        for (int seat = 0; seat <= 7; ++seat) {
            const unsigned char team = battle_teams_[seat];
            if (team && team <= 0xfd) {
                ++team_member[team];
                if (battle_live_flags_[seat]) {
                    ++team_live[team];
                }
            }
        }

        int surviving_team_count = 0;
        int surviving_team = 0;
        for (int team = 1; team <= 2; ++team) {
            if (team_live[team] > 0) {
                ++surviving_team_count;
                surviving_team = team;
            }
        }
        if (surviving_team_count == 1) {
            for (int seat = 0; seat <= 7; ++seat) {
                battle_live_flags_[seat] =
                    battle_teams_[seat] == surviving_team;
            }
        }
        unlock();

        if (surviving_team_count == 1) {
            user_table_.SetWinTeam(surviving_team);
            return 1;
        }
        break;
    }
    case 4: {
        int player_count = 0;
        for (int seat = 0; seat <= 7; ++seat) {
            if (users_[seat] && teams_[seat] != 4) {
                ++player_count;
            }
        }
        cMyTrace trace("bool PvP_Room::check_winner()", 2620, 0);
        trace("PvP_Room::check_winner single : %d", player_count);
        if (death_match_battle_mgr_.checkGameOverForSinglePlay(player_count)) {
            death_match_battle_mgr_.checkWinnerForSinglePlay(
                users_,
                reinterpret_cast<bool*>(battle_live_flags_),
                user_table_);
            return 1;
        }
        break;
    }
    case 5: {
        const int team_count = get_pvp_team_cnt();
        cMyTrace trace("bool PvP_Room::check_winner()", 2630, 0);
        trace("PvP_Room::check_winner team  : %d", team_count);
        if (death_match_battle_mgr_.checkGameOverForTeamPlay(
                users_, team_count, battle_teams_)) {
            death_match_battle_mgr_.checkWinnerForTeamPlay(
                users_,
                reinterpret_cast<bool*>(battle_live_flags_),
                battle_teams_,
                user_table_);
            return 1;
        }
        break;
    }
    default: {
        lock();
        int live_count = 0;
        int winning_seat = 0;
        for (int seat = 0; seat <= 7; ++seat) {
            if (battle_live_flags_[seat] && teams_[seat] != 4) {
                ++live_count;
                winning_seat = seat;
            }
        }
        unlock();
        if (live_count == 1) {
            user_table_.SetWinTeam(winning_seat);
            return 1;
        }
        break;
    }
    }
    return 0;
}
