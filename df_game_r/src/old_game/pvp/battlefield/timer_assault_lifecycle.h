#ifndef OLDSRV_RUNTIME_TIMER_ASSAULT_LIFECYCLE_H_
#define OLDSRV_RUNTIME_TIMER_ASSAULT_LIFECYCLE_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA units: 99_misc_global/Timer{DetectDisconnect,CompleteLoadAssaultTimeout,
// ConnectP2PAssaultTimeout,EndAssault,CloseAssault,
// ConnectP2PAfterAssaultTimeout}.  Each object is TimerSig-only and is owned
// by TimerDispatcher at its original message-table slot.
class TimerDetectDisconnect : public TimerSig {
public:
    TimerDetectDisconnect();
    virtual ~TimerDetectDisconnect();
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);
    static int registNextTimer();
};

class TimerCompleteLoadAssaultTimeout : public TimerSig {
public:
    TimerCompleteLoadAssaultTimeout();
    virtual ~TimerCompleteLoadAssaultTimeout();
    virtual int dispatch_sig(int assault_place_id, int authen_value,
                             unsigned int param2);
    static int registNextTimer(long delay_seconds,
                               int assault_place_id,
                               int authen_value);
};

class TimerConnectP2PAssaultTimeout : public TimerSig {
public:
    TimerConnectP2PAssaultTimeout();
    virtual ~TimerConnectP2PAssaultTimeout();
    virtual int dispatch_sig(int assault_place_id, int authen_value,
                             unsigned int param2);
    static int registNextTimer(long delay_seconds,
                               int assault_place_id,
                               int authen_value);
};

class TimerEndAssault : public TimerSig {
public:
    TimerEndAssault();
    virtual ~TimerEndAssault();
    virtual int dispatch_sig(int assault_place_id, int authen_value,
                             unsigned int param2);
    static int registNextTimer(long delay_seconds,
                               int assault_place_id,
                               int authen_value);
};

class TimerCloseAssault : public TimerSig {
public:
    TimerCloseAssault();
    virtual ~TimerCloseAssault();
    virtual int dispatch_sig(int assault_place_id, int authen_value,
                             unsigned int param2);
    static int registNextTimer(long delay_seconds,
                               int assault_place_id,
                               int authen_value);
};

class TimerConnectP2PAfterAssaultTimeout : public TimerSig {
public:
    TimerConnectP2PAfterAssaultTimeout();
    virtual ~TimerConnectP2PAfterAssaultTimeout();
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);
    static int registNextTimer(long delay_seconds,
                               int party_id,
                               int timer_key);
};

class TimerCompleteLoadAfterAssaultTimeout : public TimerSig {
public:
    TimerCompleteLoadAfterAssaultTimeout();
    virtual ~TimerCompleteLoadAfterAssaultTimeout();
    virtual int dispatch_sig(int party_id,
                             int timer_key,
                             unsigned int param2);
    static int registNextTimer(long delay_seconds,
                              int party_id,
                              int timer_key);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerDetectDisconnect) == 0x04u &&
                  sizeof(TimerCompleteLoadAssaultTimeout) == 0x04u &&
                  sizeof(TimerConnectP2PAssaultTimeout) == 0x04u &&
                  sizeof(TimerEndAssault) == 0x04u &&
                  sizeof(TimerCloseAssault) == 0x04u &&
                  sizeof(TimerConnectP2PAfterAssaultTimeout) == 0x04u &&
                  sizeof(TimerCompleteLoadAfterAssaultTimeout) == 0x04u,
              "assault lifecycle timer signatures must stay TimerSig-only");
#endif

#endif  // OLDSRV_RUNTIME_TIMER_ASSAULT_LIFECYCLE_H_
