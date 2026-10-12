#ifndef OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_SCRIPT_IMPORT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_SCRIPT_IMPORT_H_

struct AttendanceScript;
struct ExchangeRandomItemRewardScript;

// IDA_EA: 0x8981539
int importAttendanceScript(AttendanceScript* script,
                           const char* filename);

// IDA_EA: 0x8981E0F
int importExchangeRandomItemRewardScript(
    ExchangeRandomItemRewardScript* script,
    const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_IN_GAME_EVENT_SCRIPT_IMPORT_H_
