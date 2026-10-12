#include "old_game/world/region/world_value_types.h"

#include <cstring>

#include "infra/log/format/trace.h"


namespace {

// The event manager owns exactly one event-22 handler.  Keeping this
// operator mode outside the recovered object preserves its 0x0c ABI and
// vtable.  Activity configuration is startup-only, matching the event flag's
// existing lifetime.
bool g_pc_room_world_drop_all_accounts = false;

}  // namespace


namespace pc_room_world_drop_event {

bool AllowsAllAccounts() {
    return g_pc_room_world_drop_all_accounts;
}

}  // namespace pc_room_world_drop_event

namespace world_map_0828 {
namespace {

typedef std::map<const WorldMapScript*, Extension> ScriptExtensionMap;
typedef std::map<int, Extension> RuntimeExtensionMap;

ScriptExtensionMap& ScriptExtensions() {
    static ScriptExtensionMap extensions;
    return extensions;
}

RuntimeExtensionMap& RuntimeExtensions() {
    static RuntimeExtensionMap extensions;
    return extensions;
}

void Erase(const WorldMapScript* script) {
    ScriptExtensions().erase(script);
}

}  // namespace

Extension::Extension()
    : has_festival_condition(false),
      festival_condition(0),
      has_exercise(false),
      exercise(0) {}

void SetFestivalCondition(const WorldMapScript* script, int32_t value) {
    Extension& extension = ScriptExtensions()[script];
    extension.has_festival_condition = true;
    extension.festival_condition = value;
}

void SetExercise(const WorldMapScript* script, int32_t value) {
    Extension& extension = ScriptExtensions()[script];
    extension.has_exercise = true;
    extension.exercise = value;
}

bool Find(const WorldMapScript* script, Extension* output) {
    if (!script || !output) {
        return false;
    }
    const ScriptExtensionMap::const_iterator it =
        ScriptExtensions().find(script);
    if (it == ScriptExtensions().end()) {
        return false;
    }
    *output = it->second;
    return true;
}

void RegisterRuntime(int map_index, const WorldMapScript* script) {
    Extension extension;
    if (Find(script, &extension)) {
        RuntimeExtensions()[map_index] = extension;
    } else {
        RuntimeExtensions().erase(map_index);
    }
}

bool FindRuntime(int map_index, Extension* output) {
    if (!output) {
        return false;
    }
    const RuntimeExtensionMap::const_iterator it =
        RuntimeExtensions().find(map_index);
    if (it == RuntimeExtensions().end()) {
        return false;
    }
    *output = it->second;
    return true;
}

void ClearRuntime() {
    RuntimeExtensions().clear();
}

}  // namespace world_map_0828


typedef char CPCRoomWorldDropEvent_Size_Must_Be_0x0c[
    (sizeof(CPCRoomWorldDropEvent) == 0x0c) ? 1 : -1];
typedef char CPCRoomWorldDropEvent2nd_Size_Must_Be_0x0c[
    (sizeof(CPCRoomWorldDropEvent2nd) == 0x0c) ? 1 : -1];
typedef char PvPMapInfo_Size_Must_Be_0x08[
    (sizeof(PvPMapInfo) == 0x08) ? 1 : -1];
typedef char stAttackedMapInfo_t_Size_Must_Be_0x10[
    (sizeof(stAttackedMapInfo_t) == 0x10) ? 1 : -1];
typedef char stVillageAttacked_t_Size_Must_Be_0x0c[
    (sizeof(stVillageAttacked_t) == 0x0c) ? 1 : -1];
typedef char mapItem_Size_Must_Be_0x10[
    (sizeof(_mapItem) == 0x10) ? 1 : -1];
typedef char map_item_Size_Must_Be_0x54[
    (sizeof(map_item) == 0x54) ? 1 : -1];
typedef char WorldDropItemChunck_Size_Must_Be_0x08[
    (sizeof(WorldDropItemChunck) == 0x08) ? 1 : -1];
