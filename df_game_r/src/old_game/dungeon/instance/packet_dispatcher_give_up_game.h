#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_GIVE_UP_GAME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_GIVE_UP_GAME_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/DisPatcher_GiveUpGame. CMD045 has no
// request payload; its state gate and all four gameplay exits live in process.
class DisPatcher_GiveUpGame
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595AA4
    DisPatcher_GiveUpGame();
    virtual ~DisPatcher_GiveUpGame();

    // IDA_EA: 0x81C40AA / 0x81C40B4
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_GiveUpGame) == 0x04u,
              "CMD045 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_GIVE_UP_GAME_H_
