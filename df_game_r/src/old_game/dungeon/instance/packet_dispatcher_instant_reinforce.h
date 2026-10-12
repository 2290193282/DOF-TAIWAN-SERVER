#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_PACKET_DISPATCHER_INSTANT_REINFORCE_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_PACKET_DISPATCHER_INSTANT_REINFORCE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_RequestInstantReinforce
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(5)> {
public:
    Dispatcher_RequestInstantReinforce();
    virtual ~Dispatcher_RequestInstantReinforce();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int check_state(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestInstantReinforce) == 0x04u,
              "CMD555 dispatcher must remain vptr-only on target i386");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_PACKET_DISPATCHER_INSTANT_REINFORCE_0828_H_
