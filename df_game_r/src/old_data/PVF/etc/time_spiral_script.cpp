#include "old_data/PVF/etc/time_spiral_script.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/manager/global_data.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/passiveobject/passive_object_script_types.h"

namespace time_spiral {
namespace {

const int kRouteWeightTotal = 1000000;
const int kBuffRateTotal = 100;
const int kExpectedTrapMapCount = 7;
const int kExpectedBuffCount = 4;
const int kExpectedBuffDurationMs = 60000;

struct Route {
    Route() : x(0), y(0), weight(0), result(0) {}

    int x;
    int y;
    int weight;
    int result;
};

struct SourceRoute {
    SourceRoute()
        : source_map(0), source_x(-1), source_y(-1),
          normal_x(-1), normal_y(-1), routes() {}

    int source_map;
    int source_x;
    int source_y;
    int normal_x;
    int normal_y;
    std::vector<Route> routes;
};

struct TrapBuff {
    TrapBuff() : id(-1), label(), stats(), duration_ms(0) {}

    int id;
    std::string label;
    std::map<std::string, int> stats;
    int duration_ms;
};

struct Config {
    Config()
        : designated_difficulty(-1), trap_maps(), sources(), buff_rates(), buffs(),
          map_by_coordinate(), coordinate_by_map(), terminals() {}

