#ifndef OLDSRV_PROTOCOL_PEER_CONNECT_RESULT_DISPATCHER_H_
#define OLDSRV_PROTOCOL_PEER_CONNECT_RESULT_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 100 / PEER_CONNECT_RESULT.  The original class carries only the exact
// state-0 MSG_BASE dispatcher base; its payload is parsed in dispatch_sig.
class Dispatcher_PeerConnectResult
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595E78
    // IDA_NAME: _ZN28Dispatcher_PeerConnectResultC2Ev
    // DEMANGLED: Dispatcher_PeerConnectResult::Dispatcher_PeerConnectResult(void)
    Dispatcher_PeerConnectResult();

    // IDA_EA: 0x8242E50
    // IDA_NAME: _ZN28Dispatcher_PeerConnectResultD2Ev
    // DEMANGLED: Dispatcher_PeerConnectResult::~Dispatcher_PeerConnectResult()
    // IDA_EA: 0x8242E80
    // IDA_NAME: _ZN28Dispatcher_PeerConnectResultD0Ev
    // DEMANGLED: Dispatcher_PeerConnectResult::~Dispatcher_PeerConnectResult()
    virtual ~Dispatcher_PeerConnectResult();

    // IDA_EA: 0x82033E6
    // IDA_NAME: _ZN28Dispatcher_PeerConnectResult12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_PeerConnectResult::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82336D4
    // IDA_NAME: _ZN28Dispatcher_PeerConnectResult4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_PeerConnectResult::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82336DE
    // IDA_NAME: _ZN28Dispatcher_PeerConnectResult7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_PeerConnectResult::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PeerConnectResult) == 0x04u,
              "CMD100 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_PROTOCOL_PEER_CONNECT_RESULT_DISPATCHER_H_