typedef char WorldDropInfo_Size_Must_Be_0x14[
    (sizeof(WorldDropInfo) == 0x14) ? 1 : -1];
typedef char STWorldmapPattern2ND_Size_Must_Be_0x1c[
    (sizeof(STWorldmapPattern2ND) == 0x1c) ? 1 : -1];
typedef char STWorldmapPattern_Size_Must_Be_0x28[
    (sizeof(STWorldmapPattern) == 0x28) ? 1 : -1];
typedef char STNamedMonsterMapPos_Size_Must_Be_0x08[
    (sizeof(STNamedMonsterMapPos) == 0x08) ? 1 : -1];
typedef char stClearTimeAverageMapKey_Size_Must_Be_0x08[
    (sizeof(stClearTimeAverageMapKey) == 0x08) ? 1 : -1];
typedef char WorldMapDungeon_Size_Must_Be_0x0c[
    (sizeof(WorldMapDungeon) == 0x0c) ? 1 : -1];
typedef char WorldMapScript_Size_Must_Be_0x54[
    (sizeof(WorldMapScript) == 0x54) ? 1 : -1];
typedef char mapMonster_Size_Must_Be_0x40[
    (sizeof(_mapMonster) == 0x40) ? 1 : -1];
typedef char RandomApplicationKey_Size_Must_Be_0x02[
    (sizeof(RandomApplicationKey) == 0x02) ? 1 : -1];
typedef char STRivalMapData_Size_Must_Be_0x0c[
    (sizeof(STRivalMapData) == 0x0c) ? 1 : -1];
typedef char PVPMAP_DATA_Size_Must_Be_0x08[
    (sizeof(PVPMAP_DATA) == 0x08) ? 1 : -1];
typedef char VillageObjectZone_Size_Must_Be_0x02[
    (sizeof(village_object::Zone) == 0x02) ? 1 : -1];
typedef char VillageAttackedZone_Size_Must_Be_0x02[
    (sizeof(village_attacked::Zone) == 0x02) ? 1 : -1];
typedef char VillageAttackedCRevengeDungeon_Size_Must_Be_0x08[
    (sizeof(village_attacked::CRevengeDungeon) == 0x08) ? 1 : -1];
typedef char StageMapSummary_Size_Must_Be_0x10[
    (sizeof(StageMapList::_StageMapSummary) == 0x10) ? 1 : -1];
typedef char StageMapList_Size_Must_Be_0x1c[
    (sizeof(StageMapList) == 0x1c) ? 1 : -1];

// IDA_EA: 0x82677F8
// IDA_NAME: _ZN21CPCRoomWorldDropEventC2Ev
// DEMANGLED: CPCRoomWorldDropEvent::CPCRoomWorldDropEvent(void)
CPCRoomWorldDropEvent::CPCRoomWorldDropEvent()
    : CEventBase() {}

// IDA_EA: 0x8267814
// IDA_NAME: _ZN21CPCRoomWorldDropEventD2Ev
// DEMANGLED: CPCRoomWorldDropEvent::~CPCRoomWorldDropEvent()
// IDA_EA: 0x8267844
// IDA_NAME: _ZN21CPCRoomWorldDropEventD0Ev
// DEMANGLED: CPCRoomWorldDropEvent::~CPCRoomWorldDropEvent() deleting destructor
CPCRoomWorldDropEvent::~CPCRoomWorldDropEvent() {}

// IDA_EA: 0x82678BE
// IDA_NAME: _ZN21CPCRoomWorldDropEvent10StartEventEv
// DEMANGLED: CPCRoomWorldDropEvent::StartEvent(void)
void CPCRoomWorldDropEvent::StartEvent() {
    g_pc_room_world_drop_all_accounts = false;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 33, 0);
    trace("PCRoom World Drop Event Start!");
}

