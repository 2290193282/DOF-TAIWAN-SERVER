#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_STACKABLE_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_STACKABLE_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD047 is a direct dispatcher. It owns the complete C2S packet decode and
// intentionally keeps the generic read/process virtual routes as the retail
// zero-return bodies. Root PacketDispatcher registration remains disabled.
class DisPatcher_UseStack
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85955F0
    // IDA_NAME: _ZN19DisPatcher_UseStackC2Ev
    // DEMANGLED: DisPatcher_UseStack::DisPatcher_UseStack(void)
    DisPatcher_UseStack();

    // IDA_EA: 0x82331C6 / 0x82331F6
    // IDA_NAME: _ZN19DisPatcher_UseStackD2Ev / _ZN19DisPatcher_UseStackD0Ev
    // DEMANGLED: DisPatcher_UseStack::~DisPatcher_UseStack()
    virtual ~DisPatcher_UseStack();

    // IDA_EA: 0x81F63C0
    // IDA_NAME: _ZN19DisPatcher_UseStack12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_UseStack::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82331B2 / 0x82331BC
    // IDA_NAME: _ZN19DisPatcher_UseStack4readER9PacketBufR8MSG_BASE /
    //           _ZN19DisPatcher_UseStack7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_UseStack) == 0x04u,
              "CMD047 dispatcher must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_STACKABLE_DISPATCHER_H_
