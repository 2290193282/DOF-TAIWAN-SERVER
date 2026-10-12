#include "old_game/pvp/arena/pvp_room.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/disconnect_detecter_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/pvp/battlefield/integrated_match_pvp_score_runtime.h"
#include "old_game/pvp/match/packet_server_match_data.h"
#include "old_game/pvp/battlefield/pvp_live_event_data_runtime.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <stdint.h>


namespace {

const int kMaxSeatCount = 8;

typedef void (*BattleModeLeaveFn)(void* handler,
                                  CUser* user,
                                  PvP_Room* room,
                                  int team,
                                  int seat,
                                  int battle_mode,
                                  bool was_manager_changed,
                                  int waiter_count);

}  // namespace

extern int IsDeathMatchMap(int map_no, bool& changed_to_individual)
    __asm__("_Z15IsDeathMatchMapiRb");

// IDA_EA: 0x822D994
// IDA_NAME: _ZN8PvP_Room9get_indexEv
// DEMANGLED: PvP_Room::get_index(void)
int PvP_Room::get_index() {
    return index_;
}

// IDA_EA: 0x82A4322
// IDA_NAME: _ZN8PvP_Room24IsInsertTimerRecvPvpRankEv
// DEMANGLED: PvP_Room::IsInsertTimerRecvPvpRank(void)
int PvP_Room::IsInsertTimerRecvPvpRank() {
    return recv_pvp_rank_timer_inserted_;
}

// IDA_EA: 0x82A4332
// IDA_NAME: _ZN8PvP_Room14IsEndPvpBattleEv
// DEMANGLED: PvP_Room::IsEndPvpBattle(void)
int PvP_Room::IsEndPvpBattle() {
    return pvp_battle_ended_;
}

// IDA_EA: 0x822D974
// IDA_NAME: _ZN8PvP_Room15IsExistPasswordEv
// DEMANGLED: PvP_Room::IsExistPassword(void)
int PvP_Room::IsExistPassword() {
    return password_exists_;
}

// IDA_EA: 0x822D984
// IDA_NAME: _ZNK8PvP_Room13isRematchableEv
// DEMANGLED: PvP_Room::isRematchable(void)const
int PvP_Room::isRematchable() const {
    return rematchable_;
}

// IDA_EA: 0x822D9BC
// IDA_NAME: _ZN8PvP_Room17GetRequestedMapNoEv
// DEMANGLED: PvP_Room::GetRequestedMapNo(void)
int PvP_Room::GetRequestedMapNo() {
    return requested_map_no_;
}

// IDA_EA: 0x8568D0A
// IDA_NAME: _ZNK8PvP_Room10GetManagerEv
// DEMANGLED: PvP_Room::GetManager(void)const
CUser* PvP_Room::GetManager() const {
    return manager_;
}

// IDA_EA: 0x8568D16
// IDA_NAME: _ZNK8PvP_Room14GetManagerSeatEv
// DEMANGLED: PvP_Room::GetManagerSeat(void)const
int PvP_Room::GetManagerSeat() const {
    return manager_seat_;
}

// IDA_EA: 0x8568D22
// IDA_NAME: _ZN8PvP_Room12GetTeamCountEi
// DEMANGLED: PvP_Room::GetTeamCount(int)
int PvP_Room::GetTeamCount(int team) {
    int count = 0;
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (teams_[seat] == team) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x8568D62
// IDA_NAME: _ZN8PvP_Room14setReamtchableEb
// DEMANGLED: PvP_Room::setReamtchable(bool)
PvP_Room* PvP_Room::setReamtchable(bool rematchable) {
    rematchable_ = rematchable;
    return this;
}

// IDA_EA: 0x8568D7E
// IDA_NAME: _ZN8PvP_Room9set_stateEi
// DEMANGLED: PvP_Room::set_state(int)
PvP_Room* PvP_Room::set_state(int state) {
    battle_state_ = state;
    return this;
}

// IDA_EA: 0x8568D90
// IDA_NAME: _ZN8PvP_Room13gen_timer_keyE13TIMER_MESSAGE
// DEMANGLED: PvP_Room::gen_timer_key(TIMER_MESSAGE)
int PvP_Room::gen_timer_key(TIMER_MESSAGE message) {
    return ++pvp_timer_keys_[static_cast<unsigned int>(message) - 48u];
}

// IDA_EA: 0x85D62AC
// IDA_NAME: _ZN8PvP_Room6SetIDXEi
// DEMANGLED: PvP_Room::SetIDX(int)
int PvP_Room::SetIDX(int index) {
    index_ = index;
    return reset();
}

// IDA_EA: 0x85D6774
// IDA_NAME: _ZN8PvP_Room16get_pvp_team_cntEv
// DEMANGLED: PvP_Room::get_pvp_team_cnt(void)
int PvP_Room::get_pvp_team_cnt() {
    const int battle_mode = get_pvp_battle_mode();
    if (battle_mode != 2 && battle_mode != 3 && battle_mode != 5) {
        return -1;
    }

    int team_one_count = 0;
    int team_two_count = 0;
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (users_[seat] && IsPvpObserver(seat) != 1) {
            if (teams_[seat] == 1) {
                ++team_one_count;
            } else if (teams_[seat] == 2) {
                ++team_two_count;
            }
        }
    }
    return team_two_count == team_one_count ? team_two_count : -1;
}

