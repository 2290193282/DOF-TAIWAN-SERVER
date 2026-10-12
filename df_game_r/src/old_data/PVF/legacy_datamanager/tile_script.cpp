#include "old_data/PVF/data/datamanager/tile_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <stdio.h>


typedef char STTileScript_Size_Must_Be_0x20[
    (sizeof(STTileScript) == 0x20) ? 1 : -1];

// IDA_EA: 0x89E82F6
// IDA_NAME: _ZN12STTileScript5ClearEv
// DEMANGLED: STTileScript::Clear(void)
STTileScript* STTileScript::Clear() {
    tiles.clear();
    flag_0c = 0;
    tile_indices.clear();
    value_1c = 0;
    return this;
}

// IDA_EA: 0x89DCBAC
// IDA_NAME: _Z14InitTileScriptv
// DEMANGLED: InitTileScript(void)
int InitTileScript() {
    return 1;
}

// IDA_EA: 0x89DCBB6
// IDA_NAME: _Z16ImportTileScriptP12STTileScriptPKc
// DEMANGLED: ImportTileScript(STTileScript *,char const*)
int ImportTileScript(STTileScript* script, const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        printf("TILE LOAD FAIL! [%s]\n", filename);
        return 0;
    }

    script->Clear();

    bool success = false;
    int first_value = 0;
    std::string tag;
    std::string text;
    while (ScanType(tag, true) == 1) {
        if (tag == "[IMAGE]") {
            std::pair<std::string, int> image;
            ScanStr(&image.first);
            image.second = ScanInt(static_cast<bool*>(0));
            script->tiles.push_back(image);
        } else if (tag == "[tile type]") {
            ScanStr(&text);
            if (text == "[basic]") {
                script->flag_0c = 0;
            } else if (text == "[extended]") {
                script->flag_0c = 1;
            }
        } else if (tag == "[img pos]") {
            script->value_1c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[pass type]") {
            script->tile_indices.clear();

            std::vector<int> values;
            while (1) {
                first_value = ScanInt(&success);
                if (!success) {
                    break;
                }

                values.clear();
                values.push_back(first_value);
                for (int index = 1; index <= 13; ++index) {
                    const int value = ScanInt(static_cast<bool*>(0));
                    values.push_back(value);
                }
                script->tile_indices.push_back(values);
            }
        }
    }

    return 1;
}
