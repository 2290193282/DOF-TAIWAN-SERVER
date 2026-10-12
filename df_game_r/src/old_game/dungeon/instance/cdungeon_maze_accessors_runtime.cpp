#include "old_data/PVF/map/datamanager/map_runtime.h"

#include <algorithm>
#include <cstdlib>

#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

// IDA unit: 09_dungeon_pvp_party/CDungeon.
//
// These are the original, typed script accessors.  They intentionally keep
// the legacy selection and failure semantics: GetRandMaze has the same
// non-empty-vector precondition as the old server, and get_clear_item() uses
// the legacy global random helper before performing the cumulative-map lookup.

// IDA_EA: 0x81455CA
// IDA_NAME: _ZNK8CDungeon29getTowerMaxClearRewardItemNumEv
// DEMANGLED: CDungeon::getTowerMaxClearRewardItemNum(void)const
int CDungeon::getTowerMaxClearRewardItemNum() const {
    return runtime_value_6b8_;
}

// IDA_EA: 0x822B4EE
// IDA_NAME: _ZNK8CDungeon26GetType_DungeonInHeritanceEv
// DEMANGLED: CDungeon::GetType_DungeonInHeritance(void)const
int CDungeon::GetType_DungeonInHeritance() const {
    return dungeon_inheritance_type_;
}

// IDA_EA: 0x834BEB8
// IDA_NAME: _ZNK8CDungeon11GetRandMazeERi
// DEMANGLED: CDungeon::GetRandMaze(int &)const
MazeScript* CDungeon::GetRandMaze(int& index) const {
    // Do not add an empty check: the original uses rand() % size() directly.
    index = static_cast<int>(static_cast<unsigned int>(std::rand()) %
                             maze_scripts_.size());
    return const_cast<MazeScript*>(&maze_scripts_[index]);
}

// IDA_EA: 0x834BF0E
// IDA_NAME: _ZNK8CDungeon7GetMazeEi
// DEMANGLED: CDungeon::GetMaze(int)const
MazeScript* CDungeon::GetMaze(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= maze_scripts_.size()) {
        return 0;
    }
    return const_cast<MazeScript*>(&maze_scripts_[index]);
}

// IDA_EA: 0x834C8C0
// IDA_NAME: _ZNK8CDungeon14get_clear_itemEv
// DEMANGLED: CDungeon::get_clear_item(void)const
int CDungeon::get_clear_item() const {
    const std::map<int, int>::const_iterator it =
        clear_item_rates_.lower_bound(static_cast<int>(get_rand_int(100)) + 1);
    return it == clear_item_rates_.end() ? -2 : it->second;
}

// IDA_EA: 0x8365288
// IDA_NAME: _ZNK8CDungeon15IsOpenEvenEnemyEv
// DEMANGLED: CDungeon::IsOpenEvenEnemy(void)const
int CDungeon::IsOpenEvenEnemy() const {
    return open_even_enemy_;
}

// IDA_EA: 0x834C9F0
// IDA_NAME: _ZNK8CDungeon10VerifyMazeEv
// DEMANGLED: CDungeon::VerifyMaze(void)const
int CDungeon::VerifyMaze() const {
    // The old validator deliberately skips the normal/quest maze check for
    // later dungeon IDs.  This is a data-version boundary, not an error path.
    if (dungeon_index_ > 1999) {
        return 1;
    }

    for (std::vector<MazeScript>::const_iterator it = maze_scripts_.begin();
         it != maze_scripts_.end(); ++it) {
        if (VerifyMaze(*it) != 1) {
            return 0;
        }
    }

    // The original constructs a local MazeScript copy for each quest entry
    // before validation.  Retain that lifecycle rather than turning this into
    // an unproved direct-reference shortcut.
    for (int quest_type = 0; quest_type <= 1; ++quest_type) {
        const std::vector<MazeScript>& quest_mazes =
            quest_maze_scripts_[quest_type];
        for (std::vector<MazeScript>::const_iterator it = quest_mazes.begin();
             it != quest_mazes.end(); ++it) {
            MazeScript maze_copy(*it);
            if (VerifyMaze(maze_copy) != 1) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x834CB7C
// IDA_NAME: _ZNK8CDungeon10VerifyMazeERK10MazeScript
// DEMANGLED: CDungeon::VerifyMaze(MazeScript const&)const
int CDungeon::VerifyMaze(const MazeScript& maze) const {
    static const char kFile[] = "data_manager.cpp";
    static const char kFunction[] =
        "bool CDungeon::VerifyMaze(const MazeScript&) const";

    // The 1x1 default maze is accepted before start/end declarations are
    // checked, exactly as in the old binary.
    if (maze.grid_width == 1 && maze.grid_height == 1) {
        return 1;
    }

    if (maze.values_14.empty()) {
        LogManager::logFormat(
            1, kFile, kFunction, 0x83e,
            "dungeon index(%d) maze(%d) has error. (Start positions were not declared!)",
            dungeon_index_, maze.maze_index);
        return 0;
    }

    for (std::vector<std::pair<int, int> >::const_iterator pos =
             maze.values_14.begin();
         pos != maze.values_14.end(); ++pos) {
        const GridScript& grid = maze.getGridR(pos->first, pos->second);
        if (start_maps_[grid.grid_type].empty()) {
            LogManager::logFormat(
                1, kFile, kFunction, 0x847,
                "dungeon index(%d) maze(%d) has error. (Can't found start map(grid: %d,%d)!)",
                dungeon_index_, maze.maze_index, pos->first, pos->second);
        }
    }

    if (maze.values_20.empty()) {
        LogManager::logFormat(
            1, kFile, kFunction, 0x84e,
            "dungeon index(%d) maze(%d) has error. (End positions were not declared!)",
            dungeon_index_, maze.maze_index);
        return 0;
    }

    for (std::vector<std::pair<int, int> >::const_iterator pos =
             maze.values_20.begin();
         pos != maze.values_20.end(); ++pos) {
        const GridScript& grid = maze.getGridR(pos->first, pos->second);
        if (boss_maps_[grid.grid_type].empty()) {
            LogManager::logFormat(
                1, kFile, kFunction, 0x857,
                "dungeon index(%d) maze(%d) has error. (Can't found boss map(grid: %d,%d)!",
                dungeon_index_, maze.maze_index, pos->first, pos->second);
        }
    }

    for (int pos_x = 0; pos_x < maze.grid_width; ++pos_x) {
        for (int pos_y = 0; pos_y < maze.grid_height; ++pos_y) {
            const GridScript& grid = maze.getGridR(pos_x, pos_y);
            const std::pair<int, int> pos(pos_x, pos_y);
            if (grid.grid_type != 0 &&
                std::find(maze.values_14.begin(), maze.values_14.end(), pos) ==
                    maze.values_14.end() &&
                std::find(maze.values_20.begin(), maze.values_20.end(), pos) ==
                    maze.values_20.end() &&
                normal_maps_[grid.grid_type].empty()) {
                LogManager::logFormat(
                    1, kFile, kFunction, 0x86b,
                    "dungeon index(%d) maze(%d) has error. (Can't found grid map(grid: %d,%d)!)",
                    dungeon_index_, maze.maze_index, pos_x, pos_y);
            }
        }
    }
    return 1;
}
