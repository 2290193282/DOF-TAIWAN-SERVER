#include "old_data/PVF/creature/datamanager/pet_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

#include <string.h>

namespace {

char g_petScriptBaseDirectory[256] = {};
STScriptFileList g_petScriptFileList;

}  // namespace

typedef char PetScript_Size_Must_Be_0x70[
    (sizeof(PetScript) == 0x70) ? 1 : -1];

// IDA_EA: 0x8A3DBAA
// IDA_NAME: _ZN9PetScript5clearEv
// DEMANGLED: PetScript::clear(void)
PetScript* PetScript::clear() {
    value_00 = 0;
    value_04 = "";
    value_08 = 0;
    value_0C = 0;
    value_10 = 0;
    value_14 = 1;
    value_18 = 0;
    value_1C = 1;
    for (int i = 0; i < 4; ++i) {
        value_20[i] = 0;
    }
    value_30 = 0;
    value_38 = "";
    value_3C.clear();
    value_48.clear();
    value_54.clear();
    value_60.clear();
    value_6C = "";
    value_34 = 0;
    return this;
}

// IDA_EA: 0x8A3581B
// IDA_NAME: _Z13initPetScriptPKcS0_
// DEMANGLED: initPetScript(char const*,char const*)
bool initPetScript(const char* src, const char* filename) {
    strncpy(g_petScriptBaseDirectory, src, sizeof(g_petScriptBaseDirectory));
    g_petScriptBaseDirectory[sizeof(g_petScriptBaseDirectory) - 1] = 0;
    const size_t length = strlen(g_petScriptBaseDirectory);
    if (length != 0 &&
        g_petScriptBaseDirectory[length - 1] != '\\' &&
        g_petScriptBaseDirectory[length - 1] != '/') {
        memcpy(g_petScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_petScriptFileList,
                                  g_petScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x8A358D7
// IDA_NAME: _Z15importPetScriptP9PetScriptPKc
// DEMANGLED: importPetScript(PetScript *,char const*)
int importPetScript(PetScript* script, const char* filename) {
    if (!loadRDARScriptFile(g_petScriptBaseDirectory, filename)) {
        return 0;
    }

    script->clear();
    getDirectoryPath(script->value_6C,
                     "%s%s",
                     g_petScriptBaseDirectory,
                     filename);

    std::string tag;
    std::string text;
    int int_value = 0;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->value_04);
        } else if (tag == "[width]") {
            script->value_08 = ScanInt(static_cast<bool*>(0));
            script->value_0C = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[floating height]") {
            script->value_10 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[layer]") {
            ScanStr(&text);
            script->value_14 = getDrawLayer(text.c_str());
        } else if (tag == "[recovery time]") {
            script->value_18 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[direction correct]") {
            script->value_1C =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[move speed]") {
            script->value_20[0] = ScanInt(static_cast<bool*>(0));
            script->value_20[1] = ScanInt(static_cast<bool*>(0));
            script->value_20[2] = ScanInt(static_cast<bool*>(0));
            script->value_20[3] = ScanInt(static_cast<bool*>(0));
            script->value_30 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[gravity]") {
            script->value_34 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[basic motion]") {
            ScanStr(&script->value_38);
        } else if (tag == "[etc motion]") {
            script->value_3C.clear();
            while (ScanStr(&text) == 1) {
                script->value_3C.push_back(text);
            }
        } else if (tag == "[attack info]") {
            script->value_48.clear();
            while (ScanStr(&text) == 1) {
                script->value_48.push_back(text);
            }
        } else if (tag == "[int data]") {
            script->value_54.clear();
            while (1) {
                int_value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->value_54.push_back(int_value);
            }
        } else if (tag == "[string data]") {
            script->value_60.clear();
            while (ScanStr(&text) == 1) {
                script->value_60.push_back(text);
            }
        }
    }

    script->value_00 = g_petScriptFileList.FindIndexByFullScan(filename);
    return 1;
}
