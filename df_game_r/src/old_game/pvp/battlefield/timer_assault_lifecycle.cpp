#include "old_game/pvp/battlefield/timer_assault_lifecycle.h"

#include "common/manager/disconnect_detecter_abi.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x863CA2A
// IDA_NAME: _ZN21TimerDetectDisconnectC2Ev
TimerDetectDisconnect::TimerDetectDisconnect() : TimerSig() {}

// IDA_EA: 0x863F816 (D2), 0x863F846 (D0)
// IDA_NAME: _ZN21TimerDetectDisconnectD2Ev / _ZN21TimerDetectDisconnectD0Ev
TimerDetectDisconnect::~TimerDetectDisconnect() {}

// IDA_EA: 0x86374BC
// IDA_NAME: _ZN21TimerDetectDisconnect12dispatch_sigEiij
// DEMANGLED: TimerDetectDisconnect::dispatch_sig(int,int,unsigned int)
int TimerDetectDisconnect::dispatch_sig(int, int, unsigned int) {
    if (disconnect_detecter::GetInstanceDisconnectDetecter()
            ->DoDetectingDisconnect()) {
        registNextTimer();
    }
    return 1;
}

// IDA_EA: 0x86374E0
// IDA_NAME: _ZN21TimerDetectDisconnect15registNextTimerEv
// DEMANGLED: TimerDetectDisconnect::registNextTimer(void)
int TimerDetectDisconnect::registNextTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 109, 10, 0, 0);
}

// IDA_EA: 0x863CA46
// IDA_NAME: _ZN31TimerCompleteLoadAssaultTimeoutC2Ev
TimerCompleteLoadAssaultTimeout::TimerCompleteLoadAssaultTimeout() : TimerSig() {}

// IDA_EA: 0x863F7C8 (D2), 0x863F7F8 (D0)
// IDA_NAME: _ZN31TimerCompleteLoadAssaultTimeoutD2Ev / _ZN31TimerCompleteLoadAssaultTimeoutD0Ev
TimerCompleteLoadAssaultTimeout::~TimerCompleteLoadAssaultTimeout() {}

// IDA_EA: 0x863752C
// IDA_NAME: _ZN31TimerCompleteLoadAssaultTimeout12dispatch_sigEiij
int TimerCompleteLoadAssaultTimeout::dispatch_sig(int assault_place_id,
                                                   int authen_value,
                                                   unsigned int) {
    pvp_assault::GetInstanceAssaultMgr()->OnTimeoutCompleteLoad(
        static_cast<unsigned int>(assault_place_id), authen_value);
    return 1;
}

// IDA_EA: 0x863755A
// IDA_NAME: _ZN31TimerCompleteLoadAssaultTimeout15registNextTimerElii
int TimerCompleteLoadAssaultTimeout::registNextTimer(long delay_seconds,
                                                      int assault_place_id,
                                                      int authen_value) {
    return G_TimerQueue()->InsertTimer(2, assault_place_id, 110,
                                       static_cast<int>(delay_seconds),
                                       static_cast<unsigned int>(authen_value),
                                       0);
}

// IDA_EA: 0x863CA62
// IDA_NAME: _ZN29TimerConnectP2PAssaultTimeoutC2Ev
TimerConnectP2PAssaultTimeout::TimerConnectP2PAssaultTimeout() : TimerSig() {}

// IDA_EA: 0x863F77A (D2), 0x863F7AA (D0)
// IDA_NAME: _ZN29TimerConnectP2PAssaultTimeoutD2Ev / _ZN29TimerConnectP2PAssaultTimeoutD0Ev
TimerConnectP2PAssaultTimeout::~TimerConnectP2PAssaultTimeout() {}

// IDA_EA: 0x863759C
// IDA_NAME: _ZN29TimerConnectP2PAssaultTimeout12dispatch_sigEiij
int TimerConnectP2PAssaultTimeout::dispatch_sig(int assault_place_id,
                                                 int authen_value,
                                                 unsigned int) {
    pvp_assault::GetInstanceAssaultMgr()->OnTimeoutConnectP2P(
        static_cast<unsigned int>(assault_place_id), authen_value);
    return 1;
}

// IDA_EA: 0x86375CA
// IDA_NAME: _ZN29TimerConnectP2PAssaultTimeout15registNextTimerElii
int TimerConnectP2PAssaultTimeout::registNextTimer(long delay_seconds,
                                                    int assault_place_id,
                                                    int authen_value) {
    return G_TimerQueue()->InsertTimer(2, assault_place_id, 111,
                                       static_cast<int>(delay_seconds),
                                       static_cast<unsigned int>(authen_value),
                                       0);
}

// IDA_EA: 0x863CA7E
// IDA_NAME: _ZN15TimerEndAssaultC2Ev
TimerEndAssault::TimerEndAssault() : TimerSig() {}

