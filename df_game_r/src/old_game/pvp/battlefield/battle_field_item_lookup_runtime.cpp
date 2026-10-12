#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x830D9C0
// IDA_NAME: _ZN13CBattle_Field14get_field_itemEiR8map_item
// DEMANGLED: CBattle_Field::get_field_item(int,map_item &)
int CBattle_Field::get_field_item(int unique_id, map_item& item) {
    MapInfo* const current_map = GetCurrentMapInfo();
    if (!current_map) return 0;

    std::map<int, map_item>::const_iterator found =
        current_map->items_.find(unique_id);
    if (found == current_map->items_.end()) return 0;
    item = found->second;
    return 1;
}
