#include <stdint.h>
#include <string.h>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x85AC59C
// IDA_NAME: _ZN6CParty26checkInoutConditionDungeonEv
// DEMANGLED: CParty::checkInoutConditionDungeon(void)
int CParty::checkInoutConditionDungeon() {
    CDungeon* const dungeon = battle_field_.dungeon_;
    if (!dungeon) {
        return 10;
    }
    if (static_cast<signed char>(GetEPLPState()) <= 1) {
        return 10;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* user = get_user(seat);
        if (user && static_cast<signed char>(
                        dungeon->get_blood_dungeon_type()) <= 0) {
            const unsigned short fatigue =
                static_cast<unsigned short>(user->getCurCharacTotalFatigue());
            const unsigned short max_fatigue = static_cast<unsigned short>(
                user->getCurCharacTotalMaxFatigue());
            if (fatigue == max_fatigue) {
                return 1;
            }
        }
    }

    unsigned char ancient_error = 0;
    Inven_Item inventory_snapshots[4];
    int ancient_counts[4];
    memset(inventory_snapshots, 0, sizeof(inventory_snapshots));
    memset(ancient_counts, 0, sizeof(ancient_counts));
    if (!CheckEnterAncientDungeon(
            dungeon,
            ancient_error,
            inventory_snapshots,
            ancient_counts)) {
        return 2;
    }

    const int dimension_result =
        CheckEnterDimensionDungeon(dungeon);
    if (dimension_result == 171 || dimension_result == 173) {
        return 4;
    }

    const int start_result = check_dungeon_start(dungeon);
    if (start_result == 10 ||
        (start_result >= 10 &&
         static_cast<unsigned int>(start_result - 243) <= 4u)) {
        return 7;
    }
    if (CheckEnterVillageAttackRevenge(dungeon)) {
        return 7;
    }

    CWorldMap* world_map = 0;
    const int world_map_index =
        G_GameWorld()->GetWorldMapIndex(manager_);
    if (world_map_index > 0) {
        CWorldMapList* const world_map_list =
            CDataManagerWorldMapList(G_CDataManager());
        world_map = world_map_list
                        ? world_map_list->find_world_map(world_map_index)
                        : 0;
    }

    const bool hell_dungeon_gate =
        world_map &&
        static_cast<unsigned char>(world_map->IsInHellDungeon()) &&
        static_cast<unsigned char>(CBattle_Field::IsEnableHellDungeon()) &&
        battle_field_.dungeon_type_value_ == 1;

    if (hell_dungeon_gate) {
        std::vector<int> clear_quests;
        std::vector<int> seals;
        if (static_cast<unsigned char>(CheckDestroyConditionSealDoor(
                dungeon,
                clear_quests,
                seals)) == 1u) {
            hell_quest_notice_ = 1;
        } else {
            hell_quest_notice_ = 0;
            return 8;
        }
    }

    const bool fatigue_gate =
        battle_field_.instance_dungeon_mode_ != 1 &&
        (!world_map ||
         static_cast<unsigned char>(
             world_map->IsInHellDungeon()) != 1u) &&
        static_cast<unsigned char>(dungeon->isNoFatigueDungeon()) != 1u &&
        dungeon->defense_dungeon_ != 1u;

    if (fatigue_gate && CheckMemberFatigue() > 0) {
        return 1;
    }
    return 9;
}
