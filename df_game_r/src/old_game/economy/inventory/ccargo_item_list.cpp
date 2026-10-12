#include "old_game/economy/inventory/inventory.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/economy/inventory/integrated_pvp_item_attr.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

// IDA_EA: 0x850BEE2
// IDA_NAME: _ZNK6CCargo12MakeItemListEP11PacketGuard
// DEMANGLED: CCargo::MakeItemList(PacketGuard *)const
int CCargo::MakeItemList(PacketGuard* packet) const {
    packet->put_header(0, 13);
    packet->put_byte(2);
    packet->put_short(GetCapacity());
    packet->put_short(GetItemCount());
    for (int slot = 0; slot <= 151; ++slot) {
        const Inven_Item& item = cargo_[slot];
        if (!item.item_id_) continue;

        packet->put_short(slot);
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
        (G_CDataManager()->get_item_upgrade())->make3rdChroniclePacket(
            owner_, item, packet);
        packet->put_packet(item);
    }
    packet->finalize(true);
    return 1;
}

// IDA_EA: 0x850C160
// IDA_NAME: _ZNK6CCargo14MakeItemPacketEP11PacketGuardi
// DEMANGLED: CCargo::MakeItemPacket(PacketGuard *,int)const
int CCargo::MakeItemPacket(PacketGuard* packet, int slot) const {
    packet->put_short(slot);

    const Inven_Item& item = cargo_[slot];
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
        (G_CDataManager()->get_item_upgrade())->make3rdChroniclePacket(
            owner_, item, packet);
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
