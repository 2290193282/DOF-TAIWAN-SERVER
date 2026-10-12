#include "old_data/PVF/town/datamanager/village_attacked_script.h"

#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>


// IDA_EA: 0x89093EE
// IDA_NAME: _Z27importVillageAttackedScriptPKcR19stVillageAttacked_t
// DEMANGLED: importVillageAttackedScript(char const*,stVillageAttacked_t &)
int importVillageAttackedScript(const char* filename,
                                stVillageAttacked_t& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 6527;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[attacked map info]") {
            while (1) {
                stAttackedMapInfo_t row;
                row.village = ScanInt(&success);
                if (!success) {
                    break;
                }

                row.area = ScanInt(&success);
                if (!success) {
                    return 6544;
                }

                row.monster_group = ScanInt(&success);
                if (!success) {
                    return 6548;
                }

                row.reward_group = ScanInt(&success);
                if (!success) {
                    return 6552;
                }

                script.push_back(row);
            }
        }
    }

    return 0;
}
