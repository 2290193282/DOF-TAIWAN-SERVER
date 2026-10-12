#include "old_data/PVF/map/datamanager/map_runtime.h"

#include <cstdlib>
#include <utility>
#include <vector>

#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

// B-owned providers required by the C MoveMap/SelectDungeon/SetGridPath
// gameplay chain.  Every body below follows the original i386 implementation;
// the declarations live on the canonical typed CMap/CDungeon layouts.

// IDA_EA: 0x834ECAA
// IDA_NAME: _ZNK4CMap13CheckEntranceEi
// DEMANGLED: CMap::CheckEntrance(int)const
int CMap::CheckEntrance(int entrance) const {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             greed_cells_.begin();
         it != greed_cells_.end();
         ++it) {
        if (entrance & it->second) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x830E610
// IDA_NAME: _ZNK8CDungeon14IsHaveDummyMapEv
// DEMANGLED: CDungeon::IsHaveDummyMap(void)const
int CDungeon::IsHaveDummyMap() const {
    return has_dummy_maps_;
}

// IDA_EA: 0x830E620
// IDA_NAME: _ZNK8CDungeon19GetDummyAppearCountEv
// DEMANGLED: CDungeon::GetDummyAppearCount(void)const
int CDungeon::GetDummyAppearCount() const {
    return dummy_appear_count_;
}

// IDA_EA: 0x830E62E
// IDA_NAME: _ZNK8CDungeon24getTryGenEventMonsterCntEv
// DEMANGLED: CDungeon::getTryGenEventMonsterCnt(void)const
int CDungeon::getTryGenEventMonsterCnt() const {
    return try_gen_event_monster_count_;
}

// IDA_EA: 0x830E63C
// IDA_NAME: _ZNK8CDungeon25getTryGenEventMonsterProbEv
// DEMANGLED: CDungeon::getTryGenEventMonsterProb(void)const
int CDungeon::getTryGenEventMonsterProb() const {
    return try_gen_event_monster_probability_;
}

// IDA_EA: 0x830E64A
// IDA_NAME: _ZNK8CDungeon24getEventMonsterTotalProbEv
// DEMANGLED: CDungeon::getEventMonsterTotalProb(void)const
int CDungeon::getEventMonsterTotalProb() const {
    return event_monster_total_probability_;
}

// IDA_EA: 0x830E658
// IDA_NAME: _ZNK8CDungeon16getEventMonstersEv
// DEMANGLED: CDungeon::getEventMonsters(void)const
const std::vector<stEventMonster_t>* CDungeon::getEventMonsters() const {
    return &event_monsters_;
}

// IDA_EA: 0x830E666
// IDA_NAME: _ZNK8CDungeon30getTryGenSecondEventMonsterCntEv
// DEMANGLED: CDungeon::getTryGenSecondEventMonsterCnt(void)const
int CDungeon::getTryGenSecondEventMonsterCnt() const {
    return try_gen_second_event_monster_count_;
}

// IDA_EA: 0x830E674
// IDA_NAME: _ZNK8CDungeon31getTryGenSecondEventMonsterProbEv
// DEMANGLED: CDungeon::getTryGenSecondEventMonsterProb(void)const
int CDungeon::getTryGenSecondEventMonsterProb() const {
    return try_gen_second_event_monster_probability_;
}

// IDA_EA: 0x830E682
// IDA_NAME: _ZNK8CDungeon30getSecondEventMonsterTotalProbEv
// DEMANGLED: CDungeon::getSecondEventMonsterTotalProb(void)const
int CDungeon::getSecondEventMonsterTotalProb() const {
    return second_event_monster_total_probability_;
}

// IDA_EA: 0x830E690
// IDA_NAME: _ZNK8CDungeon22getSecondEventMonstersEv
// DEMANGLED: CDungeon::getSecondEventMonsters(void)const
const std::vector<stSecondEventMonster_t>*
CDungeon::getSecondEventMonsters() const {
    return &second_event_monsters_;
}

// IDA_EA: 0x830E69E
// IDA_NAME: _ZNK8CDungeon24GetQuestMazeScriptVectorE20ENUM_MAZE_QUEST_TYPE
// DEMANGLED:
// CDungeon::GetQuestMazeScriptVector(ENUM_MAZE_QUEST_TYPE)const
const std::vector<MazeScript>* CDungeon::GetQuestMazeScriptVector(
    ENUM_MAZE_QUEST_TYPE quest_type) const {
    return &quest_maze_scripts_[static_cast<int>(quest_type)];
}

// IDA_EA: 0x834BF56
// IDA_NAME: _ZNK8CDungeon15ChooseRandomMapEiii
// DEMANGLED: CDungeon::ChooseRandomMap(int,int,int)const
int CDungeon::ChooseRandomMap(int map_type,
                              int direction,
                              int entrance) const {
    if (map_type < 0 || direction < 0 || entrance < 0) {
        LogManager::logFormat(
            1,
            "data_manager.cpp",
            "int CDungeon::ChooseRandomMap(int, int, int) const",
            0x71b,
            "CDungeon::ChooseRandomMap [1](Dungeon Index(%d))(map_type(%d) < 0 || direction(%d) < 0 || entrance(%d) < 0)==false",
            dungeon_index_,
            map_type,
            direction,
            entrance);
        return -1;
    }

    const DirectionMapList* map_list = 0;
    int empty_log_line = 0;
    const char* empty_log_text = 0;
    const char* direct_failure_text = 0;
    int direct_failure_line = 0;
    if (map_type == 1) {
        map_list = &boss_maps_[direction];
        empty_log_line = 0x72c;
        empty_log_text =
            "CDungeon::ChooseRandomMap (boss_maplist_[direction].size()==0)(Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)";
        direct_failure_line = 0x736;
        direct_failure_text =
            "CDungeon::ChooseRandomMap (p_map->get_index()==-1)[1](Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)";
    } else if (map_type == 2) {
        map_list = &dummy_maps_[direction];
        empty_log_line = 0x745;
        empty_log_text =
            "CDungeon::ChooseRandomMap (dummy_maplist_[direction].size()==0)(Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)";
        direct_failure_line = 0x74f;
        direct_failure_text =
            "CDungeon::ChooseRandomMap (p_map->get_index()==-1)[1](Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)";
    } else {
        map_list = &normal_maps_[direction];
        empty_log_line = 0x75e;
        empty_log_text =
            "CDungeon::ChooseRandomMap (maplist_[direction].size()==0)(Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)";
        direct_failure_line = 0x768;
        direct_failure_text =
            "CDungeon::ChooseRandomMap (p_map->get_index()==-1)[2](Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)";
    }

    if (entrance == 0) {
        if (map_list->empty()) {
            LogManager::logFormat(
                1,
                "data_manager.cpp",
                "int CDungeon::ChooseRandomMap(int, int, int) const",
                empty_log_line,
                empty_log_text,
                dungeon_index_,
                map_type,
                direction,
                entrance);
            return -1;
        }

        DirectionMapList::const_iterator selected = map_list->begin();
        for (unsigned int index = 0;
             static_cast<unsigned int>(std::rand()) %
                     map_list->size() >
                 index;
             ++index) {
            ++selected;
        }
        CMap* const map = selected->second;
        if (map->get_index() == -1) {
            LogManager::logFormat(
                1,
                "data_manager.cpp",
                "int CDungeon::ChooseRandomMap(int, int, int) const",
                direct_failure_line,
                direct_failure_text,
                dungeon_index_,
                map_type,
                direction,
                entrance);
        }
        return map->get_index();
    }

    const std::pair<DirectionMapList::const_iterator,
                    DirectionMapList::const_iterator>
        range = map_list->equal_range(entrance);
    const int key_count =
        static_cast<int>(map_list->count(entrance));
    if (!key_count) {
        LogManager::logFormat(
            1,
            "data_manager.cpp",
            "int CDungeon::ChooseRandomMap(int, int, int) const",
            0x77b,
            "CDungeon::ChooseRandomMap (Key count is zero! : plz check map script)(Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)",
            dungeon_index_,
            map_type,
            direction,
            entrance);
        return -1;
    }

    DirectionMapList::const_iterator selected = range.first;
    for (int index = 0;
         std::rand() % key_count > index;
         ++index) {
        ++selected;
    }
    CMap* const map = selected->second;
    if (map->get_index() == -1) {
        LogManager::logFormat(
            1,
            "data_manager.cpp",
            "int CDungeon::ChooseRandomMap(int, int, int) const",
            0x786,
            "CDungeon::ChooseRandomMap (p_map->get_index()==-1)[3](Dungeon Index(%d))(map_type(%d),direction(%d),entrance(%d)",
            dungeon_index_,
            map_type,
            direction,
            entrance);
    }
    return map->get_index();
}

// IDA_EA: 0x834C75A
// IDA_NAME: _ZNK8CDungeon11GetStartMapEi
// DEMANGLED: CDungeon::GetStartMap(int)const
int CDungeon::GetStartMap(int direction) const {
    if (static_cast<unsigned int>(direction) >= 16u) {
        return -1;
    }

    const StartMapHash& map_list = start_maps_[direction];
    if (map_list.empty()) {
        LogManager::logFormat(
            1,
            "data_manager.cpp",
            "int CDungeon::GetStartMap(int) const",
            0x79a,
            "CDungeon::GetStartMap() : direction(%d), dungeon index(%d), start_maplist_[direction].size() == 0, May be Script Error!",
            direction,
            dungeon_index_);
        return -1;
    }

    const int random_index =
        static_cast<int>(
            get_rand_int(static_cast<int>(map_list.size())));
    StartMapHash::const_iterator selected = map_list.begin();
    for (int index = 0; index < random_index; ++index) {
        ++selected;
        if (selected == map_list.end()) {
            return -1;
        }
    }
    return selected->first;
}

// IDA_EA: 0x834D06C
// IDA_NAME: _ZNK8CDungeon23GetRandMazebyPartyCountEi
// DEMANGLED: CDungeon::GetRandMazebyPartyCount(int)const
MazeScript* CDungeon::GetRandMazebyPartyCount(int member_count) const {
    if (maze_scripts_.size() == 1u) {
        return const_cast<MazeScript*>(&maze_scripts_[0]);
    }

    std::vector<MazeScript*> matching;
    for (std::vector<MazeScript>::const_iterator maze =
             maze_scripts_.begin();
         maze != maze_scripts_.end();
         ++maze) {
        if (maze->value_68 <= member_count &&
            maze->value_6c >= member_count) {
            matching.push_back(const_cast<MazeScript*>(&*maze));
        }
    }

    if (matching.empty()) {
        const unsigned int index =
            static_cast<unsigned int>(std::rand()) %
            maze_scripts_.size();
        return const_cast<MazeScript*>(&maze_scripts_[index]);
    }
    if (matching.size() == 1u) {
        return matching[0];
    }
    const unsigned int index =
        static_cast<unsigned int>(std::rand()) % matching.size();
    return matching[index];
}
