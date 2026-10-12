#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/dungeon_runtime.h"

namespace {

bool IsPalaceOfLoadIsabellaSceneActor(int dungeon_index,
                                      int map_index,
                                      int monster_index) {
    // OFFICIAL_0828_PVF: quests 4673/4674 select dungeon 15 boss maps
    // 2423/2424.  Monster 56164 is Isabella's persistent dialogue actor in
    // both maps; it has no attack action and is not a room-clear target.
    // Keep this exception data-exact so ordinary non-combat-looking monsters
    // and the same template outside these two official scene maps retain the
    // original 0627 clear semantics.
    return dungeon_index == 15 && monster_index == 56164 &&
           (map_index == 2423 || map_index == 2424);
}

}  // namespace

// IDA_EA: 0x830A0E8
// IDA_NAME: _ZN13CBattle_Field16check_grid_clearEv
// DEMANGLED: CBattle_Field::check_grid_clear(void)
int CBattle_Field::check_grid_clear() {
    MapInfo* const map_info = GetCurrentMapInfo();
    dungeon_runtime::MapMoveMonster cart_projection;
    const bool has_golden_cart =
        dungeon_runtime::IsGoldenCommerceDungeon(dungeon_index_) &&
        dungeon_runtime::GetMapMoveMonsterProjection(
            dungeon_index_, 0, &cart_projection);
    for (std::map<int, map_monster>::const_iterator it =
             map_info->monsters_.begin();
         it != map_info->monsters_.end(); ++it) {
        const map_monster& monster = it->second;

        if (IsPalaceOfLoadIsabellaSceneActor(
                dungeon_index_,
                map_info->map_index_,
                monster.monster_id_)) {
            continue;
        }

        // The cart is a protected map-move controller object, not an enemy
        // clear target.  It remains in MapInfo for UID/HP/SDC authority, but
        // must not keep every escort room (including the boss room) uncleared.
        if (has_golden_cart &&
            monster.monster_id_ == cart_projection.monster_id) {
            continue;
        }

        // Original 0x830A0E8: copied object starts at EBP-0x360; the
        // n100 local at EBP-0x334 is +0x2c (assigned_team_), NOT +0x30.
        // Fixed APC construction zeroes spawn_weight_ but assigns enemy
        // team 100.  Requiring weight 100 incorrectly clears while hostile
        // APC bosses (e.g. Moonlight 5310 / Ghent 10409) are still alive.
        if (monster.assigned_team_ == 100 &&
            monster.packet_value_13_ == 0) {
            return 0;
        }
    }
    return 1;
}