// IDA_EA: 0x85D6844
// IDA_NAME: _ZNK8PvP_Room15get_remain_seatEv
// DEMANGLED: PvP_Room::get_remain_seat(void)const
int PvP_Room::get_remain_seat() const {
    return kMaxSeatCount - get_waiter_count();
}

// IDA_EA: 0x85D68A0
// IDA_NAME: _ZN8PvP_Room16get_player_countEv
// DEMANGLED: PvP_Room::get_player_count(void)
int PvP_Room::get_player_count() {
    int count = 0;
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (users_[seat] && IsPvpObserver(seat) != 1) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x85D7726
// IDA_NAME: _ZN8PvP_Room23is_closeable_seat_stateEv
// DEMANGLED: PvP_Room::is_closeable_seat_state(void)
int PvP_Room::is_closeable_seat_state() {
    int player_count = 0;
    int free_seat_count = 0;
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (users_[seat] && IsPvpObserver(seat) != 1) {
            ++player_count;
        }
        if (teams_[seat] == 255) {
            ++free_seat_count;
        }
    }
    return player_count != 1 || free_seat_count > 1;
}

// IDA_EA: 0x85DA046
// IDA_NAME: _ZN8PvP_Room13IsPvpObserverEi
// DEMANGLED: PvP_Room::IsPvpObserver(int)
int PvP_Room::IsPvpObserver(int seat) {
    if (seat > 7) {
        return 0;
    }
    return teams_[seat] == 3 || teams_[seat] == 4;
}

