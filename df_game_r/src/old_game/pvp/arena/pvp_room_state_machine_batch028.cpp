#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"

#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <vector>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/pvp_map_public_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

extern int IsDeathMatchMap(int map_no, bool& individual_mode);

// IDA_EA: 0x85D6BE6
// IDA_NAME: _ZN8PvP_Room22IsQuickJoinablePVPRoomEP5CUser
// DEMANGLED: PvP_Room::IsQuickJoinablePVPRoom(CUser *)
bool PvP_Room::IsQuickJoinablePVPRoom(CUser* user) {
    if (!check_joinable(user)) {
        if (user->find_pvp_masterid_walkingout_me(
                static_cast<unsigned int>(manager_->getCurCharacNo())) != 1) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x85D6C44
// IDA_NAME: _ZN8PvP_Room14check_joinableEP5CUser
// DEMANGLED: PvP_Room::check_joinable(CUser *)
int PvP_Room::check_joinable(CUser* user) {
    if (battle_state_ != 1) {
        return 4;
    }
    const int matching_result = current_match_->CheckJoinable(this, user);
    if (matching_result) {
        return matching_result;
    }
    for (int seat = 0; seat <= 7; ++seat) {
        if (!users_[seat] && teams_[seat] == 255) {
            return 0;
        }
    }
    return 4;
}

// IDA_EA: 0x85D8290
// IDA_NAME: _ZN8PvP_Room15set_ready_stateEP5CUserb
// DEMANGLED: PvP_Room::set_ready_state(CUser *,bool)
int PvP_Room::set_ready_state(CUser* user, bool ready) {
    // IDA 0x85D82D0 dispatches through current_match_->vptr + 72, i.e.
    // IMatch slot 18: OnReady(CUser*, bool).  Do not use the former slot-9
    // raw call: that slot is OnDestroyRoom(CUser*) in the same vtable.
    if (current_match_->OnReady(user, ready) != true) {
        return 0;
    }

    PacketGuard packet;
    if (manager_ == user) {
        if (!ready) {
            return 0;
        }

        const int manager_seat = manager_seat_;
        lock();
        ready_flags_[manager_seat] = ready;
        relay_battle_mgr_.OnReady(manager_seat, ready);
        unlock();

        if (!check_start_state()) {
            ready_flags_[manager_seat] = 0;
            relay_battle_mgr_.OnReady(manager_seat, false);
            return 22;
        }
        if (!check_team_pvp()) {
            return 14;
        }

        make_ready_info(reinterpret_cast<char*>(&packet), manager_seat);
        send_to_pvp(packet);
        return -1;
    }

    int seat = 0;
    lock();
    for (; seat <= 7; ++seat) {
        if (users_[seat] == user) {
            ready_flags_[seat] = ready;
            relay_battle_mgr_.OnReady(seat, ready);
            break;
        }
    }
    unlock();
    if (seat <= 7) {
        make_ready_info(reinterpret_cast<char*>(&packet), seat);
        send_to_pvp(packet);
    }
    return 0;
}

// IDA_EA: 0x85DDFBE
// IDA_NAME: _ZN8PvP_Room13request_fightEP5CUser
// DEMANGLED: PvP_Room::request_fight(CUser *)
int PvP_Room::request_fight(CUser* user) {
    if (timed_out_) {
        return 1;
    }

    const int seat = get_user_seat(user);
    if (seat < 0) {
        return 0;
    }

    relay_battle_mgr_.OnRequestFight(seat);
    return 1;
}

// IDA_EA: 0x85D7D50
// IDA_NAME: _ZN8PvP_Room11change_teamEii
// DEMANGLED: PvP_Room::change_team(int,int)
void PvP_Room::change_team(int seat, int team) {
    lock();
    teams_[seat] = team;
    relay_battle_mgr_.OnChangeTeam(seat, team);
    unlock();
}

// IDA_EA: 0x85D87BE
// IDA_NAME: _ZN8PvP_Room24GetCandidateMapIndexListERSt6vectorIiSaIiEE
// DEMANGLED: PvP_Room::GetCandidateMapIndexList(std::vector<int,std::allocator<int>> &)
bool PvP_Room::GetCandidateMapIndexList(std::vector<int>& map_indexes) {
    bool individual_mode = false;
    CMapList* const map_list = G_CDataManager()->get_map_list();
    const int map_count = map_list->GetPVPMapCount();
    int map_no = 1;
    for (; map_no < map_count + 1 && map_no <= 19; ++map_no) {
        if (!IsDeathMatchMap(map_no, individual_mode) ||
            (battle_mode_ != 3 &&
             ((battle_mode_ != 1 && battle_mode_ != 4) ||
              individual_mode))) {
            if (G_GameWorld()->GetChannelType() != 7) {
                if (PVPMAP_ONLY_NORMAL_EXPOSURE[map_no] == 1) {
                    map_indexes.push_back(map_no);
                }
            } else if (PVPMAP_ONLY_TOURNAMENT_RANDOM[map_no] == 1) {
                map_indexes.push_back(map_no);
            }
        }
    }
    return map_no < map_count + 1 && map_no <= 19;
}

// IDA_EA: 0x85D88AE
// IDA_NAME: _ZN8PvP_Room18DrawRandomMapIndexERSt6vectorIiSaIiEE
// DEMANGLED: PvP_Room::DrawRandomMapIndex(std::vector<int,std::allocator<int>> &)
int PvP_Room::DrawRandomMapIndex(std::vector<int>& map_indexes) {
    bool individual_mode = false;
    std::vector<int> death_match_maps;
    for (std::vector<int>::iterator it = map_indexes.begin();
         it != map_indexes.end();) {
        if (IsDeathMatchMap(*it, individual_mode)) {
            death_match_maps.push_back(*it);
            it = map_indexes.erase(it);
        } else {
            ++it;
        }
    }

    if (death_match_maps.empty() && map_indexes.empty()) {
        CMapList* const map_list = G_CDataManager()->get_map_list();
        return std::rand() % map_list->GetPVPMapCount() + 1;
    }
    if (death_match_maps.empty()) {
        return map_indexes[static_cast<unsigned int>(std::rand()) %
                           map_indexes.size()];
    }
    if (map_indexes.empty()) {
        return death_match_maps[static_cast<unsigned int>(std::rand()) %
                                death_match_maps.size()];
    }
    return map_indexes[static_cast<unsigned int>(std::rand()) %
                       map_indexes.size()];
}

// IDA_EA: 0x85D8B2A
// IDA_NAME: _ZN8PvP_Room13ChangePvPModeEi
// DEMANGLED: PvP_Room::ChangePvPMode(int)
PvP_Room* PvP_Room::ChangePvPMode(int map_no) {
    bool individual_mode = false;
    PVP_BATTLE_MODE& mode = battle_mode_;
    if (IsDeathMatchMap(map_no, individual_mode)) {
        if (mode != 4 && mode != 5) {
            if (mode == 1) {
                mode = static_cast<PVP_BATTLE_MODE>(4);
                return this;
            }
            if (mode == 2) {
                mode = static_cast<PVP_BATTLE_MODE>(5);
                return this;
            }
        }
    } else if (mode == 4) {
        mode = PVP_BATTLE_MODE_1;
        return this;
    } else if (mode == 5) {
        mode = PVP_BATTLE_MODE_2;
        return this;
    }
    return reinterpret_cast<PvP_Room*>(static_cast<intptr_t>(mode));
}

// IDA_EA: 0x85D8BBC
// IDA_NAME: _ZN8PvP_Room15SelectRandomMapEv
// DEMANGLED: PvP_Room::SelectRandomMap(void)
int PvP_Room::SelectRandomMap() {
    std::vector<int> map_indexes;
    GetCandidateMapIndexList(map_indexes);
    const int map_no = DrawRandomMapIndex(map_indexes);
    ChangePvPMode(map_no);
    return map_no;
}

// IDA_EA: 0x85DA086
// IDA_NAME: _ZN8PvP_Room33CalculateVictoryPointOfSinglePlayEibi
// DEMANGLED: PvP_Room::CalculateVictoryPointOfSinglePlay(int,bool,int)
int PvP_Room::CalculateVictoryPointOfSinglePlay(int rank,
                                                bool won,
                                                int point) {
    if (won) {
        return point + 1;
    }
    switch (rank) {
    case 1: return (point + 1) / 2;
    case 2: return (point + 1) / 4;
    case 3: return (point + 1) / 8;
    case 4: return (point + 1) / 16;
    case 5: return (point + 1) / 32;
    case 6: return (point + 1) / 64;
    case 7: return (point + 1) / 128;
    case 0: return 1;
    }
    cMyTrace trace(
        "int PvP_Room::CalculateVictoryPointOfSinglePlay(int, bool, int)",
        2896,
        5);
    trace("invalid rank %d", rank);
    int undefined_result;
    return undefined_result;
}

// IDA_EA: 0x85DB7F6
// IDA_NAME: _ZN8PvP_Room14make_room_infoEPc
// DEMANGLED: PvP_Room::make_room_info(char *)
int PvP_Room::make_room_info(char* packet_buffer) {
    PacketGuard& packet = *reinterpret_cast<PacketGuard*>(packet_buffer);
    packet.put_short(index_);
    packet.put_byte(room_title_mode_);
    if (!room_title_mode_) {
        const int length = static_cast<int>(std::strlen(room_title_));
        packet.put_int(length);
        packet.put_str(room_title_, length);
    }
    packet.put_byte(battle_state_);
    packet.put_byte(manager_seat_);
    packet.put_short(requested_map_no_);
    packet.put_byte(battle_mode_);
    for (int seat = 0; seat <= 7; ++seat) {
        packet.put_byte(teams_[seat]);
        CUser* const user = users_[seat];
        if (user) {
            packet.put_short(user->get_unique_id());
            packet.put_byte(user->isAllowLocationGeoState());
        } else {
            packet.put_short(0xFFFF);
            packet.put_byte(0);
        }
    }
    packet.put_byte(password_exists_);
    return packet.put_int(current_match_->GetIndentification());
}

// IDA_EA: 0x85DBCCE
// IDA_NAME: _ZN8PvP_Room15make_ready_infoEPci
// DEMANGLED: PvP_Room::make_ready_info(char *,int)
int PvP_Room::make_ready_info(char* packet_buffer, int seat) {
    PacketGuard& packet = *reinterpret_cast<PacketGuard*>(packet_buffer);
    packet.put_header(0, 44);
    packet.put_byte(seat);
    packet.put_byte(ready_flags_[seat]);
    return packet.finalize(true);
}

// IDA_EA: 0x85DBE18
// IDA_NAME: _ZN8PvP_Room11send_to_pvpER11PacketGuard
// DEMANGLED: PvP_Room::send_to_pvp(PacketGuard &)
bool PvP_Room::send_to_pvp(PacketGuard& packet) {
    int seat = 0;
    for (; seat <= 7; ++seat) {
        CUser* const user = users_[seat];
        if (user) {
            user->Send(packet);
        }
    }
    return seat <= 7;
}

// IDA_EA: 0x85DBE64
// IDA_NAME: _ZN8PvP_Room16send_to_pvp_teamER11PacketGuardP5CUser
// DEMANGLED: PvP_Room::send_to_pvp_team(PacketGuard &,CUser *)
bool PvP_Room::send_to_pvp_team(PacketGuard& packet, CUser* sender) {
    if (battle_mode_ <= 1) {
        return battle_mode_ != 0;
    }

    int sender_team = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (users_[seat] &&
            !IsPvpObserver(seat) &&
            users_[seat] == sender) {
            sender_team = teams_[seat];
            break;
        }
    }

    if (sender_team) {
        for (int seat = 0; seat <= 7; ++seat) {
            if (users_[seat] &&
                !IsPvpObserver(seat) &&
                teams_[seat] == sender_team) {
                users_[seat]->Send(packet);
            }
        }
    }
    return false;
}

// IDA_EA: 0x85DC23C
// IDA_NAME: _ZN8PvP_Room32insert_timer_recv_pvp_end_resultEv
// DEMANGLED: PvP_Room::insert_timer_recv_pvp_end_result(void)
int PvP_Room::insert_timer_recv_pvp_end_result() {
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(static_cast<TIMER_MESSAGE>(61)));
    const int delay_msec = current_match_->GetEndPvPResultTimeout();
    G_TimerQueue()->InsertTimerInMilisecond(
        1, index_, 61, delay_msec, key, 0);
    return 1;
}

