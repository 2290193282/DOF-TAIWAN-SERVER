#ifndef OLDSRV_DATAMANAGER_STAGE_MAP_SCRIPT_H_
#define OLDSRV_DATAMANAGER_STAGE_MAP_SCRIPT_H_

namespace stagemap {
struct StageMapScript;
}  // namespace stagemap

// IDA_EA: initStageMapScript @0x88A9E6C.
int initStageMapScript(char* src, char* filename)
    __asm__("_Z18initStageMapScriptPcS_");
// IDA_EA: importStageMapScript @0x88A9F28.
int importStageMapScript(stagemap::StageMapScript* script,
                         const char* filename)
    __asm__("_Z20importStageMapScriptPN8stagemap14StageMapScriptEPKc");

#endif  // OLDSRV_DATAMANAGER_STAGE_MAP_SCRIPT_H_
