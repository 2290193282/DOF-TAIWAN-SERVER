#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>
#include <cstring>
#include <list>
#include <map>
#include <vector>

#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/item_generator_types.h"

namespace {

// The old routine constructs one map_monster and then clears its complete
// 0x34c-byte storage before every population pass.  Keeping that behavior is
// important: the constructor's normal spawn weight of 100 is intentionally
// cleared for these map-script records.
void ResetConsistMonster(map_monster& monster) {
    std::memset(static_cast<void*>(&monster), 0, sizeof(monster));
}

void DecrementAppearanceCount(std::map<int, int>& counts,
                              int appearance_point) {
    std::map<int, int>::iterator found = counts.find(appearance_point);
    if (found == counts.end()) {
        return;
    }
    found->second = std::max(0, found->second - 1);
}

int CountAvailableAppearances(const std::map<int, int>& counts,
                              int maximum_point) {
    int available = 0;
    const std::map<int, int>::const_iterator finish =
        counts.upper_bound(maximum_point);
    for (std::map<int, int>::const_iterator it = counts.begin();
         it != finish;
         ++it) {
        available += it->second;
    }
    return available;
}

uint8_t ResolveMapMonsterLevel(const _mapMonster& source,
                               const CDungeon* dungeon) {
    if (!source.enabled) {
        return static_cast<uint8_t>(source.raw_08);
    }

    uint8_t level = static_cast<uint8_t>(
        dungeon->get_standard_level() + source.raw_08);
    if (level == 0u) {
        level = 1u;
    } else if (level > 99u) {
        level = 99u;
    }
    return level;
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x814BEF4
// IDA_NAME: _ZN8WongWork10CBossStage10consistMapEPK4CMap
// DEMANGLED: WongWork::CBossStage::consistMap(CMap const*)
int CBossStage::consistMap(const CMap* map) {
    map_info_.map_index_ = map->get_index();

    CDataManager* const data_manager = G_CDataManager();
    CAICharacterList* const ai_characters =
        data_manager->get_ai_character_list();
    std::vector<unsigned int> selected_ai_indices;
    std::map<int, int> available_ap_counts = ai_characters->getAPCount();

    int sequence = 0;
    map_monster monster;

    const std::vector<STAICharacterMapArrangeData>& arrangements =
        map->getAICharacters();
    for (std::vector<STAICharacterMapArrangeData>::const_iterator it =
             arrangements.begin();
         it != arrangements.end();
         ++it) {
        CAICharacter* const ai =
            ai_characters->get(static_cast<uint32_t>(it->raw_00));
        if (!ai) {
            return 0;
        }

        selected_ai_indices.push_back(
            static_cast<unsigned int>(ai->getIdx()));
        const int appearance_point = ai->getAppearancePoint();
        if (appearance_point != 0) {
            DecrementAppearanceCount(available_ap_counts, appearance_point);
        }

        ResetConsistMonster(monster);
        monster.spawn_sequence_ = sequence;
        monster.monster_id_ = it->raw_00;
        monster.map_monster_unique_id_ =
            map_info_.next_monster_unique_id_++;
        monster.monster_kind_ = 5;
        monster.monster_level_ =
            static_cast<uint8_t>(ai->getLevel());
        monster.specific_drop_count_ = 0;
        monster.random_drop_count_ = 0;
        monster.assigned_team_ = it->raw_10;
        map_info_.Add_Mob(monster);
        ++sequence;
    }

    int remaining_ap = map->getAPCRandomPoint();
    if (remaining_ap != 0) {
        sequence = 64;
        for (;;) {
            if (ai_characters->getMinAP() > remaining_ap) {
                break;
            }

            const unsigned int rolled_ap =
                get_rand_int(remaining_ap) + 1u;
            CAICharacter* const ai = ai_characters->getByAP(rolled_ap);
            if (!ai) {
                return 0;
            }

            const unsigned int ai_index =
                static_cast<unsigned int>(ai->getIdx());
            if (std::find(selected_ai_indices.begin(),
                          selected_ai_indices.end(),
                          ai_index) != selected_ai_indices.end()) {
                if (CountAvailableAppearances(available_ap_counts,
                                              remaining_ap) == 0) {
                    break;
                }
                continue;
            }

            selected_ai_indices.push_back(ai_index);
            const int appearance_point = ai->getAppearancePoint();
            DecrementAppearanceCount(available_ap_counts, appearance_point);
            remaining_ap -= appearance_point;

            ResetConsistMonster(monster);
            monster.spawn_sequence_ = sequence;
            monster.monster_id_ = ai->getIdx();
            monster.map_monster_unique_id_ =
                map_info_.next_monster_unique_id_++;
            monster.monster_kind_ = 5;
            monster.monster_level_ =
                static_cast<uint8_t>(ai->getLevel());
            monster.specific_drop_count_ = 0;
            monster.random_drop_count_ = 0;
            monster.assigned_team_ = 100;
            map_info_.Add_Mob(monster);
            ++sequence;
        }
    }

    map_info_.random_map_seed_ = static_cast<int32_t>(get_rand_int(0));
    map_info_.random_map_seed_ &= ~0x40000;

    sequence = 0;
    std::vector<int> unnamed_monster_unique_ids;
    const std::list<_mapMonster> map_monsters = map->get_monster_list();
    for (std::list<_mapMonster>::const_iterator it = map_monsters.begin();
         it != map_monsters.end();
         ++it) {
        ResetConsistMonster(monster);
        monster.spawn_sequence_ = sequence;
        monster.monster_level_ = ResolveMapMonsterLevel(*it, dungeon_);

        if (it->map_index == -1) {
            monster.monster_id_ =
                data_manager->get_boss_consist_monster_manager()
                    ->get_levelrandom_mob(monster.monster_level_);
        } else {
            monster.monster_id_ = it->map_index;
        }

        monster.specific_drop_count_ = static_cast<uint8_t>(it->raw_10);
        monster.random_drop_count_ = static_cast<uint8_t>(it->raw_14);
        monster.monster_kind_ = static_cast<int8_t>(it->raw_18);
        monster.map_monster_unique_id_ =
            map_info_.next_monster_unique_id_++;
        if (monster.monster_kind_ == 0 && !monster.isNamedMonster()) {
            unnamed_monster_unique_ids.push_back(
                monster.map_monster_unique_id_);
        }
        monster.packet_value_13_ = it->raw_1c;
        monster.stage_monster_type_ =
            static_cast<uint8_t>(it->raw_20);
        monster.assigned_team_ = it->raw_28;
        map_info_.Add_Mob(monster);
        ++sequence;
    }

    const std::vector<stDungeonAssignItem_t>& assigned_items =
        dungeon_->getSpecialPassiveObjectItems();
    stGenerateRefData_t object_drop_reference;
    stGenerateRefData_t dungeon_drop_reference;
    dungeon_drop_reference.generator_data.dungeon_specific_items =
        const_cast<std::vector<std::pair<int, int> >*>(
            &dungeon_->getObjectDropItems());

    const std::list<_mapItem> map_items = map->get_item_list();
    std::list<_mapItem>::const_iterator item_it = map_items.begin();
    while (item_it != map_items.end()) {
        const _mapItem source = *item_it;
        const stDungeonAssignItem_t& assigned =
            assigned_items.at(source.raw_00);

        uint8_t standard_level = 0;
        if (assigned.dungeon_index == -1) {
            const int dungeon_standard_level =
                dungeon_->get_standard_level();
            if (dungeon_standard_level <= 0 ||
                dungeon_standard_level > 200) {
                // The old nested loop retries this same map-item entry.
                continue;
            }
            standard_level =
                static_cast<uint8_t>(dungeon_standard_level);
        } else {
            standard_level =
                static_cast<uint8_t>(assigned.dungeon_index);
        }

        stGenerateResult_t result;
        object_drop_reference.generator_data.object_drop.items =
            const_cast<std::list<std::pair<int, int> >*>(&assigned.items);
        object_drop_reference.generator_data.object_drop.standard_level =
            standard_level;
        object_drop_reference.generator_data.object_drop.item_count =
            static_cast<uint8_t>(source.item_id);

        data_manager->getItemGenerator()
            ->getGenerator(CItemGeneratorMgr::kObjectDrop)
            ->generateSpecificItem(object_drop_reference, result);
        data_manager->getItemGenerator()
            ->getGenerator(CItemGeneratorMgr::kDungeonDrop)
            ->generateSpecificItem(dungeon_drop_reference, result);

        for (std::vector<Inven_Item>::iterator generated =
                 result.items.begin();
             generated != result.items.end();
             ++generated) {
            map_item item;
            item.routing_eligible_ = 1;
            item.unique_id_ =
                static_cast<uint32_t>(map_info_.next_drop_item_unique_id_++);
            item.packet_value_01_ =
                static_cast<int8_t>(source.drop_id);
            item.item.setCopy(*generated);
            map_info_.Add_Item(item);
        }

        ++item_it;
    }

    return 1;
}

}  // namespace WongWork
