#ifndef OLDSRV_DATAMANAGER_LIMIT_ITEM_USAGE_INFO_H_
#define OLDSRV_DATAMANAGER_LIMIT_ITEM_USAGE_INFO_H_

#include "old_game/economy/inventory/limit_item_usage_info_abi.h"


// IDA_EA: 0x890D871
// IDA_NAME: _Z24importLimitItemUsageInfoPcP22stLimitItemUsageInfoEx
// DEMANGLED: importLimitItemUsageInfo(char *,stLimitItemUsageInfoEx *)
int importLimitItemUsageInfo(char* filename,
                             stLimitItemUsageInfoEx* info);

#endif  // OLDSRV_DATAMANAGER_LIMIT_ITEM_USAGE_INFO_H_
