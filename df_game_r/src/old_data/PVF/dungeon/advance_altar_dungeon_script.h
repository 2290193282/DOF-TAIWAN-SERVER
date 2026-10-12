#ifndef OLDSRV_OLD_DATA_PVF_DUNGEON_ADVANCE_ALTAR_DUNGEON_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_DUNGEON_ADVANCE_ALTAR_DUNGEON_SCRIPT_H_

#include <string>

struct STDungeonScript;

namespace advancealtar {

// IDA_EA: 0x88990BA.  The original type is an int-backed nested enum used
// by the dungeon script and the advance-altar stage runtime.
struct StageType {
    enum T {
        NORMAL = 0,
        SURVIVAL = 1
    };

    static bool getStageType(const std::string& name, T& type);
};

}  // namespace advancealtar

// IDA_EA: 0x88F20A4
// IDA_NAME: _Z28ImportAdvanceAltarDungeonTagP15STDungeonScriptSsPKc
// DEMANGLED: ImportAdvanceAltarDungeonTag(STDungeonScript *,std::string,char const*)
int ImportAdvanceAltarDungeonTag(STDungeonScript* script,
                                 std::string tag,
                                 const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_DUNGEON_ADVANCE_ALTAR_DUNGEON_SCRIPT_H_
