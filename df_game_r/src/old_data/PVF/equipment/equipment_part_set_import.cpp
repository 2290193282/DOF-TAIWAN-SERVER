#include "old_game/economy/equipment/equipment_part_set_public_abi.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "infra/log/format/trace.h"

#include <cstdio>
#include <new>
#include <string.h>
#include <string>

namespace {

const char* const kEquipmentTypeTags[oldsrv::config::kEquipmentTypeCount] = {
    "[hat avatar]", "[hair avatar]", "[face avatar]", "[coat avatar]",
    "[pants avatar]", "[shoes avatar]", "[breast avatar]", "[waist avatar]",
    "[skin avatar]", "[aurora avatar]", "[weapon avatar]", "[weapon]",
    "[title name]", "[coat]", "[shoulder]", "[pants]", "[shoes]",
    "[waist]", "[amulet]", "[wrist]", "[ring]", "[support]",
    "[magic stone]", "[creature]", "[artifact red]", "[artifact blue]",
    "[artifact green]",
};

}  // namespace

// IDA_EA: 0x8907416
// IDA_NAME: _Z26importEquipmentPartSetInfoPKc16ENUM_MODULE_TYPE
// DEMANGLED: importEquipmentPartSetInfo(char const*, ENUM_MODULE_TYPE)
int importEquipmentPartSetInfo(const char* filename, int module_type) {
    if (loadRDARScriptFile("", filename) != 1) {
        LogManager::logFormat(0, "../RDARScriptEtcParameter.cpp",
                              "importEquipmentPartSetInfo", 0x168B,
                              "ERROR : %s", filename);
        return 0;
    }

    std::string tag;
    std::string script_filename;
    std::string part_name;
    std::string part_type;
    g_equipmentPartSetMap_.clear();

    while (ScanType(tag, true) == 1) {
        if (tag != "[equipment part set]") {
            continue;
        }

        STEquipmentPartSetData data;
        data.set_id = ScanInt(static_cast<bool*>(0));
        ScanStr(&script_filename);
        data.script_filename = script_filename;
        while (1) {
            STEquipmentPartSetPiece piece;
            if (ScanStr(&part_name) != 1) {
                break;
            }
            piece.part_name = part_name;
            ScanStr(&part_type);
            piece.part_type = -1;
            for (int index = 0;
                 index < oldsrv::config::kEquipmentTypeCount;
                 ++index) {
                if (strcmp(part_type.c_str(), kEquipmentTypeTags[index]) == 0) {
                    piece.part_type = index;
                    break;
                }
            }
            piece.part_grade = ScanInt(static_cast<bool*>(0));
            piece.part_rarity = ScanInt(static_cast<bool*>(0));
            data.pieces.push_back(piece);
            g_equipmentPartSetMap_[data.set_id] = data;
        }
    }

    for (std::map<int, STEquipmentPartSetData>::iterator it =
             g_equipmentPartSetMap_.begin();
         it != g_equipmentPartSetMap_.end(); ++it) {
        STEquipmentScript* script =
            static_cast<STEquipmentScript*>(
                ::operator new(sizeof(STEquipmentScript)));
        new (script) STEquipmentScript();
        it->second.script = script;
        if (ImportEquipmentScript(script, it->second.script_filename.c_str(), false,
                                  module_type, it->first) != 1) {
            std::fprintf(
                stderr,
                "[equipment-part-set][IMPORT_FAIL] set_id=%d file=%s\n",
                it->first,
                it->second.script_filename.c_str());
            LogManager::logFormat(0, "../RDARScriptEtcParameter.cpp",
                                  "importEquipmentPartSetInfo", 0x16C2,
                                  "ERROR : ImportEquipmentScript() fail file - %s",
                                  it->second.script_filename.c_str());
            script->~STEquipmentScript();
            ::operator delete(script);
            it->second.script = 0;
            return 0;
        }

        // IDA_EA: importEquipmentPartSetInfo @0x89073D9,
        // 0x89076B8..0x890786A.  A legacy set script may publish its only
        // parameter block under the -1 sentinel.  The official importer
        // snapshots that block, clears the set map, and republishes it at the
        // actual number of pieces in this part-set definition.  This is a
        // replacement (not an accumulation) and must happen before the map is
        // exposed to the DataManager DB synchronizer.
        std::map<int, EquipmentParameterInfo>::iterator legacy_parameter =
            script->set_parameters_678.find(-1);
        if (legacy_parameter != script->set_parameters_678.end()) {
            EquipmentParameterInfo normalized_parameter(
                legacy_parameter->second);
            script->set_parameters_678.clear();
            script->set_parameters_678[
                static_cast<int>(it->second.pieces.size())] =
                normalized_parameter;
        }
    }
    return 1;
}
