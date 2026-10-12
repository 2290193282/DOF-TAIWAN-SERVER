#include "old_game/dungeon/instance/timer_dungeon_statistic_entrance_hour_runtime.h"

#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const int kTimerMessageDungeonStatisticEntranceHour = 171;

}  // namespace

// IDA_EA: 0x863BC54
// IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_HourC2Ev
// DEMANGLED: Timer_DungeonStatisticEntrance_Hour::Timer_DungeonStatisticEntrance_Hour(void)
Timer_DungeonStatisticEntrance_Hour::Timer_DungeonStatisticEntrance_Hour()
    : TimerSig() {}

// IDA_EA: 0x863E3EE
// IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_HourD2Ev
// IDA_EA: 0x863E41E
// IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_HourD0Ev
// DEMANGLED: Timer_DungeonStatisticEntrance_Hour::~Timer_DungeonStatisticEntrance_Hour()
Timer_DungeonStatisticEntrance_Hour::~Timer_DungeonStatisticEntrance_Hour() {}

// IDA_EA: 0x863B9DA
// IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_Hour12dispatch_sigEiij
// DEMANGLED: Timer_DungeonStatisticEntrance_Hour::dispatch_sig(int,int,unsigned int)
int Timer_DungeonStatisticEntrance_Hour::dispatch_sig(int, int, unsigned int) {
    GetInstanceDungeonEntranceLog()->InsertDungeonEntrance_Hour();
    RegistNextTimer();
    return 1;
}

// IDA_EA: 0x863B9FA
// IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_Hour15RegistNextTimerEv
// DEMANGLED: Timer_DungeonStatisticEntrance_Hour::RegistNextTimer(void)
int Timer_DungeonStatisticEntrance_Hour::RegistNextTimer() {
    return G_TimerQueue()->InsertTimer(
        2, 0, kTimerMessageDungeonStatisticEntranceHour, 3600, 0, 0);
}
