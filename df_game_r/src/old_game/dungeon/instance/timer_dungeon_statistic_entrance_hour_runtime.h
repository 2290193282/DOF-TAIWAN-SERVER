#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_STATISTIC_ENTRANCE_HOUR_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_STATISTIC_ENTRANCE_HOUR_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/Timer_DungeonStatisticEntrance_Hour. The
// object is a TimerSig-only vtable object in dispatcher message slot 171.
class Timer_DungeonStatisticEntrance_Hour : public TimerSig {
public:
    // IDA_EA: 0x863BC54
    // IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_HourC2Ev
    Timer_DungeonStatisticEntrance_Hour();

    // IDA_EA: 0x863E3EE / 0x863E41E
    // IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_HourD2Ev
    //           _ZN35Timer_DungeonStatisticEntrance_HourD0Ev
    virtual ~Timer_DungeonStatisticEntrance_Hour();

    // IDA_EA: 0x863B9DA
    // IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_Hour12dispatch_sigEiij
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x863B9FA
    // IDA_NAME: _ZN35Timer_DungeonStatisticEntrance_Hour15RegistNextTimerEv
    // The raw body has no instance-state accesses; this static declaration
    // keeps the exact no-argument ABI used by the old Init call.
    static int RegistNextTimer();
};

#if defined(__i386__)
static_assert(sizeof(Timer_DungeonStatisticEntrance_Hour) == 0x04u,
              "Timer_DungeonStatisticEntrance_Hour must remain TimerSig-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_STATISTIC_ENTRANCE_HOUR_RUNTIME_H_
