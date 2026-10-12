#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <utility>
#include <vector>

#include "infra/concurrent/thread/mutex.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/pvp/battlefield/zombie_preferred_start_runtime.h"

namespace zombie_preferred_start_runtime {
namespace {

Mutex g_mutex;
std::map<CBattle_Field*, std::pair<int, int> > g_preferred_starts;

bool Consume(CBattle_Field* battle_field,
             const MazeScript& maze,
             std::pair<int, int>& start) {
    std::pair<int, int> requested;
    {
        MutexGuard guard(&g_mutex);
        std::map<CBattle_Field*, std::pair<int, int> >::iterator found =
            g_preferred_starts.find(battle_field);
        if (found == g_preferred_starts.end()) return false;
        requested = found->second;
        g_preferred_starts.erase(found);
    }

    if (std::find(maze.values_14.begin(), maze.values_14.end(), requested) ==
        maze.values_14.end()) {
        return false;
    }
    start = requested;
    return true;
}

}  // namespace

bool Register(CBattle_Field* battle_field, int pos_x, int pos_y) {
    if (!battle_field) return false;
    MutexGuard guard(&g_mutex);
    g_preferred_starts[battle_field] = std::make_pair(pos_x, pos_y);
    return true;
}

void Cancel(CBattle_Field* battle_field) {
    if (!battle_field) return;
    MutexGuard guard(&g_mutex);
    g_preferred_starts.erase(battle_field);
}

}  // namespace zombie_preferred_start_runtime

