#ifndef OLDSRV_DATAMANAGER_DB_LOG_ITEM_H_
#define OLDSRV_DATAMANAGER_DB_LOG_ITEM_H_

#include <stdint.h>

#include <vector>


// IDA_EA: 0x893EEA0
// IDA unit: 99_misc_global/stDBLogItem plus the B-owned DataManager/PVF
// import call from CDataManager::init.
struct stDBLogItem {
    ~stDBLogItem();

    int32_t item_id;
    int32_t expire_time;
    int32_t log_type;
};

// IDA_EA: 0x890DBBA
// IDA_NAME: _Z15importDBLogItemPcPSt6vectorI11stDBLogItemSaIS1_EE
// DEMANGLED: importDBLogItem(char *,std::vector<stDBLogItem,std::allocator<stDBLogItem>> *)
int importDBLogItem(char* filename, std::vector<stDBLogItem>* items);

#endif  // OLDSRV_DATAMANAGER_DB_LOG_ITEM_H_
