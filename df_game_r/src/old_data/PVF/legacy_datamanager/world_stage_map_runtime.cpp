#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"

#include "old_game/world/region/world_stage_map_scripts.h"
#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/map/datamanager/stage_map_script.h"
#include "old_data/PVF/map/datamanager/world_map_script.h"
#include "common/manager/global_lifecycle.h"

#include <algorithm>
#include <limits.h>
#include <stdio.h>
#include <string.h>

extern STScriptFileList g_worldMapScriptFileList;
extern STScriptFileList g_stageMapScriptFileList;
namespace {

CDungeonList* GetDungeonListFromDataManager() {
    CDataManager* data_manager = G_CDataManager();
    return data_manager ? data_manager->getDungeonIndexList() : 0;
}

void PushStageSummary(std::vector<StageMapList::_StageMapSummary>* summaries,
                      int area_index,
                      int district_index,
                      int stage_ordinal,
                      int stage_index,
                      bool is_survival_stage) {
    StageMapList::_StageMapSummary summary;
    // Official 0627 loadList passes the loop ordinal here.  The stage id is
    // the map key below, while this value only defines progression order.
    summary.unique_id =
        StageMapList::makeStageMapUniqueId(area_index,
                                           district_index,
                                           stage_ordinal);
    summary.stage_index = stage_index;
    summary.next_stage_index = 0;
    summary.is_survival_stage = is_survival_stage ? 1 : 0;
    summaries->push_back(summary);
}

}  // namespace

CWorldMapList* CDataManagerWorldMapList(CDataManager* data_manager) {
    return data_manager ? data_manager->get_world_map_list() : 0;
}

const CWorldMapList* CDataManagerWorldMapList(
    const CDataManager* data_manager) {
    return CDataManagerWorldMapList(const_cast<CDataManager*>(data_manager));
}

CWorldMapList* CDataManager::get_world_map_list() {
    return &world_map_list_;
}

const CWorldMapList* CDataManager::get_world_map_list() const {
    return &world_map_list_;
}

// IDA_EA: 0x830E6BA
// IDA_NAME: _ZN9CWorldMap15IsInHellDungeonEv
// DEMANGLED: CWorldMap::IsInHellDungeon(void)
int CWorldMap::IsInHellDungeon() {
    return in_hell_dungeon_;
}

// IDA_EA: 0x836421A
// IDA_NAME: _ZN9CWorldMap13set_world_mapEP14WorldMapScript
// DEMANGLED: CWorldMap::set_world_map(WorldMapScript *)
int CWorldMap::set_world_map(WorldMapScript* script) {
    if (!script) {
        return 0;
    }

    map_index_ = script->raw_00;
    in_hell_dungeon_ = script->raw_20;
    hell_dungeons_ = script->hell_dungeons;
    hell_quests_ = script->hell_quests;
    freepass_items_ = script->freepass_items;
    world_map_0828::RegisterRuntime(map_index_, script);
    min_dungeon_level_ = INT_MAX;
    has_death_tower_ = 0;
    dungeons_.clear();

    CDungeonList* dungeon_list = GetDungeonListFromDataManager();
    for (std::vector<WorldMapDungeon>::const_iterator it =
             script->dungeons.begin();
         it != script->dungeons.end();
         ++it) {
        CDungeon* dungeon =
            dungeon_list ? dungeon_list->find_dungeon(it->dungeon_index) : 0;
        if (!dungeon) {
            continue;
        }

        dungeons_.push_back(it->dungeon_index);
        const int dungeon_kind = dungeon->getDungeonKind();
        if (dungeon_kind == 1) {
            min_dungeon_level_ =
                std::min(min_dungeon_level_, dungeon->get_min_level());
            has_death_tower_ = 1;
        } else if (dungeon_kind == 2) {
            min_dungeon_level_ =
                std::min(min_dungeon_level_, dungeon->get_min_level());
        } else if (dungeon->isRiskDungeon()) {
            // RECONSTRUCTED_0828: no 140828 server body is available for
            // this function.  The official 0828 PVF delta adds only the
            // seven TimeGate Requiem risk dungeons to world maps; their
            // [basis level] is the monster/stat standard (100), while
            // [minimum required level] is the admission contract (81).
            // Preserve the recovered CMD15 gate and project that authoritative
            // admission value through the existing world-map cache.
            min_dungeon_level_ =
                std::min(min_dungeon_level_, dungeon->get_min_level());
        } else if (!dungeon->isRevengeDungeon()) {
            if (dungeon->get_index() == 100) {
                min_dungeon_level_ =
                    std::min(min_dungeon_level_, dungeon->get_min_level());
            }
            min_dungeon_level_ =
                std::min(min_dungeon_level_, dungeon->get_standard_level());
        }
    }

    return 0;
}

