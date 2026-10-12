#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "common/config.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"

#include "common/manager/os_api.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x84FD7B6
// IDA_NAME: _ZNK10CInventory12MakeItemListE10INVEN_TYPEPv
// DEMANGLED: CInventory::MakeItemList(INVEN_TYPE,void *)const
int CInventory::MakeItemList(INVEN_TYPE inventory_type, void* output) const {
    if (!parent_) {
        return 0;
    }

    PacketGuard& packet = *static_cast<PacketGuard*>(output);
    packet.put_header(0, 13);

    switch (inventory_type) {
    case static_cast<INVEN_TYPE>(2): {
        packet.put_byte(1);
        int count_index = packet.get_index();
        int count = 0;
        packet.put_short(0);
        const int tick = OS_API::GetDateTimeTick();
        const Inven_Item* const table = avatar_items_;
        const WongWork::CAvatarItemMgr* const avatar_manager =
            &avatar_item_manager_;
        for (int slot = 0;
             slot < oldsrv::config::kAvatarInventoryItemCount;
             ++slot) {
            const Inven_Item& item = table[slot];
            if (!item.item_id_) continue;

            packet.put_short(slot);
            packet.put_int(item.item_id_);
            packet.put_int(avatar_manager->GetRemainDate(
                item.add_info_, tick));
            packet.put_byte(item.GetItemAttr());
            packet.put_short(item.avatar_select_);
            packet.put_byte(item.seal_or_state_);
            packet.put_int(item.extend_info_);
            packet.put_byte(0);
            packet.put_short(0);
            packet.put_int(30);
            const stAvatarEmblemInfo_t* const jewel =
                avatar_manager->getJewelSocketData(
                    item.add_info_);
            // NOTI13 avatar records always contain 30 socket bytes.  A
            // missing manager entry is represented by the official empty
            // socket value, never by a null binary pointer.
            stAvatarEmblemInfo_t empty_jewel;
            empty_jewel.init();
            packet.put_binary(reinterpret_cast<const char*>(
                                  jewel ? jewel : &empty_jewel),
                              30);
            packet.put_int(4);
            const stAvatarExpansionInfo_t* const expansion =
                avatar_manager->GetExpansionInfo(
                    item.add_info_);
            if (expansion) {
                packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
            } else {
                stAvatarExpansionInfo_t empty_expansion;
                empty_expansion.init();
                packet.put_binary(reinterpret_cast<const char*>(&empty_expansion),
                                  4);
            }
            // 0828 NOTI13 在 avatar 扩展块后读取第三套 Chronicle 列表。
            // 该列表由 CItemUpgrade 根据物品持久化属性生成，不能用插件式零填充。
            (G_CDataManager()->get_item_upgrade())->make3rdChroniclePacket(
                parent_, item, &packet);
            packet.put_packet(item);
            ++count;
        }
        packet.put_short(count_index, count);
        break;
    }

    case static_cast<INVEN_TYPE>(3): {
        // NOTI13 itemspace 7 owns a u16 record count.  The creature list is
        // sparse: only non-empty creature_items_ records are serialized, and
        // the count is incremented once for each emitted MakeItemPacket row.
        packet.put_byte(7);
        int count_index = packet.get_index();
        int count = 0;
        packet.put_short(0);
        for (int slot = 0;
             slot < oldsrv::config::kCreatureInventoryItemCount;
             ++slot) {
            if (!creature_items_[slot].item_id_) continue;
            MakeItemPacket(inventory_type, slot, packet);
            ++count;
        }
        packet.put_short(count_index, count);
        break;
    }

    case static_cast<INVEN_TYPE>(1): {
        packet.put_byte(0);
        packet.put_short(get_inventory_capacity());
        int count_index = packet.get_index();
        int count = 3;
        packet.put_short(6);
        packet.put_short(0);
        packet.put_int(0);
        packet.put_int(money_);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_byte(0);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_byte(0);
        packet.put_packet(g_emptySlot);
        MakeItemPacket(inventory_type, 1, packet);
        MakeItemPacket(inventory_type, 2, packet);
        for (int slot = 3;
             slot <= oldsrv::config::kMainInventoryLastSlot;
             ++slot) {
            if (!stackable_items_[slot].item_id_) continue;
            MakeItemPacket(inventory_type, slot, packet);
            ++count;
        }
        packet.put_short(count_index, count);
        break;
    }
    }

    packet.finalize(true);
    return 1;
}
