#include "infra/concurrent/timer/timer_daily_schedule.h"

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/activity/schedule/event_shop_attendance.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/racing/racing_dungeon.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/pvp/tournament/pvp_tournament.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "old_game/activity/schedule/sea_bottom_event.h"


// IDA_EA: 0x812C920
// IDA_NAME: _ZN19Timer_DailySchedule12dispatch_sigEiij
// DEMANGLED: Timer_DailySchedule::dispatch_sig(int,int,unsigned int)
int Timer_DailySchedule::dispatch_sig(int /*object_id*/,
                                      int /*param1*/,
                                      unsigned int /*param2*/) {
    GlobalData::s_DailyScheduleManager->CheckAndSendToAllUserSchedule();
    // This slot is installed during startup and self-reschedules every minute.
    // Drive attendance for idle town users independently of client traffic.
    event_attendance::OnGlobalMinuteTick();
    king_of_pick::OnGlobalMinuteTick();
    freely_give_item::OnGlobalMinuteTick();
    pvp_tournament::OnPeriodicTick(
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec()));
    dual_raid::OnPeriodicTick(
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec()));
    racing_dungeon::OnPeriodicTick(
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec()));
    golden_commerce::OnPeriodicTick(
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec()));
    shooting_star_dimension_pocket::OnGlobalMinuteTick();
    sea_bottom_event::OnGlobalMinuteTick();
    RegistNextTimer();
    return 1;
}

// IDA_EA: 0x812C940
// IDA_NAME: _ZN19Timer_DailySchedule15RegistNextTimerEv
// DEMANGLED: Timer_DailySchedule::RegistNextTimer(void)
int Timer_DailySchedule::RegistNextTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 145, 60, 0, 0);
}

// IDA_EA: 0x863CEC6
// IDA_NAME: _ZN19Timer_DailyScheduleC2Ev
// DEMANGLED: Timer_DailySchedule::Timer_DailySchedule(void)
Timer_DailySchedule::Timer_DailySchedule() : TimerSig() {}

// IDA_EA: 0x812CA26 / 0x812CA56
// IDA_NAME: _ZN19Timer_DailyScheduleD2Ev / _ZN19Timer_DailyScheduleD0Ev
// DEMANGLED: Timer_DailySchedule::~Timer_DailySchedule()
Timer_DailySchedule::~Timer_DailySchedule() {}
