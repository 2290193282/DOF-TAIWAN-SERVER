#include "old_data/PVF/map/datamanager/stage_map_script.h"

#include "old_game/world/region/world_stage_map_scripts.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"

#include <string.h>

#include <string>

namespace {

char g_stageMapScriptBaseDirectory[256] = {};

}  // namespace

STScriptFileList g_stageMapScriptFileList;

// IDA_EA: 0x88A9E6C
// IDA_NAME: _Z18initStageMapScriptPcS_
// DEMANGLED: initStageMapScript(char *,char *)
int initStageMapScript(char* src, char* filename) {
    strncpy(g_stageMapScriptBaseDirectory, src, 0xFFu);
    g_stageMapScriptBaseDirectory[0xFF] = 0;
    const size_t length = strlen(g_stageMapScriptBaseDirectory);
    if (length != 0 &&
        g_stageMapScriptBaseDirectory[length - 1] != '\\' &&
        g_stageMapScriptBaseDirectory[length - 1] != '/') {
        memcpy(g_stageMapScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_stageMapScriptFileList,
                                  g_stageMapScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x88A9F28
// IDA_NAME: _Z20importStageMapScriptPN8stagemap14StageMapScriptEPKc
// DEMANGLED: importStageMapScript(stagemap::StageMapScript *,char const*)
int importStageMapScript(stagemap::StageMapScript* script,
                         const char* filename) {
    if (loadRDARScriptFile(g_stageMapScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    script->clear();
    std::string tag;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->name);
        } else if (tag == "[map image]") {
            ScanStr(&script->map_image);
            script->map_image_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[dungeon]") {
            const int dungeon_index = ScanInt(&success);
            if (!success) {
                break;
            }
            script->dungeon_index = dungeon_index;
        } else if (tag == "[area]") {
            std::string area_tag;
            stagemap::AreaData area;
            area.clear();

            const int area_index = ScanInt(&success);
            if (!success) {
                break;
            }
            area.area_index = area_index;

            stagemap::DistrictData district;
            district.clear();

            while (ScanType(area_tag, true) == 1) {
                if (area_tag == "[district]") {
                    district.clear();
                    const int district_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    district.district_index = district_index;
                } else if (area_tag == "[slot image]") {
                    ScanStr(&area.area_name);
                } else if (area_tag == "[append image]") {
                    ScanStr(&district.slot_image);
                    district.raw_14 = ScanInt(static_cast<bool*>(0));
                } else if (area_tag == "[slot type]") {
                    while (1) {
                        stagemap::SlotType slot_type;
                        slot_type.clear();
                        const int slot_key = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        slot_type.slots[0] = slot_key;
                        for (int index = 1; index < 5; ++index) {
                            const int slot_value = ScanInt(&success);
                            if (!success) {
                                break;
                            }
                            slot_type.slots[index] = slot_value;
                        }
                        if (!success) {
                            break;
                        }
                        if (area.slot_types.find(slot_key) ==
                            area.slot_types.end()) {
                            area.slot_types[slot_key] = slot_type;
                        }
                    }
                } else if (area_tag == "[stage]") {
                    while (1) {
                        stagemap::StageData stage;
                        stage.clear();
                        const int stage_index = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        stage.stage_index = stage_index;
                        stage.raw_04 = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        stage.raw_08 = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        stage.raw_0c = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        district.stages.push_back(stage);
                    }
                    area.districts.push_back(district);
                } else if (area_tag == "[survival]") {
                    const int survival_stage_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    area.survival_stage_index = survival_stage_index;
                } else if (area_tag == "[/area]") {
                    script->areas.push_back(area);
                    break;
                }
            }
        }
    }

    script->script_index = g_stageMapScriptFileList.GetIndex(filename);
    return 1;
}