    int designated_difficulty;
    std::vector<int> trap_maps;
    std::map<int, SourceRoute> sources;
    std::map<int, int> buff_rates;
    std::map<int, TrapBuff> buffs;
    std::map<std::pair<int, int>, int> map_by_coordinate;
    std::map<int, std::pair<int, int> > coordinate_by_map;
    std::set<std::pair<int, int> > terminals;
};

Config& PendingConfig() {
    static Config config;
    return config;
}

Config& PublishedConfig() {
    static Config config;
    return config;
}

bool& ParsedFlag() {
    static bool parsed = false;
    return parsed;
}

bool& ReadyFlag() {
    static bool ready = false;
    return ready;
}

std::set<int>& ValidatedTrapMaps() {
    static std::set<int> maps;
    return maps;
}

class ScriptUnloadGuard {
public:
    ScriptUnloadGuard() {}
    ~ScriptUnloadGuard() { unloadRDARScriptFile(); }

private:
    ScriptUnloadGuard(const ScriptUnloadGuard&);
    ScriptUnloadGuard& operator=(const ScriptUnloadGuard&);
};

bool ReadToken(int* type, std::string* text, int* value) {
    if (!type || !text || !value) return false;
    float real_value = 0.0f;
    const DNFLEX_TOKEN token =
        ScanAll(value, &real_value, text);
    if (token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL) {
        *type = 1;
        return true;
    }
    if (token == DNF_TOK_STRING) {
        *type = 0;
        return true;
    }
    if (token == DNF_TOK_TYPE) {
        *type = 2;
        return true;
    }
    // FLOAT, custom data, connector/include, lexer error and EOF are all
    // distinct from this grammar.  Consume exactly one token and reject;
    // never scan forward to a later legal value.
    *type = 3;
    return false;
}

bool ReadType(std::string* value) {
    int type = 3;
    int ignored = 0;
    return value && ReadToken(&type, value, &ignored) && type == 2;
}

bool ReadInt(int* value) {
    int type = 3;
    std::string ignored;
    return value && ReadToken(&type, &ignored, value) && type == 1;
}

bool ReadString(std::string* value) {
    int type = 3;
    int ignored = 0;
    return value && ReadToken(&type, value, &ignored) && type == 0;
}

bool AtEof() {
    int value = 0;
    float real_value = 0.0f;
    std::string text;
    return ScanAll(&value, &real_value, &text) == DNF_TOK_EOF;
}

bool ReadTrapMapList(Config* config) {
    std::set<int> unique;
    while (1) {
        int type = 3;
        int value = 0;
        std::string text;
        if (!ReadToken(&type, &text, &value)) return false;
        if (type == 2) {
            return text == "[/timespiral trap map list]" &&
                   config->trap_maps.size() ==
                       static_cast<unsigned int>(kExpectedTrapMapCount);
        }
        if (type != 1 || value <= 0 || !unique.insert(value).second ||
            config->trap_maps.size() >=
                static_cast<unsigned int>(kExpectedTrapMapCount)) {
            return false;
        }
        config->trap_maps.push_back(value);
    }
}

bool ReadMoveList(SourceRoute* source) {
    std::set<std::pair<int, int> > targets;
    int total_weight = 0;
    int normal_count = 0;
    while (1) {
        int type = 3;
        int first = 0;
        std::string text;
        if (!ReadToken(&type, &text, &first)) return false;
        if (type == 2) {
            return text == "[/move list]" && !source->routes.empty() &&
                   total_weight == kRouteWeightTotal && normal_count == 1;
        }
        if (type != 1) return false;

        Route route;
        route.x = first;
        if (!ReadInt(&route.y) || !ReadInt(&route.weight) ||
            !ReadInt(&route.result) || route.x < 0 ||
            route.x >= kGridWidth || route.y < 0 ||
            route.y >= kGridHeight || route.weight <= 0 ||
            (route.result != 0 && route.result != 1) ||
            !targets.insert(std::make_pair(route.x, route.y)).second ||
            total_weight > kRouteWeightTotal - route.weight) {
            return false;
        }
        total_weight += route.weight;
        if (route.result == 0) {
            ++normal_count;
            source->normal_x = route.x;
            source->normal_y = route.y;
        }
        source->routes.push_back(route);
    }
}

bool ReadTeleportTrap(Config* config, int expected_source) {
    SourceRoute source;
    std::string tag;
    if (!ReadInt(&source.source_map) ||
        source.source_map != expected_source ||
        config->sources.find(source.source_map) != config->sources.end() ||
        !ReadType(&tag) || tag != "[move list]" ||
        !ReadMoveList(&source) || !ReadType(&tag) ||
        tag != "[/teleport trap]") {
        return false;
    }
    config->sources.insert(std::make_pair(source.source_map, source));
    return true;
}

bool ReadBuffRates(Config* config) {
    int total = 0;
    while (1) {
        int type = 3;
        int id = 0;
        std::string text;
        if (!ReadToken(&type, &text, &id)) return false;
        if (type == 2) {
            return text == "[/trap buff rate]" &&
                   config->buff_rates.size() ==
                       static_cast<unsigned int>(kExpectedBuffCount) &&
                   total == kBuffRateTotal;
        }
        int weight = 0;
        if (type != 1 || id < 0 || id >= kExpectedBuffCount ||
            !ReadInt(&weight) || weight <= 0 ||
            total > kBuffRateTotal - weight ||
            !config->buff_rates.insert(std::make_pair(id, weight)).second) {
            return false;
        }
        total += weight;
    }
}

bool IsAllowedBuffStat(const std::string& tag) {
    return tag == "[physical attack]" || tag == "[magical attack]" ||
           tag == "[move speed]" || tag == "[attack speed]" ||
           tag == "[cast speed]";
}

bool ReadTrapBuff(Config* config, int expected_id) {
    TrapBuff buff;
    if (!ReadInt(&buff.id) || buff.id != expected_id ||
        config->buffs.find(buff.id) != config->buffs.end() ||
        !ReadString(&buff.label) || buff.label.empty()) {
        return false;
    }

    bool saw_duration = false;
    while (1) {
        std::string tag;
        if (!ReadType(&tag)) return false;
        if (tag == "[/trap buff]") {
            if (!saw_duration || buff.stats.empty() ||
                buff.duration_ms != kExpectedBuffDurationMs) {
                return false;
            }
            config->buffs.insert(std::make_pair(buff.id, buff));
            return true;
        }
        if (saw_duration) return false;

        int value = 0;
        if (tag == "[buff time]") {
            if (!ReadInt(&value) || value != kExpectedBuffDurationMs) {
                return false;
            }
            buff.duration_ms = value;
            saw_duration = true;
        } else if (!IsAllowedBuffStat(tag) || !ReadInt(&value) ||
                   value < -10000 || value > 10000 || value == 0 ||
                   !buff.stats.insert(std::make_pair(tag, value)).second) {
            return false;
        }
    }
}

bool ParseRoot(Config* config) {
    std::string tag;
    if (!ReadType(&tag) || tag != "[timespiral trap map list]" ||
        !ReadTrapMapList(config)) {
        return false;
    }

    for (int index = 0; index < kExpectedTrapMapCount; ++index) {
        if (!ReadType(&tag) || tag != "[teleport trap]" ||
            !ReadTeleportTrap(config, config->trap_maps[index])) {
            return false;
        }
    }

    if (!ReadType(&tag) || tag != "[trap buff rate]" ||
        !ReadBuffRates(config)) {
        return false;
    }
    for (int id = 0; id < kExpectedBuffCount; ++id) {
        if (!ReadType(&tag) || tag != "[trap buff]" ||
            !ReadTrapBuff(config, id)) {
            return false;
        }
    }
    return AtEof();
}

bool ExtractGridMapIndex(const GridScript& grid, int* map_index) {
    if (!map_index || !grid.values_1c.empty()) return false;
    const unsigned int ordinary =
        static_cast<unsigned int>(grid.values_04.size());
    const unsigned int boss =
        static_cast<unsigned int>(grid.values_10.size());
    if (ordinary + boss != 1u) return false;
    *map_index = ordinary == 1u ? grid.values_04[0] : grid.values_10[0];
    return *map_index > 0;
}

bool ValidateSpecialItemRow(const CDungeon* dungeon) {
    if (!dungeon) return false;
    const std::vector<stDungeonAssignItem_t>& items =
        dungeon->getSpecialPassiveObjectItems();
    if (items.size() != 2u) return false;
    const stDungeonAssignItem_t& ordinary_row = items[0];
    const stDungeonAssignItem_t& trap_row = items[1];
    return ordinary_row.dungeon_index == -1 &&
           ordinary_row.items.size() == 1u &&
           ordinary_row.items.front().first == 10092630 &&
           ordinary_row.items.front().second == 9000 &&
           trap_row.dungeon_index == -1 && trap_row.items.size() == 1u &&
           trap_row.items.front().first == 10092630 &&
           trap_row.items.front().second == 20000;
}

bool ValidateDungeonAndMaps(Config* config, CDataManager* data_manager) {
    if (!config || !data_manager) return false;
    CDungeon* const dungeon = data_manager->find_dungeon(kDungeonIndex);
    if (!dungeon || config->designated_difficulty != kDesignatedDifficulty ||
        !dungeon->isTimeSpiralScript() ||
        dungeon->isEventDungeon() || dungeon->isTournamentDungeon() ||
        dungeon->get_dimension_possible() != 0 ||
        dungeon->get_blood_dungeon_type() != 0 ||
        dungeon->get_limit_party_count() != 1 ||
        dungeon->getRequiredItem().item_id != 10093393 ||
        dungeon->getRequiredItem().item_count != 1 ||
        dungeon->getRequiredItem().delete_after_start != 1u ||
        !ValidateSpecialItemRow(dungeon)) {
        return false;
    }

    MazeScript* const maze = dungeon->GetMaze(0);
    if (!maze || dungeon->GetMaze(1) || maze->grid_width != kGridWidth ||
        maze->grid_height != kGridHeight ||
        maze->grids.size() !=
            static_cast<unsigned int>(kGridWidth * kGridHeight) ||
        !maze->clear_conditions.empty() ||
        maze->values_14.size() != 1u ||
        maze->values_14[0] != std::make_pair(0, 0) ||
        maze->values_20.size() != 1u) {
        return false;
    }

    std::set<int> map_indexes;
    for (int y = 0; y < kGridHeight; ++y) {
        for (int x = 0; x < kGridWidth; ++x) {
            int map_index = 0;
            const GridScript& grid = maze->getGridR(x, y);
            CMap* map = 0;
            if (!ExtractGridMapIndex(grid, &map_index) ||
                ((std::make_pair(x, y) == maze->values_20[0])
                     ? (grid.values_10.size() != 1u ||
                        !grid.values_04.empty())
                     : (grid.values_04.size() != 1u ||
                        !grid.values_10.empty())) ||
                !map_indexes.insert(map_index).second ||
                !(map = data_manager->find_map(map_index)) ||
                std::find(map->getDungeonLists().begin(),
                          map->getDungeonLists().end(),
                          kDungeonIndex) == map->getDungeonLists().end()) {
                return false;
            }
            const std::pair<int, int> coordinate(x, y);
            config->map_by_coordinate[coordinate] = map_index;
            config->coordinate_by_map[map_index] = coordinate;
        }
    }
    if (config->map_by_coordinate.find(maze->values_20[0]) ==
        config->map_by_coordinate.end()) {
        return false;
    }

    for (std::map<int, SourceRoute>::iterator source =
             config->sources.begin(); source != config->sources.end();
         ++source) {
        std::map<int, std::pair<int, int> >::const_iterator coordinate =
            config->coordinate_by_map.find(source->first);
        CMap* const map = data_manager->find_map(source->first);
        if (coordinate == config->coordinate_by_map.end() || !map ||
            !map->HasPassiveObject(kTrapPassiveObjectId) ||
            ValidatedTrapMaps().find(source->first) ==
                ValidatedTrapMaps().end()) {
            return false;
        }
        source->second.source_x = coordinate->second.first;
        source->second.source_y = coordinate->second.second;
        if (std::abs(source->second.normal_x - source->second.source_x) +
                std::abs(source->second.normal_y - source->second.source_y) !=
            1) {
            return false;
        }

        for (std::vector<Route>::const_iterator route =
                 source->second.routes.begin();
             route != source->second.routes.end(); ++route) {
            if (route->x == source->second.source_x &&
                route->y == source->second.source_y) {
                return false;
            }
            if (config->map_by_coordinate.find(
                    std::make_pair(route->x, route->y)) ==
                config->map_by_coordinate.end()) {
                return false;
            }
            if (route->result == 0) {
                config->terminals.insert(std::make_pair(route->x, route->y));
            }
        }
    }

    config->terminals.insert(maze->values_20[0]);
    return config->terminals.size() == 8u;
}

}  // namespace

RouteSelection::RouteSelection()
    : target_x(-1), target_y(-1), result(-1), weight(0) {}

bool ImportConfig(const char* filename) {
    ParsedFlag() = false;
    ReadyFlag() = false;
    PendingConfig() = Config();
    PublishedConfig() = Config();
    ValidatedTrapMaps().clear();
    if (!filename || loadRDARScriptFile("", filename) != 1) return false;
    ScriptUnloadGuard unload_guard;

    Config parsed;
    if (!ParseRoot(&parsed)) return false;
    PendingConfig() = parsed;
    ParsedFlag() = true;
    return true;
}

bool FinalizeConfig(CDataManager* data_manager) {
    ReadyFlag() = false;
    PublishedConfig() = Config();
    if (!ParsedFlag()) return false;

    Config finalized = PendingConfig();
    if (!ValidateDungeonAndMaps(&finalized, data_manager)) return false;
    PublishedConfig() = finalized;
    ReadyFlag() = true;
    return true;
}

bool IsReady() { return ReadyFlag(); }

// This is a build-closure capability flag, not a runtime activation flag.
// PacketDispatcher is constructed before CDataManager::init(); startup still
// leaves all TimeSpiral business predicates closed if ImportConfig or
// FinalizeConfig cannot publish a ready config.
bool HasNativeClosure() { return true; }

bool IsDungeon(int dungeon_index) {
    return ReadyFlag() && dungeon_index == kDungeonIndex;
}

void RegisterDesignatedDifficulty(int dungeon_index, int difficulty) {
    if (dungeon_index == kDungeonIndex) {
        PendingConfig().designated_difficulty = difficulty;
    }
}

int GetDesignatedDifficulty(int dungeon_index) {
    return IsDungeon(dungeon_index)
        ? PublishedConfig().designated_difficulty
        : -1;
}

int GetTrapPassiveObjectId() { return kTrapPassiveObjectId; }

bool IsValidatedTrapSpecialObject(
    const STAssignSpecialPassiveObj& object) {
    // ImportMapScript numbers item-producing special objects from zero.  The
    // official 0828 trap maps place item object 12940 first and trap 12577
    // second, so the trap's runtime item-action index is exactly one.
    if (object.object_id != kTrapPassiveObjectId || object.action_index != 1 ||
        object.actions.size() != 1u) {
        return false;
    }
    const SPOBJ_ACTION_DATAS& action = object.actions[0];
    if (action.action_type != 2 || action.value_04 != 1 ||
        action.value_08 != 1 || action.value_0c != -1 ||
        action.value_10 != -1 || action.value_14 != -1) {
        return false;
    }

    const char* const filename =
        g_stPassiveObjectScriptFileList.GetFileName(object.object_id);
    if (!filename || !*filename) return false;
    STPassiveObjectScript script;
    const int imported = ImportPassiveObjectScript(&script, filename, false);
    unloadRDARScriptFile();
    return imported == 1 && script.object_index == object.object_id &&
           script.state_5a == 1;
}

void RegisterValidatedTrapMap(int map_index) {
    if (map_index > 0) ValidatedTrapMaps().insert(map_index);
}

bool IsTrapSource(int map_index, int pos_x, int pos_y) {
    if (!ReadyFlag()) return false;
    const std::map<int, SourceRoute>& sources = PublishedConfig().sources;
    const std::map<int, SourceRoute>::const_iterator found =
        sources.find(map_index);
    return found != sources.end() && found->second.source_x == pos_x &&
           found->second.source_y == pos_y;
}

bool FindCoordinateForMap(int map_index, int* pos_x, int* pos_y) {
    if (!ReadyFlag() || !pos_x || !pos_y) return false;
    const std::map<int, std::pair<int, int> >::const_iterator found =
        PublishedConfig().coordinate_by_map.find(map_index);
    if (found == PublishedConfig().coordinate_by_map.end()) return false;
    *pos_x = found->second.first;
    *pos_y = found->second.second;
    return true;
}

bool GetNormalExit(int source_map_index, int* pos_x, int* pos_y) {
    if (!ReadyFlag() || !pos_x || !pos_y) return false;
    const std::map<int, SourceRoute>::const_iterator found =
        PublishedConfig().sources.find(source_map_index);
    if (found == PublishedConfig().sources.end()) return false;
    *pos_x = found->second.normal_x;
    *pos_y = found->second.normal_y;
    return true;
}

bool SelectRouteByRoll(int source_map_index,
                       int roll,
                       RouteSelection* selection) {
    if (!ReadyFlag() || !selection || roll < 0 ||
        roll >= kRouteWeightTotal) {
        return false;
    }
    const std::map<int, SourceRoute>::const_iterator found =
        PublishedConfig().sources.find(source_map_index);
    if (found == PublishedConfig().sources.end()) return false;

    int remaining = roll;
    for (std::vector<Route>::const_iterator route =
             found->second.routes.begin();
         route != found->second.routes.end(); ++route) {
        if (remaining < route->weight) {
            selection->target_x = route->x;
            selection->target_y = route->y;
            selection->result = route->result;
            selection->weight = route->weight;
            return true;
        }
        remaining -= route->weight;
    }
    return false;
}

bool IsTerminalCoordinate(int dungeon_index, int pos_x, int pos_y) {
    return IsDungeon(dungeon_index) &&
           PublishedConfig().terminals.find(std::make_pair(pos_x, pos_y)) !=
               PublishedConfig().terminals.end();
}

}  // namespace time_spiral
