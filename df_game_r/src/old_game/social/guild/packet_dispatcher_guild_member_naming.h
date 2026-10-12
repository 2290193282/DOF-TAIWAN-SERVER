#ifndef OLDSRV_OLD_GAME_SOCIAL_GUILD_PACKET_DISPATCHER_GUILD_MEMBER_NAMING_H_
#define OLDSRV_OLD_GAME_SOCIAL_GUILD_PACKET_DISPATCHER_GUILD_MEMBER_NAMING_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// 0828 CMD125.  The 0627 game binary retained the Monitor reply owner but did
// not register a client request reader.  This vptr-only dispatcher restores
// the 0828 client boundary and forwards the mutation to the Guild/Monitor
// service; it never acknowledges or writes guild persistence locally.
class Dispatcher_GuildMemberNaming
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_GuildMemberNaming();
    virtual ~Dispatcher_GuildMemberNaming();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GuildMemberNaming) == 0x04u,
              "CMD125 must remain a vptr-only dispatcher");
#endif

#endif  // OLDSRV_OLD_GAME_SOCIAL_GUILD_PACKET_DISPATCHER_GUILD_MEMBER_NAMING_H_
