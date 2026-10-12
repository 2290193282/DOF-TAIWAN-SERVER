#include "old_data/PVF/event/blue_marble_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <new>
#include <stddef.h>
#include <string>


// IDA_EA: 0x88D8EE0
// IDA_NAME: _ZN27BlueMarbleRandomDungeonInfoD2Ev
// DEMANGLED: BlueMarbleRandomDungeonInfo::~BlueMarbleRandomDungeonInfo()
BlueMarbleRandomDungeonInfo::~BlueMarbleRandomDungeonInfo() {}

// IDA_EA: 0x88D97E0
// IDA_NAME: _ZN27BlueMarbleDungeonDifficultyD2Ev
// DEMANGLED: BlueMarbleDungeonDifficulty::~BlueMarbleDungeonDifficulty()
BlueMarbleDungeonDifficulty::~BlueMarbleDungeonDifficulty() {}

// IDA_EA: 0x88D9120
// IDA_NAME: _ZN24BlueMarbleRandomItemInfoD2Ev
// DEMANGLED: BlueMarbleRandomItemInfo::~BlueMarbleRandomItemInfo()
BlueMarbleRandomItemInfo::~BlueMarbleRandomItemInfo() {}

// IDA_EA: 0x88D9360
// IDA_NAME: _ZN20BlueMarbleRewardInfoD2Ev
// DEMANGLED: BlueMarbleRewardInfo::~BlueMarbleRewardInfo()
BlueMarbleRewardInfo::~BlueMarbleRewardInfo() {}

// IDA_EA: 0x88D95A0
// IDA_NAME: _ZN17BlueMarbleTilePosD2Ev
// DEMANGLED: BlueMarbleTilePos::~BlueMarbleTilePos()
BlueMarbleTilePos::~BlueMarbleTilePos() {}

// IDA_EA: 0x88D5068
// IDA_NAME: _ZN20BlueMarbleTileScriptC2Ev
// DEMANGLED: BlueMarbleTileScript::BlueMarbleTileScript(void)
BlueMarbleTileScript::BlueMarbleTileScript() {
    clear();
}

// IDA_EA: 0x88D507C
// IDA_NAME: _ZN20BlueMarbleTileScript5clearEv
// DEMANGLED: BlueMarbleTileScript::clear(void)
BlueMarbleTileScript* BlueMarbleTileScript::clear() {
    tile_type = 0;
    enter_dungeon = 0;
    value = 0;
    return this;
}

// IDA_EA: 0x88D520A
// IDA_NAME: _ZN20BlueMarbleTileScript6toTypeESs
// DEMANGLED: BlueMarbleTileScript::toType(std::string)
int BlueMarbleTileScript::toType(std::string type_name) {
    if (type_name == "empty") return 0;
    if (type_name == "start") return 1;
    if (type_name == "end") return 2;
    if (type_name == "gold") return 3;
    if (type_name == "exp") return 4;
    if (type_name == "drop") return 5;
    if (type_name == "item") return 6;
    if (type_name == "unique item") return 7;
    if (type_name == "boss unique") return 8;
    if (type_name == "move") return 9;
    if (type_name == "jump") return 10;
    if (type_name == "gold goblin") return 11;
    if (type_name == "rope") return 12;
    if (type_name == "kick") return 13;
    return 0;
}

