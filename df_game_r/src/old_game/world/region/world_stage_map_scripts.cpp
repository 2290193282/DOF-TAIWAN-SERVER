#include "old_game/world/region/world_stage_map_scripts.h"

namespace stagemap {

typedef char StageData_Size_Must_Be_0x10[
    (sizeof(StageData) == 0x10) ? 1 : -1];
typedef char DistrictData_Size_Must_Be_0x18[
    (sizeof(DistrictData) == 0x18) ? 1 : -1];
typedef char AreaData_Size_Must_Be_0x30[
    (sizeof(AreaData) == 0x30) ? 1 : -1];
typedef char StageMapScript_Size_Must_Be_0x20[
    (sizeof(StageMapScript) == 0x20) ? 1 : -1];

// IDA_EA: 0x88AA5D8
// IDA_NAME: _ZN8stagemap9StageData5clearEv
// DEMANGLED: stagemap::StageData::clear(void)
StageData* StageData::clear() {
    stage_index = 0;
    raw_04 = 0;
    raw_08 = 0;
    raw_0c = 0;
    return this;
}

// IDA_EA: 0x88AA604
// IDA_NAME: _ZN8stagemap9StageDataC2Ev
// DEMANGLED: stagemap::StageData::StageData(void)
StageData::StageData() {
    clear();
}

// IDA_EA: 0x88AC168
// IDA_NAME: _ZN8stagemap9StageDataD2Ev
// DEMANGLED: stagemap::StageData::~StageData()
StageData::~StageData() {}

// IDA_EA: 0x88AA618
// IDA_NAME: _ZN8stagemap12DistrictData5clearEv
// DEMANGLED: stagemap::DistrictData::clear(void)
int DistrictData::clear() {
    district_index = 0;
    slot_image = "";
    raw_14 = 0;
    stages.clear();
    return 0;
}

// IDA_EA: 0x88AA658
// IDA_NAME: _ZN8stagemap12DistrictDataC2Ev
// DEMANGLED: stagemap::DistrictData::DistrictData(void)
DistrictData::DistrictData()
    : stages(),
      slot_image() {
    clear();
}

// IDA_EA: 0x88AB100
// IDA_NAME: _ZN8stagemap12DistrictDataC2ERKS0_
// DEMANGLED: stagemap::DistrictData::DistrictData(stagemap::DistrictData const&)
DistrictData::DistrictData(const DistrictData& rhs)
    : district_index(rhs.district_index),
      stages(rhs.stages),
      slot_image(rhs.slot_image),
      raw_14(rhs.raw_14) {}

// IDA_EA: 0x88AB1D2
// IDA_NAME: _ZN8stagemap12DistrictDataaSERKS0_
// DEMANGLED: stagemap::DistrictData::operator=(stagemap::DistrictData const&)
DistrictData& DistrictData::operator=(const DistrictData& rhs) {
    district_index = rhs.district_index;
    stages = rhs.stages;
    slot_image = rhs.slot_image;
    raw_14 = rhs.raw_14;
    return *this;
}

// IDA_EA: 0x83E803C
// IDA_NAME: _ZN8stagemap12DistrictDataD2Ev
// DEMANGLED: stagemap::DistrictData::~DistrictData()
DistrictData::~DistrictData() {}

// IDA_EA: 0x88AA710
// IDA_NAME: _ZN8stagemap8AreaData5clearEv
// DEMANGLED: stagemap::AreaData::clear(void)
AreaData* AreaData::clear() {
    area_index = 0;
    slot_types.clear();
    districts.clear();
    area_name = "";
    survival_stage_index = 0;
    return this;
}

// IDA_EA: 0x88AA75E
// IDA_NAME: _ZN8stagemap8AreaDataC2Ev
// DEMANGLED: stagemap::AreaData::AreaData(void)
AreaData::AreaData()
    : area_name(),
      slot_types(),
      districts() {}

// IDA_EA: 0x88AB532
// IDA_NAME: _ZN8stagemap8AreaDataC2ERKS0_
// DEMANGLED: stagemap::AreaData::AreaData(stagemap::AreaData const&)
AreaData::AreaData(const AreaData& rhs)
    : area_index(rhs.area_index),
      area_name(rhs.area_name),
      slot_types(rhs.slot_types),
      districts(rhs.districts),
      survival_stage_index(rhs.survival_stage_index) {}

// IDA_EA: 0x88AB634
// IDA_NAME: _ZN8stagemap8AreaDataaSERKS0_
// DEMANGLED: stagemap::AreaData::operator=(stagemap::AreaData const&)
AreaData& AreaData::operator=(const AreaData& rhs) {
    area_index = rhs.area_index;
    area_name = rhs.area_name;
    slot_types = rhs.slot_types;
    districts = rhs.districts;
    survival_stage_index = rhs.survival_stage_index;
    return *this;
}

// IDA_EA: 0x83DD7BC
// IDA_NAME: _ZN8stagemap8AreaDataD2Ev
// DEMANGLED: stagemap::AreaData::~AreaData()
AreaData::~AreaData() {}

// IDA_EA: 0x8373428
// IDA_NAME: _ZN8stagemap14StageMapScript5clearEv
// DEMANGLED: stagemap::StageMapScript::clear(void)
int StageMapScript::clear() {
    script_index = 0;
    dungeon_index = 0;
    name = "";
    map_image = "";
    map_image_index = 0;
    areas.clear();
    return 0;
}

// IDA_EA: 0x8373488
// IDA_NAME: _ZN8stagemap14StageMapScriptC2Ev
// DEMANGLED: stagemap::StageMapScript::StageMapScript(void)
StageMapScript::StageMapScript()
    : name(),
      map_image(),
      areas() {
    clear();
}

// IDA_EA: 0x837F782
// IDA_NAME: _ZN8stagemap14StageMapScriptD2Ev
// DEMANGLED: stagemap::StageMapScript::~StageMapScript()
StageMapScript::~StageMapScript() {}

}  // namespace stagemap
