#include "old_data/PVF/common/arad_script_data.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

namespace ARAD {
namespace SCRIPT {

// IDA_EA: 0x837170A
// IDA_NAME: _ZN4ARAD6SCRIPT21AradJumpingScriptDataC2Ev
// DEMANGLED: ARAD::SCRIPT::AradJumpingScriptData::AradJumpingScriptData(void)
AradJumpingScriptData::AradJumpingScriptData() {}

// IDA_EA: 0x837171E
// IDA_NAME: _ZN4ARAD6SCRIPT21AradJumpingScriptDataD2Ev
// DEMANGLED: ARAD::SCRIPT::AradJumpingScriptData::~AradJumpingScriptData()
AradJumpingScriptData::~AradJumpingScriptData() {}

// IDA_EA: 0x8371732
// IDA_NAME: _ZN4ARAD6SCRIPT18AradJumping_ScriptC2Ev
// DEMANGLED: ARAD::SCRIPT::AradJumping_Script::AradJumping_Script(void)
AradJumping_Script::AradJumping_Script() {}

// IDA_EA: 0x8371746
// IDA_NAME: _ZN4ARAD6SCRIPT18AradJumping_ScriptD2Ev
// DEMANGLED: ARAD::SCRIPT::AradJumping_Script::~AradJumping_Script()
AradJumping_Script::~AradJumping_Script() {}

// IDA_EA: 0x88B4128
// IDA_NAME: _ZN4ARAD6SCRIPT18AradJumping_Script12importScriptEPcRS1_
// DEMANGLED: ARAD::SCRIPT::AradJumping_Script::importScript(char *,ARAD::SCRIPT::AradJumping_Script&)
int AradJumping_Script::importScript(char* filename,
                                     AradJumping_Script& script) {
    std::string type;
    bool success = false;
    if (!loadRDARScriptFile("", filename)) {
        return 1;
    }

    AradJumping_Script* data = script.getRDARScriptData();
    while (ScanType(type, true) == 1) {
        if (type == "[Jumping Gift]") {
            while (true) {
                _AradJumpingRewardItem item;
                item.charac_job = ScanInt(&success);
                if (!success) {
                    break;
                }
                item.growth_value = ScanInt(&success);
                if (!success) {
                    break;
                }
                item.reward_item_index =
                    static_cast<unsigned long>(ScanInt(&success));
                if (!success) {
                    break;
                }
                data->push_back(item);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x88B424A
// IDA_NAME: _ZN4ARAD6SCRIPT18AradJumping_Script16getCurRewardItemEiiRm
// DEMANGLED: ARAD::SCRIPT::AradJumping_Script::getCurRewardItem(int,int,unsigned long &)
bool AradJumping_Script::getCurRewardItem(int charac_job,
                                          int growth_value,
                                          unsigned long& reward_item_index) {
    for (std::vector<_AradJumpingRewardItem>::iterator it = begin();
         it != end();
         ++it) {
        if (it->charac_job == charac_job && it->growth_value == growth_value) {
            reward_item_index = it->reward_item_index;
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x88B43EE
// IDA_NAME: _ZN4ARAD6SCRIPT18AradJumping_Script17getRDARScriptDataEv
// DEMANGLED: ARAD::SCRIPT::AradJumping_Script::getRDARScriptData(void)
AradJumping_Script* AradJumping_Script::getRDARScriptData() {
    return this;
}

// IDA_EA: 0x88B597E
// IDA_NAME: _ZN4ARAD6SCRIPT22_AradJumpingRewardItemD2Ev
// DEMANGLED: ARAD::SCRIPT::_AradJumpingRewardItem::~_AradJumpingRewardItem()
_AradJumpingRewardItem::~_AradJumpingRewardItem() {}

}  // namespace SCRIPT
}  // namespace ARAD
