#include "old_game/pvp/arena/pvp_room.h"

#include <cstring>

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const int kMaxPvpSeats = 8;

typedef unsigned char (*CanSetSeatStateFn)(void*, int);
typedef void (*UpdateMemberFn)(void*, CUser*, PvP_Room*, int, unsigned int,
                               int, unsigned int, int, int);
typedef void (*RemoveMemberFn)(void*, CUser*, PvP_Room*, int, int);

}  // namespace

// IDA_EA: 0x85D77C0
// IDA_NAME: _ZN8PvP_Room14set_seat_stateEP5CUseri15ENUM_SEAT_STATEPS1_RbS4_S4_
// DEMANGLED: PvP_Room::set_seat_state(CUser *,int,ENUM_SEAT_STATE,CUser **,bool &,bool &,bool &)
int PvP_Room::set_seat_state(CUser* user,
                             int seat,
                             ENUM_SEAT_STATE state,
                             CUser** leaving_user,
                             bool& seat_info_changed,
                             bool& state_info_changed,
                             bool& room_reset) {
    if (!user) return 0;

    const int waiter_count = get_waiter_count();
    const int battle_mode = get_pvp_battle_mode();
    const int state_value = static_cast<int>(state);

    if (state_value != 254) {
        if (state_value == 255) {
            if (manager_ != user) return 8;
        } else if (users_[seat] != user) {
            return 8;
        }
        change_team(seat, state_value);
        seat_info_changed = true;
        return 0;
    }

    if (battle_state_ == 2) {
        if (!users_[seat]) return 4;
        if (manager_ == users_[seat]) {
            select_new_manager();
            seat_info_changed = true;
            state_info_changed = true;
        }
        *leaving_user = users_[seat];
        GiveOuterPenalty(*leaving_user);
        users_[seat]->SetPvpIndex(static_cast<short>(-1));
        const int team = get_team(seat);
        lock();
        ready_flags_[seat] = 0;
        teams_[seat] = 255;
        users_[seat] = 0;
        --member_count_;
        battle_live_flags_[seat] = 0;
        unlock();
        relay_battle_mgr_.OnLeaveRoom(seat);

        void* const matching = current_match_;
        void** const vtable = *reinterpret_cast<void***>(matching);
        reinterpret_cast<UpdateMemberFn>(vtable[11])(
            matching, *leaving_user, this, team,
            static_cast<unsigned int>(seat), battle_mode,
            static_cast<unsigned int>(seat), 254, waiter_count);

        if (get_waiter_count()) {
            if (check_winner()) {
                state_info_changed = true;
                pvp_request_pvp_rank();
            }
            seat_info_changed = true;
            return 0;
        }
        reset();
        seat_info_changed = true;
        state_info_changed = true;
        room_reset = true;
        return 0;
    }

    if (manager_ == user) {
        if (!users_[seat]) {
            if (!is_closeable_seat_state()) return 19;
            change_team(seat, state_value);
            seat_info_changed = true;
            return 0;
        }
        users_[seat]->SetPvpIndex(static_cast<short>(-1));
        if (users_[seat] == manager_) {
            select_new_manager();
            state_info_changed = true;
        }
        *leaving_user = users_[seat];
        ready_flags_[seat] = 0;
        teams_[seat] = 255;
        users_[seat] = 0;
        relay_battle_mgr_.OnChangeTeam(seat, 255);
        if (!get_waiter_count()) {
            reset();
            seat_info_changed = true;
            state_info_changed = true;
            room_reset = true;
            return 0;
        }
        seat_info_changed = true;
        state_info_changed = true;
        return 0;
    }

    if (!users_[seat]) return 0;

    users_[seat]->SetPvpIndex(static_cast<short>(-1));
    *leaving_user = users_[seat];
    const int team = get_team(seat);
    ready_flags_[seat] = 0;
    teams_[seat] = 255;
    users_[seat] = 0;
    --member_count_;
    relay_battle_mgr_.OnLeaveRoom(seat);

    void* const matching = current_match_;
    void** const vtable = *reinterpret_cast<void***>(matching);
    reinterpret_cast<UpdateMemberFn>(vtable[11])(
        matching, *leaving_user, this, team, static_cast<unsigned int>(seat),
        battle_mode, static_cast<unsigned int>(seat), 254, waiter_count);
    if (get_waiter_count()) {
        seat_info_changed = true;
        return 0;
    }

    reset();
    seat_info_changed = true;
    state_info_changed = true;
    room_reset = true;
    return 0;
}

// IDA_EA: 0x85DD070
// IDA_NAME: _ZN8PvP_Room17OnSetPVPSeatStateEP5CUseri15ENUM_SEAT_STATE17ENUM_WALKOUT_TYPE
// DEMANGLED: PvP_Room::OnSetPVPSeatState(CUser *,int,ENUM_SEAT_STATE,ENUM_WALKOUT_TYPE)
int PvP_Room::OnSetPVPSeatState(CUser* user,
                                 int seat,
                                 ENUM_SEAT_STATE state,
                                 ENUM_WALKOUT_TYPE walkout_type) {
    if (!user) return 1;

    PacketGuard packet;
    int result = 0;
    if (!check_authority(user, seat)) {
        result = 8;
    } else {
        void* const matching = current_match_;
        void** const vtable = *reinterpret_cast<void***>(matching);
        if (reinterpret_cast<CanSetSeatStateFn>(vtable[29])(
                matching, static_cast<int>(state)) != 1) {
            result = 8;
        } else if (battle_state_ == 2) {
            result = 19;
        } else {
            CUser* leaving_user = 0;
            bool seat_info_changed = false;
            bool state_info_changed = false;
            bool room_reset = false;
            const int team = get_team(seat);

            if (static_cast<int>(state) == 254) {
                if (static_cast<int>(walkout_type) == 3) {
                    SendWalkOut(seat, walkout_type);
                } else if (manager_ == user && users_[seat] &&
                           users_[seat] != user) {
                    SendWalkOut(seat, walkout_type);
                }
            }

            result = set_seat_state(user, seat, state, &leaving_user,
                                    seat_info_changed, state_info_changed,
                                    room_reset);
            if (!get_waiter_count() && !room_reset) {
                // The raw body logs this inconsistent empty-room state.
                // It remains a diagnostic only and does not alter the state.
            }
            if (!result) {
                if (leaving_user) {
                    void* const callback_matching = current_match_;
                    void** const callback_vtable =
                        *reinterpret_cast<void***>(callback_matching);
                    reinterpret_cast<RemoveMemberFn>(callback_vtable[11])(
                        callback_matching, leaving_user, this, team, seat);
                    G_GameWorld()->out_from_pvp(leaving_user);
                    leaving_user->insert_pvp_masterid_walkingout_me(
                        static_cast<unsigned int>(user->getCurCharacNo()));
                }
                if (seat_info_changed) {
                    packet.clear();
                    make_seat_info(reinterpret_cast<char*>(&packet), seat);
                    G_GameWorld()->send_all(packet);
                }
                if (state_info_changed) {
                    packet.clear();
                    make_state_info(reinterpret_cast<char*>(&packet));
                    G_GameWorld()->send_all(packet);
                }
                if (room_reset) return 0;
            }
        }
    }

    if (result > 0) {
        packet.put_header(1, ENUM_CMDPACKET_SET_PVP_SEAT_STATE);
        packet.put_byte(0);
        packet.put_byte(result);
        packet.finalize(true);
        user->Send(packet);
    }
    return 1;
}
