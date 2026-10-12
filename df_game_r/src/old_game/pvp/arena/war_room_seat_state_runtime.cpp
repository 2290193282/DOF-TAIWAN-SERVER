#include "old_game/pvp/battlefield/war_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86BD0F8
// IDA_NAME: _ZN7WarRoom12SetSlotStateEP5CUseri15ENUM_SEAT_STATEPS1_RbS4_S4_
// DEMANGLED: WarRoom::SetSlotState(CUser *,int,ENUM_SEAT_STATE,CUser **,
//                                  bool &,bool &,bool &)
int WarRoom::SetSlotState(CUser* user,
                          int slot,
                          ENUM_SEAT_STATE state,
                          CUser** leaving_user,
                          bool& slot_info_changed,
                          bool&,
                          bool&) {
    if (!user) {
        return 0;
    }

    if (slot < 0 || slot >= 6) {
        return 1;
    }

    const int state_value = static_cast<int>(state);
    if (state_value == 255) {
        if (!war_field_.room_users_[slot]) {
            cMyTrace trace(
                "int WarRoom::SetSlotState(CUser*, int, ENUM_SEAT_STATE, "
                "CUser**, bool&, bool&, bool&)",
                0x590,
                5);
            trace("WARAREA  WarRoom::SetSlotState players_[pos] is NULL "
                  "pos(%d)",
                  slot);
            return 0;
        }

        // Every native CMD91 leave sender passes the caller's current seat.
        // Never allow a forged seat byte to evict another room occupant.
        if (war_field_.room_users_[slot] != user) {
            return 8;
        }

        *leaving_user = war_field_.room_users_[slot];
        war_field_.seat_states_[slot] = 255;
        war_field_.room_users_[slot] = 0;
        war_field_.live_flags_[slot] = 0;
        slot_info_changed = true;

        if (GetState() > 2) {
            AddBanList(user);
        }
        user->getHackAnalyzer()->reportHackInfo();
        user->getHackAnalyzer()->resetHackInfo();

        if (war_field_.dungeon_) {
            CUser_history_log(user)->LeaveDungeon(
                war_field_.dungeon_->GetDungeonName(),
                static_cast<int>(room_mode_));
        }
        return 0;
    }

    if (state_value < 0 || state_value > 2 ||
        war_field_.room_users_[slot] != user) {
        return 8;
    }

    // 140828 adds the visible occupant states 0/1/2.  They do not change the
    // WarRoom's authoritative membership or live flags; NOTI89 carries the
    // updated role to every client.
    war_field_.seat_states_[slot] = state_value;
    slot_info_changed = true;

    return 0;
}

// IDA_EA: 0x86BF484
// IDA_NAME: _ZN7WarRoom18HandleSetSeatStateEP5CUseri15ENUM_SEAT_STATE
// DEMANGLED: WarRoom::HandleSetSeatState(CUser *,int,ENUM_SEAT_STATE)
void WarRoom::HandleSetSeatState(CUser* user,
                                 int slot,
                                 ENUM_SEAT_STATE state) {
    PacketGuard packet;
    CUser* leaving_user = 0;
    bool slot_info_changed = false;
    bool state_info_changed = false;
    bool room_reset = false;

    const int result = SetSlotState(user,
                                    slot,
                                    state,
                                    &leaving_user,
                                    slot_info_changed,
                                    state_info_changed,
                                    room_reset);
    if (!result) {
        if (leaving_user) {
            G_GameWorld()->out_from_warroom(leaving_user);
        }
        if (slot_info_changed) {
            packet.clear();
            MakeSlotInfo(reinterpret_cast<char*>(&packet), slot);
            G_GameWorld()->send_all(packet);
        }
    }

    if (result > 0) {
        packet.put_header(1, ENUM_CMDPACKET_SET_WARROOM_SEAT_STATE);
        packet.put_byte(0);
        packet.put_byte(result);
        packet.finalize(true);
        user->Send(packet);
    }
}
