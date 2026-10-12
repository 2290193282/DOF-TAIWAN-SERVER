#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REDEEM_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REDEEM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD311.  Registration remains forbidden until all 606 derived objects have
// their corresponding i386 object proof.
class Dispatcher_Redeem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Redeem();
    virtual ~Dispatcher_Redeem();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Redeem) == 0x04u,
              "CMD311 must preserve the old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REDEEM_H_
