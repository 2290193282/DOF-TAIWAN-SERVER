#ifndef OLDSRV_PROTOCOL_TRACE_ERROR_DISPATCHER_H_
#define OLDSRV_PROTOCOL_TRACE_ERROR_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 118 / TRACE_ERROR.  This is the old concrete state-0 dispatcher class.
// It has no fields beyond its IPacketDispatcher base (the old constructor
// table allocates exactly four i386 bytes).  It remains unregistered until the
// complete PacketDispatcher registry is restored atomically.
class Dispatcher_TraceError
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596038
    // IDA_NAME: _ZN21Dispatcher_TraceErrorC2Ev
    // DEMANGLED: Dispatcher_TraceError::Dispatcher_TraceError(void)
    Dispatcher_TraceError();

    // IDA_EA: 0x8242B44
    // IDA_NAME: _ZN21Dispatcher_TraceErrorD2Ev
    // DEMANGLED: Dispatcher_TraceError::~Dispatcher_TraceError()
    // IDA_EA: 0x8242B74
    // IDA_NAME: _ZN21Dispatcher_TraceErrorD0Ev
    // DEMANGLED: Dispatcher_TraceError::~Dispatcher_TraceError()
    virtual ~Dispatcher_TraceError();

    // IDA_EA: 0x8204B0C
    // IDA_NAME: _ZN21Dispatcher_TraceError12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_TraceError::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233812
    // IDA_NAME: _ZN21Dispatcher_TraceError4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_TraceError::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823381C
    // IDA_NAME: _ZN21Dispatcher_TraceError7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_TraceError::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_PROTOCOL_TRACE_ERROR_DISPATCHER_H_
