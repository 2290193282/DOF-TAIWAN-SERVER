#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x830DB84
// IDA_NAME: _ZN13CBattle_Field9drop_itemE8map_item
// DEMANGLED: CBattle_Field::drop_item(map_item)
int CBattle_Field::drop_item(map_item item) {
    MapInfo* const map = GetCurrentMapInfo();
    const int unique_id = map->next_drop_item_unique_id_++;
    item.unique_id_ = static_cast<uint32_t>(unique_id);
    map->Add_Item(item);
    return unique_id;
}
