#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_STAGE_MAP_SCRIPTS_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_STAGE_MAP_SCRIPTS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x88AA5D8 in the companion source.


#include <map>
#include <string>
#include <vector>

#include "old_game/world/region/stage_map_data.h"

namespace stagemap {

// IDA unit: 05_world_enter_village/stagemap/StageData.
struct StageData {
    StageData();
    ~StageData();

    StageData* clear();

    int stage_index;
    int raw_04;
    int raw_08;
    int raw_0c;
};

// IDA unit: 05_world_enter_village/stagemap/DistrictData.
struct DistrictData {
    DistrictData();
    DistrictData(const DistrictData& rhs);
    ~DistrictData();

    int clear();
    DistrictData& operator=(const DistrictData& rhs);

    int district_index;
    std::vector<StageData> stages;
    std::string slot_image;
    int raw_14;
};

// IDA unit: 05_world_enter_village/stagemap/AreaData.
struct AreaData {
    AreaData();
    AreaData(const AreaData& rhs);
    ~AreaData();

    AreaData* clear();
    AreaData& operator=(const AreaData& rhs);

    int area_index;
    std::string area_name;
    std::map<int, SlotType> slot_types;
    std::vector<DistrictData> districts;
    int survival_stage_index;
};

// IDA unit: 05_world_enter_village/stagemap/StageMapScript.
struct StageMapScript {
    StageMapScript();
    ~StageMapScript();

    int clear();

    int script_index;
    int dungeon_index;
    int map_image_index;
    std::string name;
    std::string map_image;
    std::vector<AreaData> areas;
};

}  // namespace stagemap

#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_STAGE_MAP_SCRIPTS_H_
