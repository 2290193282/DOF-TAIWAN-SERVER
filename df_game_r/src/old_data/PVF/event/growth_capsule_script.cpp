#include "old_data/PVF/event/level_reward_scripts.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

// IDA_EA: 0x837114A
// IDA_NAME: _ZN21stGrowthCapsuleScriptC2Ev
// DEMANGLED: stGrowthCapsuleScript::stGrowthCapsuleScript(void)
stGrowthCapsuleScript::stGrowthCapsuleScript()
    : max_gage_exp(0),
      gage_percent(0),
      capsule_id(0),
      capsule_exp(0),
      capsule_limit_lv(0) {}

// IDA_EA: 0x891120C
// IDA_NAME: _Z25ImportGrowthCapsuleScriptPKcR21stGrowthCapsuleScript
// DEMANGLED: ImportGrowthCapsuleScript(char const*,stGrowthCapsuleScript &)
int ImportGrowthCapsuleScript(const char* filename,
                              stGrowthCapsuleScript& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 11170;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[gage percent]") {
            script.gage_percent = ScanInt(&success);
            if (!success) {
                return 11184;
            }
        } else if (tag == "[max gage exp]") {
            script.max_gage_exp = ScanInt(&success);
            if (!success) {
                return 11190;
            }
        } else if (tag == "[capsule id]") {
            script.capsule_id = ScanInt(&success);
            if (!success) {
                return 11196;
            }
        } else if (tag == "[capsule exp]") {
            script.capsule_exp = ScanInt(&success);
            if (!success) {
                return 11202;
            }
        } else if (tag == "[capsule limit lv]") {
            script.capsule_limit_lv = ScanInt(&success);
            if (!success) {
                return 11208;
            }
        }
    }

    return 0;
}
