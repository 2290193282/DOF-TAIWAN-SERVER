#include "old_data/db/taiwan/inter_slots_331_353.h"

#include "infra/concurrent/timer/timer_connect_monitor_server.h"


// IDA_EA: 0x84D9C8E
// IDA_NAME: _ZN29Inter_DisconnectMonitorServer12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DisconnectMonitorServer::dispatch_sig(CUser *,char *,int)
int Inter_DisconnectMonitorServer::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_DISCONNECT_MONITOR_SERVER* const reply =
        reinterpret_cast<const SIG_DISCONNECT_MONITOR_SERVER*>(payload);
    TimerConnectMonitorServer::registNextTimer(
        30L, 0, static_cast<int>(reply->reconnect_delay_seconds));
    return 0;
}


// IDA_EA: 0x84EE014
// IDA_NAME: _ZN29Inter_DisconnectMonitorServerC2Ev
// DEMANGLED: Inter_DisconnectMonitorServer::Inter_DisconnectMonitorServer(void)
Inter_DisconnectMonitorServer::Inter_DisconnectMonitorServer() {}

// IDA_EA: 0x84F81EA (D2), 0x84F821A (D0)
Inter_DisconnectMonitorServer::~Inter_DisconnectMonitorServer() {}
