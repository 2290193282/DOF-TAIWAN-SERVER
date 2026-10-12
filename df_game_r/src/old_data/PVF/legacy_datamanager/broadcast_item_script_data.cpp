#include "old_data/PVF/equipment/datamanager/broadcast_item_script_data.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <algorithm>
#include <string>


typedef char stBroadCastItemScript_Size_Must_Be_0x24[
    (sizeof(stBroadCastItemScript) == 0x24) ? 1 : -1];

// IDA_EA: 0x8371694
// IDA_NAME: _ZN21stBroadCastItemScriptC2Ev
// DEMANGLED: stBroadCastItemScript::stBroadCastItemScript(void)
stBroadCastItemScript::stBroadCastItemScript()
    : common_items(), uncommon_items() {
    common_items.clear();
    uncommon_items.clear();
}

// IDA_EA: 0x837DE92
// IDA_NAME: _ZN21stBroadCastItemScriptD2Ev
// DEMANGLED: stBroadCastItemScript::~stBroadCastItemScript()
stBroadCastItemScript::~stBroadCastItemScript() {}

// IDA_EA: 0x8918CDC
// IDA_NAME: _ZNK21stBroadCastItemScript15checkCommonItemEi
// DEMANGLED: stBroadCastItemScript::checkCommonItem(int)const
int stBroadCastItemScript::checkCommonItem(int32_t item_id) const {
    return std::find(common_items.begin(), common_items.end(), item_id) !=
           common_items.end();
}

// IDA_EA: 0x8918D9C
// IDA_NAME: _ZNK21stBroadCastItemScript14isUnCommonItemEi
// DEMANGLED: stBroadCastItemScript::isUnCommonItem(int)const
int stBroadCastItemScript::isUnCommonItem(int32_t item_id) const {
    return uncommon_items.find(item_id) != uncommon_items.end();
}

// IDA_EA: 0x8918DFE
// IDA_NAME: _ZNK21stBroadCastItemScript23checkUnCommonNestedItemEii
// DEMANGLED: stBroadCastItemScript::checkUnCommonNestedItem(int,int)const
int stBroadCastItemScript::checkUnCommonNestedItem(
    int32_t item_id,
    int32_t nested_item_id) const {
    std::map<int, std::vector<int> >::const_iterator found =
        uncommon_items.find(item_id);
    if (found == uncommon_items.end()) {
        return 0;
    }
    const std::vector<int>& nested_items = found->second;
    return std::find(nested_items.begin(), nested_items.end(), nested_item_id) !=
           nested_items.end();
}

// IDA_EA: 0x8918F24
// IDA_NAME: _Z25ImportBroadCastItemScriptPKcR21stBroadCastItemScript
// DEMANGLED: ImportBroadCastItemScript(char const*,stBroadCastItemScript &)
int ImportBroadCastItemScript(const char* filename,
                              stBroadCastItemScript& script) {
    if (loadRDARScriptFile("", filename)) {
        std::string tag;
        bool success = false;
        while (ScanType(tag, true) == 1) {
            if (tag == "[common items]") {
                while (1) {
                    const int item_id = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script.common_items.push_back(item_id);
                }
            } else if (tag == "[uncommon items]") {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                std::vector<int> nested_items;
                while (1) {
                    const int nested_item_id = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    nested_items.push_back(nested_item_id);
                }
                script.uncommon_items.insert(
                    std::make_pair(item_id, nested_items));
            }
        }
    }
    return 0;
}
