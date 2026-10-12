#include "old_game/world/region/world_area_types.h"


typedef char AppearMap_Size_Must_Be_0x10[(sizeof(AppearMap) == 0x10) ? 1 : -1];
typedef char MapArea_Size_Must_Be_0x1c[(sizeof(MapArea) == 0x1c) ? 1 : -1];
typedef char TownPath_Size_Must_Be_0x30[(sizeof(TownPath) == 0x30) ? 1 : -1];
typedef char stMapMonsterKillChecker_t_Size_Must_Be_0x18[
    (sizeof(stMapMonsterKillChecker_t) == 0x18) ? 1 : -1];
typedef char stMapPlayInfo_t_Size_Must_Be_0x10[
    (sizeof(stMapPlayInfo_t) == 0x10) ? 1 : -1];

// IDA_EA: 0x836DE16
// IDA_NAME: _ZN9AppearMap5clearEv
// DEMANGLED: AppearMap::clear(void)
AppearMap* AppearMap::clear() {
    map_id = -1;
    x = -1;
    y = -1;
    area_id = -1;
    return this;
}

// IDA_EA: 0x836DE42
// IDA_NAME: _ZN9AppearMapC2Ev
// DEMANGLED: AppearMap::AppearMap(void)
AppearMap::AppearMap() {
    clear();
}

// IDA_EA: 0x82F0298
// IDA_NAME: _ZN7MapArea7includeEii
// DEMANGLED: MapArea::include(int,int)
bool MapArea::include(int x_value, int y_value) {
    return x <= x_value &&
           x + width >= x_value &&
           y <= y_value &&
           y + height >= y_value;
}

// IDA_EA: 0x82F02E6
// IDA_NAME: _ZN7MapArea7includeEiiRiS0_i
// DEMANGLED: MapArea::include(int,int,int &,int &,int)
bool MapArea::include(int x_value, int y_value, int& clamped_x, int& clamped_y,
                      int margin) {
    if (margin + x <= x_value) {
        if (x + width - margin >= x_value) {
            clamped_x = x_value;
        } else {
            clamped_x = x + width - margin;
        }
    } else {
        clamped_x = margin + x;
    }

    if (margin + y <= y_value) {
        if (y + height - margin >= y_value) {
            clamped_y = y_value;
        } else {
            clamped_y = y + height - margin;
        }
    } else {
        clamped_y = margin + y;
    }

    return clamped_x == x_value && clamped_y == y_value;
}

// IDA_EA: 0x8366050
// IDA_NAME: _ZN7MapArea5clearEv
// DEMANGLED: MapArea::clear(void)
int MapArea::clear() {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    map_indices.clear();
    return 0;
}

// IDA_EA: 0x836608E
// IDA_NAME: _ZN7MapAreaC2Ev
// DEMANGLED: MapArea::MapArea(void)
MapArea::MapArea() {
    clear();
}

// IDA_EA: 0x83660D6
// IDA_NAME: _ZN7MapAreaD2Ev
// DEMANGLED: MapArea::~MapArea()
MapArea::~MapArea() {}

// IDA_EA: 0x837E5CA
// IDA_NAME: _ZN7MapAreaC2ERKS_
// DEMANGLED: MapArea::MapArea(MapArea const&)
MapArea::MapArea(const MapArea& rhs)
    : x(rhs.x),
      y(rhs.y),
      width(rhs.width),
      height(rhs.height),
      map_indices(rhs.map_indices) {}

// IDA_EA: 0x86D886A
// IDA_NAME: _ZN7MapAreaaSERKS_
// DEMANGLED: MapArea::operator=(MapArea const&)
MapArea& MapArea::operator=(const MapArea& rhs) {
    x = rhs.x;
    y = rhs.y;
    width = rhs.width;
    height = rhs.height;
    map_indices = rhs.map_indices;
    return *this;
}

// IDA_EA: 0x83D9B78
// IDA_NAME: _ZN8TownPathC2ERKS_
// DEMANGLED: TownPath::TownPath(TownPath const&)
TownPath::TownPath(const TownPath& rhs)
    : MapArea(rhs),
      start_area(rhs.start_area),
      end_area(rhs.end_area),
      path_nodes(rhs.path_nodes) {}

// IDA_EA: 0x83DC924
// IDA_NAME: _ZN8TownPathD2Ev
// DEMANGLED: TownPath::~TownPath()
TownPath::~TownPath() {}

// IDA_EA: 0x89E8258
// IDA_NAME: _ZN8TownPath5clearEv
// DEMANGLED: TownPath::clear(void)
int TownPath::clear() {
    MapArea::clear();
    start_area = 0;
    end_area = 0;
    path_nodes.clear();
    return 0;
}

// IDA_EA: 0x89E828E
// IDA_NAME: _ZN8TownPathC2Ev
// DEMANGLED: TownPath::TownPath(void)
TownPath::TownPath() {
    clear();
}

