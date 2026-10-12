#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>


namespace {

bool IsFinishPointItem(const Inven_Item& item) {
    const uint32_t item_id = static_cast<uint32_t>(item.item_id_);
    // 0x850A2D4 first rejects zero, then compares the physical +0x02 dword
    // as unsigned: >0x1963 and <=0x1b57.
    return item_id != 0u && item_id > 0x1963u && item_id <= 0x1B57u;
}

}  // namespace

// IDA_EA: 0x850A2BC
// IDA_NAME: _ZN10CInventory12RemoveFPItemEP10Inven_ItemS1_
// DEMANGLED: CInventory::RemoveFPItem(Inven_Item *,Inven_Item *)
bool CInventory::RemoveFPItem(Inven_Item* equipment_items,
                              Inven_Item* stackable_items) {
    for (int index = 0;
         index < oldsrv::config::kEquipmentTypeCount;
         ++index) {
        Inven_Item& item = equipment_items[index];
        if (IsFinishPointItem(item)) {
            item.reset();
        }
    }

    bool result = false;
    for (int index = 3;; ++index) {
        result = index <= oldsrv::config::kMainInventoryLastSlot;
        if (index > oldsrv::config::kMainInventoryLastSlot) {
            break;
        }
        Inven_Item& item = stackable_items[index];
        if (IsFinishPointItem(item)) {
            item.reset();
        }
    }
    return result;
}

// IDA_EA: 0x850A392
// IDA_NAME: _ZN10CInventory12RemoveFPItemEv
// DEMANGLED: CInventory::RemoveFPItem(void)
bool CInventory::RemoveFPItem() {
    return RemoveFPItem(equipment_items_, stackable_items_);
}
