#include "old_data/PVF/data/datamanager/db_log_item.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string.h>

#include <string>


typedef char stDBLogItem_Size_Must_Be_0x0c[
    (sizeof(stDBLogItem) == 0x0c) ? 1 : -1];

// IDA_EA: 0x893EEA0
// IDA_NAME: _ZN11stDBLogItemD2Ev
// DEMANGLED: stDBLogItem::~stDBLogItem()
stDBLogItem::~stDBLogItem() {}

// IDA_EA: 0x890DBBA
// IDA_NAME: _Z15importDBLogItemPcPSt6vectorI11stDBLogItemSaIS1_EE
// DEMANGLED: importDBLogItem(char *,std::vector<stDBLogItem,std::allocator<stDBLogItem>> *)
int importDBLogItem(char* filename, std::vector<stDBLogItem>* items) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string time_text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[dblog buyitem]") {
            while (1) {
                stDBLogItem item;
                memset(&item, 0, sizeof(item));
                item.item_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                if (ScanStr(&time_text) != 1) {
                    return 0;
                }

                long expire_time = 0;
                if (!ConvertStringToTime(time_text, expire_time)) {
                    return 0;
                }

                item.expire_time = static_cast<int32_t>(expire_time);
                item.log_type = 0;
                items->push_back(item);
            }
        }

        if (ScanType(tag, true) != 1) {
            break;
        }

        if (tag == "[dblog getitem]") {
            while (1) {
                stDBLogItem item;
                memset(&item, 0, sizeof(item));
                item.item_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                if (ScanStr(&time_text) != 1) {
                    return 0;
                }

                long expire_time = 0;
                if (!ConvertStringToTime(time_text, expire_time)) {
                    return 0;
                }

                item.expire_time = static_cast<int32_t>(expire_time);
                item.log_type = 1;
                items->push_back(item);
            }
        }
    }

    return 1;
}
