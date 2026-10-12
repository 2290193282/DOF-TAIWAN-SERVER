#include "old_game/pvp/battlefield/war_field_runtime.h"

#include <algorithm>
#include <vector>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "infra/network/packet/interface_packet_buf.h"

namespace {

const char kConsistMapFunction[] =
    "void WarField::ConsistMap(void*, const CDungeon*, const CMap*, byte, "
    "int)";
const char kWarFieldFile[] = "WarField.cpp";

}  // namespace

// IDA_EA: 0x86B8A38
// IDA_NAME: _ZN8WarField10ConsistMapEPvPK8CDungeonPK4CMaphi
// DEMANGLED: WarField::ConsistMap(void *,CDungeon const*,CMap const*,
//                                 unsigned char,int)
void WarField::ConsistMap(void* packet_buffer,
                          const CDungeon* dungeon,
                          const CMap* map,
                          unsigned char monster_level,
                          int player_count) {
    if (!player_count) {
        cMyTrace trace(kConsistMapFunction, 59, 5);
        trace("WarField::ConsistMap playerCount is 0");
        return;
    }

    InterfacePacketBuf* const packet =
        static_cast<InterfacePacketBuf*>(packet_buffer);
    packet->put_header(0, 91);
    packet->put_byte(dungeon->war_room_data_.condition_value_);
    int count_packet_index = packet->get_index();
    packet->put_short(0);

    // The old body snapshots this scalar before it potentially clears the
    // map, then restores the evolved value at function exit.
    int next_monster_unique_id = map_info_.next_monster_unique_id_;
    int remaining_spawn_budget = std::max(
        player_count * dungeon->war_room_data_
                           .monster_group_order_[current_spawn_group_index_] /
            6,
        10);

    if (!map_info_.monsters_.empty()) {
        cMyTrace trace(kConsistMapFunction, 83, 5);
        trace("WarField::ConsistMap, ");
        map_info_.Clear();
    }

    while (remaining_spawn_budget > 9) {
        const STMonsterTypeSpawnProb& spawn_prob =
            dungeon->war_room_data_
                .spawn_probs_[current_spawn_group_index_];

        int group_weight_total = 0;
        for (int group = 0; group <= 3; ++group) {
            group_weight_total += spawn_prob.group_weights_[group];
        }
        if (!group_weight_total) {
            LogManager::logFormat(1,
                                  kWarFieldFile,
                                  kConsistMapFunction,
                                  0x65,
                                  "WARROOM::SpawnMonster : ");
            return;
        }

        const int group_random = static_cast<int>(get_rand_int(group_weight_total));
        int group_weight_sum = 0;
        int monster_group = 0;
        for (; monster_group <= 3; ++monster_group) {
            group_weight_sum += spawn_prob.group_weights_[monster_group];
            if (group_random < group_weight_sum) {
                break;
            }
        }

        const std::vector<int>& monster_ids =
            dungeon->war_room_data_.monster_groups_[monster_group];
        const int monster_count = static_cast<int>(monster_ids.size());
        int monster_weights[256];
        int monster_weight_total = 0;
        for (int monster_index = 0; monster_index < monster_count;
             ++monster_index) {
            const int monster_id = monster_ids.at(monster_index);
            CMonster* const monster =
                G_CDataManager()->find_monster(monster_id);
            if (!monster) {
                LogManager::logFormat(1,
                                      kWarFieldFile,
                                      kConsistMapFunction,
                                      0x96,
                                      "WARROOM Find Monster[%d] Error",
                                      monster_ids.at(monster_index));
                return;
            }
            monster_weights[monster_index] = monster->war_room_spawn_weight_;
            monster_weight_total += monster_weights[monster_index];
        }
        if (!monster_weight_total) {
            LogManager::logFormat(1,
                                  kWarFieldFile,
                                  kConsistMapFunction,
                                  0x9f,
                                  "WARROOM::SpawnMonster : ");
            return;
        }

        const int monster_random =
            static_cast<int>(get_rand_int(monster_weight_total));
        int monster_weight_sum = 0;
        int monster_index = 0;
        for (; monster_index < monster_count; ++monster_index) {
            monster_weight_sum += monster_weights[monster_index];
            if (monster_random < monster_weight_sum) {
                break;
            }
        }

        CMonster* const selected_monster = G_CDataManager()->find_monster(
            monster_ids.at(monster_index));
        if (!selected_monster) {
            LogManager::logFormat(1,
                                  kWarFieldFile,
                                  kConsistMapFunction,
                                  0xbc,
                                  "WARROOM::ConsistMap pMob is Null");
            return;
        }

        int position_weight_total = 0;
        const int position_count =
            static_cast<int>(map->rival_positions_.size());
        for (int index = 0; index < position_count; ++index) {
            position_weight_total += map->rival_positions_[index].spawn_weight;
        }
        if (!position_weight_total) {
            LogManager::logFormat(1,
                                  kWarFieldFile,
                                  kConsistMapFunction,
                                  0xd0,
                                  "WARROOM::SpawnMonster : ");
            return;
        }

        const int position_random =
            static_cast<int>(get_rand_int(position_weight_total));
        int position_weight_sum = 0;
        int position_index = 0;
        for (; position_index < position_count; ++position_index) {
            position_weight_sum +=
                map->rival_positions_[position_index].spawn_weight;
            if (position_random < position_weight_sum) {
                break;
            }
        }

        map_monster monster;
        monster.map_monster_unique_id_ = next_monster_unique_id++;
        monster.monster_kind_ = static_cast<int8_t>(monster_group);
        monster.monster_id_ = selected_monster->get_index();
        monster.monster_level_ = monster_level;
        monster.specific_drop_count_ = 1;
        monster.random_drop_count_ = 1;
        monster.war_room_scale_a_ =
            dungeon->war_room_data_.post_spawn_values_[0] *
                static_cast<long double>(6 - player_count) +
            1.0;
        monster.war_room_scale_b_ =
            dungeon->war_room_data_.post_spawn_values_[1] *
                static_cast<long double>(6 - player_count) +
            1.0;
        map_info_.Add_Mob(monster);

        remaining_spawn_budget -=
            dungeon->war_room_data_.condition_values_[monster_group];
        packet->put_short(monster.map_monster_unique_id_);
        packet->put_byte(monster_group);
        packet->put_byte(monster_level);
        packet->put_byte(monster_index);
        packet->put_byte(position_index);

        const int random_base = dungeon->war_room_data_.tail_values_[0] +
                                static_cast<int>(get_rand_int(
                                    dungeon->war_room_data_.tail_values_[1]));
        const int scaled_base = static_cast<int>(
            static_cast<long double>(random_base) *
            dungeon->war_room_data_.event_values_[monster_group]);
        const int normalized_value = static_cast<int>(
            static_cast<long double>(scaled_base) /
            (static_cast<long double>(player_count - 1) * 0.3 + 1.0));
        packet->put_short(normalized_value);
    }

    map_info_.next_monster_unique_id_ = next_monster_unique_id;
    ++current_spawn_group_index_;
    packet->put_short(count_packet_index,
                      static_cast<int>(map_info_.monsters_.size()));
    packet->finalize(true);
}
