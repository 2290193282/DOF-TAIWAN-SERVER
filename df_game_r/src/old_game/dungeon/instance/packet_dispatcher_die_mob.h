#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DIE_MOB_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DIE_MOB_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_DieMob (CMD042).
// The dispatch path owns only the packet boundary and state fan-out.  Normal
// dungeon mutation remains a CParty responsibility, while tower/altar/PvP
// routes retain their respective module owners.
class DisPatcher_DieMob
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595414
    DisPatcher_DieMob();
    // IDA_EA: 0x8232B08 / 0x8232B38
    virtual ~DisPatcher_DieMob();

    // IDA_EA: 0x81EB0C4
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8232AF4 / 0x8232AFE
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DieMob) == 0x04u,
              "CMD042 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DIE_MOB_H_
