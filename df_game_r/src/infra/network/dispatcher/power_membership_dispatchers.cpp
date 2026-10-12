#include "infra/network/dispatcher/power_membership_dispatchers.h"

#include "common/manager/global_tools.h"


// IDA_EA: 0x8596604
// IDA_NAME: _ZN21Dispatcher_Join_PowerC2Ev
// DEMANGLED: Dispatcher_Join_Power::Dispatcher_Join_Power(void)
Dispatcher_Join_Power::Dispatcher_Join_Power() {}

// IDA_EA: 0x8241EC6
// IDA_NAME: _ZN21Dispatcher_Join_PowerD2Ev
// DEMANGLED: Dispatcher_Join_Power::~Dispatcher_Join_Power()
// IDA_EA: 0x8241EF6
// IDA_NAME: _ZN21Dispatcher_Join_PowerD0Ev
// DEMANGLED: Dispatcher_Join_Power::~Dispatcher_Join_Power()
Dispatcher_Join_Power::~Dispatcher_Join_Power() {}

// IDA_EA: 0x820BA38
// IDA_NAME: _ZN21Dispatcher_Join_Power12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Join_Power::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Join_Power::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)user;
    (void)packet;
    return LineFunc(
        43175,
        "virtual int Dispatcher_Join_Power::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233AF6
// IDA_NAME: _ZN21Dispatcher_Join_Power4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Join_Power::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Join_Power::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B00
// IDA_NAME: _ZN21Dispatcher_Join_Power7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Join_Power::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Join_Power::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596620
// IDA_NAME: _ZN23Dispatcher_Secede_PowerC2Ev
// DEMANGLED: Dispatcher_Secede_Power::Dispatcher_Secede_Power(void)
Dispatcher_Secede_Power::Dispatcher_Secede_Power() {}

// IDA_EA: 0x8241E78
// IDA_NAME: _ZN23Dispatcher_Secede_PowerD2Ev
// DEMANGLED: Dispatcher_Secede_Power::~Dispatcher_Secede_Power()
// IDA_EA: 0x8241EA8
// IDA_NAME: _ZN23Dispatcher_Secede_PowerD0Ev
// DEMANGLED: Dispatcher_Secede_Power::~Dispatcher_Secede_Power()
Dispatcher_Secede_Power::~Dispatcher_Secede_Power() {}

// IDA_EA: 0x820BA64
// IDA_NAME: _ZN23Dispatcher_Secede_Power12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Secede_Power::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Secede_Power::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)user;
    (void)packet;
    return LineFunc(
        43290,
        "virtual int Dispatcher_Secede_Power::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233B0A
// IDA_NAME: _ZN23Dispatcher_Secede_Power4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Secede_Power::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Secede_Power::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B14
// IDA_NAME: _ZN23Dispatcher_Secede_Power7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Secede_Power::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Secede_Power::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
