#include "old_game/world/region/world_map_pattern_abi.h"

#include <string.h>


// IDA_EA: 0x88E5EF8
// IDA_NAME: _Z25GetWorldMapCoordFromTableiiR17STWorldmapPattern
// DEMANGLED: GetWorldMapCoordFromTable(int,int,STWorldmapPattern &)
STWorldmapPattern* GetWorldMapCoordFromTable(
    int map_type,
    int pattern_index,
    STWorldmapPattern& pattern) {
    if (map_type > 0 && pattern_index > 0) {
        pattern = g_worldMapPatternTable[map_type - 1][pattern_index - 1];
        return &pattern;
    }
    return 0;
}

// IDA_EA: 0x88E5F44
// IDA_NAME: _Z28GetWorldMapCoordFromTable2NDiR20STWorldmapPattern2ND
// DEMANGLED: GetWorldMapCoordFromTable2ND(int,STWorldmapPattern2ND &)
int GetWorldMapCoordFromTable2ND(
    int row_index,
    STWorldmapPattern2ND& pattern) {
    if (row_index >= 0) {
        const STWorldmapPattern2ND& source =
            g_worldMapPatternTable2ND[row_index];
        memcpy(&pattern, &source, sizeof(pattern));
        int result = 0;
        memcpy(&result, source.raw_04 + 0x14u, sizeof(result));
        return result;
    }
    return 0;
}
