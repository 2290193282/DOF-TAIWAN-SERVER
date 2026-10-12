#include "infra/concurrent/timer/timer_connect_monitor_server.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/timer/timer_queue.h"


// IDA_EA: 0x863CBCE
// IDA_NAME: _ZN25TimerConnectMonitorServerC2Ev
// DEMANGLED: TimerConnectMonitorServer::TimerConnectMonitorServer(void)
TimerConnectMonitorServer::TimerConnectMonitorServer()
    : TimerSig() {}

// IDA_EA: 0x863F384 (D2), 0x863F3B4 (D0)
// IDA_NAME: _ZN25TimerConnectMonitorServerD2Ev / _ZN25TimerConnectMonitorServerD0Ev
TimerConnectMonitorServer::~TimerConnectMonitorServer() {}

// IDA_EA: 0x86380BC
// IDA_NAME: _ZN25TimerConnectMonitorServer12dispatch_sigEiij
// DEMANGLED: TimerConnectMonitorServer::dispatch_sig(int,int,unsigned int)
int TimerConnectMonitorServer::dispatch_sig(int object_id,
                                            int connection_kind,
                                            unsigned int ignored_param) {
    (void)object_id;
    (void)ignored_param;

    CStreamGuard stream_guard(
        GlobalData::stream_pool()->Acquire("TimerDispatcher.cpp", 3754),
        true);
    (*stream_guard) << 289;
    (*stream_guard) << -1;
    Packet_Tcp_Server_Connect* const packet =
        reinterpret_cast<Packet_Tcp_Server_Connect*>(
            stream_guard.GetInBufferUntyped(sizeof(Packet_Tcp_Server_Connect)));
    // TimerEntry::param1 carries the original 0xC9/0xCB connection kind.
    // registNextTimer writes it as InsertTimer's fifth argument, while
    // TimerDispatcher forwards param1 as dispatch_sig's second argument.
    packet->connection_kind = static_cast<uint8_t>(connection_kind);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
    return 1;
}

// IDA_EA: 0x86381A0
// IDA_NAME: _ZN25TimerConnectMonitorServer15registNextTimerElii
// DEMANGLED: TimerConnectMonitorServer::registNextTimer(long,int,int)
int TimerConnectMonitorServer::registNextTimer(long delay_seconds,
                                                int object_id,
                                                int connection_kind) {
    return G_TimerQueue()->InsertTimer(
        2, object_id, 122, static_cast<int>(delay_seconds),
        static_cast<unsigned int>(connection_kind), 0);
}
