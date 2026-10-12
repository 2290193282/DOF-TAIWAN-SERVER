#include "old_data/PVF/dungeon/dead_tower_dialog_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

namespace {

int scanDialog(std::vector<STDeadTowerDialog>& target,
               int missing_index_error,
               int missing_text_error) {
    STDeadTowerDialog dialog;
    bool scanned = false;
    dialog.raw_00 = ScanInt(&scanned);
    if (!scanned) {
        return missing_index_error;
    }
    if (ScanStr(&dialog.dialog) != 1) {
        return missing_text_error;
    }
    dialog.raw_08 = 1;
    target.push_back(dialog);
    return 0;
}

}  // namespace

// IDA_EA: 0x8908D46
// IDA_NAME: _Z27importDeadTowerDialogScriptPKcR21STDeadTowerFullDialog
// DEMANGLED: importDeadTowerDialogScript(char const*,STDeadTowerFullDialog &)
int importDeadTowerDialogScript(const char* filename,
                                STDeadTowerFullDialog& dialogs) {
    if (!loadRDARScriptFile("", filename)) {
        return 6361;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        int result = 0;
        if (tag == "[dialog on map start]") {
            result = scanDialog(dialogs.start_dialogs, 6374, 6378);
        } else if (tag == "[dialog on map complete]") {
            result = scanDialog(dialogs.complete_dialogs, 6390, 6394);
        } else if (tag == "[dialog on die]") {
            result = scanDialog(dialogs.die_dialogs, 6406, 6410);
        }

        if (result != 0) {
            return result;
        }
    }
    return 0;
}