// IDA_EA: 0x8267910
// IDA_NAME: _ZN21CPCRoomWorldDropEvent10StartEventE10Word_Param
// DEMANGLED: CPCRoomWorldDropEvent::StartEvent(Word_Param)
void CPCRoomWorldDropEvent::StartEvent(Word_Param param) {
    StartEvent();

    // Official 0828/0627 ignores both words and therefore remains the
    // default 22,0,0 path.  The single exact value 22,1,0 is a documented
    // local operator extension; every other parameter tuple fails closed to
    // the official PC-room-only rule.
    const uint32_t packed = static_cast<uint32_t>(param);
    g_pc_room_world_drop_all_accounts = packed == 1u;
    if (g_pc_room_world_drop_all_accounts) {
        cMyTrace trace("StartEvent", 49, 0);
        trace("PCRoom World Drop Event all-account mode enabled");
    }
}

// IDA_EA: 0x826792A
// IDA_NAME: _ZN21CPCRoomWorldDropEvent8EndEventEv
// DEMANGLED: CPCRoomWorldDropEvent::EndEvent(void)
void CPCRoomWorldDropEvent::EndEvent() {
    g_pc_room_world_drop_all_accounts = false;
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 44, 0);
    trace("PCRoom World Drop Event End!");
}

// IDA_EA: 0x811BF92
// IDA_NAME: _ZN24CPCRoomWorldDropEvent2ndC2Ev
// DEMANGLED: CPCRoomWorldDropEvent2nd::CPCRoomWorldDropEvent2nd(void)
CPCRoomWorldDropEvent2nd::CPCRoomWorldDropEvent2nd()
    : CEventBase() {}

// IDA_EA: 0x811BFAE
// IDA_NAME: _ZN24CPCRoomWorldDropEvent2ndD2Ev
// DEMANGLED: CPCRoomWorldDropEvent2nd::~CPCRoomWorldDropEvent2nd()
// IDA_EA: 0x811BFDE
// IDA_NAME: _ZN24CPCRoomWorldDropEvent2ndD0Ev
// DEMANGLED: CPCRoomWorldDropEvent2nd::~CPCRoomWorldDropEvent2nd() deleting
// destructor
CPCRoomWorldDropEvent2nd::~CPCRoomWorldDropEvent2nd() {}

// IDA_EA: 0x811BFFC
// IDA_NAME: _ZN24CPCRoomWorldDropEvent2nd10StartEventEv
// DEMANGLED: CPCRoomWorldDropEvent2nd::StartEvent(void)
void CPCRoomWorldDropEvent2nd::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CPCRoomWorldDropEvent2nd::StartEvent()",
                   17,
                   0);
    trace("PCRoom World Drop Event 2nd Start!");
}

// IDA_EA: 0x811C04E
// IDA_NAME: _ZN24CPCRoomWorldDropEvent2nd10StartEventE10Word_Param
// DEMANGLED: CPCRoomWorldDropEvent2nd::StartEvent(Word_Param)
void CPCRoomWorldDropEvent2nd::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C068
// IDA_NAME: _ZN24CPCRoomWorldDropEvent2nd8EndEventEv
// DEMANGLED: CPCRoomWorldDropEvent2nd::EndEvent(void)
void CPCRoomWorldDropEvent2nd::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CPCRoomWorldDropEvent2nd::EndEvent()", 27, 0);
    trace("PCRoom World Drop Event 2nd End!");
}

// IDA_EA: 0x822B49E
// IDA_NAME: _ZN10PvPMapInfoC2Ev
// DEMANGLED: PvPMapInfo::PvPMapInfo(void)
PvPMapInfo::PvPMapInfo()
    : map_index(0),
      enabled(0) {}

// IDA_EA: 0x893C436
// IDA_NAME: _ZN19stAttackedMapInfo_tD2Ev
// DEMANGLED: stAttackedMapInfo_t::~stAttackedMapInfo_t()
stAttackedMapInfo_t::~stAttackedMapInfo_t() {}

