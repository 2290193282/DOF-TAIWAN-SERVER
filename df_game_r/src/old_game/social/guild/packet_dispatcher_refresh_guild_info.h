#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_REFRESH_GUILD_INFO_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_REFRESH_GUILD_INFO_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD340.  This object is a state-3, vptr-only dispatcher.  Its packet has no
// body; process requests the authoritative Guild proxy to refresh the current
// character's guild data.  Registration remains intentionally disabled until
// the complete old PacketDispatcher table and dispatch chain are restored.
class Dispatcher_RefreshGuildInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597442
    // IDA_NAME: _ZN27Dispatcher_RefreshGuildInfoC2Ev
    Dispatcher_RefreshGuildInfo();

    // IDA_EA: 0x82452E0
    // IDA_NAME: _ZN27Dispatcher_RefreshGuildInfoD2Ev
    // IDA_EA: 0x8245310
    // IDA_NAME: _ZN27Dispatcher_RefreshGuildInfoD0Ev
    virtual ~Dispatcher_RefreshGuildInfo();

    // IDA_EA: 0x81D487C
    // IDA_NAME: _ZN27Dispatcher_RefreshGuildInfo4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D4886
    // IDA_NAME: _ZN27Dispatcher_RefreshGuildInfo7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RefreshGuildInfo) == 0x04u,
              "CMD340 dispatcher must retain the old vptr-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_REFRESH_GUILD_INFO_H_
