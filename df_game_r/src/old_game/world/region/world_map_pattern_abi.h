#ifndef OLDSRV_GLOBAL_WORLD_MAP_PATTERN_ABI_H_
#define OLDSRV_GLOBAL_WORLD_MAP_PATTERN_ABI_H_

#include "old_game/world/region/world_value_types.h"


// IDA unit: 99_misc_global/Global WorldMap pattern lookup.
// IDA_EA: GetWorldMapCoordFromTable @0x88E5EF8;
// GetWorldMapCoordFromTable2ND @0x88E5F44.
// B-owned initWorldMapScript/loadWorldMapCoordPattern fill these tables from
// the real RDAR path; this public ABI only exposes the old lookup boundary.
extern STWorldmapPattern g_worldMapPatternTable[3][10];
extern STWorldmapPattern2ND g_worldMapPatternTable2ND[20];

STWorldmapPattern* GetWorldMapCoordFromTable(
    int map_type,
    int pattern_index,
    STWorldmapPattern& pattern)
    __asm__("_Z25GetWorldMapCoordFromTableiiR17STWorldmapPattern");
int GetWorldMapCoordFromTable2ND(
    int row_index,
    STWorldmapPattern2ND& pattern)
    __asm__("_Z28GetWorldMapCoordFromTable2NDiR20STWorldmapPattern2ND");


#endif  // OLDSRV_GLOBAL_WORLD_MAP_PATTERN_ABI_H_
