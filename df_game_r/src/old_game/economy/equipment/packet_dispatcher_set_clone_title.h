#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_PACKET_DISPATCHER_SET_CLONE_TITLE_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_PACKET_DISPATCHER_SET_CLONE_TITLE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class Dispatcher_SetCloneTitle
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SetCloneTitle();
    virtual ~Dispatcher_SetCloneTitle();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user,
                        MSG_BASE& message,
                        ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SetCloneTitle) == 0x04u,
              "CMD562 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_PACKET_DISPATCHER_SET_CLONE_TITLE_H_
