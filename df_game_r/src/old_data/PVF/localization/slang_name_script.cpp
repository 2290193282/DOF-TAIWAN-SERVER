#include "old_data/PVF/localization/slang_name_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

// IDA_EA: 0x89081A3
// IDA_NAME: _Z15importSlangNamePcP13SlangNameData
// DEMANGLED: importSlangName(char *,SlangNameData *)
int importSlangName(char* filename, SlangNameData* data) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string slang;
    while (ScanType(tag, true) == 1) {
        if (tag == "[SLANG]") {
            while (ScanStr(&slang)) {
                data->slang_names.push_back(slang);
            }
        }

        if (tag == "[one byte skip char]") {
            while (1) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }

                const unsigned char skip_char =
                    static_cast<unsigned char>(value);
                data->one_byte_skips.push_back(skip_char);
            }
        }

        if (tag == "[two byte skip char]") {
            while (1) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }

                const unsigned short skip_char =
                    static_cast<unsigned short>(value);
                data->two_byte_skips.push_back(skip_char);
            }
        }
    }

    return 1;
}
