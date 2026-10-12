#ifndef OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_DISGUISE_REQUEST_H_
#define OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_DISGUISE_REQUEST_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// 0627 server owner: 13_database_dispatch/Dispatcher_DisguiseRequest.
// The 0828 client retains this class contract while adding an optional native
// u32 PVF-template key after the original u8/u16 request prefix.
class Dispatcher_DisguiseRequest
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85969A0 (0627 server)
    Dispatcher_DisguiseRequest();

    // IDA_EA: 0x824168C / 0x82416BC (0627 server)
    virtual ~Dispatcher_DisguiseRequest();

    // IDA_EA: 0x821B256 (0627 server)
    // CLIENT_EA: sub_BC73E0 @0x00BC73E0 (0828 client sender)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233D8A (0627 server)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233D94 (0627 server)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DisguiseRequest) == 0x04u,
              "CMD211 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_PLAYER_CORE_PACKET_DISPATCHER_DISGUISE_REQUEST_H_
