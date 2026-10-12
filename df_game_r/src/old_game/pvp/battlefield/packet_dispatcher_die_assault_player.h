#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DIE_ASSAULT_PLAYER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DIE_ASSAULT_PLAYER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 09_dungeon_pvp_party/Dispatcher_DieAssaultPlayer (CMD149).
class Dispatcher_DieAssaultPlayer
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85962A0
    Dispatcher_DieAssaultPlayer();

    // IDA_EA: 0x82426B2 / 0x82426E2
    virtual ~Dispatcher_DieAssaultPlayer();

    // IDA_EA: 0x8207208
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823393E / 0x8233948
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DieAssaultPlayer) == 0x04u,
              "CMD149 must retain the old vptr-only dispatcher ABI");
#endif

#endif
