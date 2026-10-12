#include "old_game/pvp/battlefield/war_room.h"

#include <vector>

#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kWarRoomFile[] = "WarRoom.cpp";
const char kSendGuildFpFunction[] = "void WarRoom::SendGuildFP()";

}  // namespace

// IDA_EA: 0x86BC0B0
// IDA_NAME: _ZN7WarRoom11SendGuildFPEv
// DEMANGLED: WarRoom::SendGuildFP(void)
void WarRoom::SendGuildFP() {
    PacketGuard packet;
    packet.put_header(0, 96);
    packet.put_byte(GetWaiterCount());

    const int kick_point =
        G_CDataManager()->get_war_area_counter()->kick_point;
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (user && user->GetFinishPointTotal() < kick_point) {
            war_field_.walkout_types_[slot] =
                static_cast<ENUM_WALKOUT_TYPE>(9);
        }
    }

    if (GetWaiterCount() > 4) {
        int minimum_finish_point = 0x7fffffff;
        std::vector<int> minimum_slots;
        for (int slot = 0; slot <= 5; ++slot) {
            CUser* const user = war_field_.room_users_[slot];
            if (!user) {
                continue;
            }

            const int finish_point = user->GetFinishPointTotal();
            if (finish_point >= minimum_finish_point) {
                if (finish_point == minimum_finish_point) {
                    minimum_slots.push_back(slot);
                }
            } else {
                minimum_finish_point = finish_point;
                minimum_slots.clear();
                minimum_slots.push_back(slot);
            }
        }

        for (std::vector<int>::const_iterator it = minimum_slots.begin();
             it != minimum_slots.end();
             ++it) {
            war_field_.walkout_types_[*it] =
                static_cast<ENUM_WALKOUT_TYPE>(10);
        }
    }

    bool ranked_slots[6] = {false, false, false, false, false, false};
    for (int rank = 0; rank < GetWaiterCount(); ++rank) {
        int selected_slot = -1;
        int selected_finish_point = -1;
        for (int slot = 0; slot <= 5; ++slot) {
            CUser* const user = war_field_.room_users_[slot];
            if (!user || ranked_slots[slot]) {
                continue;
            }

            const int finish_point = user->GetFinishPointTotal();
            if (finish_point >= selected_finish_point) {
                selected_finish_point = finish_point;
                selected_slot = slot;
            }
        }

        if (selected_slot == -1) {
            LogManager::logFormat(1,
                                  kWarRoomFile,
                                  kSendGuildFpFunction,
                                  0x430,
                                  "maxPlayer (%d)",
                                  -1);
        }

        ranked_slots[selected_slot] = true;
        packet.put_byte(selected_slot);
        packet.put_byte(static_cast<int>(
            war_field_.walkout_types_[selected_slot]));
    }

    packet.finalize(true);
    SendToRoom(packet);
}
