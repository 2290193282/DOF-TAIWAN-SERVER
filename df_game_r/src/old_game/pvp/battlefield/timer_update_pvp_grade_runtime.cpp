#include "old_game/pvp/battlefield/timer_update_pvp_grade_runtime.h"

#include <ctime>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_data/db/pvp_party/db_update_pvp_grade.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kTimerMessageUpdatePvPGrade = 108;

}  // namespace

// IDA_EA: 0x863CA0E
// IDA_NAME: _ZN19TimerUpdatePvPGradeC2Ev
// DEMANGLED: TimerUpdatePvPGrade::TimerUpdatePvPGrade(void)
TimerUpdatePvPGrade::TimerUpdatePvPGrade() : TimerSig() {}

// IDA_EA: 0x863F864
// IDA_NAME: _ZN19TimerUpdatePvPGradeD2Ev
// IDA_EA: 0x863F894
// IDA_NAME: _ZN19TimerUpdatePvPGradeD0Ev
// DEMANGLED: TimerUpdatePvPGrade::~TimerUpdatePvPGrade()
TimerUpdatePvPGrade::~TimerUpdatePvPGrade() {}

// IDA_EA: 0x86373C6
// IDA_NAME: _ZN19TimerUpdatePvPGrade12dispatch_sigEiij
// DEMANGLED: TimerUpdatePvPGrade::dispatch_sig(int,int,unsigned int)
int TimerUpdatePvPGrade::dispatch_sig(int, int, unsigned int) {
    DB_UpdatePvPGrade::makeRequest();
    registNextTimer(static_cast<time_t>(
                         GlobalData::s_systemTime_.getCurSec()) +
                     86400);
    return 1;
}

// IDA_EA: 0x86373F2
// IDA_NAME: _ZN19TimerUpdatePvPGrade15registNextTimerEl
// DEMANGLED: TimerUpdatePvPGrade::registNextTimer(long)
int TimerUpdatePvPGrade::registNextTimer(time_t timer) {
    if (G_CEnvironment()->get_channel_no() != 1) {
        return 1;
    }

    std::tm local_time;
    localtime_r(&timer, &local_time);
    local_time.tm_hour = 2;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    const time_t target = mktime(&local_time);
    const int delay = target -
                      static_cast<time_t>(
                          GlobalData::s_systemTime_.getCurSec()) >
                          0
                          ? static_cast<int>(target -
                                             GlobalData::s_systemTime_
                                                 .getCurSec())
                          : 1;
    return G_TimerQueue()->InsertTimer(2, 0, kTimerMessageUpdatePvPGrade,
                                       delay, 0, 0);
}