namespace {

std::vector<std::pair<int, int> > BuildNonEmptyGridPositions(
    const MazeScript& maze) {
    std::vector<std::pair<int, int> > positions;
    for (int pos_y = 0; pos_y < maze.grid_height; ++pos_y) {
        for (int pos_x = 0; pos_x < maze.grid_width; ++pos_x) {
            const GridScript& grid = maze.getGridR(pos_x, pos_y);
            if (grid.grid_type) {
                positions.push_back(std::make_pair(pos_x, pos_y));
            }
        }
    }
    return positions;
}

std::vector<std::pair<int, int> > BuildBossConditionPositions(
    const MazeScript& maze,
    const std::pair<int, int>& start,
    const std::pair<int, int>& boss) {
    std::vector<std::pair<int, int> > positions;
    for (int pos_y = 0; pos_y < maze.grid_height; ++pos_y) {
        for (int pos_x = 0; pos_x < maze.grid_width; ++pos_x) {
            const std::pair<int, int> position =
                std::make_pair(pos_x, pos_y);
            if (position == start || position == boss) {
                continue;
            }

            const GridScript& grid = maze.getGridR(pos_x, pos_y);
            if (grid.values_04.empty()) {
                continue;
            }

            bool eligible = true;
            for (std::vector<int>::const_iterator it =
                     grid.values_04.begin();
                 it != grid.values_04.end();
                 ++it) {
                CMap* const map = G_CDataManager()->find_map(*it);
                if (!map ||
                    map->get_condition_monster_list().empty()) {
                    eligible = false;
                    break;
                }
            }
            if (eligible) {
                positions.push_back(position);
            }
        }
    }
    return positions;
}

void BuildBossRoomConditions(
                             std::vector<CBattle_Field::
                                 stConditionEnterBossRoomStatus_t>& statuses,
                             const MazeScript& maze,
                             int condition_mode,
                             const std::vector<int>& values,
                             const std::pair<int, int>& start,
                             const std::pair<int, int>& boss) {
    if (condition_mode != 1) {
        return;
    }

    const std::vector<std::pair<int, int> > candidates =
        BuildBossConditionPositions(maze, start, boss);
    for (std::vector<int>::const_iterator it = values.begin();
         it != values.end();) {
        const int condition_type = *it++;
        const int condition_count = *it++;
        const int condition_status = *it++;

        CBattle_Field::stConditionEnterBossRoomStatus_t status;
        status.condition_type = condition_type;
        status.boss_room_id = 0;
        status.status = condition_status;
        for (int index = 0; index < condition_count; ++index) {
            const unsigned int candidate_index =
                get_rand_int(static_cast<int>(candidates.size()));
            status.condition_values.push_back(
                candidates.at(candidate_index));
        }
        statuses.push_back(status);
    }
}

void BuildEventMonsters(
                        std::vector<CBattle_Field::stEventMonsterStatus_t>&
                            statuses,
                        const MazeScript& maze,
                        const CDungeon* dungeon) {
    const int try_count = dungeon->getTryGenEventMonsterCnt();
    if (!try_count) {
        return;
    }

    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(36);
    if (!event ||
        !static_cast<unsigned char>(event->IsEventing(0))) {
        return;
    }

    const std::vector<std::pair<int, int> > positions =
        BuildNonEmptyGridPositions(maze);
    std::list<std::pair<int, int> > used_positions;
    const std::vector<stEventMonster_t>* const monsters =
        dungeon->getEventMonsters();
    for (unsigned int attempt = 0;
         attempt < static_cast<unsigned int>(try_count);
         ++attempt) {
        if (static_cast<unsigned int>(
                dungeon->getTryGenEventMonsterProb()) <
            get_rand_int(10000)) {
            continue;
        }

        unsigned int accumulated = 0;
        const unsigned int selected_probability =
            get_rand_int(
                dungeon->getEventMonsterTotalProb());
        for (std::vector<stEventMonster_t>::const_iterator it =
                 monsters->begin();
             it != monsters->end();
             ++it) {
            accumulated += static_cast<unsigned int>(it->probability);
            if (selected_probability >= accumulated) {
                continue;
            }

            CBattle_Field::stEventMonsterStatus_t status;
            status.monster_id = it->monster_id;
            status.status_value = it->status_value;
            const unsigned int position_index =
                get_rand_int(static_cast<int>(positions.size()));
            status.map_position = positions.at(position_index);
            status.status_first = it->status_first;
            status.status_second = it->status_second;

            if (std::find(used_positions.begin(),
                          used_positions.end(),
                          status.map_position) ==
                used_positions.end()) {
                cMyTrace trace(
                    "void CBattle_Field::SetGridPath(MazeScript*, int)",
                    995,
                    0);
                trace("Event monster generated (%d,%d ->%d(%d)",
                      status.map_position.first,
                      status.map_position.second,
                      status.monster_id,
                      status.status_value);
                used_positions.push_back(status.map_position);
                statuses.push_back(status);
            } else {
                cMyTrace trace(
                    "void CBattle_Field::SetGridPath(MazeScript*, int)",
                    1000,
                    0);
                trace("Event monster SameMap (not generated) (%d,%d ->%d(%d)",
                      status.map_position.first,
                      status.map_position.second,
                      status.monster_id,
                      status.status_value);
            }
            break;
        }
    }
    used_positions.clear();
}

void BuildSecondEventMonsters(
                              std::vector<CBattle_Field::
                                  stEventMonsterStatus_t>& statuses,
                              const MazeScript& maze,
                              const CDungeon* dungeon) {
    const int try_count = dungeon->getTryGenSecondEventMonsterCnt();
    if (!try_count) {
        return;
    }

    const std::vector<std::pair<int, int> > positions =
        BuildNonEmptyGridPositions(maze);
    const std::vector<stSecondEventMonster_t>* const monsters =
        dungeon->getSecondEventMonsters();
    for (unsigned int attempt = 0;
         attempt < static_cast<unsigned int>(try_count);
         ++attempt) {
        if (static_cast<unsigned int>(
                dungeon->getTryGenSecondEventMonsterProb()) <
            get_rand_int(10000)) {
            continue;
        }

        unsigned int accumulated = 0;
        const unsigned int selected_probability =
            get_rand_int(
                dungeon->getSecondEventMonsterTotalProb());
        for (std::vector<stSecondEventMonster_t>::const_iterator it =
                 monsters->begin();
             it != monsters->end();
             ++it) {
            accumulated += static_cast<unsigned int>(it->probability);
            if (selected_probability >= accumulated) {
                continue;
            }

            CBattle_Field::stEventMonsterStatus_t status;
            status.monster_id = it->monster_id;
            status.status_value = it->status_value;
            const unsigned int position_index =
                get_rand_int(static_cast<int>(positions.size()));
            status.map_position = positions.at(position_index);
            status.status_first = it->status_first;
            status.status_second = it->status_second;
            statuses.push_back(status);
            break;
        }
    }
}

}  // namespace

