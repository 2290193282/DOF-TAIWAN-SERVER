#ifndef OLDSRV_OLD_GAME_NPC_NPC_SHOP_TIMER_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_NPC_SHOP_TIMER_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 07_npc_shop_dialog/TimerNPCMoodChange.  TimerDispatcher owns the
// stateless object in message-table slot 116.
class TimerNPCMoodChange : public TimerSig {
public:
    // IDA_EA: 0x863CC06
    // IDA_NAME: _ZN18TimerNPCMoodChangeC2Ev
    TimerNPCMoodChange();

    // IDA_EA: 0x863F336 / 0x863F366
    // IDA_NAME: _ZN18TimerNPCMoodChangeD2Ev / D0Ev
    virtual ~TimerNPCMoodChange();

    // IDA_EA: 0x86381E2
    // IDA_NAME: _ZN18TimerNPCMoodChange12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);

    // IDA_EA: 0x86381EC
    // IDA_NAME: _ZN18TimerNPCMoodChange15registNextTimerEl
    // The recovered function has no body and no instance access.
    static void registNextTimer(long delay_seconds);
};

// IDA unit: 07_npc_shop_dialog/Timer_SecretShopStatistic.  The dispatcher
// object is vptr-only and resides in message-table slot 144.
class Timer_SecretShopStatistic : public TimerSig {
public:
    // IDA_EA: 0x863CEAA
    // IDA_NAME: _ZN25Timer_SecretShopStatisticC2Ev
    Timer_SecretShopStatistic();

    // IDA_EA: 0x863EB8C / 0x863EBBC
    // IDA_NAME: _ZN25Timer_SecretShopStatisticD2Ev / D0Ev
    virtual ~Timer_SecretShopStatistic();

    // IDA_EA: 0x86397CE
    // IDA_NAME: _ZN25Timer_SecretShopStatistic12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);

    // IDA_EA: 0x86397EE
    // IDA_NAME: _ZN25Timer_SecretShopStatistic15RegistNextTimerEv
    static int RegistNextTimer();
};

// IDA unit: 07_npc_shop_dialog/Timer_OneADayItemShop.  The dispatcher object
// is vptr-only and resides in message-table slot 153.
class Timer_OneADayItemShop : public TimerSig {
public:
    // IDA_EA: 0x863CFDE
    // IDA_NAME: _ZN21Timer_OneADayItemShopC2Ev
    Timer_OneADayItemShop();

    // IDA_EA: 0x863E832 / 0x863E862
    // IDA_NAME: _ZN21Timer_OneADayItemShopD2Ev / D0Ev
    virtual ~Timer_OneADayItemShop();

    // IDA_EA: 0x863A916
    // IDA_NAME: _ZN21Timer_OneADayItemShop12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);

    // IDA_EA: 0x863A93C
    // IDA_NAME: _ZN21Timer_OneADayItemShop15RegistNextTimerEv
    static int RegistNextTimer();
};

#if defined(__i386__)
static_assert(sizeof(TimerNPCMoodChange) == 0x04u,
              "TimerNPCMoodChange must remain TimerSig-only on i386");
static_assert(sizeof(Timer_SecretShopStatistic) == 0x04u,
              "Timer_SecretShopStatistic must remain TimerSig-only on i386");
static_assert(sizeof(Timer_OneADayItemShop) == 0x04u,
              "Timer_OneADayItemShop must remain TimerSig-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_NPC_SHOP_TIMER_RUNTIME_H_
