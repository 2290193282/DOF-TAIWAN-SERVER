#include "old_game/dungeon/instance/timer_dungeon_inout_runtime.h"

#include <ctime>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kTimerMessageDungeonInoutOpen = 147;
const int kTimerMessageDungeonInoutClose = 148;
// The old open timer deliberately gates its state transition against this
// fixed ultimate-dungeon entry before setting the queued dungeon's state.
// This is not a substitute for the queued id; it is the original guard.
const int kUltimateDungeonStateGate = 0x2AFF;

const char kOpenFunction[] =
    "virtual bool Timer_DungeonInoutOpenTime::dispatch_sig(int, int, "
    "memberIdentificationNumber_t)";
const char kCloseFunction[] =
    "virtual bool Timer_DungeonInoutCloseTime::dispatch_sig(int, int, "
    "memberIdentificationNumber_t)";

const DungeonInoutTimeMap& DungeonInoutTimes() {
    return G_CDataManager()->get_server_parameter_script()
        ->get_dungeon_inout_times();
}

int DeltaOrZero(time_t target, time_t current) {
    int delta = static_cast<int>(target - current);
    if (delta < 0) {
        delta = 0;
    }
    return delta;
}

bool DungeonInoutOverrideActive() {
    return CPowerManager::IsPowerWarEventOn() ||
           GlobalData::s_villageMonsterMgr->OnEvent();
}

}  // namespace

// IDA_EA: 0x863CF36
// IDA_NAME: _ZN26Timer_DungeonInoutOpenTimeC2Ev
// DEMANGLED: Timer_DungeonInoutOpenTime::Timer_DungeonInoutOpenTime(void)
Timer_DungeonInoutOpenTime::Timer_DungeonInoutOpenTime() : TimerSig() {}

// IDA_EA: 0x863EA54
// IDA_NAME: _ZN26Timer_DungeonInoutOpenTimeD2Ev
// IDA_EA: 0x863EA84
// IDA_NAME: _ZN26Timer_DungeonInoutOpenTimeD0Ev
// DEMANGLED: Timer_DungeonInoutOpenTime::~Timer_DungeonInoutOpenTime()
Timer_DungeonInoutOpenTime::~Timer_DungeonInoutOpenTime() {}

// IDA_EA: 0x863CF52
// IDA_NAME: _ZN27Timer_DungeonInoutCloseTimeC2Ev
// DEMANGLED: Timer_DungeonInoutCloseTime::Timer_DungeonInoutCloseTime(void)
Timer_DungeonInoutCloseTime::Timer_DungeonInoutCloseTime() : TimerSig() {}

// IDA_EA: 0x863EA06
// IDA_NAME: _ZN27Timer_DungeonInoutCloseTimeD2Ev
// IDA_EA: 0x863EA36
// IDA_NAME: _ZN27Timer_DungeonInoutCloseTimeD0Ev
// DEMANGLED: Timer_DungeonInoutCloseTime::~Timer_DungeonInoutCloseTime()
Timer_DungeonInoutCloseTime::~Timer_DungeonInoutCloseTime() {}

// IDA_EA: 0x8639A12
// IDA_NAME: _ZN26Timer_DungeonInoutOpenTime21checkDungeonInoutTimeEi
// DEMANGLED: Timer_DungeonInoutOpenTime::checkDungeonInoutTime(int)
bool Timer_DungeonInoutOpenTime::checkDungeonInoutTime(int dungeon_id) {
    const time_t current =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm current_tm;
    const std::tm* const local_time = localtime_r(&current, &current_tm);

    const DungeonInoutTimeMap& times = DungeonInoutTimes();
    const DungeonInoutTimeMap::const_iterator found = times.find(dungeon_id);
    if (found == times.end()) {
        return false;
    }

    const STDungeonInoutTime& entry = found->second;
    return entry.start_hour <= local_time->tm_hour &&
           entry.end_hour >= local_time->tm_hour &&
           entry.start_minute <= local_time->tm_min &&
           entry.end_minute > local_time->tm_min;
}

// IDA_EA: 0x8639B20
// IDA_NAME: _ZN26Timer_DungeonInoutOpenTime15RegistNextTimerEv
// DEMANGLED: Timer_DungeonInoutOpenTime::RegistNextTimer(void)
int Timer_DungeonInoutOpenTime::RegistNextTimer() {
    const time_t current =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm schedule_storage;
    std::tm* const schedule = localtime_r(&current, &schedule_storage);
    const int current_hour = schedule->tm_hour;
    const int current_minute = schedule->tm_min;

    const DungeonInoutTimeMap& times = DungeonInoutTimes();
    for (DungeonInoutTimeMap::const_iterator it = times.begin();
         it != times.end();
         ++it) {
        const int dungeon_id = it->first;
        const STDungeonInoutTime& entry = it->second;
        time_t next_time;

        if (entry.start_hour <= current_hour &&
            entry.end_hour >= current_hour &&
            entry.start_minute <= current_minute &&
            entry.end_minute >= current_minute) {
            schedule->tm_hour = 0;
            schedule->tm_min = 0;
            schedule->tm_sec = 0;
            next_time = mktime(schedule);
        } else if (entry.start_hour >= current_hour ||
                   (entry.end_hour >= current_hour &&
                    entry.start_minute > current_minute) ||
                   (entry.end_hour >= current_hour &&
                    entry.end_minute < current_minute)) {
            if (entry.start_hour <= current_hour) {
                schedule->tm_hour = entry.start_minute <= current_minute
                                        ? current_hour + 1
                                        : current_hour;
            } else {
                schedule->tm_hour = entry.start_hour;
            }
            schedule->tm_min = entry.start_minute;
            schedule->tm_sec = 0;
            next_time = mktime(schedule);
        } else {
            schedule->tm_hour = entry.start_hour;
            schedule->tm_min = entry.start_minute;
            schedule->tm_sec = 0;
            next_time = mktime(schedule) + 86400;
        }

        G_TimerQueue()->InsertTimer(2, 0, kTimerMessageDungeonInoutOpen,
                                     DeltaOrZero(next_time, current), 0,
                                     dungeon_id);
    }
    return 1;
}

