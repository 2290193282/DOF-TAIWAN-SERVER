#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_FAIR_PVP_REMATCH_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_FAIR_PVP_REMATCH_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/Timer_FairPvPRematch.  This is the
// vptr-only callback for a room's fair-PvP rematch timeout (message 62).
class Timer_FairPvPRematch : public TimerSig {
public:
    // IDA_EA: 0x863CF6E
    // IDA_NAME: _ZN20Timer_FairPvPRematchC2Ev
    // DEMANGLED: Timer_FairPvPRematch::Timer_FairPvPRematch(void)
    Timer_FairPvPRematch();

    // IDA_EA: 0x863E9B8 (D2), 0x863E9E8 (D0)
    // IDA_NAME: _ZN20Timer_FairPvPRematchD2Ev
    // IDA_NAME: _ZN20Timer_FairPvPRematchD0Ev
    // DEMANGLED: Timer_FairPvPRematch::~Timer_FairPvPRematch()
    virtual ~Timer_FairPvPRematch();

    // IDA_EA: 0x863A430
    // IDA_NAME: _ZN20Timer_FairPvPRematch12dispatch_sigEiij
    // DEMANGLED: Timer_FairPvPRematch::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

#if defined(__i386__)
static_assert(sizeof(Timer_FairPvPRematch) == 4u,
              "Timer_FairPvPRematch must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_FAIR_PVP_REMATCH_RUNTIME_H_