// IDA_EA: 0x88D509C
// IDA_NAME: _ZN20BlueMarbleTileScript12importScriptEPKc
// DEMANGLED: BlueMarbleTileScript::importScript(char const*)
int BlueMarbleTileScript::importScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    int enter_dungeon = 0;
    while (ScanType(tag, true) == 1) {
        if (tag == "[type]") {
            ScanStr(&text);
            tile_type = toType(text);
        } else if (tag == "[enter dungeon]") {
            ScanInt(&enter_dungeon);
            if (enter_dungeon) {
                this->enter_dungeon = 1;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x88D53D6
// IDA_NAME: _ZN19BlueMarbleMapScriptC2Ev
// DEMANGLED: BlueMarbleMapScript::BlueMarbleMapScript(void)
BlueMarbleMapScript::BlueMarbleMapScript() : tiles() {
    clear();
}

// IDA_EA: 0x88D5418
// IDA_NAME: _ZN19BlueMarbleMapScriptD2Ev
// DEMANGLED: BlueMarbleMapScript::~BlueMarbleMapScript()
BlueMarbleMapScript::~BlueMarbleMapScript() {
    for (std::vector<BlueMarbleTileScript*>::size_type index = 0;
         index < tiles.size(); ++index) {
        delete tiles[index];
        tiles[index] = 0;
    }
    tiles.clear();
}

// IDA_EA: 0x88D54CE
// IDA_NAME: _ZN19BlueMarbleMapScript5clearEv
// DEMANGLED: BlueMarbleMapScript::clear(void)
int BlueMarbleMapScript::clear() {
    tiles.clear();
    return 0;
}

// IDA_EA: 0x88D54E2
// IDA_NAME: _ZN19BlueMarbleMapScript12importScriptEPKc
// DEMANGLED: BlueMarbleMapScript::importScript(char const*)
int BlueMarbleMapScript::importScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    int tile_index = 0;
    while (ScanType(tag, true) == 1) {
        if (tag == "[tile]") {
            tiles.clear();
            while (ScanInt(&tile_index)) {
                BlueMarbleTileScript* base =
                    BlueMarbleScriptManager::getInsance()->getTile(tile_index);
                if (!base) {
                    return 0;
                }
                if (!ScanInt(&tile_index)) {
                    break;
                }

                BlueMarbleTileScript* row = new BlueMarbleTileScript();
                *row = *base;
                row->value = tile_index;
                tiles.push_back(row);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x88D5644
// IDA_NAME: _ZN19BlueMarbleMapScript7getTileEj
// DEMANGLED: BlueMarbleMapScript::getTile(unsigned int)
BlueMarbleTileScript* BlueMarbleMapScript::getTile(unsigned int board_zone) {
    if (tiles.size() < board_zone || board_zone == 0) {
        return 0;
    }
    return tiles[board_zone - 1];
}

// IDA_EA: 0x837E218
// IDA_NAME: _ZN20BlueMarbleInfoScriptD2Ev
// DEMANGLED: BlueMarbleInfoScript::~BlueMarbleInfoScript()
BlueMarbleInfoScript::~BlueMarbleInfoScript() {}

// IDA_EA: 0x88D5690
// IDA_NAME: _ZN20BlueMarbleInfoScriptC2Ev
// DEMANGLED: BlueMarbleInfoScript::BlueMarbleInfoScript(void)
BlueMarbleInfoScript::BlueMarbleInfoScript()
    : player_max(0),
      enter_level(0),
      enter_max_level(0),
      enter_count(0),
      die_count(0),
      dice_min(0),
      dice_max(0),
      item_probability_total(0),
      default_dungeon_difficulty(0),
      dungeon_difficulties(),
      super_armor_time(0),
      buff_max_value(0),
      gold_base(0.0f),
      gold_multi(0.0f),
      equipment_upgrade_max(1000),
      random_dungeons(),
      boss_dungeons(),
      unique_dungeons(),
      random_items(),
      reward_items(),
      tile_positions() {
    clear();
}

// IDA_EA: 0x88D57C0
// IDA_NAME: _ZN20BlueMarbleInfoScript5clearEv
// DEMANGLED: BlueMarbleInfoScript::clear(void)
BlueMarbleInfoScript* BlueMarbleInfoScript::clear() {
    player_max = 0;
    enter_level = 0;
    enter_max_level = 0;
    enter_count = 0;
    die_count = 0;
    dice_min = 0;
    dice_max = 0;
    item_probability_total = 0;
    default_dungeon_difficulty = 0;
    dungeon_difficulties.clear();
    super_armor_time = 0;
    buff_max_value = 0;
    gold_base = 0.0f;
    gold_multi = 0.0f;
    random_dungeons.clear();
    boss_dungeons.clear();
    random_items.clear();
    tile_positions.clear();
    equipment_upgrade_max = 1000;
    return this;
}

// IDA_EA: 0x88D589C
// IDA_NAME: _ZN20BlueMarbleInfoScript12importScriptEPKcS1_
// DEMANGLED: BlueMarbleInfoScript::importScript(char const*,char const*)
int BlueMarbleInfoScript::importScript(const char* dir, const char* filename) {
    if (!loadRDARScriptFile(dir, filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[dungeon list]") {
            random_dungeons.clear();
            BlueMarbleRandomDungeonInfo row;
            while (ScanInt(&row.min_level) && ScanInt(&row.max_level) &&
                   ScanInt(&row.dungeon_index)) {
                random_dungeons.push_back(row);
            }
        } else if (tag == "[boss dungeon list]") {
            boss_dungeons.clear();
            BlueMarbleRandomDungeonInfo row;
            while (ScanInt(&row.min_level) && ScanInt(&row.max_level) &&
                   ScanInt(&row.dungeon_index)) {
                boss_dungeons.push_back(row);
            }
        } else if (tag == "[item list]") {
            random_items.clear();
            BlueMarbleRandomItemInfo row;
            while (ScanInt(&row.first_dword) && ScanInt(&row.second_dword)) {
                item_probability_total += row.second_dword;
                random_items.push_back(row);
            }
        } else if (tag == "[player max]") {
            ScanInt(&player_max);
        } else if (tag == "[enter level]") {
            ScanInt(&enter_level);
        } else if (tag == "[enter max level]") {
            ScanInt(&enter_max_level);
        } else if (tag == "[enter count]") {
            ScanInt(&enter_count);
        } else if (tag == "[die count]") {
            ScanInt(&die_count);
        } else if (tag == "[dice value]") {
            ScanInt(&dice_min);
            ScanInt(&dice_max);
        } else if (tag == "[reward item]") {
            reward_items.clear();
            BlueMarbleRewardInfo row;
            while (ScanInt(&row.first_dword) && ScanInt(&row.second_dword)) {
                reward_items.push_back(row);
            }
        } else if (tag == "[tile pos]") {
            tile_positions.clear();
            BlueMarbleTilePos row;
            while (ScanInt(&row.first_dword) && ScanInt(&row.second_dword)) {
                tile_positions.push_back(row);
            }
        } else if (tag == "[dungeon difficulty]") {
            default_dungeon_difficulty = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[dungeon difficulty list]") {
            dungeon_difficulties.clear();
            BlueMarbleDungeonDifficulty row;
            while (ScanInt(&row.dungeon_index) && ScanInt(&row.difficulty)) {
                dungeon_difficulties.push_back(row);
            }
        } else if (tag == "[super armor time]") {
            super_armor_time = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[unique dungeon list]") {
            unique_dungeons.clear();
            BlueMarbleRandomDungeonInfo row;
            while (ScanInt(&row.min_level) && ScanInt(&row.max_level) &&
                   ScanInt(&row.dungeon_index)) {
                unique_dungeons.push_back(row);
            }
        } else if (tag == "[buff max value]") {
            ScanInt(&buff_max_value);
        } else if (tag == "[gold base]") {
            ScanFloat(&gold_base);
        } else if (tag == "[gold multi]") {
            ScanFloat(&gold_multi);
        } else if (tag == "[equipment upgrade max]") {
            equipment_upgrade_max = ScanInt(static_cast<bool*>(0));
        }
    }
    return 1;
}

// IDA_EA: 0x88D5E92
// IDA_NAME: _ZN20BlueMarbleInfoScript21getRandomDungeonIndexEi
// DEMANGLED: BlueMarbleInfoScript::getRandomDungeonIndex(int)
std::vector<int> BlueMarbleInfoScript::getRandomDungeonIndex(
    int charac_level) {
    std::vector<int> result;
    for (std::vector<BlueMarbleRandomDungeonInfo>::size_type index = 0;
         index < random_dungeons.size(); ++index) {
        const BlueMarbleRandomDungeonInfo& row = random_dungeons[index];
        if (row.min_level <= charac_level && row.max_level >= charac_level) {
            result.push_back(row.dungeon_index);
        }
    }
    return result;
}

// IDA_EA: 0x88D5F38
// IDA_NAME: _ZN20BlueMarbleInfoScript19getBossDungeonIndexEi
// DEMANGLED: BlueMarbleInfoScript::getBossDungeonIndex(int)
std::vector<int> BlueMarbleInfoScript::getBossDungeonIndex(int charac_level) {
    std::vector<int> result;
    for (std::vector<BlueMarbleRandomDungeonInfo>::size_type index = 0;
         index < boss_dungeons.size(); ++index) {
        const BlueMarbleRandomDungeonInfo& row = boss_dungeons[index];
        if (row.min_level <= charac_level && row.max_level >= charac_level) {
            result.push_back(row.dungeon_index);
        }
    }
    return result;
}

// IDA_EA: 0x88D5FDE
// IDA_NAME: _ZN20BlueMarbleInfoScript21getUniqueDungeonIndexEi
// DEMANGLED: BlueMarbleInfoScript::getUniqueDungeonIndex(int)
std::vector<int> BlueMarbleInfoScript::getUniqueDungeonIndex(
    int charac_level) {
    std::vector<int> result;
    for (std::vector<BlueMarbleRandomDungeonInfo>::size_type index = 0;
         index < unique_dungeons.size(); ++index) {
        const BlueMarbleRandomDungeonInfo& row = unique_dungeons[index];
        if (row.min_level <= charac_level && row.max_level >= charac_level) {
            result.push_back(row.dungeon_index);
        }
    }
    return result;
}

// IDA_EA: 0x88D6084
// IDA_NAME: _ZN20BlueMarbleInfoScript20getDungeonDifficultyEi
// DEMANGLED: BlueMarbleInfoScript::getDungeonDifficulty(int)
int BlueMarbleInfoScript::getDungeonDifficulty(int dungeon_index) {
    for (std::vector<BlueMarbleDungeonDifficulty>::size_type index = 0;
         index < dungeon_difficulties.size(); ++index) {
        if (dungeon_difficulties[index].dungeon_index == dungeon_index) {
            return dungeon_difficulties[index].difficulty;
        }
    }
    return default_dungeon_difficulty;
}

// IDA_EA: 0x837E354
// IDA_NAME: _ZN23BlueMarbleScriptManagerD2Ev
// DEMANGLED: BlueMarbleScriptManager::~BlueMarbleScriptManager()
BlueMarbleScriptManager::~BlueMarbleScriptManager() {}

// IDA_EA: 0x88D60F2
// IDA_NAME: _ZN23BlueMarbleScriptManager10getInsanceEv
// DEMANGLED: BlueMarbleScriptManager::getInsance(void)
BlueMarbleScriptManager* BlueMarbleScriptManager::getInsance() {
    static BlueMarbleScriptManager* instance = 0;
    if (!instance) {
        instance = new BlueMarbleScriptManager();
    }
    return instance;
}

// IDA_EA: 0x88D614A
// IDA_NAME: _ZN23BlueMarbleScriptManagerC2Ev
// DEMANGLED: BlueMarbleScriptManager::BlueMarbleScriptManager(void)
BlueMarbleScriptManager::BlueMarbleScriptManager()
    : map_file_list(),
      tile_file_list(),
      info_script(),
      tile_scripts(),
      map_scripts() {
    clear();
}

// IDA_EA: 0x88D622C
// IDA_NAME: _ZN23BlueMarbleScriptManager5clearEv
// DEMANGLED: BlueMarbleScriptManager::clear(void)
int BlueMarbleScriptManager::clear() {
    map_file_list.Clear();
    tile_file_list.Clear();
    tile_scripts.clear();
    map_scripts.clear();
    return 0;
}

// IDA_EA: 0x88D626E
// IDA_NAME: _ZN23BlueMarbleScriptManager12initTileListEPKcS1_
// DEMANGLED: BlueMarbleScriptManager::initTileList(char const*,char const*)
int BlueMarbleScriptManager::initTileList(const char* dir,
                                          const char* filename) {
    if (loadRDARScriptListFile(&tile_file_list, dir, filename) != 1) {
        return 0;
    }

    tile_scripts.clear();
    for (STScriptFileList::iterator it = tile_file_list.begin();
         it != tile_file_list.end(); ++it) {
        BlueMarbleTileScript* script = new BlueMarbleTileScript();
        const int index = it->first;
        std::string full_name(dir != 0 ? dir : "");
        full_name += it->second;
        if (script->importScript(full_name.c_str()) == 1) {
            tile_scripts[index] = script;
        } else {
            delete script;
            script = 0;
        }
    }
    return 1;
}

// IDA_EA: 0x88D6438
// IDA_NAME: _ZN23BlueMarbleScriptManager11initMapListEPKcS1_
// DEMANGLED: BlueMarbleScriptManager::initMapList(char const*,char const*)
int BlueMarbleScriptManager::initMapList(const char* dir,
                                         const char* filename) {
    if (loadRDARScriptListFile(&map_file_list, dir, filename) != 1) {
        return 0;
    }

    map_scripts.clear();
    for (STScriptFileList::iterator it = map_file_list.begin();
         it != map_file_list.end(); ++it) {
        BlueMarbleMapScript* script = new BlueMarbleMapScript();
        const int index = it->first;
        std::string full_name(dir != 0 ? dir : "");
        full_name += it->second;
        if (script->importScript(full_name.c_str()) == 1) {
            map_scripts[index] = script;
        } else {
            delete script;
            script = 0;
        }
    }
    return 1;
}

// IDA_EA: 0x88D6622
// IDA_NAME: _ZN23BlueMarbleScriptManager18initBlueMarbleInfoEPKcS1_
// DEMANGLED: BlueMarbleScriptManager::initBlueMarbleInfo(char const*,char const*)
int BlueMarbleScriptManager::initBlueMarbleInfo(const char* dir,
                                                const char* filename) {
    return info_script.importScript(dir, filename);
}

// IDA_EA: 0x88D6646
// IDA_NAME: _ZN23BlueMarbleScriptManager7getTileEi
// DEMANGLED: BlueMarbleScriptManager::getTile(int)
BlueMarbleTileScript* BlueMarbleScriptManager::getTile(int tile_index) {
    std::map<int, BlueMarbleTileScript*>::iterator it =
        tile_scripts.find(tile_index);
    if (it != tile_scripts.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x88D66B6
// IDA_NAME: _ZN23BlueMarbleScriptManager6getMapEi
// DEMANGLED: BlueMarbleScriptManager::getMap(int)
BlueMarbleMapScript* BlueMarbleScriptManager::getMap(int map_index) {
    std::map<int, BlueMarbleMapScript*>::iterator it =
        map_scripts.find(map_index);
    if (it != map_scripts.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x88D6726
// IDA_NAME: _ZN23BlueMarbleScriptManager16getMapTotalCountEv
// DEMANGLED: BlueMarbleScriptManager::getMapTotalCount(void)
int BlueMarbleScriptManager::getMapTotalCount() {
    return static_cast<int>(map_scripts.size());
}