// IDA_EA: 0x83746AC
// IDA_NAME: _ZN9CWorldMap9get_indexEv
// DEMANGLED: CWorldMap::get_index(void)
int CWorldMap::get_index() {
    return map_index_;
}

// IDA_EA: 0x822B514
// IDA_NAME: _ZNK9CWorldMap18getDungeonMinLevelEv
// DEMANGLED: CWorldMap::getDungeonMinLevel(void) const
int CWorldMap::getDungeonMinLevel() const {
    return min_dungeon_level_;
}

// IDA_EA: 0x822B520
// IDA_NAME: _ZNK9CWorldMap13hasDeathTowerEv
// DEMANGLED: CWorldMap::hasDeathTower(void) const
int CWorldMap::hasDeathTower() const {
    return has_death_tower_;
}

// IDA_EA: 0x822B52C
// IDA_NAME: _ZNK9CWorldMap10hasDungeonEj
// DEMANGLED: CWorldMap::hasDungeon(unsigned int) const
bool CWorldMap::hasDungeon(unsigned int dungeon_index) const {
    return std::find(dungeons_.begin(),
                     dungeons_.end(),
                     static_cast<int>(dungeon_index)) != dungeons_.end();
}

bool CWorldMap::get0828Extension(
    world_map_0828::Extension* output) const {
    return world_map_0828::FindRuntime(map_index_, output);
}

// IDA_EA: 0x83746B6
// IDA_NAME: _ZN9CWorldMapD2Ev
// DEMANGLED: CWorldMap::~CWorldMap()
CWorldMap::~CWorldMap() {}

// IDA_EA: 0x837F6EA
// IDA_NAME: _ZN9CWorldMapC2Ev
// DEMANGLED: CWorldMap::CWorldMap(void)
CWorldMap::CWorldMap()
    : map_index_(0),
      in_hell_dungeon_(0),
      reserved_05_(),
      min_dungeon_level_(0),
      has_death_tower_(0),
      reserved_0d_(),
      dungeons_(),
      hell_dungeons_(),
      hell_quests_(),
      freepass_items_() {}

// IDA_EA: 0x85BEDA4
// IDA_NAME: _ZN9CWorldMap14GetDungeonListEv
// DEMANGLED: CWorldMap::GetDungeonList(void)
std::vector<int>* CWorldMap::GetDungeonList() {
    return &dungeons_;
}

// IDA_EA: 0x8364506
// IDA_NAME: _ZN13CWorldMapList9load_listEv
// DEMANGLED: CWorldMapList::load_list(void)
int CWorldMapList::load_list() {
    WorldMapScript script;
    for (STScriptFileList::const_iterator it =
             g_worldMapScriptFileList.begin();
         it != g_worldMapScriptFileList.end();
         ++it) {
        const char* filename = it->second.c_str();
        if (!filename) {
            break;
        }
        if (importWorldMapScript(&script, filename) != 1) {
            printf("World Map Script Error: %s\n", filename);
            return 0;
        }

        CWorldMap* world_map = new CWorldMap();
        world_map->set_world_map(&script);
        insert_world_map(world_map);
    }
    return 1;
}

