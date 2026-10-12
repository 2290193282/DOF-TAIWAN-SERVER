#ifndef OLDSRV_DATAMANAGER_DAILY_SCHEDULE_SCRIPT_H_
#define OLDSRV_DATAMANAGER_DAILY_SCHEDULE_SCRIPT_H_

#include "infra/concurrent/timer/daily_schedule_manager.h"

#include <vector>


// IDA_EA: importDailyScheduleEtcScript @0x891826C.
int importDailyScheduleEtcScript(const char* filename,
                                 std::vector<_DailySchedule>& schedules);

#endif  // OLDSRV_DATAMANAGER_DAILY_SCHEDULE_SCRIPT_H_
