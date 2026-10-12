#ifndef OLDSRV_GLOBAL_TIMER_CONNECT_MONITOR_SERVER_H_
#define OLDSRV_GLOBAL_TIMER_CONNECT_MONITOR_SERVER_H_

#include <stdint.h>

#include "infra/concurrent/timer/timer_sig.h"


// IDA_EA: CStreamGuard::GetInBuffer<Packet_Tcp_Server_Connect> @0x863D2EE.
// The timer writes only the final connection-kind byte.  Preserve the first
// ten wire bytes as an opaque dispatcher-owned envelope instead of assigning
// an invented sub-protocol to them.
#pragma pack(push, 1)
struct Packet_Tcp_Server_Connect {
    uint8_t dispatcher_envelope[10];  // +0x00
    uint8_t connection_kind;          // +0x0a
};
#pragma pack(pop)

static_assert(sizeof(Packet_Tcp_Server_Connect) == 0x0bu,
              "TCP server connect packet must remain 11 bytes");


class TimerConnectMonitorServer : public TimerSig {
public:
    // IDA_EA: 0x863CBCE
    // IDA_NAME: _ZN25TimerConnectMonitorServerC2Ev
    TimerConnectMonitorServer();

    // IDA_EA: 0x863F384 (D2), 0x863F3B4 (D0)
    virtual ~TimerConnectMonitorServer();

    // IDA_EA: 0x86380BC
    // IDA_NAME: _ZN25TimerConnectMonitorServer12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int connection_kind,
                             unsigned int ignored_param);

    // IDA_EA: 0x86381A0
    // IDA_NAME: _ZN25TimerConnectMonitorServer15registNextTimerElii
    // The long first argument is the delay (seconds), just as it is for the
    // other TimerSig scheduling helpers; Hex-Rays' synthetic `this` label is
    // a decompiler artifact for this static entry point.
    static int registNextTimer(long delay_seconds,
                               int object_id,
                               int connection_kind);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerConnectMonitorServer) == 0x04u,
              "TimerConnectMonitorServer must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_TIMER_CONNECT_MONITOR_SERVER_H_
