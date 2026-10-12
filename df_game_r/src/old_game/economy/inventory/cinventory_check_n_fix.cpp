#include "old_game/economy/inventory/inventory.h"
#include "common/config.h"

#include <stdint.h>


extern int checkStackableLimit(unsigned long item_id, int stack_count)
    __asm__("_Z19checkStackableLimitmi");

// IDA_EA: 0x8509738
// IDA_NAME: _ZN10CInventory9CheckNFixEv
// DEMANGLED: CInventory::CheckNFix(void)
bool CInventory::CheckNFix() {
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuickSlotLast;
         ++slot) {
        Inven_Item& item = stackable_items_[slot];
        if (item.item_id_ != 0 && item.item_category_ != 1u &&
            !checkStackableLimit(
                static_cast<unsigned long>(
                    static_cast<uint32_t>(item.item_id_)),
                item.get_add_info())) {
            item.add_info_ = 1;
        }
    }

    bool result = false;
    for (int slot = oldsrv::config::kMainInventoryConsumableSlotFirst;;
         ++slot) {
        result = slot <= oldsrv::config::kMainInventoryQuestSlotLast;
        if (slot > oldsrv::config::kMainInventoryQuestSlotLast) {
            break;
        }
        Inven_Item& item = stackable_items_[slot];
        if (item.item_id_ != 0 &&
            !checkStackableLimit(
                static_cast<unsigned long>(
                    static_cast<uint32_t>(item.item_id_)),
                item.get_add_info())) {
            item.add_info_ = 1;
        }
    }
    return result;
}
