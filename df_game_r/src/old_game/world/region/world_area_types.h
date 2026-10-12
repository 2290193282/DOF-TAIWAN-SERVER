#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_AREA_TYPES_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_AREA_TYPES_H_

#include <stdint.h>

#include <vector>


// IDA unit: 05_world_enter_village/AppearMap.
struct AppearMap {
    AppearMap();

    AppearMap* clear();

    int map_id;
    int x;
    int y;
    int area_id;
};

// IDA unit: 05_world_enter_village/MapArea.
struct MapArea {
    MapArea();
    MapArea(const MapArea& rhs);
    ~MapArea();

    bool include(int x_value, int y_value);
    bool include(int x_value, int y_value, int& clamped_x, int& clamped_y,
                 int margin);
    int clear();
    MapArea& operator=(const MapArea& rhs);

    int x;
    int y;
    int width;
    int height;
    std::vector<int> map_indices;
};

// IDA unit: 05_world_enter_village/TownPath.
struct TownPath : public MapArea {
    TownPath();
    TownPath(const TownPath& rhs);
    ~TownPath();

    int clear();
    TownPath& operator=(const TownPath& rhs);

    int start_area;
    int end_area;
    std::vector<int> path_nodes;
};

// IDA unit: 05_world_enter_village/stMapMonsterKillChecker_t.
struct stMapMonsterKillChecker_t {
    stMapMonsterKillChecker_t* init();
    int getUseSkillMaterialCount();
    stMapMonsterKillChecker_t* initMonsterInfo();
    stMapMonsterKillChecker_t* setUseSkillMaterial(uint32_t tick);
    int dieMonster(uint32_t tick, uint32_t interval);
    int getUseSkillMaterialTime();
    stMapMonsterKillChecker_t* setLastMobDieTime(uint32_t tick);
    int checkLongTermMobDie(uint32_t tick, uint32_t interval);
    stMapMonsterKillChecker_t* setUncontinuallyMobDieCnt(uint32_t count);

    uint32_t use_skill_material_time;
    uint32_t last_die_bucket_time;
    uint32_t continuous_die_count;
    uint32_t uncontinually_mob_die_count;
    uint32_t last_mob_die_time;
    uint32_t use_skill_material_count;
};

// IDA unit: 05_world_enter_village/stMapPlayInfo_t.
struct stMapPlayInfo_t {
    stMapPlayInfo_t* init();
    int getPlayTick();

    uint32_t start_tick;
    uint32_t end_tick;
    uint32_t field_08;
    uint32_t field_0c;
};


#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_AREA_TYPES_H_
