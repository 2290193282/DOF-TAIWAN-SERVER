#include "old_game/economy/inventory/item_list.h"

#include <stdio.h>

// IDA_EA: 0x8511E8A
// IDA_NAME: _ZN9CItemList9find_itemEi
// DEMANGLED: CItemList::find_item(int)
CItem* CItemList::find_item(int item_index) {
    CItemIndexList::iterator it = items_by_index_.find(item_index);
    if (it != items_by_index_.end()) {
        return it->second;
    }

    ::printf("[CItemList::find_item] Can't find item : index(%d)\n",
             item_index);
    return 0;
}