// IDA_EA: 0x82FFB30
// IDA_NAME: _ZN13CBattle_Field11SetGridPathEP10MazeScripti
// DEMANGLED: CBattle_Field::SetGridPath(MazeScript *,int)
void CBattle_Field::SetGridPath(MazeScript* maze,
                                int dummy_appear_count) {
    maze_ = maze;
    map_width_ = maze->grid_width;
    map_height_ = maze->grid_height;
    if (map_width_ + map_height_ <= 3) {
        hell_party_enabled_ = 0;
        small_maze_hell_party_suppressed_ = 1;
    }

    std::pair<int, int> start;
    std::pair<int, int> boss;
    std::pair<int, int> boss_neighbor;
    if (maze->values_14.empty()) {
        return;
    }

    if (!zombie_preferred_start_runtime::Consume(this, *maze, start)) {
        start = maze->values_14[
            static_cast<unsigned int>(std::rand()) %
            maze->values_14.size()];
    }
    if (maze->values_20.empty()) {
        return;
    }

    const unsigned int boss_index =
        static_cast<unsigned int>(std::rand()) %
        maze->values_20.size();
    boss = maze->values_20[boss_index];

    if (dummy_appear_count > 0) {
        unsigned int previous_index = boss_index;
        unsigned int selected_index = 0;
        map_path_index_ = 0;
        map_path_.clear();
        for (int index = 0;
             index < dummy_appear_count;
             ++index) {
            if (maze->values_20.size() > 1u) {
                selected_index =
                    static_cast<unsigned int>(std::rand()) %
                    (maze->values_20.size() - 1u);
                if (selected_index >= previous_index) {
                    ++selected_index;
                }
                map_path_.push_back(maze->values_20[selected_index]);
                previous_index = selected_index;
            }
        }
    }

    if (boss_index > 0u) {
        boss_neighbor = maze->values_20[boss_index - 1u];
    }
    if (boss_index + 1u <= maze->values_20.size() - 1u) {
        boss_neighbor = maze->values_20[boss_index + 1u];
    }

    maze_start_position_ = start;
    current_position_ = maze_start_position_;
    previous_position_ = current_position_;

    CDungeon* const dungeon = dungeon_;
    if (dungeon &&
        static_cast<unsigned char>(
            dungeon->IsEnterEachMap())) {
        for (int seat = 0; seat <= 3; ++seat) {
            start_positions_[seat] = start;
            last_positions_[seat] = start;
        }
    }

    maze_boss_position_ = boss;
    maze_boss_neighbor_position_ = boss_neighbor;
    quest_stage_pos_x_ = maze->value_30;
    quest_stage_pos_y_ = maze->value_34;
    quest_stage_map_index_ = maze->value_2c;
    ridable_.SetRidableObject(maze->ridable);

    BuildBossRoomConditions(
        boss_room_conditions_,
        *maze,
        dungeon->enter_boss_room_condition_.enabled,
        dungeon->enter_boss_room_condition_.conditions,
        start,
        boss);
    BuildEventMonsters(event_monsters_, *maze, dungeon);
    BuildSecondEventMonsters(event_monsters_, *maze, dungeon);
}
