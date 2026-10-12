#include "old_game/pvp/arena/pvp_room.h"

#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85DD4A4
// IDA_NAME: _ZN8PvP_Room7WalkOutEP5CUseri15ENUM_SEAT_STATE17ENUM_WALKOUT_TYPE
// DEMANGLED: PvP_Room::WalkOut(CUser *,int,ENUM_SEAT_STATE,ENUM_WALKOUT_TYPE)
int PvP_Room::WalkOut(CUser* user,
                      int seat,
                      ENUM_SEAT_STATE state,
                      ENUM_WALKOUT_TYPE walkout_type) {
    PacketGuard packet;
    CUser* leaving_user = 0;
    bool seat_info_changed = false;
    bool state_info_changed = false;
    bool room_reset = false;

    const int team = get_team(seat);
    SendWalkOut(seat, walkout_type);
    const int result = set_seat_state(user,
                                      seat,
                                      state,
                                      &leaving_user,
                                      seat_info_changed,
                                      state_info_changed,
                                      room_reset);
    if (!result) {
        if (leaving_user) {
            current_match_->OnLeaveRoom(leaving_user, this, team, seat);
            G_GameWorld()->out_from_pvp(leaving_user);
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
    }
    return room_reset ? 0 : 1;
}

// IDA_EA: 0x85DD754
// IDA_NAME: _ZN8PvP_Room10DisconnectEP5CUser
// DEMANGLED: PvP_Room::Disconnect(CUser *)
int PvP_Room::Disconnect(CUser* user) {
    int seat = 0;
    for (; seat <= 7; ++seat) {
        if (get_user_seat(seat) == user) {
            break;
        }
    }
    if (seat > 7) {
        return 0;
    }

    current_match_->OnWalkOut(this, seat);
    if (WalkOut(user,
                seat,
                static_cast<ENUM_SEAT_STATE>(254),
                static_cast<ENUM_WALKOUT_TYPE>(3)) != 1) {
        ClearPvPRoom(user);
    }
    return 1;
}
