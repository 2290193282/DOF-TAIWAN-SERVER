#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x830DAEE
// IDA_NAME: _ZN13CBattle_Field11pickup_itemEi
// DEMANGLED: CBattle_Field::pickup_item(int)
int CBattle_Field::pickup_item(int unique_id) {
    MapInfo* const map_info = GetCurrentMapInfo();
    std::map<int, map_item>::iterator item =
        map_info->items_.find(unique_id);
    if (item == map_info->items_.end()) {
        return 21;
    }
    map_info->items_.erase(item);
    return 0;
}
