#include "old_game/player/core/cuser.h"

#include "old_game/economy/inventory/inventory.h"

#include <cstdio>
#include <cstring>

// IDA_EA: 0x8686B52
// IDA_NAME: _ZN15cUserHistoryLog16UseSelectBoosterEjiiRSt6vectorI10Inven_ItemSaIS1_EE
// DEMANGLED: cUserHistoryLog::UseSelectBooster(unsigned int,int,int,std::vector<Inven_Item,std::allocator<Inven_Item>> &)
int cUserHistoryLog::UseSelectBooster(
    unsigned int source_item_id,
    int first_category,
    int second_category,
    std::vector<Inven_Item>& selected_items) {
    char selected_item_ids[1024] = {};
    for (unsigned int index = 0;
         index < selected_items.size() &&
         std::strlen(selected_item_ids) + 13u <= 0x3FEu;
         ++index) {
        std::sprintf(selected_item_ids + std::strlen(selected_item_ids),
                     "%d,",
                     selected_items[index].item_id_);
    }
    return owner_->LogHistory("UseSelectBooster,%u,%d,%d,%s",
                              source_item_id,
                              first_category,
                              second_category,
                              selected_item_ids);
}
