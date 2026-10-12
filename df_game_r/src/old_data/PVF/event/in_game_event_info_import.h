#ifndef OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_INFO_IMPORT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_INFO_IMPORT_H_

struct InGameEventInfoList;

// IDA_EA: 0x897EB59
int importInGameEventInfoList(InGameEventInfoList* info_list,
                              const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_INFO_IMPORT_H_
