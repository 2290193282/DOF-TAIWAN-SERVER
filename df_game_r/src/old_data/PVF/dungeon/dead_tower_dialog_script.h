#ifndef OLDSRV_OLD_DATA_PVF_DUNGEON_DEAD_TOWER_DIALOG_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_DUNGEON_DEAD_TOWER_DIALOG_SCRIPT_H_

#include <vector>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA_EA: imported by importDeadTowerDialogScript @ 0x8908D46.  The death
// tower container is physically three adjacent vectors, but its element type
// is STDeadTowerDialog, not STTowerDialog.
struct STDeadTowerFullDialog {
    std::vector<STDeadTowerDialog> start_dialogs;
    std::vector<STDeadTowerDialog> complete_dialogs;
    std::vector<STDeadTowerDialog> die_dialogs;
};

// IDA_EA: 0x8908D46
// IDA_NAME: _Z27importDeadTowerDialogScriptPKcR21STDeadTowerFullDialog
// DEMANGLED: importDeadTowerDialogScript(char const*,STDeadTowerFullDialog &)
int importDeadTowerDialogScript(const char* filename,
                                STDeadTowerFullDialog& dialogs);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STDeadTowerFullDialog) == 0x24u,
              "STDeadTowerFullDialog must retain the original three-vector ABI");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_DUNGEON_DEAD_TOWER_DIALOG_SCRIPT_H_