// IDA_EA: 0x837CF1E
// IDA_NAME: _ZN19stVillageAttacked_tC2Ev
// DEMANGLED: stVillageAttacked_t::stVillageAttacked_t(void)
stVillageAttacked_t::stVillageAttacked_t()
    : std::vector<stAttackedMapInfo_t>() {}

// IDA_EA: 0x837CF32
// IDA_NAME: _ZN19stVillageAttacked_tD2Ev
// DEMANGLED: stVillageAttacked_t::~stVillageAttacked_t()
stVillageAttacked_t::~stVillageAttacked_t() {}

// IDA_EA: 0x815088E
// IDA_NAME: _ZN8_mapItemC2Ev
// DEMANGLED: _mapItem::_mapItem(void)
_mapItem::_mapItem()
    : item_id(0),
      item_count(0),
      drop_id(0) {}

// IDA_EA: 0x81512CC
// IDA_NAME: _ZN8map_itemC2Ev
// DEMANGLED: map_item::map_item(void)
map_item::map_item()
    : item(),
      luck_point_recipient_(0) {}

// IDA_EA: 0x89385F4
// IDA_NAME: _ZN19WorldDropItemChunckD2Ev
// DEMANGLED: WorldDropItemChunck::~WorldDropItemChunck()
WorldDropItemChunck::~WorldDropItemChunck() {}

// IDA_EA: 0x8374EC8
// IDA_NAME: _ZN13WorldDropInfoC2Ev
// DEMANGLED: WorldDropInfo::WorldDropInfo(void)
WorldDropInfo::WorldDropInfo()
    : item_chunks() {}

// IDA_EA: 0x8374EDE
// IDA_NAME: _ZN13WorldDropInfoD2Ev
// DEMANGLED: WorldDropInfo::~WorldDropInfo()
WorldDropInfo::~WorldDropInfo() {}

// IDA_EA: 0x8AB0996
// IDA_NAME: _ZN20STWorldmapPattern2NDC2Ev
// DEMANGLED: STWorldmapPattern2ND::STWorldmapPattern2ND(void)
STWorldmapPattern2ND::STWorldmapPattern2ND()
    : map_no(-1),
      raw_04() {}

// IDA_EA: 0x88F2886
// IDA_NAME: _ZN17STWorldmapPatternC2Ev
// DEMANGLED: STWorldmapPattern::STWorldmapPattern(void)
STWorldmapPattern::STWorldmapPattern()
    : pattern(""),
      raw_04(0xffffffffu),
      raw_08(0),
      raw_0c(0),
      raw_10(0),
      raw_14(0),
      raw_18(0),
      raw_1c(0),
      raw_20(0),
      raw_24(0) {}

// IDA_EA: 0x88F2E04
// IDA_NAME: _ZN17STWorldmapPatternaSERKS_
// DEMANGLED: STWorldmapPattern::operator=(STWorldmapPattern const&)
STWorldmapPattern& STWorldmapPattern::operator=(const STWorldmapPattern& rhs) {
    pattern = rhs.pattern;
    raw_04 = rhs.raw_04;
    raw_08 = rhs.raw_08;
    raw_0c = rhs.raw_0c;
    raw_10 = rhs.raw_10;
    raw_14 = rhs.raw_14;
    raw_18 = rhs.raw_18;
    raw_1c = rhs.raw_1c;
    raw_20 = rhs.raw_20;
    raw_24 = rhs.raw_24;
    return *this;
}

// IDA_EA: 0x88F2E70
// IDA_NAME: _ZN17STWorldmapPatternD2Ev
// DEMANGLED: STWorldmapPattern::~STWorldmapPattern()
STWorldmapPattern::~STWorldmapPattern() {}

// IDA_EA: 0x88F2D6C
// IDA_NAME: _ZN20STNamedMonsterMapPosC2Ev
// DEMANGLED: STNamedMonsterMapPos::STNamedMonsterMapPos(void)
STNamedMonsterMapPos::STNamedMonsterMapPos()
    : x(0),
      y(0) {}

