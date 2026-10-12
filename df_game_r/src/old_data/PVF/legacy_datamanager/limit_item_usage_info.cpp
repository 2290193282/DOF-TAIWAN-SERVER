#include "old_data/PVF/equipment/datamanager/limit_item_usage_info.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>


// IDA_EA: 0x890D871
// IDA_NAME: _Z24importLimitItemUsageInfoPcP22stLimitItemUsageInfoEx
// DEMANGLED: importLimitItemUsageInfo(char *,stLimitItemUsageInfoEx *)
int importLimitItemUsageInfo(char* filename,
                             stLimitItemUsageInfoEx* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string time_text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[Limit Item Usage Info]") {
            while (1) {
                const char flag = static_cast<char>(ScanInt(&success));
                if (!success) {
                    break;
                }

                const int item_id = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                const int usage_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                info->limit_item_usage_info.Add(flag, item_id, usage_count);
            }
        } else if (tag == "[reset item]") {
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                if (ScanStr(&time_text) != 1) {
                    return 0;
                }

                long reset_time = 0;
                if (!ConvertStringToTime(time_text, reset_time)) {
                    return 0;
                }

                info->reset_item_info.Add(item_id,
                                          static_cast<int>(reset_time));
            }
        } else if (tag == "[refill item]") {
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                const int refill_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                if (ScanStr(&time_text) != 1) {
                    return 0;
                }

                long refill_time = 0;
                if (!ConvertStringToTime(time_text, refill_time)) {
                    return 0;
                }

                info->refill_item_info.Add(item_id,
                                           refill_count,
                                           static_cast<int>(refill_time));
            }
        }
    }

    return 1;
}
