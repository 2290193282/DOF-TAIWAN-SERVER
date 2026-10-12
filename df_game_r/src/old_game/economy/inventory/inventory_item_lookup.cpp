#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x84E91A4
// IDA_NAME: _Z18GetItemIdByAddInfoP10Inven_Itemii
// DEMANGLED: GetItemIdByAddInfo(Inven_Item *,int,int)
int GetItemIdByAddInfo(Inven_Item* items, int item_count, int add_info) {
    for (int index = 0; index < item_count; ++index) {
        if (items[index].add_info_ == add_info && items[index].add_info_) {
            return items[index].item_id_;
        }
    }
    return 0;
}
