#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_RES_PEER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_RES_PEER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA packet shape consumed directly by CMD011.  This is the response to the
// CMD010 peer invitation, with the original target and a type-specific value.
#pragma pack(push, 1)
struct MSG_RES_TO_PEER : MSG_BASE {
    uint16_t target_unique_id;  // +0x0d
    uint8_t response_type;      // +0x0f
    int32_t input_value;        // +0x10
};
#pragma pack(pop)

static_assert(sizeof(MSG_RES_TO_PEER) == 0x14u,
              "CMD011 peer response must retain its 20-byte client ABI");

// IDA unit: 13_database_dispatch/DisPatcher_ResPeer.
class DisPatcher_ResPeer
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595510
    DisPatcher_ResPeer();
    // IDA_EA: 0x8232EB6 / 0x8232EE6
    virtual ~DisPatcher_ResPeer();

    // IDA_EA: 0x81F088E
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8232EA2 / 0x8232EAC
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_ResPeer) == 0x04u,
              "CMD011 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_RES_PEER_H_
