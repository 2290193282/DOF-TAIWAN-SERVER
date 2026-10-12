#include "infra/security/policy/inter_hades_disconnect_dispatcher.h"

#include "infra/security/policy/timer_hades_heartbeat.h"

// IDA_EA: 0x84EEACC
// IDA_NAME: _ZN21Inter_HadesDisConnectC2Ev
// DEMANGLED: Inter_HadesDisConnect::Inter_HadesDisConnect(void)
Inter_HadesDisConnect::Inter_HadesDisConnect()
    : ISigDispatcher() {}

// IDA_EA: 0x84F64F8 (D2), 0x84F6528 (D0)
// IDA_NAME: _ZN21Inter_HadesDisConnectD2Ev / _ZN21Inter_HadesDisConnectD0Ev
Inter_HadesDisConnect::~Inter_HadesDisConnect() {}

// IDA_EA: 0x84E4342
// IDA_NAME: _ZN21Inter_HadesDisConnect12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_HadesDisConnect::dispatch_sig(CUser *,char *,int)
int Inter_HadesDisConnect::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;
    Timer_HadesHeartBeat::registNextTimer();
    return 0;
}