// IDA_EA: 0x89EA970
// IDA_NAME: _ZN8TownPathaSERKS_
// DEMANGLED: TownPath::operator=(TownPath const&)
TownPath& TownPath::operator=(const TownPath& rhs) {
    MapArea::operator=(rhs);
    start_area = rhs.start_area;
    end_area = rhs.end_area;
    path_nodes = rhs.path_nodes;
    return *this;
}

// IDA_EA: 0x81512EC
// IDA_NAME: _ZN25stMapMonsterKillChecker_t4initEv
// DEMANGLED: stMapMonsterKillChecker_t::init(void)
stMapMonsterKillChecker_t* stMapMonsterKillChecker_t::init() {
    use_skill_material_time = 0;
    last_die_bucket_time = 0;
    continuous_die_count = 0;
    uncontinually_mob_die_count = 0;
    last_mob_die_time = 0;
    use_skill_material_count = 0;
    return this;
}

// IDA_EA: 0x822CB86
// IDA_NAME: _ZN25stMapMonsterKillChecker_t24getUseSkillMaterialCountEv
// DEMANGLED: stMapMonsterKillChecker_t::getUseSkillMaterialCount(void)
int stMapMonsterKillChecker_t::getUseSkillMaterialCount() {
    return use_skill_material_count;
}

// IDA_EA: 0x830E71C
// IDA_NAME: _ZN25stMapMonsterKillChecker_t15initMonsterInfoEv
// DEMANGLED: stMapMonsterKillChecker_t::initMonsterInfo(void)
stMapMonsterKillChecker_t* stMapMonsterKillChecker_t::initMonsterInfo() {
    continuous_die_count = 0;
    return this;
}

// IDA_EA: 0x830E72C
// IDA_NAME: _ZN25stMapMonsterKillChecker_t19setUseSkillMaterialEj
// DEMANGLED: stMapMonsterKillChecker_t::setUseSkillMaterial(unsigned int)
stMapMonsterKillChecker_t*
stMapMonsterKillChecker_t::setUseSkillMaterial(uint32_t tick) {
    use_skill_material_time = tick;
    ++use_skill_material_count;
    return this;
}

// IDA_EA: 0x830E748
// IDA_NAME: _ZN25stMapMonsterKillChecker_t10dieMonsterEjj
// DEMANGLED: stMapMonsterKillChecker_t::dieMonster(unsigned int,unsigned int)
int stMapMonsterKillChecker_t::dieMonster(uint32_t tick, uint32_t interval) {
    if (tick - last_die_bucket_time > interval) {
        continuous_die_count = 0;
        last_die_bucket_time = tick;
    } else {
        ++continuous_die_count;
    }
    return continuous_die_count;
}

// IDA_EA: 0x830E78C
// IDA_NAME: _ZN25stMapMonsterKillChecker_t23getUseSkillMaterialTimeEv
// DEMANGLED: stMapMonsterKillChecker_t::getUseSkillMaterialTime(void)
int stMapMonsterKillChecker_t::getUseSkillMaterialTime() {
    return use_skill_material_time;
}

// IDA_EA: 0x830E796
// IDA_NAME: _ZN25stMapMonsterKillChecker_t17setLastMobDieTimeEj
// DEMANGLED: stMapMonsterKillChecker_t::setLastMobDieTime(unsigned int)
stMapMonsterKillChecker_t*
stMapMonsterKillChecker_t::setLastMobDieTime(uint32_t tick) {
    last_mob_die_time = tick;
    return this;
}

// IDA_EA: 0x830E7A4
// IDA_NAME: _ZN25stMapMonsterKillChecker_t19checkLongTermMobDieEjj
// DEMANGLED: stMapMonsterKillChecker_t::checkLongTermMobDie(unsigned int,unsigned int)
int stMapMonsterKillChecker_t::checkLongTermMobDie(uint32_t tick,
                                                   uint32_t interval) {
    if (last_mob_die_time && tick - last_mob_die_time >= interval) {
        ++uncontinually_mob_die_count;
    }
    return uncontinually_mob_die_count;
}

// IDA_EA: 0x830E7DC
// IDA_NAME: _ZN25stMapMonsterKillChecker_t25setUncontinuallyMobDieCntEj
// DEMANGLED: stMapMonsterKillChecker_t::setUncontinuallyMobDieCnt(unsigned int)
stMapMonsterKillChecker_t*
stMapMonsterKillChecker_t::setUncontinuallyMobDieCnt(uint32_t count) {
    uncontinually_mob_die_count = count;
    return this;
}

// IDA_EA: 0x815132C
// IDA_NAME: _ZN15stMapPlayInfo_t4initEv
// DEMANGLED: stMapPlayInfo_t::init(void)
stMapPlayInfo_t* stMapPlayInfo_t::init() {
    start_tick = 0;
    end_tick = 0;
    field_08 = 0;
    field_0c = 0;
    return this;
}

// IDA_EA: 0x830E7EA
// IDA_NAME: _ZN15stMapPlayInfo_t11getPlayTickEv
// DEMANGLED: stMapPlayInfo_t::getPlayTick(void)
int stMapPlayInfo_t::getPlayTick() {
    return end_tick - start_tick;
}
