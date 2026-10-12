#ifndef OLDSRV_SECURITY_PACKET_DISPATCHER_SECURITY_PEER_CONTROL_H_
#define OLDSRV_SECURITY_PACKET_DISPATCHER_SECURITY_PEER_CONTROL_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD312. The old object contains only the inherited state-0 dispatcher vptr.
class Dispatcher_SecuDataControl
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_SecuDataControl();
    virtual ~Dispatcher_SecuDataControl();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecuDataControl) == 0x04u,
              "CMD312 must remain the old vptr-only dispatcher object");
#endif

#endif  // OLDSRV_SECURITY_PACKET_DISPATCHER_SECURITY_PEER_CONTROL_H_
