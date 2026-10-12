#ifndef OLDSRV_PROTOCOL_CHANGE_HOST_DISPATCHER_H_
#define OLDSRV_PROTOCOL_CHANGE_HOST_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 124 / CHANGE_HOST.  The class is protocol-owned while the party state it
// consumes remains in the canonical C group provider.
class Dispatcher_ChangeHost
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85960E0
    // IDA_NAME: _ZN21Dispatcher_ChangeHostC2Ev
    // DEMANGLED: Dispatcher_ChangeHost::Dispatcher_ChangeHost(void)
    Dispatcher_ChangeHost();

    // IDA_EA: 0x8242AF6
    // IDA_NAME: _ZN21Dispatcher_ChangeHostD2Ev
    // DEMANGLED: Dispatcher_ChangeHost::~Dispatcher_ChangeHost()
    // IDA_EA: 0x8242B26
    // IDA_NAME: _ZN21Dispatcher_ChangeHostD0Ev
    // DEMANGLED: Dispatcher_ChangeHost::~Dispatcher_ChangeHost()
    virtual ~Dispatcher_ChangeHost();

    // IDA_EA: 0x8204C1E
    // IDA_NAME: _ZN21Dispatcher_ChangeHost12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ChangeHost::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233826
    // IDA_NAME: _ZN21Dispatcher_ChangeHost4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeHost::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233830
    // IDA_NAME: _ZN21Dispatcher_ChangeHost7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ChangeHost::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeHost) == 0x04u,
              "CMD124 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_PROTOCOL_CHANGE_HOST_DISPATCHER_H_
