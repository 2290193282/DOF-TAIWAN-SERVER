#include "old_data/PVF/localization/slang_list_script.h"

#include "old_data/PVF/common/rdar_script.h"


// IDA_EA: 0x88FFAA9
// IDA_NAME: _Z15importSlangListPcPSt6vectorISsSaISsEE
// DEMANGLED: importSlangList(char *,std::vector<std::string,std::allocator<std::string>> *)
int importSlangList(char* filename, std::vector<std::string>* slang_list) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string first;
    std::string second;
    while (ScanType(tag, true) == 1) {
        if (tag == "[SLANG]") {
            while (ScanStr(&first) && ScanStr(&second)) {
                slang_list->push_back(first);
                slang_list->push_back(second);
            }
        }
    }

    return 1;
}
