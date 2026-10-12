#include "old_game/player/core/timer_insert_user_play_time_runtime.h"

#include <ctime>

#include "common/manager/global_data.h"
#include "old_game/player/core/user_play_time_statistic_runtime.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kTimerMessageInsertUserPlayTime = 105;

}  // namespace

// IDA_EA: 0x863C966
// IDA_NAME: _ZN23TimerInsertUserPlayTimeC2Ev
// DEMANGLED: TimerInsertUserPlayTime::TimerInsertUserPlayTime(void)
TimerInsertUserPlayTime::TimerInsertUserPlayTime() : TimerSig() {}

// IDA_EA: 0x863FA38
// IDA_NAME: _ZN23TimerInsertUserPlayTimeD2Ev
// IDA_EA: 0x863FA68
// IDA_NAME: _ZN23TimerInsertUserPlayTimeD0Ev
// DEMANGLED: TimerInsertUserPlayTime::~TimerInsertUserPlayTime()
TimerInsertUserPlayTime::~TimerInsertUserPlayTime() {}

// IDA_EA: 0x8636D0A
// IDA_NAME: _ZN23TimerInsertUserPlayTime12dispatch_sigEiij
// DEMANGLED: TimerInsertUserPlayTime::dispatch_sig(int,int,unsigned int)
int TimerInsertUserPlayTime::dispatch_sig(int, int, unsigned int) {
    GetInstanceUserPlayTimeStatisticMgr()->InsertUserPlayTime();
    return RegistNextTimer() == 1;
}

// IDA_EA: 0x8636D44
// IDA_NAME: _ZN23TimerInsertUserPlayTime15RegistNextTimerEv
// DEMANGLED: TimerInsertUserPlayTime::RegistNextTimer(void)
int TimerInsertUserPlayTime::RegistNextTimer() {
    const time_t current_time = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    tm local_time;
    tm* const schedule = localtime_r(&current_time, &local_time);
    const int current_hour = schedule->tm_hour;
    schedule->tm_hour = 4;
    schedule->tm_min = 30;
    schedule->tm_sec = 0;

    time_t next_time = mktime(schedule);
    if (schedule->tm_hour <= current_hour) {
        next_time += 86400;
    }

    G_TimerQueue()->InsertTimer(
        2,
        0,
        kTimerMessageInsertUserPlayTime,
        static_cast<int>(next_time - current_time),
        0,
        0);
    return 1;
}
