#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DEATH_TOWER_TIMER_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DEATH_TOWER_TIMER_RUNTIME_H_

#include <time.h>

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/TimerDeathTowerTopRankerReload.  This is a
// vptr-only TimerSig: the scheduled work and its state are owned by the DB
// request queue and TimerQueue, not by this callback object.
class TimerDeathTowerTopRankerReload : public TimerSig {
public:
    // IDA_EA: 0x863CB0A.
    // IDA_NAME: _ZN30TimerDeathTowerTopRankerReloadC2Ev
    // DEMANGLED: TimerDeathTowerTopRankerReload::TimerDeathTowerTopRankerReload(void)
    TimerDeathTowerTopRankerReload();

    // IDA_EA: 0x863F5A6 (D2), 0x863F5D6 (D0).
    // IDA_NAME: _ZN30TimerDeathTowerTopRankerReloadD2Ev /
    //           _ZN30TimerDeathTowerTopRankerReloadD0Ev
    // DEMANGLED: TimerDeathTowerTopRankerReload::~TimerDeathTowerTopRankerReload()
    virtual ~TimerDeathTowerTopRankerReload();

    // IDA_EA: 0x8637816.
    // IDA_NAME: _ZN30TimerDeathTowerTopRankerReload12dispatch_sigEiij
    // DEMANGLED: TimerDeathTowerTopRankerReload::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id,
                             int timer_key,
                             unsigned int parameter);

    // IDA_EA: 0x8637854.
    // IDA_NAME: _ZN30TimerDeathTowerTopRankerReload15registNextTimerEl
    // DEMANGLED: TimerDeathTowerTopRankerReload::registNextTimer(long)
    // The raw body reads exactly one stack argument, so this is a static
    // scheduling entry point rather than hidden callback-instance state.
    static int registNextTimer(time_t timer);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerDeathTowerTopRankerReload) == 0x04u,
              "TimerDeathTowerTopRankerReload must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DEATH_TOWER_TIMER_RUNTIME_H_
