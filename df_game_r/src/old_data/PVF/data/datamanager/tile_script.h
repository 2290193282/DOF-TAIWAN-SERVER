#ifndef OLDSRV_DATAMANAGER_TILE_SCRIPT_H_
#define OLDSRV_DATAMANAGER_TILE_SCRIPT_H_

#include <stdint.h>
#include <string>
#include <utility>
#include <vector>


// IDA_EA: 0x89E82F6, 0x89DCBAC, 0x89DCBB6.
// IDA unit: 12_pvf_datamanager_scripts/STTileScript.
class STTileScript {
public:
    typedef std::vector<std::pair<std::string, int> > TileList;
    typedef std::vector<std::vector<int> > TileIndexList;

    STTileScript* Clear();

    TileList tiles;
    uint8_t flag_0c;
    TileIndexList tile_indices;
    int32_t value_1c;
};

int InitTileScript();
int ImportTileScript(STTileScript* script, const char* filename);

#endif  // OLDSRV_DATAMANAGER_TILE_SCRIPT_H_
