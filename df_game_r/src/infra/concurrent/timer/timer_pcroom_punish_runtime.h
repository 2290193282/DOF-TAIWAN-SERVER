#ifndef OLDSRV_GLOBAL_TIMER_PCROOM_PUNISH_RUNTIME_H_
#define OLDSRV_GLOBAL_TIMER_PCROOM_PUNISH_RUNTIME_H_

#include <stdint.h>

#include "infra/concurrent/timer/timer_sig.h"

// These three stateless callbacks are allocated by TimerDispatcher. Their
// only state is the TimerSig vptr, so the timer payload remains in TimerEntry.
class Timer_PcRoomAuthRetry : public TimerSig {
public:
    // IDA_EA: 0x863CF1A
    // IDA_NAME: _ZN21Timer_PcRoomAuthRetryC2Ev
    // DEMANGLED: Timer_PcRoomAuthRetry::Timer_PcRoomAuthRetry(void)
    Timer_PcRoomAuthRetry();

    // IDA_EA: 0x863EAA2 (D2), 0x863EAD2 (D0)
    // IDA_NAME: _ZN21Timer_PcRoomAuthRetryD2Ev / _ZN21Timer_PcRoomAuthRetryD0Ev
    // DEMANGLED: Timer_PcRoomAuthRetry::~Timer_PcRoomAuthRetry()
    virtual ~Timer_PcRoomAuthRetry();

    // IDA_EA: 0x8639924
    // IDA_NAME: _ZN21Timer_PcRoomAuthRetry12dispatch_sigEiij
    // DEMANGLED: Timer_PcRoomAuthRetry::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid, int unique_id,
                             unsigned int charac_no);

    // IDA_EA: 0x86399D0
    // IDA_NAME: _ZN21Timer_PcRoomAuthRetry15registNextTimerEliij
    // DEMANGLED: Timer_PcRoomAuthRetry::registNextTimer(long,int,int,unsigned int)
    static int registNextTimer(long delay_seconds, int user_uid,
                               int unique_id, unsigned int charac_no);
};

class TimerSetPunishUser : public TimerSig {
public:
    // IDA_EA: 0x863CCEA
    // IDA_NAME: _ZN18TimerSetPunishUserC2Ev
    // DEMANGLED: TimerSetPunishUser::TimerSetPunishUser(void)
    TimerSetPunishUser();

    // IDA_EA: 0x863F108 (D2), 0x863F138 (D0)
    // IDA_NAME: _ZN18TimerSetPunishUserD2Ev / _ZN18TimerSetPunishUserD0Ev
    // DEMANGLED: TimerSetPunishUser::~TimerSetPunishUser()
    virtual ~TimerSetPunishUser();

    // IDA_EA: 0x8638BA0
    // IDA_NAME: _ZN18TimerSetPunishUser12dispatch_sigEiij
    // DEMANGLED: TimerSetPunishUser::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid, int ignored_param,
                             unsigned int unique_id);
};

class TimerSetTradePunishUser : public TimerSig {
public:
    // IDA_EA: 0x863CD06
    // IDA_NAME: _ZN23TimerSetTradePunishUserC2Ev
    // DEMANGLED: TimerSetTradePunishUser::TimerSetTradePunishUser(void)
    TimerSetTradePunishUser();

    // IDA_EA: 0x863F0BA (D2), 0x863F0EA (D0)
    // IDA_NAME: _ZN23TimerSetTradePunishUserD2Ev / _ZN23TimerSetTradePunishUserD0Ev
    // DEMANGLED: TimerSetTradePunishUser::~TimerSetTradePunishUser()
    virtual ~TimerSetTradePunishUser();

    // IDA_EA: 0x8638C48
    // IDA_NAME: _ZN23TimerSetTradePunishUser12dispatch_sigEiij
    // DEMANGLED: TimerSetTradePunishUser::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid, int ignored_param,
                             unsigned int unique_id);
};

#if defined(__i386__)
static_assert(sizeof(Timer_PcRoomAuthRetry) == 0x04u,
              "Timer_PcRoomAuthRetry must retain its TimerSig-only ABI");
static_assert(sizeof(TimerSetPunishUser) == 0x04u,
              "TimerSetPunishUser must retain its TimerSig-only ABI");
static_assert(sizeof(TimerSetTradePunishUser) == 0x04u,
              "TimerSetTradePunishUser must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_TIMER_PCROOM_PUNISH_RUNTIME_H_
