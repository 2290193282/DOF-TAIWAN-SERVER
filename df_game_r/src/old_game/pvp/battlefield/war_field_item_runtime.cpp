#include "old_game/pvp/battlefield/war_field_runtime.h"

// IDA_EA: 0x86BA190
// IDA_NAME: _ZN8WarField8DropItemE8map_item
// DEMANGLED: WarField::DropItem(map_item)
int WarField::DropItem(map_item item) {
    const int unique_id = map_info_.next_drop_item_unique_id_++;
    item.unique_id_ = static_cast<uint32_t>(unique_id);
    map_info_.Add_Item(item);
    return unique_id;
}