// IDA_EA: 0x863F72C (D2), 0x863F75C (D0)
// IDA_NAME: _ZN15TimerEndAssaultD2Ev / _ZN15TimerEndAssaultD0Ev
TimerEndAssault::~TimerEndAssault() {}

// IDA_EA: 0x863760C
// IDA_NAME: _ZN15TimerEndAssault12dispatch_sigEiij
int TimerEndAssault::dispatch_sig(int assault_place_id,
                                  int authen_value,
                                  unsigned int) {
    pvp_assault::GetInstanceAssaultMgr()->OnTimeoutEndAssault(
        static_cast<unsigned int>(assault_place_id), authen_value);
    return 1;
}

// IDA_EA: 0x863763A
// IDA_NAME: _ZN15TimerEndAssault15registNextTimerElii
int TimerEndAssault::registNextTimer(long delay_seconds,
                                     int assault_place_id,
                                     int authen_value) {
    return G_TimerQueue()->InsertTimer(2, assault_place_id, 112,
                                       static_cast<int>(delay_seconds),
                                       static_cast<unsigned int>(authen_value),
                                       0);
}

// IDA_EA: 0x863CA9A
// IDA_NAME: _ZN17TimerCloseAssaultC2Ev
TimerCloseAssault::TimerCloseAssault() : TimerSig() {}

// IDA_EA: 0x863F6DE (D2), 0x863F70E (D0)
// IDA_NAME: _ZN17TimerCloseAssaultD2Ev / _ZN17TimerCloseAssaultD0Ev
TimerCloseAssault::~TimerCloseAssault() {}

// IDA_EA: 0x863767C
// IDA_NAME: _ZN17TimerCloseAssault12dispatch_sigEiij
int TimerCloseAssault::dispatch_sig(int assault_place_id,
                                    int authen_value,
                                    unsigned int) {
    pvp_assault::GetInstanceAssaultMgr()->OnTimeoutCloseAssault(
        static_cast<unsigned int>(assault_place_id), authen_value);
    return 1;
}

// IDA_EA: 0x86376AA
// IDA_NAME: _ZN17TimerCloseAssault15registNextTimerElii
int TimerCloseAssault::registNextTimer(long delay_seconds,
                                       int assault_place_id,
                                       int authen_value) {
    return G_TimerQueue()->InsertTimer(2, assault_place_id, 113,
                                       static_cast<int>(delay_seconds),
                                       static_cast<unsigned int>(authen_value),
                                       0);
}

// IDA_EA: 0x863CAD2
// IDA_NAME: _ZN34TimerConnectP2PAfterAssaultTimeoutC2Ev
TimerConnectP2PAfterAssaultTimeout::TimerConnectP2PAfterAssaultTimeout()
    : TimerSig() {}

// IDA_EA: 0x863F642 (D2), 0x863F672 (D0)
// IDA_NAME: _ZN34TimerConnectP2PAfterAssaultTimeoutD2Ev / _ZN34TimerConnectP2PAfterAssaultTimeoutD0Ev
TimerConnectP2PAfterAssaultTimeout::~TimerConnectP2PAfterAssaultTimeout() {}

// IDA_EA: 0x863775C
// IDA_NAME: _ZN34TimerConnectP2PAfterAssaultTimeout12dispatch_sigEiij
int TimerConnectP2PAfterAssaultTimeout::dispatch_sig(int party_id,
                                                      int timer_key,
                                                      unsigned int) {
    pvp_assault::GetInstanceAssaultMgr()->OnTimeoutConnectP2PAfterAssault(
        static_cast<unsigned int>(party_id), timer_key);
    return 1;
}

// IDA_EA: 0x863778A
// IDA_NAME: _ZN34TimerConnectP2PAfterAssaultTimeout15registNextTimerElii
int TimerConnectP2PAfterAssaultTimeout::registNextTimer(long delay_seconds,
                                                         int party_id,
                                                         int timer_key) {
    return G_TimerQueue()->InsertTimer(1, party_id, 114,
                                       static_cast<int>(delay_seconds),
                                       static_cast<unsigned int>(timer_key), 0);
}

// IDA_EA: 0x863771A
// IDA_NAME: _ZN36TimerCompleteLoadAfterAssaultTimeout15registNextTimerElii
// DEMANGLED: TimerCompleteLoadAfterAssaultTimeout::registNextTimer(long,int,int)
int TimerCompleteLoadAfterAssaultTimeout::registNextTimer(
    long delay_seconds,
    int party_id,
    int timer_key) {
    return G_TimerQueue()->InsertTimer(
        1,
        party_id,
        42,
        static_cast<int>(delay_seconds),
        static_cast<unsigned int>(timer_key),
        0);
}
