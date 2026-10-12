#include "old_game/pvp/battlefield/war_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86BD87E
// IDA_NAME: _ZN7WarRoom17HandleDieCharacerEP5CUseri
// DEMANGLED: WarRoom::HandleDieCharacer(CUser *,int)
int WarRoom::HandleDieCharacer(CUser* user, int killer_slot) {
    if (GetState() != 3 && GetState() != 4 && GetState() != 5) {
        return 19;
    }

    const int user_slot = GetUserSlot(user);
    if (user_slot < 0) {
        return 4;
    }
    if (war_field_.live_flags_[user_slot] != 1) {
        return 18;
    }

    if (killer_slot != 255 && (killer_slot < 0 || killer_slot > 5)) {
        cMyTrace trace("int WarRoom::HandleDieCharacer(CUser*, int)",
                       0x6d0,
                       5);
        trace("WarRoom::HandleDieCharacer killerSlotIndex Error(%d)",
              killer_slot);
        return 1;
    }

    short killer_unique_id = -1;
    if (killer_slot != 255) {
        CUser* const killer = war_field_.room_users_[killer_slot];
        if (killer) {
            killer_unique_id = static_cast<short>(killer->get_unique_id());
        } else {
            cMyTrace trace("int WarRoom::HandleDieCharacer(CUser*, int)",
                           0x6de,
                           5);
            trace("WARAREA WarRoom::HandleDieCharacer player_[] is NULL "
                  "killerSlotIndex(%d)",
                  killer_slot);
        }
    }

    SetCharacterLive(user_slot,
                     user,
                     false,
                     false,
                     killer_unique_id,
                     killer_slot);

    if (killer_slot != 255 && war_field_.room_users_[killer_slot]) {
        const int kill_point =
            battle_finished_
                ? 0
                : war_field_.dungeon_->war_room_data_.condition_mode_;
        war_field_.room_users_[killer_slot]->WarAreaKill(4, kill_point, 1.0f);

        PacketGuard killer_packet;
        killer_packet.clear();
        killer_packet.put_header(1, ENUM_CMDPACKET_DIE_WARROOM_CHARACTER);
        killer_packet.put_byte(1);
        killer_packet.put_byte(killer_slot);
        killer_packet.finalize(true);
        SendToRoom(killer_packet);
    }

    // +0x10 gates the final ranking phase.  +0x12 only changes the
    // immediate killer-point award above.
    if (result_phase_active_) {
        ReviveAll();
        if (current_spawn_step_ ==
                war_field_.dungeon_->war_room_data_.timing_values_[0] ||
            result_phase_sent_) {
            return 0;
        }

        PacketGuard result_packet;
        result_packet.put_header(0, 342);
        result_packet.put_byte(GetWaiterCount());
        const int penalty =
            G_CDataManager()->get_war_area_counter()->wp_penalty;
        for (int slot = 0; slot <= 5; ++slot) {
            CUser* const room_user = war_field_.room_users_[slot];
            if (!room_user) {
                continue;
            }

            if (room_user->GetFinishPointTotal() > penalty) {
                room_user->DecreaseWarPoint(penalty);
            } else {
                room_user->DecreaseWarPoint(0);
            }
            result_packet.put_byte(slot);
            result_packet.put_int(room_user->GetFinishPointTotal());
        }
        result_packet.finalize(true);
        SendToRoom(result_packet);
        result_phase_sent_ = 1;
    } else if (!GetLiveCount()) {
        SendWarRoomFailMsg();
    }

    return 0;
}
