#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_DATA_STATISTIC_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_DATA_STATISTIC_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/TimerDungeonDataStatistic.  This is the
// fixed table callback at TIMER_MESSAGE 102; it owns no state beyond TimerSig.
class TimerDungeonDataStatistic : public TimerSig {
public:
    // IDA_EA: 0x863C912
    // IDA_NAME: _ZN25TimerDungeonDataStatisticC2Ev
    TimerDungeonDataStatistic();

    // IDA_EA: 0x863FB22 / 0x863FB52
    // IDA_NAME: _ZN25TimerDungeonDataStatisticD2Ev
    //           _ZN25TimerDungeonDataStatisticD0Ev
    virtual ~TimerDungeonDataStatistic();

    // IDA_EA: 0x863672A
    // IDA_NAME: _ZN25TimerDungeonDataStatistic12dispatch_sigEiij
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x8636856
    // IDA_NAME: _ZN25TimerDungeonDataStatistic15RegistNextTimerEv
    int RegistNextTimer();
};

#if defined(__i386__)
static_assert(sizeof(TimerDungeonDataStatistic) == 0x04u,
              "TimerDungeonDataStatistic must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_DATA_STATISTIC_RUNTIME_H_
