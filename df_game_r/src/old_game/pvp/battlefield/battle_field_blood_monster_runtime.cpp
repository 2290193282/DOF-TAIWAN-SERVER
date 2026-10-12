#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"

// IDA_EA: 0x830D58C
// IDA_NAME: _ZN13CBattle_Field18kill_blood_monsterEP5CUseriPt
// DEMANGLED: CBattle_Field::kill_blood_monster(CUser *,int,unsigned short *)
int CBattle_Field::kill_blood_monster(CUser* /*user*/,
                                      int count,
                                      unsigned short* monster_ids) {
    CDungeon* const dungeon = GetDungeon();
    if (!dungeon || dungeon->get_blood_dungeon_type() == 0) {
        return 0;
    }

    MapInfo* const map_info = GetCurrentMapInfo();
    bool removed = false;
    for (int index = 0; index < count; ++index) {
        std::map<int, map_monster>::iterator monster =
            map_info->monsters_.find(monster_ids[index]);
        if (monster == map_info->monsters_.end()) {
            continue;
        }

        map_monster copied_monster(monster->second);
        if (copied_monster.blood_runtime_flag_ == 1u) {
            map_info->monsters_.erase(monster);
            removed = true;
        }
    }

    if (removed) {
        checkKilledSpawnBloodMonsters(
            static_cast<int>(map_info->monsters_.size()));
    }
    return 1;
}