// IDA_EA: 0x85DC340
// IDA_NAME: _ZN8PvP_Room26insert_timer_recv_pvp_rankEv
// DEMANGLED: PvP_Room::insert_timer_recv_pvp_rank(void)
int PvP_Room::insert_timer_recv_pvp_rank() {
    if (!recv_pvp_rank_timer_inserted_) {
        const unsigned int key = static_cast<unsigned int>(
            gen_timer_key(static_cast<TIMER_MESSAGE>(60)));
        G_TimerQueue()->InsertTimerInMilisecond(
            1, index_, 60, 10000, key, 0);
        recv_pvp_rank_timer_inserted_ = 1;
    }
    return 1;
}

// IDA_EA: 0x85DC3CA
// IDA_NAME: _ZN8PvP_Room19check_recv_pvp_rankEv
// DEMANGLED: PvP_Room::check_recv_pvp_rank(void)
int PvP_Room::check_recv_pvp_rank() {
    int checked_out = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = users_[seat];
        if (user && teams_[seat] != 255 &&
            recv_pvp_rank_.IsRecv(seat) != 1) {
            state_flag_0c0_ = 0;
            G_CGameManager()->CheckOutPvp(user, true);
            checked_out = 1;
        }
    }
    return checked_out;
}

// IDA_EA: 0x85DCD7E
// IDA_NAME: _ZN8PvP_Room20pvp_request_pvp_rankEv
// DEMANGLED: PvP_Room::pvp_request_pvp_rank(void)
void PvP_Room::pvp_request_pvp_rank() {
    if (!pvp_rank_requested_) {
        pvp_rank_requested_ = 1;
        PacketGuard packet;
        packet.put_header(0, 49);
        packet.finalize(true);
        send_to_pvp(packet);
        state_flag_0c0_ = 1;
    }
}

