#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_VALUE_TYPES_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_VALUE_TYPES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x822B49E in the companion source.


#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_game/economy/inventory/inventory.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/village_attacked_abi.h"
#include "old_game/activity/reward/event_base.h"
#include "common/manager/random_application_key_abi.h"


// IDA unit: 05_world_enter_village/CPCRoomWorldDropEvent.
class CPCRoomWorldDropEvent : public CEventBase {
public:
    CPCRoomWorldDropEvent();
    virtual ~CPCRoomWorldDropEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

namespace pc_room_world_drop_event {

// Event 22 keeps the official 22,0,0 PC-room qualification rule.  The local
// operational extension 22,1,0 opens only WorldDropPCRoom.etc to every party;
// it deliberately does not manufacture premium type 12 or affect any other
// PC-room reward family.
bool AllowsAllAccounts();

}  // namespace pc_room_world_drop_event

// IDA unit: 05_world_enter_village/CPCRoomWorldDropEvent2nd.
// IDA_EA: ctor/dtor/start/end @0x811BF92/@0x811BFAE/@0x811BFDE/
// @0x811BFFC/@0x811C04E/@0x811C068.
class CPCRoomWorldDropEvent2nd : public CEventBase {
public:
    CPCRoomWorldDropEvent2nd();
    virtual ~CPCRoomWorldDropEvent2nd();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 05_world_enter_village/PvPMapInfo.
struct PvPMapInfo {
    PvPMapInfo();

    uint32_t map_index;
    uint8_t enabled;
    uint8_t pad[3];
};

// IDA unit: 05_world_enter_village/_mapItem.
struct _mapItem {
    _mapItem();

    uint32_t raw_00;
    uint32_t item_id;
    uint32_t item_count;
    uint32_t drop_id;
};

// IDA unit: 05_world_enter_village/map_item.
struct map_item {
    map_item();

    // IDA 0x814FD76 / 0x814FE36 branches on this member before choosing
    // random/routing pickup.  The original field name is not recovered, so
    // keep its established gameplay role instead of a raw byte facade.
    uint8_t routing_eligible_;               // +0x00
    int8_t packet_value_01_;                 // +0x01
    uint8_t raw_02_03_[0x02];
    // IDA 0x81517FA indexes MapInfo::items_ with this dword.
    uint32_t unique_id_;                     // +0x04
    // The boss-stage kill drop path stores its fourth virtual argument here
    // and re-emits it at the end of packet 0/38 (IDA 0x814D891/0x814DCBA).
    uint16_t monster_type_;                  // +0x08
    uint16_t pad_0a_;
    // IDA 0x814D8A0 records the server second at this physical dword when a
    // monster-generated item enters the map.
    uint32_t created_at_sec_;                // +0x0c
    Inven_Item item;
    uint8_t raw_4d_4f_[0x03];
    // _MakeDropItems @0x830ADF6 stores
    // CLuckPoint::GetUserGainItem(item_index) here.  The pickup path can
    // therefore charge the luck point to the exact selected party member.
    CUser* luck_point_recipient_;            // +0x50
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(map_item, luck_point_recipient_) == 0x50u,
              "map_item luck-point recipient must stay at +0x50");
#endif

// IDA unit: 05_world_enter_village/WorldDropItemChunck.
struct WorldDropItemChunck {
    ~WorldDropItemChunck();

    int32_t item_id;
    int32_t weight;
};

// IDA unit: 05_world_enter_village/WorldDropInfo.
struct WorldDropInfo {
    WorldDropInfo();
    ~WorldDropInfo();

    int32_t drop_rate_or_total_weight;
    std::vector<WorldDropItemChunck> item_chunks;
    int32_t total_weight;
};

// IDA unit: 05_world_enter_village/STWorldmapPattern2ND.
struct STWorldmapPattern2ND {
    STWorldmapPattern2ND();

    int map_no;
    uint8_t raw_04[0x18];
};

// IDA unit: 05_world_enter_village/STWorldmapPattern.
struct STWorldmapPattern {
    STWorldmapPattern();
    ~STWorldmapPattern();

    STWorldmapPattern& operator=(const STWorldmapPattern& rhs);

    std::string pattern;
    uint32_t raw_04;
    uint32_t raw_08;
    uint32_t raw_0c;
    uint32_t raw_10;
    uint32_t raw_14;
    uint32_t raw_18;
    uint32_t raw_1c;
    uint32_t raw_20;
    uint32_t raw_24;
};

// IDA unit: 05_world_enter_village/STNamedMonsterMapPos.
struct STNamedMonsterMapPos {
    STNamedMonsterMapPos();
    ~STNamedMonsterMapPos();

    uint32_t x;
    uint32_t y;
};

// IDA unit: 05_world_enter_village/stClearTimeAverageMapKey.
struct stClearTimeAverageMapKey {
    stClearTimeAverageMapKey();

