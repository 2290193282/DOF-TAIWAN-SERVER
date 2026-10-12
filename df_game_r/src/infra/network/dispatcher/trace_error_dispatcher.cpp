#include "infra/network/dispatcher/trace_error_dispatcher.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"


// IDA_EA: 0x8596038
// IDA_NAME: _ZN21Dispatcher_TraceErrorC2Ev
// DEMANGLED: Dispatcher_TraceError::Dispatcher_TraceError(void)
Dispatcher_TraceError::Dispatcher_TraceError() {}

// IDA_EA: 0x8242B44
// IDA_NAME: _ZN21Dispatcher_TraceErrorD2Ev
// DEMANGLED: Dispatcher_TraceError::~Dispatcher_TraceError()
// IDA_EA: 0x8242B74
// IDA_NAME: _ZN21Dispatcher_TraceErrorD0Ev
// DEMANGLED: Dispatcher_TraceError::~Dispatcher_TraceError()
Dispatcher_TraceError::~Dispatcher_TraceError() {}

// IDA_EA: 0x8204B0C
// IDA_NAME: _ZN21Dispatcher_TraceError12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_TraceError::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_TraceError::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)user;

    char error_group = 0;
    if (packet.get_byte(error_group) != 1) {
        return LineFunc(
            40052,
            "virtual int Dispatcher_TraceError::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    if (error_group == 2) {
        char error_type = 0;
        if (packet.get_byte(error_type) != 1) {
            return LineFunc(
                40115,
                "virtual int Dispatcher_TraceError::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }

        cMyTrace trace(
            "virtual int Dispatcher_TraceError::dispatch_sig(CUser*, PacketBuf&)",
            40117,
            0);
        trace("KILLBOSS2 client - error type(%d)", error_type);
    } else if (error_group == 3) {
        return LineFunc(
            40069,
            "virtual int Dispatcher_TraceError::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    return 0;
}

// IDA_EA: 0x8233812
// IDA_NAME: _ZN21Dispatcher_TraceError4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TraceError::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TraceError::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823381C
// IDA_NAME: _ZN21Dispatcher_TraceError7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TraceError::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TraceError::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