// IDA_EA: 0x85D6862
// IDA_NAME: _ZNK8PvP_Room16get_waiter_countEv
// DEMANGLED: PvP_Room::get_waiter_count(void)const
int PvP_Room::get_waiter_count() const {
    int count = 0;
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (users_[seat]) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x822D99E
// IDA_NAME: _ZN8PvP_Room17SetRequestedMapNoEs
// DEMANGLED: PvP_Room::SetRequestedMapNo(short)
PvP_Room* PvP_Room::SetRequestedMapNo(int16_t map_no) {
    requested_map_no_ = map_no;
    return this;
}

// IDA_EA: 0x822D9CC
// IDA_NAME: _ZN8PvP_Room19get_pvp_battle_modeEv
// DEMANGLED: PvP_Room::get_pvp_battle_mode(void)
int PvP_Room::get_pvp_battle_mode() {
    return static_cast<int>(battle_mode_);
}

// IDA_EA: 0x85DC2CC
// IDA_NAME: _ZN8PvP_Room23get_recv_pvp_rank_countEv
// DEMANGLED: PvP_Room::get_recv_pvp_rank_count(void)
int PvP_Room::get_recv_pvp_rank_count() {
    int count = 0;
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (users_[seat] && teams_[seat] != 255 &&
            static_cast<unsigned char>(recv_pvp_rank_.IsRecv(seat))) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x85D84C6
// IDA_NAME: _ZN8PvP_Room12set_pvp_modeEP5CUseri
// DEMANGLED: PvP_Room::set_pvp_mode(CUser *,int)
int PvP_Room::set_pvp_mode(CUser* user, int mode) {
    if (battle_state_ != 1) {
        return 19;
    }
    if (manager_ != user) {
        return 8;
    }

    const int old_mode = static_cast<int>(battle_mode_);
    battle_mode_ = static_cast<PVP_BATTLE_MODE>(mode);
    switch (mode) {
        case 1:
            for (int seat = 0; seat < kMaxSeatCount; ++seat) {
                if (users_[seat]) {
                    teams_[seat] = 0;
                    relay_battle_mgr_.OnChangeTeam(seat, 0);
                }
            }
            break;
        case 3:
            if (old_mode == 2) {
                for (int seat = 0; seat < kMaxSeatCount; ++seat) {
                    if (users_[seat]) {
                        relay_battle_mgr_.OnChangeTeam(seat, teams_[seat]);
                    }
                }
            } else if (old_mode == 1) {
                int team = 1;
                for (int seat = 0; seat < kMaxSeatCount; ++seat) {
                    if (users_[seat]) {
                        teams_[seat] = team;
                        relay_battle_mgr_.OnChangeTeam(seat, teams_[seat]);
                        team = team == 1 ? 2 : 1;
                    }
                }
            }
            break;
        case 4:
            for (int seat = 0; seat < kMaxSeatCount; ++seat) {
                if (users_[seat]) {
                    teams_[seat] = 0;
                }
            }
            break;
        case 5: {
            int team = 1;
            for (int seat = 0; seat < kMaxSeatCount; ++seat) {
                if (users_[seat]) {
                    teams_[seat] = team;
                    relay_battle_mgr_.OnChangeTeam(seat, team);
                    team = team == 1 ? 2 : 1;
                }
            }
            break;
        }
        default:
            if (old_mode != 3) {
                int team = 1;
                for (int seat = 0; seat < kMaxSeatCount; ++seat) {
                    if (users_[seat]) {
                        teams_[seat] = team;
                        relay_battle_mgr_.OnChangeTeam(seat, teams_[seat]);
                        team = team == 1 ? 2 : 1;
                    }
                }
            }
            break;
    }
    return 0;
}

// IDA_EA: 0x85DD936
// IDA_NAME: _ZN8PvP_Room15CheckPvPMapModeEP5CUseriRb
// DEMANGLED: PvP_Room::CheckPvPMapMode(CUser *,int,bool &)
int PvP_Room::CheckPvPMapMode(CUser* user,
                              int map_no,
                              bool& changed_to_individual) {
    int battle_mode = static_cast<int>(battle_mode_);
    if (battle_mode == 6) {
        return 1;
    }

    const unsigned char matched = static_cast<unsigned char>(
        IsDeathMatchMap(map_no, changed_to_individual));
    if (matched && !changed_to_individual && battle_mode == 4) {
        return 1;
    }
    if (matched && battle_mode == 3) {
        battle_mode = 4;
        battle_mode_ = static_cast<PVP_BATTLE_MODE>(battle_mode);
    }
    if (matched && battle_mode == 1) {
        set_pvp_mode(user, 4);
    }
    if (matched && battle_mode == 2) {
        battle_mode = 5;
        battle_mode_ = static_cast<PVP_BATTLE_MODE>(battle_mode);
    }
    if (matched != 1 && battle_mode == 4) {
        set_pvp_mode(user, 1);
    }
    if (matched != 1 && battle_mode == 5) {
        battle_mode = 2;
        battle_mode_ = static_cast<PVP_BATTLE_MODE>(battle_mode);
    }
    return 1;
}

// IDA_EA: 0x85D91E2
// IDA_NAME: _ZN8PvP_Room13get_user_seatEP5CUser
// DEMANGLED: PvP_Room::get_user_seat(CUser *)
int PvP_Room::get_user_seat(CUser* user) {
    for (int seat = 0; seat < kMaxSeatCount; ++seat) {
        if (users_[seat] == user) {
            return seat;
        }
    }
    return -1;
}

// IDA_EA: 0x85D921E
// IDA_NAME: _ZN8PvP_Room13get_user_seatEi
// DEMANGLED: PvP_Room::get_user_seat(int)
CUser* PvP_Room::get_user_seat(int seat) {
    return static_cast<unsigned int>(seat) < kMaxSeatCount
               ? users_[seat]
               : 0;
}

// IDA_EA: 0x85D9242
// IDA_NAME: _ZN8PvP_Room16get_manager_teamEv
// DEMANGLED: PvP_Room::get_manager_team(void)
int PvP_Room::get_manager_team() {
    const int manager_seat = manager_seat_;
    return static_cast<unsigned int>(manager_seat) < kMaxSeatCount
               ? teams_[manager_seat]
               : 254;
}

// IDA_EA: 0x85D9272
// IDA_NAME: _ZN8PvP_Room8get_teamEi
// DEMANGLED: PvP_Room::get_team(int)
int PvP_Room::get_team(int seat) {
    return static_cast<unsigned int>(seat) < kMaxSeatCount
               ? teams_[seat]
               : 254;
}

// IDA_EA: 0x85DF946
// IDA_NAME: _ZN8PvP_Room9get_stateEv
// DEMANGLED: PvP_Room::get_state(void)
int PvP_Room::get_state() {
    return battle_state_;
}

// IDA_EA: 0x863BCB4
// IDA_NAME: _ZN8PvP_Room13get_timer_keyE13TIMER_MESSAGE
// DEMANGLED: PvP_Room::get_timer_key(TIMER_MESSAGE)
int PvP_Room::get_timer_key(TIMER_MESSAGE message) {
    return pvp_timer_keys_[static_cast<unsigned int>(message) - 48u];
}

// IDA_EA: 0x863BCCC
// IDA_NAME: _ZN8PvP_Room13CheckTimerKeyE13TIMER_MESSAGEi
// DEMANGLED: PvP_Room::CheckTimerKey(TIMER_MESSAGE,int)
bool PvP_Room::CheckTimerKey(TIMER_MESSAGE message, int key) {
    return get_timer_key(message) == key;
}

// IDA_EA: 0x85DB9E2
// IDA_NAME: _ZN8PvP_Room14make_seat_infoEPci
// DEMANGLED: PvP_Room::make_seat_info(char *,int)
int PvP_Room::make_seat_info(char* packet, int seat_filter) {
    InterfacePacketBuf* const packet_buf =
        reinterpret_cast<InterfacePacketBuf*>(packet);
    packet_buf->put_header(0, 43);
    packet_buf->put_short(get_index());
    packet_buf->put_byte(get_pvp_battle_mode());
    if (seat_filter == 0xFFFF) {
        const int waiter_count = get_waiter_count();
        packet_buf->put_byte(waiter_count);
        if (!waiter_count) {
            LogManager::logFormat(
                1,
                "pvp.cpp",
                "void PvP_Room::make_seat_info(char*, int)",
                0xE66,
                "Send Member 0 PvpRoom %d State(%d)",
                get_index(),
                get_state());
        }
        lock();
        for (int seat = 0; seat < kMaxSeatCount; ++seat) {
            CUser* const user = users_[seat];
            if (user) {
                packet_buf->put_byte(seat);
                packet_buf->put_byte(teams_[seat]);
                packet_buf->put_short(user->get_unique_id());
                packet_buf->put_byte(user->isAllowLocationGeoState());
                cMyTrace trace(
                    "void PvP_Room::make_seat_info(char*, int)", 0xE74, 0);
                trace("RELAY_BATTLE SeatInfo player(%d), team(%d), uid(%d)\\n",
                      seat,
                      teams_[seat],
                      user->get_unique_id());
            }
        }
        unlock();
    } else {
        packet_buf->put_byte(1);
        lock();
        packet_buf->put_byte(seat_filter);
        packet_buf->put_byte(teams_[seat_filter]);
        CUser* const user = users_[seat_filter];
        if (user) {
            packet_buf->put_short(user->get_unique_id());
            packet_buf->put_byte(user->isAllowLocationGeoState());
        } else {
            packet_buf->put_short(0xFFFF);
            packet_buf->put_byte(0);
        }
        unlock();
    }
    return packet_buf->finalize(true);
}

// IDA_EA: 0x85DBD3A
// IDA_NAME: _ZN8PvP_Room15make_state_infoEPc
// DEMANGLED: PvP_Room::make_state_info(char *)
int PvP_Room::make_state_info(char* packet) {
    InterfacePacketBuf* const packet_buf =
        reinterpret_cast<InterfacePacketBuf*>(packet);
    packet_buf->put_header(0, 42);
    packet_buf->put_short(get_index());
    packet_buf->put_byte(battle_state_);
    packet_buf->put_byte(manager_seat_);
    packet_buf->put_short(requested_map_no_);
    packet_buf->put_byte(get_pvp_battle_mode());
    packet_buf->put_int(current_match_->GetIndentification());
    return packet_buf->finalize(true);
}

// IDA_EA: 0x85D7F6C
// IDA_NAME: _ZN8PvP_Room10leave_roomEP5CUserRb
// DEMANGLED: PvP_Room::leave_room(CUser *,bool &)
int PvP_Room::leave_room(CUser* user, bool& manager_changed) {
    const int waiter_count = get_waiter_count();
    const bool was_manager_changed = manager_changed;
    const int battle_mode = get_pvp_battle_mode();

    lock();
    if (manager_ == user) {
        select_new_manager();
        manager_changed = true;
    }

    const int seat = get_user_seat(user);
    if (seat < 0) {
        return -1;
    }
    if (get_state() == 2) {
        GiveOuterPenalty(user);
    }

    const int team = get_team(seat);
    user->SetPvpIndex(-1);
    ready_flags_[seat] = 0;
    teams_[seat] = 255;
    users_[seat] = 0;
    --member_count_;
    battle_live_flags_[seat] = 0;
    battle_teams_[seat] = 0;
    unlock();

    user->getHackAnalyzer()->reportHackInfo();
    user->getHackAnalyzer()->resetHackInfo();
    if (battle_state_ == 2 && check_winner()) {
        pvp_request_pvp_rank();
    }

    relay_battle_mgr_.OnLeaveRoom(seat);
    void* const handler = current_match_;
    void** const vtable = *reinterpret_cast<void***>(handler);
    reinterpret_cast<BattleModeLeaveFn>(vtable[11])(
        handler, user, this, team, seat, battle_mode, was_manager_changed,
        waiter_count);
    return seat;
}

bool PvP_Room::detach_zombie_loading_seat(CUser* user,
                                          int expected_seat,
                                          bool& manager_changed) {
    manager_changed = false;
    if (!user || expected_seat < 0 || expected_seat >= 8) return false;

    lock();
    if (battle_state_ != 1 || member_count_ != 6 ||
        users_[expected_seat] != user) {
        unlock();
        return false;
    }
    if (manager_ == user) {
        if (!select_new_manager()) {
            unlock();
            return false;
        }
        manager_changed = true;
    }
    user->SetPvpIndex(-1);
    ready_flags_[expected_seat] = 0u;
    teams_[expected_seat] = 255;
    users_[expected_seat] = 0;
    --member_count_;
    battle_live_flags_[expected_seat] = 0u;
    battle_teams_[expected_seat] = 0u;
    unlock();
    return true;
}

// IDA_EA: 0x85D81D0
// IDA_NAME: _ZN8PvP_Room12destroy_roomEi
// DEMANGLED: PvP_Room::destroy_room(int)
int PvP_Room::destroy_room(int preferred_seat) {
    int user = reinterpret_cast<int>(users_[preferred_seat]);
    if (!user) {
        for (int seat = 0; seat <= 7; ++seat) {
            user = reinterpret_cast<int>(users_[seat]);
            if (user) {
                break;
            }
        }
    }
    reset();
    return user;
}

// IDA_EA: 0x85DC476
// IDA_NAME: _ZN8PvP_Room12send_pvp_endEv
// DEMANGLED: PvP_Room::send_pvp_end(void)
int PvP_Room::send_pvp_end() {
    pvp_battle_ended_ = 1;
    if (zombie_mode::IsActiveRoom(this) ||
        zombie_mode::IsFinishedRoom(this)) {
        return 1;
    }
    if (G_GameWorld()->GetChannelType() != 6) {
        update_pvp_point();
    }
    pvp_reward();
    return 1;
}

// IDA_EA: 0x85DC866
// IDA_NAME: _ZN8PvP_Room16update_pvp_pointEv
// DEMANGLED: PvP_Room::update_pvp_point(void)
bool PvP_Room::update_pvp_point() {
    if (G_GameWorld()->IsPVPChannel() &&
        G_GameWorld()->IsFreePvPChannel()) {
        for (int seat = 0; seat != kMaxSeatCount; ++seat) {
            CUser* const user = users_[seat];
            if (user && !IsPvpObserver(seat)) {
                user->update_old_pvp_point();
            }
        }
    } else {
        for (int seat = 0; seat != kMaxSeatCount; ++seat) {
            CUser* const user = users_[seat];
            if (!user || IsPvpObserver(seat)) {
                continue;
            }

            user->update_old_pvp_point();
            const int pvp_grade = user->get_pvp_grade();
            if (battle_mode_ == 1 || battle_mode_ == 4) {
                user->update_pvp_point(
                    user_table_.GetResult(seat, false, pvp_grade));
            } else {
                const bool special_result = pvp_grade > 10;
                user->update_pvp_point(user_table_.GetResult(
                    battle_teams_[seat], special_result, pvp_grade));
            }
        }
    }

    // The original do/while-style decompilation assigns its boolean return
    // from `seat <= 7` immediately before leaving either loop, so its public
    // result is always false after applying the side effects above.
    return false;
}

// IDA_EA: 0x85DA882
// IDA_NAME: _ZN8PvP_Room10pvp_rewardEv
// DEMANGLED: PvP_Room::pvp_reward(void)
void PvP_Room::pvp_reward() {
    // Defense in depth for direct/timer callers: Zombie settlement and
    // NOTI475 are owned by the dedicated sidecar, never by ordinary PvP
    // point, mission, reward, or NOTI47 production.
    if (zombie_mode::IsActiveRoom(this) ||
        zombie_mode::IsFinishedRoom(this)) {
        return;
    }

    int ranking[8];
    const int ranking_count = GetRanking(ranking);
    const int ace = GetAce();

    GameWorld* const world = G_GameWorld();
    if (world->IsPVPChannel() && !world->IsFreePvPChannel()) {
        CalcuateVictoryPoint(ranking, ranking_count);
    }

    integrated_match_pvp_score::MatchMode integrated_match_mode =
        integrated_match_pvp_score::MATCH_MODE_INVALID;
    if (world->IsIntegratedPvPBaseChannel()) {
        int team_one_count = 0;
        int team_two_count = 0;
        for (int seat = 0; seat < kMaxSeatCount; ++seat) {
            if (!users_[seat] || IsPvpObserver(seat)) {
                continue;
            }
            if (battle_teams_[seat] == 1) {
                ++team_one_count;
            } else if (battle_teams_[seat] == 2) {
                ++team_two_count;
            }
        }

        // A 1v1 leaver is removed before the surviving user's reward loop.
        // battle_player_count_ is the start-of-battle snapshot, so retain the
        // original 1+1 topology after that departure.
        if ((battle_mode_ == 2 || battle_mode_ == 5) &&
            battle_player_count_ == 2 &&
            (team_one_count != 1 || team_two_count != 1)) {
            team_one_count = 1;
            team_two_count = 1;
        }
        integrated_match_mode =
            integrated_match_pvp_score::ResolveMatchMode(
                static_cast<int>(battle_mode_),
                team_one_count,
                team_two_count);
    }

    // 原始 i386/GCC4.9 未初始化 byte-stack read，待目标对象反汇编复核。
    // Only the live-seat branch below writes this byte, matching 0x85DA930.
    volatile uint8_t winner_seat;
    bool no_winner = true;
    for (int seat = 0; seat <= 7; ++seat) {
        if (battle_live_flags_[seat] && !IsPvpObserver(seat)) {
            winner_seat = static_cast<uint8_t>(seat);
            no_winner = false;
            break;
        }
    }

    const bool can_acquire_guild_point = check_acquire_guild_point();
    current_match_->OnReward(
        this, get_team(static_cast<int>(winner_seat)), no_winner);

    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = users_[seat];
        if (!user) {
            continue;
        }

        disconnect_detecter::GetInstanceDisconnectDetecter()
            ->UnregisterUser(user);
        user->add_pvp_play_info(
            static_cast<unsigned int>(
                GlobalData::s_systemTime_.getCurSec() -
                battle_start_time_sec_),
            1u);

        bool send_integrated_match_result = false;
        if (world->IsIntegratedPvPBaseChannel()) {
            CEventBase* const match_event =
                GlobalData::s_event_manager->GetRepeatEvent(56);
            if (match_event->IsEventing(0) && !IsPvpObserver(seat)) {
                send_integrated_match_result = true;
            }
        }
        if (send_integrated_match_result && user->GetServerGroup()) {
            Packet_Server_Match_data match_data;
            match_data.server_group =
                static_cast<uint8_t>(user->GetServerGroup());
            if (battle_live_flags_[seat]) {
                match_data.win_flag = 1;
            } else {
                match_data.lose_or_leave_flag = 1;
            }
            CStatisticServerProxy* const proxy =
                GlobalData::s_statistic_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(
                        user->GetServerGroup()));
            proxy->SendPacket(reinterpret_cast<char*>(&match_data),
                              sizeof(match_data));
        }

        if (integrated_match_mode !=
                integrated_match_pvp_score::MATCH_MODE_INVALID &&
            !IsPvpObserver(seat)) {
            int integrated_kill_count = 0;
            if (battle_mode_ == 3) {
                integrated_kill_count =
                    relay_battle_mgr_.GetKillCount(seat);
            } else if (battle_mode_ == 4 || battle_mode_ == 5) {
                integrated_kill_count =
                    death_match_battle_mgr_.getKillCount(seat);
            } else {
                integrated_kill_count = battle_kill_counts_[seat];
            }
            if (integrated_kill_count < 0) {
                integrated_kill_count = 0;
            }

            const bool integrated_won = battle_live_flags_[seat] != 0;
            const bool integrated_lost =
                !integrated_won && !no_winner;
            const int legacy_wins = user->getPVPResultR()
                                        ? user->getPVPResultR()->values[0]
                                        : 0;
            (void)integrated_match_pvp_score::Record(
                user,
                integrated_match_mode,
                integrated_won,
                integrated_lost,
                static_cast<uint32_t>(integrated_kill_count),
                legacy_wins > 0
                    ? static_cast<uint32_t>(legacy_wins)
                    : 0u);
        }

        uint32_t reward_exp = 0;
        if (can_acquire_guild_point && battle_live_flags_[seat] &&
            !IsPvpObserver(seat)) {
            user->add_guild_point_item();
        }

        CMissionList_Charac* const missions =
            static_cast<CMissionList_Charac*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
        const bool is_team_mode_two = battle_mode_ == PVP_BATTLE_MODE_2;
        missions->setIssue(
            battle_live_flags_[seat] != 0, is_team_mode_two);
        MissionClearCondition_Parameter mission_parameter(battle_mode_);
        mission_parameter.issue_history = static_cast<uint16_t>(
            missions->getIssue_mode(is_team_mode_two));
        bool within_mission_cleared = false;
        missions->Update_Win_event(
            *user, mission_parameter, within_mission_cleared);
        if (within_mission_cleared) {
            const int grade =
                GlobalData::s_ref_pvp_grade->GetPvpGrade(
                    user->getPVPResultRefR()->values[2]);
            if (user->getPVPResultRefR()->values[5] < grade) {
                user->getPVPResultRefW()->values[5] = grade;
            }
        }
        user->send_MissionList();

        if (world->IsPVPChannel()) {
            if (world->IsWinPointPvPChannel()) {
                user->add_pvp_result(
                    battle_live_flags_[seat] != 0, &reward_exp);
                if (battle_mode_ == 4 || battle_mode_ == 5) {
                    reward_exp = 0;
                }

                if (battle_live_flags_[seat]) {
                    int action_group = 0;
                    if (battle_mode_ == 1 || battle_mode_ == 4) {
                        action_group = 0x12;
                    } else if (battle_mode_ == 2 ||
                               battle_mode_ == 5) {
                        action_group = 0x13;
                    } else if (battle_mode_ == 3) {
                        action_group = 0x14;
                    }
                    if (action_group) {
                        APSystem::CUserProc::ClearActionAndSendtoUser(
                            user,
                            static_cast<APSystem::_ActionGroupIndex>(
                                action_group),
                            0,
                            ENUM_PACKETCLASS_NOTIPACKET);
                    }
                }
            } else if (world->GetChannelType() == 6) {
                int guild_point_change;
                if (!battle_live_flags_[seat]) {
                    guild_point_change = -1;
                } else if (battle_mode_ == 2 || battle_mode_ == 3 ||
                           battle_mode_ == 5) {
                    guild_point_change = 1;
                } else {
                    guild_point_change = count_pvp_user() > 3 ? 2 : 1;
                }
                user->add_guild_pvp_result(guild_point_change);
            }
        }

        int all_kill_uid = 0xffff;
        int all_kill_team = 0;
        if (battle_mode_ == 3) {
            const int all_kill_seat =
                relay_battle_mgr_.GetAllKillUserIndex(&all_kill_team);
            if (all_kill_seat >= 0 && all_kill_seat <= 7 &&
                users_[all_kill_seat]) {
                all_kill_uid =
                    static_cast<uint16_t>(
                        users_[all_kill_seat]->get_unique_id());
            }
        }

        PacketGuard packet;
        packet.clear();
        packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 47);
        packet.put_byte(winner_seat);

        const int channel_type = world->GetChannelType();
        if (channel_type == 15) {
            fair_pvp::CFairPvPScore* const score =
                static_cast<fair_pvp::CFairPvPScore*>(
                    user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
            packet.put_int(score->GetLastTrainingSealCount());
        } else if (channel_type == 16 || channel_type == 14) {
            packet.put_int(0);
        } else {
            packet.put_int(user->GetWinPoint());
        }

        packet.put_byte(user->get_pvp_grade());
        if (channel_type == 6) {
            packet.put_int(0);
        } else {
            int gained_pvp_exp =
                user->get_pvp_result_exp_point() -
                user->get_pvp_result_oldexp_point();
            if (all_kill_uid != 0xffff &&
                teams_[seat] == all_kill_team) {
                gained_pvp_exp +=
                    gained_pvp_exp * 5 *
                    GetTeamCount(all_kill_team) / 100;
            }
            user->getPVPResultRefW()->values[2] =
                user->get_pvp_result_oldexp_point() + gained_pvp_exp;
            packet.put_int(gained_pvp_exp);
            user->LogHistory(
                "PvPExp,%d,%d,%d",
                user->getPVPResultRefR()->values[2],
                user->get_pvp_result_oldexp_point(),
                gained_pvp_exp);
        }

        if (!user->has_within_Mission()) {
            user->acceptable_within_mission();
        }

        packet.put_byte(ranking_count);
        for (int rank = 0; rank < ranking_count; ++rank) {
            const int ranked_seat = ranking[rank];
            CUser* const ranked_user = users_[ranked_seat];
            if (!ranked_user) {
                continue;
            }

            packet.put_short(ranked_user->get_unique_id());
            packet.put_int(battle_scores_[ranked_seat]);
            const int pvp_grade = ranked_user->get_pvp_grade();
            packet.put_byte(pvp_grade);
            packet.put_int(ranked_user->get_pvp_result_exp_point());
            packet.put_int(
                GlobalData::s_ref_pvp_grade->GetPvpCurrRankPoint(
                    pvp_grade));
            packet.put_int(
                GlobalData::s_ref_pvp_grade->GetPvpNextRankPoint(
                    pvp_grade));
            const int kill_count =
                death_match_battle_mgr_.getKillCount(ranked_seat);
            const int death_count =
                death_match_battle_mgr_.getDeathCount(ranked_seat);
            packet.put_int(kill_count);
            packet.put_int(death_count);

            if (battle_mode_ == 4 || battle_mode_ == 5) {
                cMyTrace trace(
                    "void PvP_Room::pvp_reward()", 3499, 0);
                trace("<PVP_REWARD> Room(%d), iRank(%d), "
                      "iRankMember(%d), Player(%s), mode(%d), "
                      "KillCnt(%d), DeathCnt(%d)",
                      get_index(),
                      rank,
                      ranked_seat,
                      ranked_user->getCurCharacName(),
                      static_cast<int>(battle_mode_),
                      kill_count,
                      death_count);
            }
        }

        if (ranking_count) {
            packet.put_short(
                ace == -1 || !users_[ace]
                    ? 0
                    : users_[ace]->get_unique_id());
        }
        packet.put_short(all_kill_uid);
        packet.put_int(static_cast<int>(reward_exp));
        packet.put_byte(no_winner);

        if (channel_type == 15) {
            fair_pvp::CFairPvPScore* const score =
                static_cast<fair_pvp::CFairPvPScore*>(
                    user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
            packet.put_short(
                score->GetLastTrainingSealBonusCount());
            packet.put_short(score->GetRemainBonusCount());
        } else if (channel_type == 16) {
            packet.put_short(0);
            packet.put_short(0);
        }

        packet.put_byte(user->has_within_Mission() != 0);
        const int relay_kill_count =
            battle_mode_ == 3
                ? relay_battle_mgr_.GetKillCount(seat)
                : 0xff;
        packet.put_byte(relay_kill_count);
        packet.finalize(true);
        user->Send(packet);

        if ((get_pvp_battle_mode() == 4 ||
             get_pvp_battle_mode() == 5) &&
            reward_exp) {
            int gained_sp = 0;
            int gained_sfp = 0;
            if (user->gain_exp_sp(
                    static_cast<int>(reward_exp),
                    gained_sp,
                    gained_sfp,
                    static_cast<eExpAddReason>(10),
                    0,
                    false)) {
                user->SendNotiPacket(
                    static_cast<CUser::eSendTarget>(2),
                    static_cast<ENUM_NOTIPACKET>(2),
                    1);
            }
        }

        user->getHackAnalyzer()->setLastMonsterDeadTime(
            GlobalData::s_systemTime_.getCurSec());

        CEventBase* const live_event =
            GlobalData::s_event_manager->GetRepeatEvent(55);
        if (live_event->IsEventing(0)) {
            CPvPLiveEventData* const live_data =
                static_cast<CPvPLiveEventData*>(
                    user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(3)));
            live_data->IncreasePlayCount(user);
        }

        const int new_grade =
            GlobalData::s_ref_pvp_grade->GetPvpGrade(
                user->getPVPResultRefR()->values[2]);
        if (new_grade > 0 &&
            user->getPVPResultRefR()->values[5] != new_grade) {
            user->getPVPResultRefW()->values[5] = new_grade;
            CMissionList_Charac* const old_user_missions =
                static_cast<CMissionList_Charac*>(
                    user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
            old_user_missions->MakeMissionList_forOldUser(*user);
            world->send_AllBasicInfo(user);
        }
    }

    if (world->GetChannelType() == 6) {
        guild_war_log_.WriteGuildWarPvPLog(
            index_, static_cast<char>(battle_mode_), users_);
    }
    current_match_->OnEndPvP(this);
}

// IDA_EA: 0x85DF93A
// IDA_NAME: _ZN8PvP_Room4lockEv
// DEMANGLED: PvP_Room::lock(void)
void PvP_Room::lock() {}

// IDA_EA: 0x85DF940
// IDA_NAME: _ZN8PvP_Room6unlockEv
// DEMANGLED: PvP_Room::unlock(void)
void PvP_Room::unlock() {}
