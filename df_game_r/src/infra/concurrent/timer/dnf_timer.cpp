#include "infra/concurrent/timer/dnf_timer.h"

#include <sys/time.h>


typedef char CDnFTimer_Size_Must_Be_4[(sizeof(CDnFTimer) == 4) ? 1 : -1];
typedef char CUnixTimer_Size_Must_Be_12[(sizeof(CUnixTimer) == 12) ? 1 : -1];

// IDA_EA: 0x85ED7E4
// IDA_NAME: _ZN9CDnFTimerC2Ev
// DEMANGLED: CDnFTimer::CDnFTimer(void)
CDnFTimer::CDnFTimer() {}

// IDA_EA: 0x85ED752
// IDA_NAME: _ZN9CDnFTimer11SetLastTimeEv
// DEMANGLED: CDnFTimer::SetLastTime(void)
void CDnFTimer::SetLastTime() {}

// IDA_EA: 0x85ED758
// IDA_NAME: _ZN9CDnFTimer15GetTimeIntervalEv
// DEMANGLED: CDnFTimer::GetTimeInterval(void)
long double CDnFTimer::GetTimeInterval() {
    return 0.0;
}

// IDA_EA: 0x85ED776
// IDA_NAME: _ZN10CUnixTimer10GetNowTimeEv
// DEMANGLED: CUnixTimer::GetNowTime(void)
long double CUnixTimer::GetNowTime() {
    timeval tv;
    gettimeofday(&tv, 0);
    return static_cast<long double>(tv.tv_sec) +
           static_cast<long double>(tv.tv_usec) / 1000000.0L;
}

// IDA_EA: 0x85ED7F2
// IDA_NAME: _ZN10CUnixTimerC2Ev
// DEMANGLED: CUnixTimer::CUnixTimer(void)
CUnixTimer::CUnixTimer() : CDnFTimer(), last_time_(0.0) {}

// IDA_EA: 0x85ED7AE
// IDA_NAME: _ZN10CUnixTimer11SetLastTimeEv
// DEMANGLED: CUnixTimer::SetLastTime(void)
void CUnixTimer::SetLastTime() {
    last_time_ = static_cast<double>(GetNowTime());
}

// IDA_EA: 0x85ED7C8
// IDA_NAME: _ZN10CUnixTimer15GetTimeIntervalEv
// DEMANGLED: CUnixTimer::GetTimeInterval(void)
long double CUnixTimer::GetTimeInterval() {
    return GetNowTime() - last_time_;
}

// IDA_EA: 0x85ECAB0
// IDA_NAME: _ZN16CDnFTimerFactory14CreateDnFTimerEv
// DEMANGLED: CDnFTimerFactory::CreateDnFTimer(void)
CDnFTimer* CDnFTimerFactory::CreateDnFTimer() {
    return new CUnixTimer();
}

// IDA_EA: 0x85ECAD8
// IDA_NAME: _ZN16CDnFTimerFactory15DestroyDnFTimerEP9CDnFTimer
// DEMANGLED: CDnFTimerFactory::DestroyDnFTimer(CDnFTimer *)
void CDnFTimerFactory::DestroyDnFTimer(CDnFTimer* timer) {
    operator delete(timer);
}
