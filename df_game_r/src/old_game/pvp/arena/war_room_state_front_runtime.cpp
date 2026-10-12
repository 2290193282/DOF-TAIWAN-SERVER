#include "old_game/pvp/battlefield/war_room.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <stdint.h>

// IDA_EA: 0x86BA850
// IDA_NAME: _ZN7WarRoom13UpdateBanListEv
// DEMANGLED: WarRoom::UpdateBanList(void)
bool WarRoom::UpdateBanList() {
    const int current_second = GlobalData::s_systemTime_.getCurSec();

    std::map<uint32_t, int>::iterator current = ban_list_.begin();
    const std::map<uint32_t, int>::iterator end = ban_list_.end();
    bool result;
    while ((result = current != end)) {
        const int elapsed_seconds = current_second - current->second;
        // IDA reads the current WarAreaCounter value for every map entry.
        const int ban_interval_seconds =
            G_CDataManager()->get_war_area_counter()->ban_interval / 1000;
        if (elapsed_seconds < ban_interval_seconds) {
            ++current;
        } else {
            ban_list_.erase(current++);
        }
    }
    return result;
}

// IDA_EA: 0x86BA948
// IDA_NAME: _ZN7WarRoom10AddBanListEP5CUser
// DEMANGLED: WarRoom::AddBanList(CUser *)
int WarRoom::AddBanList(CUser* user) {
    int result = user->isGMUser();
    if (!static_cast<unsigned char>(result)) {
        const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
        int& ban_second = ban_list_[account_id];
        result = GlobalData::s_systemTime_.getCurSec();
        ban_second = result;
    }
    return result;
}

// IDA_EA: 0x86BA99E
// IDA_NAME: _ZN7WarRoom9IsBanUserEP5CUser
// DEMANGLED: WarRoom::IsBanUser(CUser *)
bool WarRoom::IsBanUser(CUser* user) {
    const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
    return ban_list_.find(account_id) != ban_list_.end();
}

// IDA_EA: 0x86BB582
// IDA_NAME: _ZN7WarRoom16InvalidUserCheckEv
// DEMANGLED: WarRoom::InvalidUserCheck(void)
bool WarRoom::InvalidUserCheck() {
    int slot = 0;
    bool result;
    for (;;) {
        result = slot <= 5;
        if (!result) {
            break;
        }

        CUser* const user = war_field_.room_users_[slot];
        if (user && !user->getCurCharacR()) {
            // The original takes the signed low 16 bits of GetWarRoomIndex.
            const int war_room_index =
                static_cast<short>(user->GetWarRoomIndex());
            cMyTrace trace("void WarRoom::InvalidUserCheck()", 0x236, 5);
            trace("WarRoom::InvalidUserCheck slot(%d) warRoomIndex(%d)",
                  slot,
                  war_room_index);
            war_field_.seat_states_[slot] = 255;
            war_field_.room_users_[slot] = 0;
            war_field_.live_flags_[slot] = 0;
        }
        ++slot;
    }
    return result;
}

// IDA_EA: 0x86BB672
// IDA_NAME: _ZN7WarRoom15SendStatePacketEv
// DEMANGLED: WarRoom::SendStatePacket(void)
void WarRoom::SendStatePacket() {
    PacketGuard packet;
    packet.put_header(0, 88);
    packet.put_short(1);
    MakeStateInfo(reinterpret_cast<char*>(&packet));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x86BE07A
// IDA_NAME: _ZN7WarRoom13MakeStateInfoEPc
// DEMANGLED: WarRoom::MakeStateInfo(char *)
int WarRoom::MakeStateInfo(char* packet_storage) {
    // `char*` is the original exported parameter spelling.  The caller gives
    // the address of a real InterfacePacketBuf (normally PacketGuard), not a
    // byte-storage facade.
    InterfacePacketBuf& packet =
        *reinterpret_cast<InterfacePacketBuf*>(packet_storage);
    packet.put_short(room_index_);
    packet.put_byte(war_field_.room_state_);
    return packet.put_short(current_spawn_step_);
}
