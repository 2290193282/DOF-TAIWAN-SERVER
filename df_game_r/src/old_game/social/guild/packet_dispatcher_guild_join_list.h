#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_JOIN_LIST_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_JOIN_LIST_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD353.  The old object carries no data beyond the state-0 dispatcher
// vptr.  Its reply path remains owned by the existing B DB/Inter providers;
// this C source restores only the client request ingress and is not
// registered in PacketDispatcher here.
class Dispatcher_GuildJoinList
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597506
    // IDA_NAME: _ZN24Dispatcher_GuildJoinListC2Ev
    // DEMANGLED: Dispatcher_GuildJoinList::Dispatcher_GuildJoinList(void)
    Dispatcher_GuildJoinList();

    // IDA_EA: 0x82450BE
    // IDA_NAME: _ZN24Dispatcher_GuildJoinListD2Ev
    // DEMANGLED: Dispatcher_GuildJoinList::~Dispatcher_GuildJoinList()
    // IDA_EA: 0x82450EE
    // IDA_NAME: _ZN24Dispatcher_GuildJoinListD0Ev
    // DEMANGLED: Dispatcher_GuildJoinList::~Dispatcher_GuildJoinList()
    virtual ~Dispatcher_GuildJoinList();

    // IDA_EA: 0x81D529C
    // IDA_NAME: _ZN24Dispatcher_GuildJoinList4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GuildJoinList::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D52A6
    // IDA_NAME: _ZN24Dispatcher_GuildJoinList7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GuildJoinList::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GuildJoinList) == 0x04u,
              "CMD353 dispatcher must retain its old vptr-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_JOIN_LIST_H_
