#ifndef OLDSRV_OLD_GAME_USERINFO_TIMER_REVIVAL_CHARAC_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_TIMER_REVIVAL_CHARAC_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 04_character_select_create/TimerRevivalCharac.
class TimerRevivalCharac : public TimerSig {
public:
    // IDA_EA: 0x863CD22
    // IDA_NAME: _ZN18TimerRevivalCharacC2Ev
    // DEMANGLED: TimerRevivalCharac::TimerRevivalCharac(void)
    TimerRevivalCharac();

    // IDA_EA: 0x863EFD0 / 0x863F000
    // IDA_NAME: _ZN18TimerRevivalCharacD2Ev / _ZN18TimerRevivalCharacD0Ev
    // DEMANGLED: TimerRevivalCharac::~TimerRevivalCharac()
    virtual ~TimerRevivalCharac();

    // IDA_EA: 0x8638E78
    // IDA_NAME: _ZN18TimerRevivalCharac12dispatch_sigEiij
    // DEMANGLED: TimerRevivalCharac::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid,
                             int revival_time,
                             unsigned int timer_value);
};

#if defined(__i386__)
static_assert(sizeof(TimerRevivalCharac) == 0x04u,
              "TimerRevivalCharac must remain a base-only timer object");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_TIMER_REVIVAL_CHARAC_RUNTIME_H_
