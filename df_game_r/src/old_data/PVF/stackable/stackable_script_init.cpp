#include "old_data/PVF/stackable/stackable_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string.h>

#include <string>

char g_szStackableScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
int STStackableScript::priceTable_[3000];

// IDA_EA: 0x89AE72E
// IDA_NAME: _Z19InitStackableScriptPKcS0_S0_
// DEMANGLED: InitStackableScript(char const*,char const*,char const*)
int InitStackableScript(const char* src,
                        const char* list_file,
                        const char* price_table) {
    strncpy(g_szStackableScriptBaseDirectory, src, 0x100u);
    if (g_szStackableScriptBaseDirectory[
                strlen(g_szStackableScriptBaseDirectory) - 1] != '\\' &&
        g_szStackableScriptBaseDirectory[
                strlen(g_szStackableScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_szStackableScriptBaseDirectory);
        memcpy(g_szStackableScriptBaseDirectory + length, "\\", 2u);
    }
    if (loadRDARScriptListFile(&g_stStackableScriptFileList,
                               g_szStackableScriptBaseDirectory,
                               list_file) != 1) {
        return 0;
    }
    if (!loadRDARScriptFile(g_szStackableScriptBaseDirectory, price_table)) {
        return 0;
    }

    for (int row = 0; row <= 29; ++row) {
        for (int column = 0; column <= 99; ++column) {
            STStackableScript::priceTable_[row * 100 + column] = 0;
        }
    }

    std::string tag;
    for (int row = 0; row <= 29 && ScanType(tag, true) == 1; ++row) {
        for (int column = 0; column <= 99; ++column) {
            STStackableScript::priceTable_[row * 100 + column] =
                ScanInt(static_cast<bool*>(0));
        }
    }
    return 1;
}
