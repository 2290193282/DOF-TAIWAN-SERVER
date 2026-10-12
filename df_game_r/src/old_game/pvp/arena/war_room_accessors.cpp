#include <stdint.h>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x822EE8A
// IDA_NAME: _ZN7WarRoom8GetIndexEv
// DEMANGLED: WarRoom::GetIndex(void)
int WarRoom::GetIndex() {
    return room_index_;
}

// IDA_EA: 0x822EE96
// IDA_NAME: _ZN7WarRoom8GetStateEv
// DEMANGLED: WarRoom::GetState(void)
int WarRoom::GetState() {
    return war_field_.room_state_;
}

// IDA_EA: 0x82A5A56
// IDA_NAME: _ZN7WarRoom8SetStateE13WARROOM_STATE
// DEMANGLED: WarRoom::SetState(WARROOM_STATE)
WarRoom* WarRoom::SetState(WARROOM_STATE state) {
    war_field_.room_state_ = static_cast<int32_t>(state);
    return this;
}

// IDA_EA: 0x863BD92
// IDA_NAME: _ZN7WarRoom13CheckTimerKeyE13TIMER_MESSAGEi
// DEMANGLED: WarRoom::CheckTimerKey(TIMER_MESSAGE,int)
bool WarRoom::CheckTimerKey(TIMER_MESSAGE message, int key) {
    return war_field_.timer_keys_[static_cast<int>(message) - 63] == key;
}

// IDA_EA: 0x86BE0CC
// IDA_NAME: _ZN7WarRoom10SendToRoomER11PacketGuard
// DEMANGLED: WarRoom::SendToRoom(PacketGuard &)
bool WarRoom::SendToRoom(PacketGuard& packet) {
    for (int slot = 0; slot <= 5; ++slot) {
        if (war_field_.room_users_[slot]) {
            war_field_.room_users_[slot]->Send(packet);
        }
    }
    return false;
}

// IDA_EA: 0x86BDD58
// IDA_NAME: _ZN7WarRoom12MakeRoomInfoEPc
// DEMANGLED: WarRoom::MakeRoomInfo(char *)
bool WarRoom::MakeRoomInfo(char* packet_buffer) {
    PacketGuard& packet = *reinterpret_cast<PacketGuard*>(packet_buffer);

    packet.put_short(room_index_);
    packet.put_byte(room_flag_008_);
    packet.put_byte(room_mode_);
    packet.put_byte(room_level_);
    packet.put_byte(war_field_.room_state_);
    if (war_field_.dungeon_) {
        packet.put_short(war_field_.dungeon_->get_index());
    } else {
        packet.put_short(0);
    }

    for (int slot = 0; slot <= 5; ++slot) {
        packet.put_byte(war_field_.seat_states_[slot]);
        CUser* const room_user = war_field_.room_users_[slot];
        packet.put_short(room_user ? room_user->get_unique_id() : 0xFFFF);
    }

    // The original loop exits with n5 == 6 and returns its n5 <= 5 test.
    return false;
}
