#ifndef OLDSRV_DATAMANAGER_WORLD_STAGE_MAP_RUNTIME_H_
#define OLDSRV_DATAMANAGER_WORLD_STAGE_MAP_RUNTIME_H_

#include <stddef.h>
#include <map>
#include <utility>
#include <vector>

#include "old_game/world/region/world_value_types.h"

struct WorldMapScript;
class CDungeon;
class CDungeonList;
class CDataManager;
class CParty;
struct CWorldMapLayoutChecks;

// IDA unit: 05_world_enter_village/CWorldMap.
// IDA_EA: 0x837F6EA, 0x836421A, 0x83746AC.
class CWorldMap {
public:
    CWorldMap();
    ~CWorldMap();

    int IsInHellDungeon();
    int set_world_map(WorldMapScript* script);
    int get_index();
    int getDungeonMinLevel() const;
    int hasDeathTower() const;
    bool hasDungeon(unsigned int dungeon_index) const;
    bool get0828Extension(world_map_0828::Extension* output) const;
    std::vector<int>* GetDungeonList();
    // CParty::CheckClearQuestHellParty iterates the original +0x1c vector
    // directly. Keep that script-owned list typed instead of reconstructing
    // its address from the CWorldMap object.
    const std::vector<int>& GetHellDungeonList() const {
        return hell_dungeons_;
    }

private:
    friend struct CWorldMapLayoutChecks;
    // CParty::CheckHellDungeonFreepassItemHaveAndDel iterates the original
    // script-owned +0x28 prerequisite/free-pass pairs directly.
    friend class CParty;
    int map_index_;
    unsigned char in_hell_dungeon_;
    unsigned char reserved_05_[3];
    int min_dungeon_level_;
    unsigned char has_death_tower_;
    unsigned char reserved_0d_[3];
    std::vector<int> dungeons_;
    std::vector<int> hell_dungeons_;
    std::vector<std::pair<int, int> > hell_quests_;
    std::vector<int> freepass_items_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CWorldMapLayoutChecks {
    static_assert(__builtin_offsetof(CWorldMap, dungeons_) == 0x10u,
                  "CWorldMap dungeon list must stay at +0x10");
    static_assert(__builtin_offsetof(CWorldMap, hell_dungeons_) == 0x1cu,
                  "CWorldMap hell-dungeon list must stay at +0x1c");
    static_assert(__builtin_offsetof(CWorldMap, hell_quests_) == 0x28u,
                  "CWorldMap hell-quest list must stay at +0x28");
    static_assert(__builtin_offsetof(CWorldMap, freepass_items_) == 0x34u,
                  "CWorldMap seal-door item list must stay at +0x34");
};
#endif

// IDA unit: 05_world_enter_village/CWorldMapList.
// IDA_EA: 0x837480A, 0x8364506, 0x8364694.
class CWorldMapList {
public:
    CWorldMapList();
    ~CWorldMapList();

    int load_list();
    int insert_world_map(CWorldMap* world_map);
    CWorldMap* find_world_map(int map_index) const;
    int destroy();

private:
    std::map<int, CWorldMap*> world_maps_;
};

// CDataManager owns this embedded +0x8780 subobject.  The helper returns null
// for an absent manager so world queries do not independently recalculate the
// offset or dereference a missing global lifecycle object.
CWorldMapList* CDataManagerWorldMapList(CDataManager* data_manager);
const CWorldMapList* CDataManagerWorldMapList(
    const CDataManager* data_manager);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CWorldMap) == 0x40u,
              "CWorldMap old ABI size must stay 0x40");
static_assert(sizeof(CWorldMapList) == 0x18u,
              "CWorldMapList old ABI size must stay 0x18");
#endif

#endif  // OLDSRV_DATAMANAGER_WORLD_STAGE_MAP_RUNTIME_H_
