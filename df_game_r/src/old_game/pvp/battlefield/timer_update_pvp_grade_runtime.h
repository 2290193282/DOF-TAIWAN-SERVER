#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_UPDATE_PVP_GRADE_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_UPDATE_PVP_GRADE_RUNTIME_H_

#include <ctime>

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/TimerUpdatePvPGrade.  This is a vptr-only
// daily DB flush callback stored at TimerDispatcher message 108.
class TimerUpdatePvPGrade : public TimerSig {
public:
    // IDA_EA: 0x863CA0E
    // IDA_NAME: _ZN19TimerUpdatePvPGradeC2Ev
    // DEMANGLED: TimerUpdatePvPGrade::TimerUpdatePvPGrade(void)
    TimerUpdatePvPGrade();

    // IDA_EA: 0x863F864 / 0x863F894
    // IDA_NAME: _ZN19TimerUpdatePvPGradeD2Ev
    //           _ZN19TimerUpdatePvPGradeD0Ev
    // DEMANGLED: TimerUpdatePvPGrade::~TimerUpdatePvPGrade()
    virtual ~TimerUpdatePvPGrade();

    // IDA_EA: 0x86373C6
    // IDA_NAME: _ZN19TimerUpdatePvPGrade12dispatch_sigEiij
    // DEMANGLED: TimerUpdatePvPGrade::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1,
                             unsigned int param2);

    // IDA_EA: 0x86373F2
    // IDA_NAME: _ZN19TimerUpdatePvPGrade15registNextTimerEl
    // DEMANGLED: TimerUpdatePvPGrade::registNextTimer(long)
    static int registNextTimer(time_t timer);
};

#if defined(__i386__)
static_assert(sizeof(TimerUpdatePvPGrade) == 0x04u,
              "TimerUpdatePvPGrade must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_UPDATE_PVP_GRADE_RUNTIME_H_
