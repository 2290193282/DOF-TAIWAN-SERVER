#include "old_game/player/core/cuser.h"

// IDA_EA: 0x868B106
// IDA_NAME: _ZN5CUser25getInDungeonItemSellCountEii
// DEMANGLED: CUser::getInDungeonItemSellCount(int,int)
int CUser::getInDungeonItemSellCount(int item_space, int slot) {
    if (in_dungeon_item_sell_count_ > 6u || !getCurCharacR()) {
        return 0;
    }

    const Inven_Item* const item = GetCurCharacInventoryRef(
        static_cast<unsigned short>(item_space),
        static_cast<unsigned short>(slot));
    if (!item) {
        return 0;
    }

    bool known_item = false;
    for (unsigned int index = 0;
         !known_item && index <= 6u;
         ++index) {
        if (in_dungeon_item_sell_ids_[index] ==
            static_cast<unsigned int>(item->item_id_)) {
            known_item = true;
        }
    }
    if (!known_item) {
        in_dungeon_item_sell_ids_[in_dungeon_item_sell_count_] =
            static_cast<unsigned int>(item->item_id_);
        ++in_dungeon_item_sell_count_;
    }
    return static_cast<int>(in_dungeon_item_sell_count_);
}
