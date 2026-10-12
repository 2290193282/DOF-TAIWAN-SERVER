#include "old_data/PVF/data/datamanager/in_game_advertisement_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <algorithm>
#include <string>

// IDA_EA: 0x83732F0
// IDA_NAME: _ZN25InGameAdvertisementScriptC2Ev
// DEMANGLED: InGameAdvertisementScript::InGameAdvertisementScript(void)
InGameAdvertisementScript::InGameAdvertisementScript() { clear(); }

// IDA_EA: 0x8373304
// IDA_NAME: _ZN25InGameAdvertisementScript5clearEv
// DEMANGLED: InGameAdvertisementScript::clear(void)
void* InGameAdvertisementScript::clear() {
    std::fill(age_condition_flags,
              age_condition_flags + sizeof(age_condition_flags),
              static_cast<uint8_t>(1));
    return this;
}

// IDA_EA: 0x897E9C4
// IDA_NAME: _Z34importInGameAdvertisementEtcScriptP25InGameAdvertisementScriptPKc
// DEMANGLED: importInGameAdvertisementEtcScript(InGameAdvertisementScript *,char const*)
int importInGameAdvertisementEtcScript(InGameAdvertisementScript* script,
                                       const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script->clear();
    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[in game advertisement expose condition of teen]") {
            while (1) {
                const int index = ScanInt(&success);
                if (!success || index > 150) {
                    break;
                }
                script->age_condition_flags[index] = 0;
            }
        }
    }
    return 1;
}
