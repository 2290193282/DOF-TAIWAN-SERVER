#include "infra/security/policy/packet_dispatcher_security_peer_control.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/security/policy/secu_data_control_runtime.h"

// IDA_EA: 0x8596D3C
// IDA_NAME: _ZN26Dispatcher_SecuDataControlC2Ev
// DEMANGLED: Dispatcher_SecuDataControl::Dispatcher_SecuDataControl(void)
// IDA_EA: 0x8241074
// IDA_NAME: _ZN26Dispatcher_SecuDataControlD2Ev
// DEMANGLED: Dispatcher_SecuDataControl::~Dispatcher_SecuDataControl()
// IDA_EA: 0x82410A4
// IDA_NAME: _ZN26Dispatcher_SecuDataControlD0Ev
// DEMANGLED: Dispatcher_SecuDataControl::~Dispatcher_SecuDataControl()
// IDA_EA: 0x8233F1A
// IDA_NAME: _ZN26Dispatcher_SecuDataControl4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecuDataControl::read(PacketBuf &,MSG_BASE &)
// IDA_EA: 0x8233F24
// IDA_NAME: _ZN26Dispatcher_SecuDataControl7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecuDataControl::process(CUser *,MSG_BASE &,ParamBase &)
Dispatcher_SecuDataControl::Dispatcher_SecuDataControl() {}
Dispatcher_SecuDataControl::~Dispatcher_SecuDataControl() {}

int Dispatcher_SecuDataControl::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SecuDataControl::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x821F612
// IDA_NAME: _ZN26Dispatcher_SecuDataControl12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecuDataControl::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecuDataControl::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint8_t command = 0;
    uint16_t first_value = 0;
    uint16_t second_value = 0;

    if (packet.get_byte(command) != 1) {
        return LineFunc(
            55352,
            "virtual int Dispatcher_SecuDataControl::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(first_value) != 1) {
        return LineFunc(
            55353,
            "virtual int Dispatcher_SecuDataControl::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(second_value) != 1) {
        return LineFunc(
            55354,
            "virtual int Dispatcher_SecuDataControl::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    GlobalData::s_pSecuDataControl->SetControlData(
        user, command, first_value, second_value);
    return 0;
}