// IDA_EA: 0x88F8CBC
// IDA_NAME: _ZN20STNamedMonsterMapPosD2Ev
// DEMANGLED: STNamedMonsterMapPos::~STNamedMonsterMapPos()
STNamedMonsterMapPos::~STNamedMonsterMapPos() {}

// IDA_EA: 0x8370D1C
// IDA_NAME: _ZN24stClearTimeAverageMapKeyC2Ev
// DEMANGLED: stClearTimeAverageMapKey::stClearTimeAverageMapKey(void)
stClearTimeAverageMapKey::stClearTimeAverageMapKey() {
    clear();
}

// IDA_EA: 0x8370D30
// IDA_NAME: _ZN24stClearTimeAverageMapKey5clearEv
// DEMANGLED: stClearTimeAverageMapKey::clear(void)
stClearTimeAverageMapKey* stClearTimeAverageMapKey::clear() {
    dungeon_index = 0;
    difficulty = 0;
    return this;
}

// IDA_EA: 0x8370D48
// IDA_NAME: _ZNK24stClearTimeAverageMapKeyltERKS_
// DEMANGLED: stClearTimeAverageMapKey::operator<(stClearTimeAverageMapKey const&)const
bool stClearTimeAverageMapKey::operator<(
    const stClearTimeAverageMapKey& rhs) const {
    if (dungeon_index < rhs.dungeon_index) {
        return true;
    }
    return dungeon_index == rhs.dungeon_index && difficulty < rhs.difficulty;
}

// IDA_EA: 0x8AB09C6
// IDA_NAME: _ZN15WorldMapDungeon5clearEv
// DEMANGLED: WorldMapDungeon::clear(void)
WorldMapDungeon* WorldMapDungeon::clear() {
    dungeon_index = -1;
    area_index = -1;
    map_index = 0;
    return this;
}

// IDA_EA: 0x8AB09E8
// IDA_NAME: _ZN15WorldMapDungeonC2Ev
// DEMANGLED: WorldMapDungeon::WorldMapDungeon(void)
WorldMapDungeon::WorldMapDungeon() {
    clear();
}

// IDA_EA: 0x8AB0F74
// IDA_NAME: _ZN15WorldMapDungeonD2Ev
// DEMANGLED: WorldMapDungeon::~WorldMapDungeon()
WorldMapDungeon::~WorldMapDungeon() {}

// IDA_EA: 0x8369E74
// IDA_NAME: _ZN14WorldMapScript5clearEv
// DEMANGLED: WorldMapScript::clear(void)
void WorldMapScript::clear() {
    world_map_0828::Erase(this);
    raw_00 = 0;
    name.clear();
    map_image.clear();
    raw_10 = 0;
    dungeons.clear();
    raw_20 = 0;
    hell_dungeons.clear();
    hell_quests.clear();
    freepass_items.clear();
    item_conditions.clear();
}

// IDA_EA: 0x8369F08
// IDA_NAME: _ZN14WorldMapScriptC2Ev
// DEMANGLED: WorldMapScript::WorldMapScript(void)
WorldMapScript::WorldMapScript()
    : raw_00(0),
      name(),
      map_image(),
      ui_path(),
      raw_10(0),
      dungeons(),
      raw_20(0),
      hell_dungeons(),
      hell_quests(),
      freepass_items(),
      item_conditions() {
    clear();
}

// IDA_EA: 0x837F5AE
// IDA_NAME: _ZN14WorldMapScriptD2Ev
// DEMANGLED: WorldMapScript::~WorldMapScript()
WorldMapScript::~WorldMapScript() {
    world_map_0828::Erase(this);
}

// IDA_EA: 0x830E4FC
// IDA_NAME: _ZN11_mapMonsterC2Ev
// DEMANGLED: _mapMonster::_mapMonster(void)
_mapMonster::_mapMonster()
    : raw_34() {
    map_index = -1;
    enabled = 1;
    raw_08 = 0;
    raw_10 = 0;
    raw_14 = 0;
    raw_0c = 0;
    raw_18 = 0;
    raw_1c = 0;
    raw_20 = 0;
    raw_2c = 0;
    raw_28 = 100;
    raw_30 = 0;
    raw_34.clear();
}