// IDA_EA: 0x85DD3F8
// IDA_NAME: _ZN8PvP_Room11SendWalkOutEi17ENUM_WALKOUT_TYPE
// DEMANGLED: PvP_Room::SendWalkOut(int,ENUM_WALKOUT_TYPE)
int PvP_Room::SendWalkOut(int seat, ENUM_WALKOUT_TYPE type) {
    PacketGuard packet;
    packet.put_header(0, 10);
    packet.put_byte(seat);
    packet.put_byte(static_cast<int>(type));
    packet.finalize(true);
    send_to_pvp(packet);
    return 1;
}

// IDA_EA: 0x85DD6BA
// IDA_NAME: _ZN8PvP_Room12ClearPvPRoomEP5CUser
// DEMANGLED: PvP_Room::ClearPvPRoom(CUser *)
void PvP_Room::ClearPvPRoom(CUser* /*user*/) {
    destroy_room(0);
    G_CGameManager()->PutPvp(this);
    PacketGuard packet;
    packet.clear();
    make_state_info(reinterpret_cast<char*>(&packet));
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x85DD824
// IDA_NAME: _ZN8PvP_Room21HandleTimerUserReviveEi
// DEMANGLED: PvP_Room::HandleTimerUserRevive(int)
void PvP_Room::HandleTimerUserRevive(int seat) {
    if (zombie_mode::IsRevivalBlocked(this)) return;
    if (static_cast<unsigned int>(seat) < 8u &&
        users_[seat] && !battle_live_flags_[seat]) {
        battle_live_flags_[seat] = 1;
        PacketGuard packet;
        packet.put_header(0, 120);
        packet.put_byte(seat);
        packet.put_byte(std::rand() % 256);
        packet.finalize(true);
        send_to_pvp(packet);
    }
}
