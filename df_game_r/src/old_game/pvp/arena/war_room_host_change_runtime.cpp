#include "old_game/pvp/battlefield/war_room.h"

#include <stdint.h>

#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kHostChangeTimerMessage = 71;
const int kHostChangeDelaySeconds = 5;
const int kHostChangeRoomPacket = 352;

}  // namespace

// IDA_EA: 0x86C08A6
// IDA_NAME: _ZN7WarRoom18HostChangeWarRoomEP5CUsercm
// DEMANGLED: WarRoom::HostChangeWarRoom(CUser *,char,unsigned long)
int WarRoom::HostChangeWarRoom(CUser* user, char seat, unsigned long value) {
    // The original indexes using its signed char parameter.  Every recovered
    // caller checks the C2S byte as an unsigned value before reaching here.
    const int seat_index = seat;
    if (war_field_.room_users_[seat_index] &&
        war_field_.room_users_[seat_index] == user) {
        host_change_values_[seat_index] = static_cast<uint32_t>(value);
        if (host_change_request_count_ == 0) {
            const TIMER_MESSAGE message =
                static_cast<TIMER_MESSAGE>(kHostChangeTimerMessage);
            const unsigned int key =
                static_cast<unsigned int>(GenTimerKey(message));
            G_TimerQueue()->InsertTimer(3,
                                        room_index_,
                                        kHostChangeTimerMessage,
                                        kHostChangeDelaySeconds,
                                        key,
                                        0);
        }
        ++host_change_request_count_;
    }
    return 0;
}

// IDA_EA: 0x86C0964
// IDA_NAME: _ZN7WarRoom15CheckHostChangeEv
// DEMANGLED: WarRoom::CheckHostChange(void)
void WarRoom::CheckHostChange() {
    // 0x86C0975 takes the reset path without constructing a packet when one
    // or fewer valid requests were accumulated during the five-second window.
    if (host_change_request_count_ <= 1) {
        for (int seat = 0; seat <= 5; ++seat) {
            host_change_values_[seat] = UINT32_MAX;
        }
        host_change_request_count_ = 0;
        return;
    }

    bool sent = false;
    {
        PacketGuard packet;
        packet.put_header(0, kHostChangeRoomPacket);

        uint32_t selected_value = UINT32_MAX;
        int selected_seat = -1;
        for (int seat = 0; seat <= 5; ++seat) {
            if (war_field_.room_users_[seat] &&
                host_change_values_[seat] != UINT32_MAX &&
                host_change_values_[seat] <= selected_value) {
                selected_value = host_change_values_[seat];
                selected_seat = seat;
            }
        }

        if (selected_seat != -1) {
            packet.put_byte(selected_seat);
            packet.finalize(true);
            SendToRoom(packet);
            sent = true;
        }
    }

    // The no-candidate path at 0x86C0A1C deliberately leaves the pending
    // values and request count intact.  Only a sent packet reaches the reset.
    if (sent) {
        for (int seat = 0; seat <= 5; ++seat) {
            host_change_values_[seat] = UINT32_MAX;
        }
        host_change_request_count_ = 0;
    }
}
