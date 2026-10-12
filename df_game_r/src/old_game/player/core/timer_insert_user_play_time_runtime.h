#ifndef OLDSRV_OLD_GAME_USERINFO_TIMER_INSERT_USER_PLAY_TIME_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_TIMER_INSERT_USER_PLAY_TIME_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 99_misc_global/TimerInsertUserPlayTime.  The object contains no
// state beyond TimerSig and is installed in TimerDispatcher slot 105.
class TimerInsertUserPlayTime : public TimerSig {
public:
    // IDA_EA: 0x863C966
    // IDA_NAME: _ZN23TimerInsertUserPlayTimeC2Ev
    // DEMANGLED: TimerInsertUserPlayTime::TimerInsertUserPlayTime(void)
    TimerInsertUserPlayTime();

    // IDA_EA: 0x863FA38 / 0x863FA68
    // IDA_NAME: _ZN23TimerInsertUserPlayTimeD2Ev
    //           _ZN23TimerInsertUserPlayTimeD0Ev
    // DEMANGLED: TimerInsertUserPlayTime::~TimerInsertUserPlayTime()
    virtual ~TimerInsertUserPlayTime();

    // IDA_EA: 0x8636D0A
    // IDA_NAME: _ZN23TimerInsertUserPlayTime12dispatch_sigEiij
    // DEMANGLED: TimerInsertUserPlayTime::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // IDA_EA: 0x8636D44
    // IDA_NAME: _ZN23TimerInsertUserPlayTime15RegistNextTimerEv
    // DEMANGLED: TimerInsertUserPlayTime::RegistNextTimer(void)
    int RegistNextTimer();
};

#if defined(__i386__)
static_assert(sizeof(TimerInsertUserPlayTime) == 0x04u,
              "TimerInsertUserPlayTime must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_TIMER_INSERT_USER_PLAY_TIME_RUNTIME_H_