// IDA_EA: 0x813D708
// IDA_NAME: _ZN11_mapMonsterC2ERKS_
// DEMANGLED: _mapMonster::_mapMonster(_mapMonster const&)
_mapMonster::_mapMonster(const _mapMonster& rhs)
    : map_index(rhs.map_index),
      enabled(rhs.enabled),
      raw_08(rhs.raw_08),
      raw_0c(rhs.raw_0c),
      raw_10(rhs.raw_10),
      raw_14(rhs.raw_14),
      raw_18(rhs.raw_18),
      raw_1c(rhs.raw_1c),
      raw_20(rhs.raw_20),
      raw_24(rhs.raw_24),
      raw_28(rhs.raw_28),
      raw_2c(rhs.raw_2c),
      raw_30(rhs.raw_30),
      raw_34(rhs.raw_34) {}

// IDA_EA: 0x81398C6
// IDA_NAME: _ZN11_mapMonsterD2Ev
// DEMANGLED: _mapMonster::~_mapMonster()
_mapMonster::~_mapMonster() {}

// IDA_EA: 0x85F4104
// IDA_NAME: _ZNK20RandomApplicationKeyltERKS_
// DEMANGLED: RandomApplicationKey::operator<(RandomApplicationKey const&)const
bool RandomApplicationKey::operator<(const RandomApplicationKey& rhs) const {
    if (rarity < rhs.rarity) {
        return true;
    }
    return rarity == rhs.rarity && grade < rhs.grade;
}

// IDA_EA: 0x85F414E
// IDA_NAME: _ZN20RandomApplicationKeyC2Ehh
// DEMANGLED: RandomApplicationKey::RandomApplicationKey(unsigned char,unsigned char)
RandomApplicationKey::RandomApplicationKey(uint8_t rarity_value,
                                           uint8_t grade_value)
    : rarity(rarity_value),
      grade(grade_value) {}

// IDA_EA: 0x8A73B7C
// IDA_NAME: _ZN20RandomApplicationKeyC2Ev
// DEMANGLED: RandomApplicationKey::RandomApplicationKey(void)
RandomApplicationKey::RandomApplicationKey()
    : rarity(0),
      grade(0) {}

// IDA_EA: 0x89E89FA
// IDA_NAME: _ZN14STRivalMapDataC2Ev
// DEMANGLED: STRivalMapData::STRivalMapData(void)
STRivalMapData::STRivalMapData()
    : position_x(0),
      position_y(0),
      spawn_weight(0) {}

// IDA_EA: 0x89F0F8E
// IDA_NAME: _ZN14STRivalMapDataD2Ev
// DEMANGLED: STRivalMapData::~STRivalMapData()
STRivalMapData::~STRivalMapData() {}

// IDA_EA: 0x891C7F8
// IDA_NAME: _ZN11PVPMAP_DATAC2Ev
// DEMANGLED: PVPMAP_DATA::PVPMAP_DATA(void)
PVPMAP_DATA::PVPMAP_DATA()
    : std::string() {}

// IDA_EA: 0x892A2D4
// IDA_NAME: _ZN11PVPMAP_DATAC2ERKS_
// DEMANGLED: PVPMAP_DATA::PVPMAP_DATA(PVPMAP_DATA const&)
PVPMAP_DATA::PVPMAP_DATA(const PVPMAP_DATA& rhs)
    : std::string(rhs),
      map_index(rhs.map_index) {}

// IDA_EA: 0x83E64BE
// IDA_NAME: _ZN11PVPMAP_DATAD2Ev
// DEMANGLED: PVPMAP_DATA::~PVPMAP_DATA()
PVPMAP_DATA::~PVPMAP_DATA() {}