// IDA_EA: 0x8639E0A
// IDA_NAME: _ZN26Timer_DungeonInoutOpenTime12dispatch_sigEiij
// DEMANGLED: Timer_DungeonInoutOpenTime::dispatch_sig(int,int,unsigned int)
int Timer_DungeonInoutOpenTime::dispatch_sig(int, int,
                                             unsigned int dungeon_id) {
    const time_t current =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm local_time_storage;
    std::tm* const local_time = localtime_r(&current, &local_time_storage);
    const int current_hour = local_time->tm_hour;

    if (dungeon_id == 0) {
        return 0;
    }
    const DungeonInoutTimeMap& times = DungeonInoutTimes();
    const DungeonInoutTimeMap::const_iterator found = times.find(dungeon_id);
    if (found == times.end()) {
        return 1;
    }
    const STDungeonInoutTime& entry = found->second;

    if (entry.end_hour >= current_hour) {
        local_time->tm_hour = current_hour;
        local_time->tm_min = entry.end_minute;
        local_time->tm_sec = 59;
        const time_t close_time = mktime(local_time);
        const int delay = DeltaOrZero(close_time, current);
        cMyTrace close_trace(kOpenFunction, 5218, 0);
        close_trace("DELTA %d , CHECK_T %d , CUR_T %d\n", delay,
                    static_cast<int>(close_time),
                    static_cast<int>(current));
        G_TimerQueue()->InsertTimer(2, 0, kTimerMessageDungeonInoutClose,
                                     delay, 0, dungeon_id);
        cMyTrace close_timer_trace(kOpenFunction, 5230, 0);
        close_timer_trace("ULTIMATE_LOG : ULTIMATE DUNGEON CLOSE TIMER ON!!");

        if (!DungeonInoutOverrideActive()) {
            ServerParameterScript* const parameters =
                G_CDataManager()->get_server_parameter_script();
            if (parameters->isDungeonOpen(kUltimateDungeonStateGate) != 1) {
                parameters->setDungeonOpen(static_cast<int>(dungeon_id), true);
                G_GameWorld()->send_all_dungeon_inout_message(
                    static_cast<int>(dungeon_id));
            }
            cMyTrace open_trace(kOpenFunction, 5240, 0);
            open_trace("ULTIMATE_LOG : ULTIMATE DUNGEON OPEN!!");
        }
        return 1;
    }

    local_time->tm_hour = entry.start_hour;
    local_time->tm_min = entry.start_minute;
    local_time->tm_sec = 0;
    const time_t next_time = mktime(local_time) + 86400;
    const int delay = static_cast<int>(next_time - current);
    if (delay >= 0) {
        G_TimerQueue()->InsertTimer(2, 0, kTimerMessageDungeonInoutOpen,
                                     delay, 0, dungeon_id);
        cMyTrace next_day_trace(kOpenFunction, 5204, 0);
        next_day_trace("ULTIMATE_LOG : NEXT DAY TIMER ON!!");
        return 1;
    }

    cMyTrace off_trace(kOpenFunction, 5189, 0);
    off_trace("ULTIMATE_LOG : TIMER OFF!!");
    return 0;
}

// IDA_EA: 0x863A1D0
// IDA_NAME: _ZN27Timer_DungeonInoutCloseTime12dispatch_sigEiij
// DEMANGLED: Timer_DungeonInoutCloseTime::dispatch_sig(int,int,unsigned int)
int Timer_DungeonInoutCloseTime::dispatch_sig(int, int,
                                              unsigned int dungeon_id) {
    const time_t current =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm local_time_storage;
    std::tm* const local_time = localtime_r(&current, &local_time_storage);
    const int current_hour = local_time->tm_hour;

    if (dungeon_id == 0) {
        return 0;
    }
    const DungeonInoutTimeMap& times = DungeonInoutTimes();
    const DungeonInoutTimeMap::const_iterator found = times.find(dungeon_id);
    if (found == times.end()) {
        return 1;
    }
    const STDungeonInoutTime& entry = found->second;

    local_time->tm_hour = current_hour + 1;
    local_time->tm_min = entry.start_minute;
    local_time->tm_sec = 0;
    const time_t next_time = mktime(local_time);
    const int delay = static_cast<int>(next_time - current);
    if (delay < 0) {
        cMyTrace off_trace(kCloseFunction, 5283, 0);
        off_trace("ULTIMATE_LOG : TIMER OFF!!");
        return 0;
    }

    G_TimerQueue()->InsertTimer(2, 0, kTimerMessageDungeonInoutOpen, delay,
                                 0, dungeon_id);
    cMyTrace open_timer_trace(kCloseFunction, 5297, 0);
    open_timer_trace("ULTIMATE_LOG : ULTIMATE DUNGEON OPEN TIMER ON!!");

    if (!DungeonInoutOverrideActive()) {
        ServerParameterScript* const parameters =
            G_CDataManager()->get_server_parameter_script();
        parameters->setDungeonOpen(static_cast<int>(dungeon_id), false);
        G_GameWorld()->send_all_dungeon_inout_message(
            static_cast<int>(dungeon_id));
        cMyTrace close_trace(kCloseFunction, 5304, 0);
        close_trace("ULTIMATE_LOG : ULTIMATE DUNGEON CLOSE!!");
    }
    return 1;
}
