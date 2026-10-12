#include <algorithm>
#include <ctime>

#include "common/manager/global_data.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

int TimerUpdateItemGenerateStatistics::registNextTimer(long timer) {
    std::time_t scheduled = static_cast<std::time_t>(timer);
    struct tm local_time;
    localtime_r(&scheduled, &local_time);
    local_time.tm_hour = 23;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    scheduled = mktime(&local_time);

    const int delay = std::max(
        1,
        static_cast<int>(scheduled - GlobalData::s_systemTime_.getCurSec()));
    return G_TimerQueue()->InsertTimer(2, 0, 107, delay, 0, 0);
}
