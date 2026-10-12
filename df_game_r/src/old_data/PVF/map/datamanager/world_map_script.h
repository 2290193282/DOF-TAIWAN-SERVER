#ifndef OLDSRV_DATAMANAGER_WORLD_MAP_SCRIPT_H_
#define OLDSRV_DATAMANAGER_WORLD_MAP_SCRIPT_H_


struct WorldMapScript;
struct STWorldmapPattern;

// IDA_EA: initWorldMapScript @0x8AB000C.
int initWorldMapScript(char* src, char* filename)
    __asm__("_Z18initWorldMapScriptPcS_");
// IDA_EA: loadWorldMapCoordPattern @0x8AB0234.
int loadWorldMapCoordPattern()
    __asm__("_Z24loadWorldMapCoordPatternv");
// IDA_EA: importWorldMapScript @0x8AB0388.
int importWorldMapScript(WorldMapScript* script, const char* filename)
    __asm__("_Z20importWorldMapScriptP14WorldMapScriptPKc");

// Typed lookup for the [A]/[B] entries already imported from
// WorldMapPatternTable.tbl.  Only the two evidence-backed 0828 tag names are
// accepted; ordinal is the exact zero-based row within that tag.
const STWorldmapPattern* findWorldMapPattern0828(const char* tag,
                                                int ordinal);

#endif  // OLDSRV_DATAMANAGER_WORLD_MAP_SCRIPT_H_
