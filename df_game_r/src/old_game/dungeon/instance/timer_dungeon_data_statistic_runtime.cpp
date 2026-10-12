#include "old_game/dungeon/instance/timer_dungeon_data_statistic_runtime.h"

#include <cstring>
#include <ctime>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_data/db/statistics_log/db_update_vending_machine_statistic.h"
#include "old_game/dungeon/instance/dungeon_stat_runtime.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kTimerMessageDungeonDataStatistic = 102;

}  // namespace

// IDA_EA: 0x863C912
// IDA_NAME: _ZN25TimerDungeonDataStatisticC2Ev
// DEMANGLED: TimerDungeonDataStatistic::TimerDungeonDataStatistic(void)
TimerDungeonDataStatistic::TimerDungeonDataStatistic() : TimerSig() {}

// IDA_EA: 0x863FB22
// IDA_NAME: _ZN25TimerDungeonDataStatisticD2Ev
// IDA_EA: 0x863FB52
// IDA_NAME: _ZN25TimerDungeonDataStatisticD0Ev
// DEMANGLED: TimerDungeonDataStatistic::~TimerDungeonDataStatistic()
TimerDungeonDataStatistic::~TimerDungeonDataStatistic() {}

// IDA_EA: 0x863672A
// IDA_NAME: _ZN25TimerDungeonDataStatistic12dispatch_sigEiij
// DEMANGLED: TimerDungeonDataStatistic::dispatch_sig(int,int,unsigned int)
int TimerDungeonDataStatistic::dispatch_sig(int, int, unsigned int) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("TimerDispatcher.cpp", 2842), true);
    guard << 175;
    guard << -1;
    SIG_DUNGEON_DATA_STATISTIC* const signal =
        guard.GetInBuffer<SIG_DUNGEON_DATA_STATISTIC>();
    std::memset(signal, 0, sizeof(*signal));
    signal->signal_kind = 3;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);

    GetInstanceVendingMachineStatistic()->updateDataIntoDB();
    return RegistNextTimer() == 1;
}

// IDA_EA: 0x8636856
// IDA_NAME: _ZN25TimerDungeonDataStatistic15RegistNextTimerEv
// DEMANGLED: TimerDungeonDataStatistic::RegistNextTimer(void)
int TimerDungeonDataStatistic::RegistNextTimer() {
    const time_t current_time = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    tm local_time;
    tm* const schedule = localtime_r(&current_time, &local_time);
    const int current_hour = schedule->tm_hour;
    schedule->tm_hour = 5;
    schedule->tm_min = G_CEnvironment()->get_channel_no() % 30;
    schedule->tm_sec = 0;

    time_t next_time = mktime(schedule);
    if (schedule->tm_hour <= current_hour) {
        next_time += 86400;
    }
    G_TimerQueue()->InsertTimer(2, 0, kTimerMessageDungeonDataStatistic,
                                 static_cast<int>(next_time - current_time),
                                 0, 0);
    return 1;
}
