#include "old_data/PVF/equipment/growth_equipment_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>


// IDA_EA: 0x8372FBC
// IDA_NAME: _ZN21growthEquipmentScriptC2Ev
// DEMANGLED: growthEquipmentScript::growthEquipmentScript(void)
growthEquipmentScript::growthEquipmentScript()
    : reward_items(),
      gift_growth_weapon_box(-1),
      growth_material_item(-1),
      growth_material_item_max(0),
      event_bubble_max(0),
      event_sand_max(0) {
    reward_items.clear();
}

// IDA_EA: 0x837C3C0
// IDA_NAME: _ZN21growthEquipmentScriptD2Ev
// DEMANGLED: growthEquipmentScript::~growthEquipmentScript()
growthEquipmentScript::~growthEquipmentScript() {}

// IDA_EA: 0x895EB3F
// IDA_NAME: _Z27importGrowthEquipmentScriptP21growthEquipmentScriptPKc
// DEMANGLED: importGrowthEquipmentScript(growthEquipmentScript *,char const*)
int importGrowthEquipmentScript(growthEquipmentScript* script,
                                const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (!script) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[gift growth weapon box]") {
            script->gift_growth_weapon_box = ScanInt(&success);
            if (!success) {
                break;
            }
        }
        if (tag == "[growth material item]") {
            script->growth_material_item = ScanInt(&success);
            if (!success) {
                break;
            }
        }
        if (tag == "[growth material item max]") {
            script->growth_material_item_max =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        }
        if (tag == "[event bubble max]") {
            script->event_bubble_max =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        }
        if (tag == "[event sand max]") {
            script->event_sand_max =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                break;
            }
        }
        if (tag == "[growth equipment reward list]") {
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->reward_items.insert(std::make_pair(key, value));
            }
        }
    }

    return 1;
}
