#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

namespace {

bool HasItemId(const Inven_Item& item, int item_id) {
    return item.item_id_ == item_id;
}

}  // namespace

// IDA_EA: 0x850552A
// IDA_NAME: _ZNK10CInventory32check_vilmark_dungeon_item_existEi
// DEMANGLED: CInventory::check_vilmark_dungeon_item_exist(int)const
int CInventory::check_vilmark_dungeon_item_exist(int item_id) const {
    const Inven_Item* const items = stackable_items_;
    for (int slot = 3; slot <= 8; ++slot) {
        if (HasItemId(items[slot], item_id)) {
            return slot;
        }
    }

    int start = 0;
    int end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(3), start, end);
    for (int slot = start; slot <= end; ++slot) {
        if (HasItemId(items[slot], item_id)) {
            return slot;
        }
    }

    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(4), start, end);
    for (int slot = start; slot <= end; ++slot) {
        if (HasItemId(items[slot], item_id)) {
            return slot;
        }
    }
    return -1;
}
