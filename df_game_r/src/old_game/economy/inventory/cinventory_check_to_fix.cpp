#include "old_game/economy/inventory/inventory.h"
#include "common/config.h"

#include <stdint.h>


extern int checkStackableLimit(unsigned long item_id, int stack_count)
    __asm__("_Z19checkStackableLimitmi");

// IDA_EA: 0x8509610
// IDA_NAME: _ZNK10CInventory10checkToFixEv
// DEMANGLED: CInventory::checkToFix(void)const
int CInventory::checkToFix() const {
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuickSlotLast;
         ++slot) {
        const Inven_Item& item = stackable_items_[slot];
        if (item.item_id_ && item.item_category_ != 1 &&
            !checkStackableLimit(
                static_cast<unsigned long>(
                    static_cast<uint32_t>(item.item_id_)),
                item.get_add_info())) {
            return 1;
        }
    }

    for (int slot = oldsrv::config::kMainInventoryConsumableSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuestSlotLast;
         ++slot) {
        const Inven_Item& item = stackable_items_[slot];
        if (item.item_id_ &&
            !checkStackableLimit(
                static_cast<unsigned long>(
                    static_cast<uint32_t>(item.item_id_)),
                item.get_add_info())) {
            return 1;
        }
    }
    return 0;
}
