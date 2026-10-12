#ifndef OLDSRV_RUNTIME_DNF_TIMER_H_
#define OLDSRV_RUNTIME_DNF_TIMER_H_


// IDA unit: 99_misc_global/CDnFTimer. Base timer object is 4 bytes: vptr only.
class CDnFTimer {
public:
    CDnFTimer();
    virtual void SetLastTime();
    virtual long double GetTimeInterval();
};

// IDA unit: 99_misc_global/CUnixTimer. Layout: CDnFTimer vptr at +0 and the
// last timestamp double at +4, total size 0xC on i386.
class CUnixTimer : public CDnFTimer {
public:
    CUnixTimer();

    static long double GetNowTime();
    virtual void SetLastTime();
    virtual long double GetTimeInterval();

private:
    double last_time_;
};

class CDnFTimerFactory {
public:
    static CDnFTimer* CreateDnFTimer();
    static void DestroyDnFTimer(CDnFTimer* timer);
};


#endif  // OLDSRV_RUNTIME_DNF_TIMER_H_
