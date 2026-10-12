#include "old_data/PVF/map/datamanager/world_drop_script.h"

#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>


// IDA_EA: 0x89022BE
// IDA_NAME: _Z15ImportWorldDropP13WorldDropInfoPKc
// DEMANGLED: ImportWorldDrop(WorldDropInfo *,char const*)
int ImportWorldDrop(WorldDropInfo* world_drop_info, const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "ImportWorldDrop",
                              0xD36,
                              "ERROR : %s ",
                              filename);
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[world drop]") {
            while (1) {
                const int world_drop_index = ScanInt(&success);
                if (!success) {
                    break;
                }

                WorldDropInfo* info = world_drop_info + world_drop_index;
                info->drop_rate_or_total_weight = ScanInt(&success);
                info->total_weight = 0;

                for (int index = 0; index <= 255; ++index) {
                    WorldDropItemChunck chunk;
                    chunk.item_id = ScanInt(&success);
                    if (chunk.item_id == -1) {
                        break;
                    }

                    chunk.weight = ScanInt(&success);
                    info->total_weight += chunk.weight;
                    info->item_chunks.push_back(chunk);
                    if (!success) {
                        break;
                    }
                }

                info->drop_rate_or_total_weight = info->total_weight;
            }
        }
    }

    return 1;
}
