#ifndef OLDSRV_OLD_GAME_USERINFO_TIMER_CREATURE_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_TIMER_CREATURE_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 06_userinfo_stat_inventory/TimerCreature.  The original object
// contains only the TimerSig vptr; its dispatcher-table ownership is kept in
// the runtime root rather than reconstructed as a fabricated data layout.
class TimerCreature : public TimerSig {
public:
    // IDA_EA: 0x8635E96
    // IDA_NAME: _ZN13TimerCreature12dispatch_sigEiij
    // DEMANGLED: TimerCreature::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid,
                             int unique_id,
                             unsigned int charac_no);
};

#if defined(__i386__)
static_assert(sizeof(TimerCreature) == 0x04u,
              "TimerCreature must retain its one-vptr TimerSig ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_TIMER_CREATURE_RUNTIME_H_
