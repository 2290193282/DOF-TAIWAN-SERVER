#include "old_data/PVF/map/datamanager/world_map_script.h"

#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/script_initializers.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

#include <string.h>

#include <cctype>
#include <cstdio>
#include <string>
#include <utility>

extern STScriptFileList g_stDungeonScriptFileList;

namespace {

char g_worldMapScriptBaseDirectory[256] = {};

bool SameRegistryPath(const std::string& lhs, const char* rhs) {
    if (!rhs) {
        return false;
    }
    const std::string expected(rhs);
    if (lhs.size() != expected.size()) {
        return false;
    }
    for (std::string::size_type index = 0; index < lhs.size(); ++index) {
        unsigned char left = static_cast<unsigned char>(lhs[index]);
        unsigned char right = static_cast<unsigned char>(expected[index]);
        if (left == '\\') {
            left = '/';
        }
        if (right == '\\') {
            right = '/';
        }
        if (std::tolower(left) != std::tolower(right)) {
            return false;
        }
    }
    return true;
}

bool HasOfficialSouthernDaleDungeonRegistry() {
    using namespace oldsrv::official_registry;

    const STScriptFileList::const_iterator entry =
        g_stDungeonScriptFileList.find(kSouthernDaleDungeonIndex);
    return entry != g_stDungeonScriptFileList.end() &&
           SameRegistryPath(entry->second,
                            SouthernDaleDungeonScriptPath());
}

void RestoreOfficialSouthernDaleWorldMap(WorldMapScript* script) {
    using namespace oldsrv::official_registry;

    if (!script || script->raw_00 != kSouthernDaleWorldMapIndex ||
        !HasOfficialSouthernDaleDungeonRegistry()) {
        return;
    }
    for (std::vector<WorldMapDungeon>::const_iterator dungeon =
             script->dungeons.begin();
         dungeon != script->dungeons.end(); ++dungeon) {
        if (dungeon->dungeon_index == kSouthernDaleDungeonIndex) {
            return;
        }
    }

    WorldMapDungeon dungeon;
    dungeon.dungeon_index = kSouthernDaleDungeonIndex;
    dungeon.area_index = -1;
    script->dungeons.push_back(dungeon);
    std::printf("SOUTHERN_DALE_0828_WORLDMAP_FALLBACK: "
                "worldmap=%d dungeon=%d\n",
                kSouthernDaleWorldMapIndex,
                kSouthernDaleDungeonIndex);
}

}  // namespace

STScriptFileList g_worldMapScriptFileList;
STWorldmapPattern g_worldMapPatternTable[3][10];
STWorldmapPattern2ND g_worldMapPatternTable2ND[20];

const STWorldmapPattern* findWorldMapPattern0828(const char* tag,
                                                int ordinal) {
    if (!tag || ordinal < 0 || ordinal >= 10) {
        return 0;
    }
    int map_type = -1;
    if (strcmp(tag, "[A]") == 0) {
        map_type = 0;
    } else if (strcmp(tag, "[B]") == 0) {
        map_type = 1;
    } else {
        return 0;
    }
    const STWorldmapPattern& entry =
        g_worldMapPatternTable[map_type][ordinal];
    return entry.pattern == tag ? &entry : 0;
}

// IDA_EA: 0x8AB000C
// IDA_NAME: _Z18initWorldMapScriptPcS_
// DEMANGLED: initWorldMapScript(char *,char *)
int initWorldMapScript(char* src, char* filename) {
    strncpy(g_worldMapScriptBaseDirectory, src, 0xFFu);
    g_worldMapScriptBaseDirectory[0xFF] = 0;
    const size_t length = strlen(g_worldMapScriptBaseDirectory);
    if (length != 0 &&
        g_worldMapScriptBaseDirectory[length - 1] != '\\' &&
        g_worldMapScriptBaseDirectory[length - 1] != '/') {
        memcpy(g_worldMapScriptBaseDirectory + length, "\\", 2u);
    }

    if (loadRDARScriptListFile(&g_worldMapScriptFileList,
                               g_worldMapScriptBaseDirectory,
                               filename) != 1) {
        return 0;
    }
    if (loadRDARScriptFile(g_worldMapScriptBaseDirectory,
                           "WorldMapPatternTable.tbl") != 1) {
        return 0;
    }

    std::string pattern;
    for (int map_type = 0;
         map_type <= 2 && ScanType(pattern, true) == 1;
         ++map_type) {
        for (int pattern_index = 0; pattern_index <= 9; ++pattern_index) {
            STWorldmapPattern& entry =
                g_worldMapPatternTable[map_type][pattern_index];
            entry.pattern = pattern;
            for (int coord = 0; coord <= 2; ++coord) {
                for (int axis = 0; axis <= 1; ++axis) {
                    const int value = ScanInt(static_cast<bool*>(0));
                    switch (coord * 2 + axis) {
                        case 0:
                            entry.raw_08 = value;
                            break;
                        case 1:
                            entry.raw_0c = value;
                            break;
                        case 2:
                            entry.raw_10 = value;
                            break;
                        case 3:
                            entry.raw_14 = value;
                            break;
                        case 4:
                            entry.raw_18 = value;
                            break;
                        case 5:
                            entry.raw_1c = value;
                            break;
                    }
                }
            }
        }
    }

    loadWorldMapCoordPattern();
    return 1;
}

