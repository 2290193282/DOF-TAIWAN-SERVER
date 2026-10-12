#ifndef OLDSRV_PROTOCOL_SET_IP_DISPATCHER_H_
#define OLDSRV_PROTOCOL_SET_IP_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 2 / SET_UDP_IP_PORT.  The old root table does not register this provider
// until its complete original construction transaction is restored.
class DisPatcher_SetIP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595564
    // IDA_NAME: _ZN16DisPatcher_SetIPC2Ev
    // DEMANGLED: DisPatcher_SetIP::DisPatcher_SetIP(void)
    DisPatcher_SetIP();

    // IDA_EA: 0x8232FDC
    // IDA_NAME: _ZN16DisPatcher_SetIPD2Ev
    // DEMANGLED: DisPatcher_SetIP::~DisPatcher_SetIP()
    // IDA_EA: 0x823300C
    // IDA_NAME: _ZN16DisPatcher_SetIPD0Ev
    // DEMANGLED: DisPatcher_SetIP::~DisPatcher_SetIP()
    virtual ~DisPatcher_SetIP();

    // IDA_EA: 0x81F468C
    // IDA_NAME: _ZN16DisPatcher_SetIP12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_SetIP::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8232FC8
    // IDA_NAME: _ZN16DisPatcher_SetIP4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_SetIP::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8232FD2
    // IDA_NAME: _ZN16DisPatcher_SetIP7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_SetIP::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetIP) == 0x04u,
              "DisPatcher_SetIP must retain the old one-vptr layout");
#endif

#endif  // OLDSRV_PROTOCOL_SET_IP_DISPATCHER_H_