// IDA_EA: 0x892A358
// IDA_NAME: _ZN11PVPMAP_DATAaSERKS_
// DEMANGLED: PVPMAP_DATA::operator=(PVPMAP_DATA const&)
PVPMAP_DATA& PVPMAP_DATA::operator=(const PVPMAP_DATA& rhs) {
    std::string::operator=(rhs);
    map_index = rhs.map_index;
    return *this;
}

namespace village_attacked {

// CRevengeDungeon implementation migrated to
// old_game/dungeon_pvp/revenge_dungeon_runtime.cpp.  This legacy translation
// unit intentionally keeps no second body for its IDA symbols.

}  // namespace village_attacked

// IDA_EA: 0x83748AA
// IDA_NAME: _ZN12StageMapList16_StageMapSummaryC2Ev
// DEMANGLED: StageMapList::_StageMapSummary::_StageMapSummary(void)
StageMapList::_StageMapSummary::_StageMapSummary() {
    std::memset(this, 0, 0x10u);
}

// IDA_EA: 0x83C9728
// IDA_NAME: _ZN12StageMapList16_StageMapSummaryD2Ev
// DEMANGLED: StageMapList::_StageMapSummary::~_StageMapSummary()
StageMapList::_StageMapSummary::~_StageMapSummary() {}

// IDA_EA: 0x81347C0
// IDA_NAME: _ZNK12StageMapList18getFirstStageIndexEv
// DEMANGLED: StageMapList::getFirstStageIndex(void)const
int StageMapList::getFirstStageIndex() const {
    return first_stage_index;
}

// IDA_EA: 0x8364818
// IDA_NAME: _ZN12StageMapList20makeStageMapUniqueIdEiii
// DEMANGLED: StageMapList::makeStageMapUniqueId(int,int,int)
int StageMapList::makeStageMapUniqueId(int area,
                                       int district,
                                       int stage_ordinal) {
    return stage_ordinal + 1000 + 100000000 * (area + 100) +
           10000 * (district + 1000);
}

// IDA_EA: 0x8364848
// IDA_NAME: _ZN12StageMapList7compareERKNS_16_StageMapSummaryES2_
// DEMANGLED: StageMapList::compare(StageMapList::_StageMapSummary const&,StageMapList::_StageMapSummary const&)
bool StageMapList::compare(const _StageMapSummary& lhs,
                           const _StageMapSummary& rhs) {
    return lhs.unique_id < rhs.unique_id;
}

// IDA_EA: 0x8364DF6
// IDA_NAME: _ZNK12StageMapList7isExistEi
// DEMANGLED: StageMapList::isExist(int)const
bool StageMapList::isExist(int stage_index) const {
    return stage_map.find(stage_index) != stage_map.end();
}

// IDA_EA: 0x8364E58
// IDA_NAME: _ZNK12StageMapList12getNextStageEiRb
// DEMANGLED: StageMapList::getNextStage(int,bool &)const
int StageMapList::getNextStage(int stage_index,
                               bool& is_survival_stage) const {
    // The mangled ABI is EiRb: the lookup key is the full int stage id.
    // Truncating it to u8 makes every official 3511+ altar stage unreachable.
    std::map<int, _StageMapSummary>::const_iterator it =
        stage_map.find(stage_index);
    if (it == stage_map.end()) {
        return 0;
    }

    const _StageMapSummary& current = it->second;
    std::map<int, _StageMapSummary>::const_iterator next_it =
        stage_map.find(current.next_stage_index);
    if (next_it != stage_map.end()) {
        is_survival_stage = next_it->second.is_survival_stage != 0;
    }
    return current.next_stage_index;
}

// IDA_EA: 0x8374874
// IDA_NAME: _ZN12StageMapListC2Ev
// DEMANGLED: StageMapList::StageMapList(void)
StageMapList::StageMapList()
    : first_stage_index(0),
      stage_map() {}

// IDA_EA: 0x8374894
// IDA_NAME: _ZN12StageMapListD2Ev
// DEMANGLED: StageMapList::~StageMapList()
StageMapList::~StageMapList() {}
