#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_CHANNEL_LIST_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_CHANNEL_LIST_H_

#include "common/manager/live_server_info_t.h"

#include <map>
#include <vector>


int importPVPChannelList(char* filename,
                         live_server_info_t* live_server_info,
                         int value);

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_CHANNEL_LIST_H_
