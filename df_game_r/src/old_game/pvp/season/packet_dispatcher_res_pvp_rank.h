#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_RES_PVP_RANK_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_RES_PVP_RANK_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

struct MSG_RES_PVP_RANK;

// IDA unit: 09_dungeon_pvp_party/DisPatcher_ResPvpRank.
class DisPatcher_ResPvpRank
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_ResPvpRank();
    virtual ~DisPatcher_ResPvpRank();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    int ExtractPacket(MSG_RES_PVP_RANK& message, PacketBuf& packet);
    int SavePvpRank(CUser* user, MSG_RES_PVP_RANK& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_ResPvpRank) == 0x04u,
              "CMD58 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_RES_PVP_RANK_H_