// IDA_EA: 0x8AB0234
// IDA_NAME: _Z24loadWorldMapCoordPatternv
// DEMANGLED: loadWorldMapCoordPattern(void)
int loadWorldMapCoordPattern() {
    if (loadRDARScriptFile(g_worldMapScriptBaseDirectory,
                           "WorldMapPatternTable2ND.tbl") != 1) {
        return 0;
    }

    bool is_absolute = true;
    bool success = false;
    std::string text;
    for (int row = 0; row <= 19; ++row) {
        const int map_no =
            static_cast<int>(ScanAbsOrRel(is_absolute, text, success, true));
        if (!success) {
            break;
        }
        g_worldMapPatternTable2ND[row].map_no = map_no;
        for (int coord = 0; coord <= 2; ++coord) {
            for (int axis = 0; axis <= 1; ++axis) {
                const int value = ScanInt(static_cast<bool*>(0));
                memcpy(&g_worldMapPatternTable2ND[row]
                            .raw_04[(coord * 2 + axis) * sizeof(int)],
                       &value,
                       sizeof(value));
            }
        }
    }
    return success ? 1 : 0;
}

// IDA_EA: 0x8AB0388
// IDA_NAME: _Z20importWorldMapScriptP14WorldMapScriptPKc
// DEMANGLED: importWorldMapScript(WorldMapScript *,char const*)
int importWorldMapScript(WorldMapScript* script, const char* filename) {
    if (loadRDARScriptFile(g_worldMapScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    script->clear();
    std::string tag;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->name);
        } else if (tag == "[map image]") {
            ScanStr(&script->map_image);
            script->raw_10 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ui path]") {
            ScanStr(&script->ui_path);
        } else if (tag == "[festival condition]") {
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            world_map_0828::SetFestivalCondition(script, value);
        } else if (tag == "[exercise]") {
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            world_map_0828::SetExercise(script, value);
        } else if (tag == "[dungeon]") {
            script->dungeons.clear();
            while (1) {
                const int dungeon_index = ScanInt(&success);
                if (!success) {
                    break;
                }

                WorldMapDungeon dungeon;
                dungeon.dungeon_index = dungeon_index;

                bool is_type = false;
                const int area_index =
                    ScanTypeOrInt(is_type, tag, success, true);
                if (!success) {
                    break;
                }
                if (is_type) {
                    if (tag == "[in progress]") {
                        dungeon.map_index = 1;
                    }
                    dungeon.area_index = ScanInt(static_cast<bool*>(0));
                } else {
                    dungeon.area_index = area_index;
                }
                script->dungeons.push_back(dungeon);
            }
        } else if (tag == "[hell dungeon]") {
            script->raw_20 =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[hell quest]") {
            while (1) {
                const int quest = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->hell_dungeons.push_back(quest);
            }
        } else if (tag == "[hell freepass item]") {
            while (1) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(static_cast<bool*>(0));
                script->hell_quests.push_back(
                    std::make_pair(item_index, item_count));
            }
        } else if (tag == "[item condition]") {
            while (1) {
                const int item_index = ScanInt(&success);
                ScanInt(static_cast<bool*>(0));
                if (!success) {
                    break;
                }
                script->freepass_items.push_back(item_index);
            }
        } else if (tag == "[minimap points]") {
            int value = ScanInt(&success);
            while (success) {
                int x = 0;
                int y = 0;
                for (int index = 0; index <= 1 && success; ++index) {
                    if (index != 0) {
                        y = value;
                    } else {
                        x = value;
                    }
                    value = ScanInt(&success);
                }
                script->item_conditions.push_back(std::make_pair(x, y));
            }
        }
    }

    script->raw_00 = g_worldMapScriptFileList.FindIndexByFullScan(filename);
    RestoreOfficialSouthernDaleWorldMap(script);
    return 1;
}
