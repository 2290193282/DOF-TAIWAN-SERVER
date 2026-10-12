#include "old_game/economy/inventory/inventory.h"

#include "common/config.h"

// IDA_EA: 0x84FB824
// IDA_NAME: _ZNK10CInventory14get_empty_slotE10INVEN_TYPEN10Inven_Item9ITEM_TYPEE
// DEMANGLED: CInventory::get_empty_slot(INVEN_TYPE,Inven_Item::ITEM_TYPE)const
int CInventory::get_empty_slot(INVEN_TYPE inventory_type,
                               Inven_Item::ITEM_TYPE item_type) const {
    switch (inventory_type) {
    case static_cast<INVEN_TYPE>(2):
        for (int slot = 0; slot <= 104; ++slot) {
            const Inven_Item* const item = GetInvenRef(2, slot);
            if (item && item->item_id_ == 0) return slot;
        }
        break;

    case static_cast<INVEN_TYPE>(3):
        for (int slot = 0;
             slot <= oldsrv::config::kCreatureInventoryLastSlot;
             ++slot) {
            const Inven_Item* const item = GetInvenRef(3, slot);
            if (item && item->item_id_ == 0) return slot;
        }
        break;

    case static_cast<INVEN_TYPE>(1): {
        int begin_slot = 0;
        int end_slot = 0;
        StartEndPos(item_type, begin_slot, end_slot);
        for (int slot = begin_slot; slot <= end_slot; ++slot) {
            const Inven_Item* const item = GetInvenRef(1, slot);
            if (item && item->item_id_ == 0) return slot;
        }
        break;
    }
    }
    return -1;
}
