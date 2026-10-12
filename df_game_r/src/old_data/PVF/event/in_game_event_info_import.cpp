#include "old_data/PVF/event/in_game_event_info_import.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>

// IDA_EA: 0x897EB59
// IDA_NAME: _Z25importInGameEventInfoListP19InGameEventInfoListPKc
// DEMANGLED: importInGameEventInfoList(InGameEventInfoList *,char const*)
int importInGameEventInfoList(InGameEventInfoList* info_list,
                              const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    InGameEventInfo event_info;
    bool saw_event_list = false;
    bool in_event = false;
    unsigned int field_mask = 0u;
    while (ScanType(tag, true) == 1) {
        if (tag == "[event_list]") {
            if (saw_event_list || in_event) {
                return 0;
            }
            saw_event_list = true;
            continue;
        }
        if (tag == "[/event_list]") {
            return saw_event_list && !in_event ? 1 : 0;
        }
        if (tag == "[event]") {
            if (!saw_event_list || in_event) {
                return 0;
            }
            in_event = true;
            field_mask = 0u;
            event_info.first_dword = 0u;
            event_info.second_dword = 0u;
            event_info.event_name.clear();
            event_info.third_dword = 0u;
            event_info.fourth_dword = 0u;
            event_info.fifth_dword = 0u;
            continue;
        }
        if (!in_event) {
            return 0;
        }
        if (tag == "[idx]") {
            if ((field_mask & 0x01u) != 0u) {
                return 0;
            }
            int value = 0;
            if (!ScanInt(&value) || ScanStr(&event_info.event_name) != 1) {
                return 0;
            }
            event_info.first_dword = static_cast<uint32_t>(value);
            field_mask |= 0x01u;
        } else if (tag == "[record type]") {
            if ((field_mask & 0x02u) != 0u) {
                return 0;
            }
            int value = 0;
            if (!ScanInt(&value)) {
                return 0;
            }
            event_info.second_dword = static_cast<uint32_t>(value);
            field_mask |= 0x02u;
        } else if (tag == "[start]") {
            if ((field_mask & 0x04u) != 0u) {
                return 0;
            }
            int value = 0;
            if (!ScanInt(&value)) {
                return 0;
            }
            event_info.third_dword = static_cast<uint32_t>(value);
            field_mask |= 0x04u;
        } else if (tag == "[end]") {
            if ((field_mask & 0x08u) != 0u) {
                return 0;
            }
            int value = 0;
            if (!ScanInt(&value)) {
                return 0;
            }
            event_info.fourth_dword = static_cast<uint32_t>(value);
            field_mask |= 0x08u;
        } else if (tag == "[state]") {
            if ((field_mask & 0x10u) != 0u) {
                return 0;
            }
            int value = 0;
            if (!ScanInt(&value)) {
                return 0;
            }
            event_info.fifth_dword = static_cast<uint32_t>(value);
            field_mask |= 0x10u;
        } else if (tag == "[/event]") {
            if (field_mask != 0x1fu) {
                return 0;
            }
            info_list->insertEventInfo(
                static_cast<int>(event_info.second_dword), event_info);
            in_event = false;
        } else {
            return 0;
        }
    }

    return 0;
}
