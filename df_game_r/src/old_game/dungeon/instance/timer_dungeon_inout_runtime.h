#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_INOUT_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_INOUT_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/Timer_DungeonInoutOpenTime. Both timer
// classes are TimerSig-only instances placed in TimerDispatcher slots 147/148.
class Timer_DungeonInoutOpenTime : public TimerSig {
public:
    // IDA_EA: 0x863CF36
    // IDA_NAME: _ZN26Timer_DungeonInoutOpenTimeC2Ev
    Timer_DungeonInoutOpenTime();

    // IDA_EA: 0x863EA54 / 0x863EA84
    // IDA_NAME: _ZN26Timer_DungeonInoutOpenTimeD2Ev
    //           _ZN26Timer_DungeonInoutOpenTimeD0Ev
    virtual ~Timer_DungeonInoutOpenTime();

    // IDA_EA: 0x8639E0A
    // IDA_NAME: _ZN26Timer_DungeonInoutOpenTime12dispatch_sigEiij
    virtual int dispatch_sig(int object_id, int param1, unsigned int dungeon_id);

    // IDA_EA: 0x8639A12
    // IDA_NAME: _ZN26Timer_DungeonInoutOpenTime21checkDungeonInoutTimeEi
    static bool checkDungeonInoutTime(int dungeon_id);

    // IDA_EA: 0x8639B20
    // IDA_NAME: _ZN26Timer_DungeonInoutOpenTime15RegistNextTimerEv
    static int RegistNextTimer();
};

class Timer_DungeonInoutCloseTime : public TimerSig {
public:
    // IDA_EA: 0x863CF52
    // IDA_NAME: _ZN27Timer_DungeonInoutCloseTimeC2Ev
    Timer_DungeonInoutCloseTime();

    // IDA_EA: 0x863EA06 / 0x863EA36
    // IDA_NAME: _ZN27Timer_DungeonInoutCloseTimeD2Ev
    //           _ZN27Timer_DungeonInoutCloseTimeD0Ev
    virtual ~Timer_DungeonInoutCloseTime();

    // IDA_EA: 0x863A1D0
    // IDA_NAME: _ZN27Timer_DungeonInoutCloseTime12dispatch_sigEiij
    virtual int dispatch_sig(int object_id, int param1, unsigned int dungeon_id);
};

#if defined(__i386__)
static_assert(sizeof(Timer_DungeonInoutOpenTime) == 0x04u,
              "open-time timer must remain TimerSig-only");
static_assert(sizeof(Timer_DungeonInoutCloseTime) == 0x04u,
              "close-time timer must remain TimerSig-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_DUNGEON_INOUT_RUNTIME_H_
