#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_JOIN_REQUEST_DB_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_JOIN_REQUEST_DB_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// The original C2S message objects retain the 13-byte MSG_BASE prefix without
// natural integer alignment.  CMD348 has a real C2; CMD350 and CMD352 do not,
// so their request dwords deliberately retain the template's stack lifetime.
#pragma pack(push, 1)
struct MSG_CHECK_GUILD_JOIN : public MSG_BASE {
    MSG_CHECK_GUILD_JOIN();

    uint32_t guild_name_length;  // +0x0d
    char guild_name[23];         // +0x11
};

struct MSG_CANCEL_JOIN_GUILD : public MSG_BASE {
    uint32_t guild_id;  // +0x0d
};

struct MSG_DENY_JOIN_GUILD : public MSG_BASE {
    uint32_t charac_no;  // +0x0d
};
#pragma pack(pop)

// MSG_BASE supplies data members, so inherited-message offsetof is only
// conditionally standard-layout.  GCC 4.9/i386 evaluates this packed ABI
// correctly; keep the proof without a target-toolchain warning.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(MSG_CHECK_GUILD_JOIN) == 0x28u,
              "CMD348 input must retain its old packed extent");
static_assert(offsetof(MSG_CHECK_GUILD_JOIN, guild_name_length) == 0x0du,
              "CMD348 guild-name length must stay at +0x0d");
static_assert(offsetof(MSG_CHECK_GUILD_JOIN, guild_name) == 0x11u,
              "CMD348 guild name must stay at +0x11");
static_assert(sizeof(MSG_CANCEL_JOIN_GUILD) == 0x11u,
              "CMD350 input must retain its old packed extent");
static_assert(offsetof(MSG_CANCEL_JOIN_GUILD, guild_id) == 0x0du,
              "CMD350 guild id must stay at +0x0d");
static_assert(sizeof(MSG_DENY_JOIN_GUILD) == 0x11u,
              "CMD352 input must retain its old packed extent");
static_assert(offsetof(MSG_DENY_JOIN_GUILD, charac_no) == 0x0du,
              "CMD352 character number must stay at +0x0d");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

class Dispatcher_CheckJoinGuild
    : public IPacketDispatcher<MSG_CHECK_GUILD_JOIN,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_CheckJoinGuild();
    virtual ~Dispatcher_CheckJoinGuild();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CancelJoinGuild
    : public IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_CancelJoinGuild();
    virtual ~Dispatcher_CancelJoinGuild();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DenyJoinGuild
    : public IPacketDispatcher<MSG_DENY_JOIN_GUILD,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DenyJoinGuild();
    virtual ~Dispatcher_DenyJoinGuild();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD366 has no request body and uses the existing MSG_BASE state-0
// specialization.  Its DB-to-Inter reply route remains outside this source.
class Dispatcher_JoinGuildInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_JoinGuildInfo();
    virtual ~Dispatcher_JoinGuildInfo();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CheckJoinGuild) == 0x04u,
              "CMD348 dispatcher must retain its old vptr-only i386 ABI");
static_assert(sizeof(Dispatcher_CancelJoinGuild) == 0x04u,
              "CMD350 dispatcher must retain its old vptr-only i386 ABI");
static_assert(sizeof(Dispatcher_DenyJoinGuild) == 0x04u,
              "CMD352 dispatcher must retain its old vptr-only i386 ABI");
static_assert(sizeof(Dispatcher_JoinGuildInfo) == 0x04u,
              "CMD366 dispatcher must retain its old vptr-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GUILD_JOIN_REQUEST_DB_H_