    stClearTimeAverageMapKey* clear();
    bool operator<(const stClearTimeAverageMapKey& rhs) const;

    uint32_t dungeon_index;
    uint32_t difficulty;
};

// IDA unit: 05_world_enter_village/WorldMapDungeon.
struct WorldMapDungeon {
    WorldMapDungeon();
    ~WorldMapDungeon();

    WorldMapDungeon* clear();

    int dungeon_index;
    int area_index;
    int map_index;
};

// IDA unit: 05_world_enter_village/WorldMapScript.
struct WorldMapScript {
    WorldMapScript();
    ~WorldMapScript();

    void clear();

    int raw_00;
    std::string name;
    std::string map_image;
    std::string ui_path;
    int raw_10;
    std::vector<WorldMapDungeon> dungeons;
    uint8_t raw_20;
    uint8_t raw_21[0x03];
    std::vector<int> hell_dungeons;
    std::vector<std::pair<int, int> > hell_quests;
    std::vector<int> freepass_items;
    std::vector<std::pair<int, int> > item_conditions;
};

// [festival condition] and [exercise] are present in the official 0828
// world-map scripts but have no recovered storage inside the fixed 0x54
// WorldMapScript ABI.  Preserve them as exact scalar values in a lifecycle
// sidecar and promote a copy to the runtime map-index registry.
namespace world_map_0828 {

struct Extension {
    Extension();

    bool has_festival_condition;
    int32_t festival_condition;
    bool has_exercise;
    int32_t exercise;
};

void SetFestivalCondition(const WorldMapScript* script, int32_t value);
void SetExercise(const WorldMapScript* script, int32_t value);
bool Find(const WorldMapScript* script, Extension* output);
void RegisterRuntime(int map_index, const WorldMapScript* script);
bool FindRuntime(int map_index, Extension* output);
void ClearRuntime();

}  // namespace world_map_0828

// IDA unit: 05_world_enter_village/_mapMonster.
struct _mapMonster {
    _mapMonster();
    _mapMonster(const _mapMonster& rhs);
    ~_mapMonster();

    int map_index;
    uint8_t enabled;
    uint8_t raw_05[0x03];
    int raw_08;
    int raw_0c;
    int raw_10;
    int raw_14;
    int raw_18;
    uint8_t raw_1c;
    uint8_t raw_1d[0x03];
    int raw_20;
    int raw_24;
    int raw_28;
    int raw_2c;
    uint8_t raw_30;
    uint8_t raw_31[0x03];
    std::vector<int> raw_34;
};

// IDA unit: 05_world_enter_village/STRivalMapData.
struct STRivalMapData {
    STRivalMapData();
    ~STRivalMapData();

    int32_t position_x;    // +0x00
    int32_t position_y;    // +0x04
    int32_t spawn_weight;  // +0x08; WarField weighted selection
};

// IDA unit: 05_world_enter_village/PVPMAP_DATA.
struct PVPMAP_DATA : public std::string {
    PVPMAP_DATA();
    PVPMAP_DATA(const PVPMAP_DATA& rhs);
    ~PVPMAP_DATA();

    PVPMAP_DATA& operator=(const PVPMAP_DATA& rhs);

    uint32_t map_index;
};

namespace village_attacked {

// IDA unit: 05_world_enter_village/village_attacked/Zone.
struct Zone {
    Zone(uint8_t village, uint8_t area);

    bool operator<(const Zone& rhs) const;

    uint8_t village;
    uint8_t area;
};

// CRevengeDungeon's IDA declarations and only implementation now live in
// old_game/dungeon_pvp/revenge_dungeon_runtime.h/.cpp.  Keep this legacy
// header as a zero-body migration consumer for the remaining Zone types.

}  // namespace village_attacked

// IDA unit: 05_world_enter_village/StageMapList/_StageMapSummary.
struct StageMapList {
    StageMapList();
    ~StageMapList();

    struct _StageMapSummary {
        _StageMapSummary();
        ~_StageMapSummary();

        int unique_id;
        int stage_index;
        int next_stage_index;
        uint8_t is_survival_stage;
        uint8_t raw_0d[0x03];
    };

    int getFirstStageIndex() const;
    static int makeStageMapUniqueId(int area, int district, int stage);
    static bool compare(const _StageMapSummary& lhs,
                        const _StageMapSummary& rhs);

    // IDA_EA: 0x836485C
    // IDA_NAME: _ZN12StageMapList8loadListEv
    // DEMANGLED: StageMapList::loadList(void)
    // Body ownership: B RDAR/PVF script loading; declaration/layout: C05.
    int loadList();

    bool isExist(int stage_index) const;
    int getNextStage(int stage_index, bool& is_survival_stage) const;

    int first_stage_index;
    std::map<int, _StageMapSummary> stage_map;
};


#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_VALUE_TYPES_H_
