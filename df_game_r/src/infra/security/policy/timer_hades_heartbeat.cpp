#include "infra/security/policy/timer_hades_heartbeat.h"

#include <stdint.h>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hades_server_proxy_abi.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x863CFA6
// IDA_NAME: _ZN20Timer_HadesHeartBeatC2Ev
// DEMANGLED: Timer_HadesHeartBeat::Timer_HadesHeartBeat(void)
Timer_HadesHeartBeat::Timer_HadesHeartBeat()
    : TimerSig() {}

// IDA_EA: 0x863E91C (D2), 0x863E94C (D0)
// IDA_NAME: _ZN20Timer_HadesHeartBeatD2Ev / _ZN20Timer_HadesHeartBeatD0Ev
Timer_HadesHeartBeat::~Timer_HadesHeartBeat() {}

// IDA_EA: 0x863A760
// IDA_NAME: _ZN20Timer_HadesHeartBeat12dispatch_sigEiij
// DEMANGLED: Timer_HadesHeartBeat::dispatch_sig(int,int,unsigned int)
int Timer_HadesHeartBeat::dispatch_sig(int object_id,
                                       int ignored_param,
                                       unsigned int ignored_value) {
    (void)object_id;
    (void)ignored_param;
    (void)ignored_value;

    CHadesServerProxy* const proxy = GlobalData::s_hades_proxy;
    if (proxy &&
        proxy->GetSocketState() == BaseServerProxy::SOCKET_STATE_DISCONNECTED) {
        proxy->SendHeartBeatPacket(
            static_cast<uint16_t>(G_CEnvironment()->get_udp_port()),
            G_CEnvironment()->get_channel_no());
        registNextTimer();
    }
    return 1;
}

// IDA_EA: 0x863A7C2
// IDA_NAME: _ZN20Timer_HadesHeartBeat15registNextTimerEv
// DEMANGLED: Timer_HadesHeartBeat::registNextTimer(void)
int Timer_HadesHeartBeat::registNextTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 150, 60, 0, 0);
}
