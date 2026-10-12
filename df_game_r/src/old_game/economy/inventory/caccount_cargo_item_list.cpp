#include "old_game/economy/inventory/inventory.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/inventory/integrated_pvp_item_attr.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

// IDA_EA: 0x828A88A
// IDA_NAME: _ZN13CAccountCargo12SendItemListEv
// DEMANGLED: CAccountCargo::SendItemList(void)
int CAccountCargo::SendItemList() {
    PacketGuard packet;
    packet.put_header(0, 13);
    packet.put_byte(12);
    packet.put_short(static_cast<int>(capacity_));
    packet.put_int(static_cast<int>(money_));
    packet.put_short(GetItemCount());
    for (unsigned int slot = 0; slot < capacity_; ++slot) {
        const Inven_Item& item = items_[slot];
        if (!item.item_id_) continue;

        packet.put_short(static_cast<int>(slot));
        packet.put_int(item.item_id_);
        packet.put_int(item.add_info_);
        packet.put_byte(GetIntegratedPvPItemAttr(item));
        packet.put_short(item.avatar_select_);
        packet.put_byte(item.seal_or_state_);
        if (G_GameWorld()->IsEnchantRevisionChannel()) {
            packet.put_int(0);
        } else {
            packet.put_int(item.extend_info_);
        }
        packet.put_byte(item.amplify_option_.getAbilityType());
        packet.put_short(item.amplify_option_.getAbilityValue());
        // 0828 NOTI13 account-cargo records use the native third-Chronicle
        // owner before the shared Inven_Item tail.
        (G_CDataManager()->get_item_upgrade())->make3rdChroniclePacket(
            user_, item, &packet);
        packet.put_packet(item);
    }
    packet.finalize(true);
    return user_->Send(packet);
}

// IDA_EA: 0x828AB1C
// IDA_NAME: _ZNK13CAccountCargo14MakeItemPacketEP11PacketGuardi
// DEMANGLED: CAccountCargo::MakeItemPacket(PacketGuard *,int)const
int CAccountCargo::MakeItemPacket(PacketGuard* packet, int slot) const {
    packet->put_short(slot);

    const Inven_Item& item = items_[slot];
    if (item.item_id_) {
        packet->put_int(item.item_id_);
        packet->put_int(item.add_info_);
        packet->put_byte(GetIntegratedPvPItemAttr(item));
        packet->put_short(item.avatar_select_);
        packet->put_byte(item.seal_or_state_);
        if (G_GameWorld()->IsEnchantRevisionChannel()) {
            packet->put_int(0);
        } else {
            packet->put_int(item.extend_info_);
        }
        packet->put_byte(item.amplify_option_.getAbilityType());
        packet->put_short(item.amplify_option_.getAbilityValue());
        // Keep incremental item updates byte-identical to the world-entry
        // record: CItemUpgrade owns the third Chronicle list.
        (G_CDataManager()->get_item_upgrade())->make3rdChroniclePacket(
            user_, item, packet);
        return packet->put_packet(item);
    }

    packet->put_int(-1);
    packet->put_int(0);
    packet->put_byte(0);
    packet->put_short(0);
    packet->put_byte(0);
    packet->put_int(0);
    packet->put_byte(0);
    packet->put_short(0);
    packet->put_byte(0);
    return packet->put_packet(g_emptySlot);
}
