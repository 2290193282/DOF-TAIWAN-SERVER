#include "old_game/world/region/game_world.h"

// IDA_EA: 0x86C4568
// IDA_NAME: _Z42Is_restrictive_commercial_transaction_zoneiiii
// DEMANGLED: Is_restrictive_commercial_transaction_zone(int,int,int,int)
bool Is_restrictive_commercial_transaction_zone(int center_x,
                                                int center_y,
                                                int pos_x,
                                                int pos_y) {
    return center_y - 150 < pos_y && center_y + 150 > pos_y &&
           center_x - 80 < pos_x && center_x + 80 > pos_x;
}

// IDA_EA: 0x86C45AA
// IDA_NAME: _ZN9GameWorld45check_restrictive_commercial_transaction_zoneEiiii
// DEMANGLED: GameWorld::check_restrictive_commercial_transaction_zone(int,int,int,int)
int GameWorld::check_restrictive_commercial_transaction_zone(int village_index,
                                                              int area_index,
                                                              int pos_x,
                                                              int pos_y) {
    if (!check_valid_area(village_index, area_index)) return 0;

    Village* const village = getUserVillage(village_index);
    Area* const area = village ? village->getArea(area_index) : 0;
    if (!area) return 0;
    if (area->private_store_enabled != 1) return 62;

    for (std::vector<std::pair<int, int> >::const_iterator it =
             area->npc_positions.begin();
         it != area->npc_positions.end(); ++it) {
        if (Is_restrictive_commercial_transaction_zone(
                it->first, it->second, pos_x, pos_y)) {
            return 82;
        }
    }
    return 0;
}
