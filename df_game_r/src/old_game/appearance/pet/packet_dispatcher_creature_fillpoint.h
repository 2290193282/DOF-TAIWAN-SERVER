#ifndef OLDSRV_OLD_GAME_APPEARANCE_PET_PACKET_DISPATCHER_CREATURE_FILLPOINT_H_
#define OLDSRV_OLD_GAME_APPEARANCE_PET_PACKET_DISPATCHER_CREATURE_FILLPOINT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// 0828 CMD549.  The client sends one u32 [fill point] after a general or over
// creature skill.  This formal owner accepts only the general-skill branch;
// the separate over-skill business remains owned by CMD372.
class Dispatcher_CreatureFillPoint
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(5)> {
public:
    Dispatcher_CreatureFillPoint();
    virtual ~Dispatcher_CreatureFillPoint();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int check_state(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CreatureFillPoint) == 0x04u,
              "CMD549 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_APPEARANCE_PET_PACKET_DISPATCHER_CREATURE_FILLPOINT_H_
