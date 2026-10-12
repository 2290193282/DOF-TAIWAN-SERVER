#include "old_data/PVF/event/level_reward_scripts.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

// IDA_EA: 0x89180EA
// IDA_NAME: _Z28ImportReturnUserRewardScriptPKcR24stReturnUserRewardScript
// DEMANGLED: ImportReturnUserRewardScript(char const*,stReturnUserRewardScript &)
int ImportReturnUserRewardScript(const char* filename,
                                 stReturnUserRewardScript& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 14004;
    }

    std::string tag;
    int result = 0;
    script.level_up_rewards.clear();

    while (ScanType(tag, true) == 1) {
        if (tag == "[pacakege item]") {
            int value = 0;
            (void)ScanInt(&value);
            script.first_dword = static_cast<uint32_t>(value);
            (void)ScanInt(&value);
            script.second_dword = static_cast<uint32_t>(value);
        } else if (tag == "[reward equip item]") {
            result = LoadRewardDataByGrowType(
                script.level_up_rewards,
                std::string("[/reward equip item]"));
            if (result) {
                return result;
            }
        }
    }

    return 0;
}
