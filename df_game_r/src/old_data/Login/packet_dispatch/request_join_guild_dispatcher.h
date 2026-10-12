#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_REQUEST_JOIN_GUILD_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_REQUEST_JOIN_GUILD_DISPATCHER_H_

#include <stddef.h>

#include "old_game/progression/quest/quest_skill_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD349 / REQUEST_JOIN_GUILD.  It uses the existing old 0x46-byte message
// C2 and the state-0 template; the database and Inter reply handlers are the
// other members of the same closure.
class Dispatcher_RequestJoinGuild
    : public IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597496
    // IDA_NAME: _ZN27Dispatcher_RequestJoinGuildC2Ev
    // DEMANGLED: Dispatcher_RequestJoinGuild::Dispatcher_RequestJoinGuild(void)
    Dispatcher_RequestJoinGuild();

    // IDA_EA: 0x82451F6 / 0x8245226
    // IDA_NAME: _ZN27Dispatcher_RequestJoinGuildD2Ev /
    //           _ZN27Dispatcher_RequestJoinGuildD0Ev
    // DEMANGLED: Dispatcher_RequestJoinGuild::~Dispatcher_RequestJoinGuild()
    virtual ~Dispatcher_RequestJoinGuild();

    // IDA_EA: 0x81D4CE2 / 0x81D4E28
    // IDA_NAME: _ZN27Dispatcher_RequestJoinGuild4readER9PacketBufR8MSG_BASE /
    //           _ZN27Dispatcher_RequestJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestJoinGuild::read(PacketBuf &,MSG_BASE &) /
    //            Dispatcher_RequestJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(MSG_REQUEST_GUILD_JOIN) == 0x46u,
              "CMD349 request must retain its old packed extent");
static_assert(offsetof(MSG_REQUEST_GUILD_JOIN, guild_name_length) == 0x0du &&
                  offsetof(MSG_REQUEST_GUILD_JOIN, guild_name) == 0x11u &&
                  offsetof(MSG_REQUEST_GUILD_JOIN, memo_length) == 0x28u &&
                  offsetof(MSG_REQUEST_GUILD_JOIN, memo) == 0x2cu,
              "CMD349 request offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestJoinGuild) == 0x04u,
              "CMD349 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_REQUEST_JOIN_GUILD_DISPATCHER_H_