// IDA_EA: 0x8364694
// IDA_NAME: _ZN13CWorldMapList16insert_world_mapEP9CWorldMap
// DEMANGLED: CWorldMapList::insert_world_map(CWorldMap *)
int CWorldMapList::insert_world_map(CWorldMap* world_map) {
    if (!world_map) {
        return 0;
    }

    const int index = world_map->get_index();
    if (world_maps_.find(index) == world_maps_.end()) {
        world_maps_.insert(std::make_pair(index, world_map));
        return 1;
    }

    printf("Already registed world map index detected : %d\n", index);
    return 0;
}

// IDA_EA: 0x83647A2
// IDA_NAME: _ZNK13CWorldMapList14find_world_mapEi
// DEMANGLED: CWorldMapList::find_world_map(int)const
CWorldMap* CWorldMapList::find_world_map(int map_index) const {
    std::map<int, CWorldMap*>::const_iterator it =
        world_maps_.find(map_index);
    if (it == world_maps_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x8374752
// IDA_NAME: _ZN13CWorldMapList7destroyEv
// DEMANGLED: CWorldMapList::destroy(void)
int CWorldMapList::destroy() {
    for (std::map<int, CWorldMap*>::iterator it = world_maps_.begin();
         it != world_maps_.end();
         ++it) {
        delete it->second;
    }
    world_maps_.clear();
    world_map_0828::ClearRuntime();
    return 0;
}

// IDA_EA: 0x837480A
// IDA_NAME: _ZN13CWorldMapListC2Ev
// DEMANGLED: CWorldMapList::CWorldMapList(void)
CWorldMapList::CWorldMapList()
    : world_maps_() {}

// IDA_EA: 0x837481E
// IDA_NAME: _ZN13CWorldMapListD2Ev
// DEMANGLED: CWorldMapList::~CWorldMapList()
CWorldMapList::~CWorldMapList() {
    destroy();
}

// IDA_EA: 0x836485C
// IDA_NAME: _ZN12StageMapList8loadListEv
// DEMANGLED: StageMapList::loadList(void)
int StageMapList::loadList() {
    std::vector<_StageMapSummary> summaries;

    for (STScriptFileList::const_iterator file_it =
             g_stageMapScriptFileList.begin();
         file_it != g_stageMapScriptFileList.end();
         ++file_it) {
        const char* filename = file_it->second.c_str();
        stagemap::StageMapScript script;
        if (importStageMapScript(&script, filename) != 1) {
            printf("Stage Map Script Error: %s\n", filename);
            return 0;
        }

        for (std::vector<stagemap::AreaData>::const_iterator area_it =
                 script.areas.begin();
             area_it != script.areas.end();
             ++area_it) {
            // The native loader carries the last district outside the inner
            // loop so that survival sorts after every normal district.
            int last_district_index = 0;
            for (std::vector<stagemap::DistrictData>::const_iterator
                     district_it = area_it->districts.begin();
                 district_it != area_it->districts.end();
                 ++district_it) {
                last_district_index = district_it->district_index;
                // The ordinal restarts for each district in the official
                // loop; dungeon ids must not participate in sort identity.
                int stage_ordinal = 0;
                for (std::vector<stagemap::StageData>::const_iterator
                         stage_it = district_it->stages.begin();
                     stage_it != district_it->stages.end();
                     ++stage_it) {
                    PushStageSummary(&summaries,
                                     area_it->area_index,
                                     district_it->district_index,
                                     stage_ordinal,
                                     stage_it->stage_index,
                                     false);
                    ++stage_ordinal;
                }
            }

            if (area_it->survival_stage_index > 0) {
                PushStageSummary(&summaries,
                                 area_it->area_index,
                                 last_district_index + 1,
                                 0,
                                 area_it->survival_stage_index,
                                 true);
            }
        }
    }

    std::sort(summaries.begin(), summaries.end(), StageMapList::compare);
    if (!summaries.empty()) {
        first_stage_index = summaries[0].stage_index;
    }

    for (std::vector<_StageMapSummary>::size_type index = 0;
         index < summaries.size();
         ++index) {
        if (index + 1 < summaries.size()) {
            summaries[index].next_stage_index = summaries[index + 1].stage_index;
        }
        stage_map.insert(
            std::make_pair(summaries[index].stage_index, summaries[index]));
    }
    return 1;
}
