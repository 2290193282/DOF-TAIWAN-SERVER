#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TIMER_CHECK_MAIL_TIMEOUT_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TIMER_CHECK_MAIL_TIMEOUT_H_

#include "infra/concurrent/timer/timer_sig.h"


// IDA unit: 10_mail_trade_auction_cash/TimerCheckMailTimeOut.
// This provider deliberately owns only the TimerSig implementation.  The
// TimerDispatcher slot-8 registration is outside this unit's ownership.
class TimerCheckMailTimeOut : public TimerSig {
public:
    // IDA_EA: 0x863C6E2
    // IDA_NAME: _ZN21TimerCheckMailTimeOutC2Ev
    // DEMANGLED: TimerCheckMailTimeOut::TimerCheckMailTimeOut(void)
    TimerCheckMailTimeOut();

    // IDA_EA: 0x8635D20
    // IDA_NAME: _ZN21TimerCheckMailTimeOut12dispatch_sigEiij
    // DEMANGLED: TimerCheckMailTimeOut::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int user_uid,
                             int login_tick,
                             unsigned int charac_no);

    // IDA_EA: 0x8640050
    // IDA_NAME: _ZN21TimerCheckMailTimeOutD2Ev
    // DEMANGLED: TimerCheckMailTimeOut::~TimerCheckMailTimeOut()
    // IDA_EA: 0x8640080
    // IDA_NAME: _ZN21TimerCheckMailTimeOutD0Ev
    // DEMANGLED: TimerCheckMailTimeOut::~TimerCheckMailTimeOut()
    virtual ~TimerCheckMailTimeOut();
};


#if defined(__i386__)
static_assert(sizeof(TimerCheckMailTimeOut) == 0x04u,
              "TimerCheckMailTimeOut IDA ABI size must stay 0x04");
#endif


#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_TIMER_CHECK_MAIL_TIMEOUT_H_
