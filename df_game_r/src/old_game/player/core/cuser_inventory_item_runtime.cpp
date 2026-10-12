#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x865CADC
// IDA_NAME: _ZN5CUser18add_inventory_itemEj
// DEMANGLED: CUser::add_inventory_item(unsigned int)
void CUser::add_inventory_item(unsigned int item_id) {
    CItem* const item_template =
        G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!item_template) {
        return;
    }

    Inven_Item item;
    // The original body assigns the physical item-id member after the slot-2
    // make_item dispatch.
    item_template->vtable_->make_item(item_template, item);
    item.set_add_info(1);
    item.item_id_ = static_cast<int>(item_id);

    const int slot = getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(24), true, true);
    if (slot < 0) {
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 14);
    packet.put_byte(0);
    packet.put_short(1);
    if (getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), slot, packet)) {
        packet.finalize(true);
        Send(packet);
    }
}
