#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_REQ_PEER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_REQ_PEER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// Native common packet shape consumed directly by CMD010.  MSG_BASE is the
// 13-byte 140828 client frame header.  Type 0 appends a required u16 after
// this common record; the other proven subtypes end here.
#pragma pack(push, 1)
struct MSG_REQ_TO_PEER : MSG_BASE {
    uint16_t target_unique_id;  // +0x0d
    uint8_t request_type;       // +0x0f
    int32_t input_value;        // +0x10
};
#pragma pack(pop)

static_assert(sizeof(MSG_REQ_TO_PEER) == 0x14u,
              "CMD010 common peer request ABI must stay 20 bytes");

// IDA unit: 13_database_dispatch/DisPatcher_ReqPeer.
class DisPatcher_ReqPeer
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85954D8
    DisPatcher_ReqPeer();
    // IDA_EA: 0x8232DF2 / 0x8232E22
    virtual ~DisPatcher_ReqPeer();

    // IDA_EA: 0x81EED08
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8232DDE / 0x8232DE8
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_ReqPeer) == 0x04u,
              "CMD010 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_REQ_PEER_H_
