#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_APPROVE_JOIN_GUILD_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_APPROVE_JOIN_GUILD_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD351 has no emitted message C2.  The old state-0 template owns this
// 17-byte packed stack request and read() supplies its sole payload dword.
#pragma pack(push, 1)
struct MSG_APPROVE_JOIN_GUILD : public MSG_BASE {
    uint32_t applicant_charac_no;  // +0x0d
};

// IDA unit: 99_misc_global/Packet_Approve_Join_Guild.  The guild endpoint
// consumes the complete 0x16-byte TCP body, not a facade request object.
struct Packet_Approve_Join_Guild : public PacketHeader {
    uint32_t guild_key;             // +0x0a
    uint32_t approver_charac_no;    // +0x0e
    uint32_t applicant_charac_no;   // +0x12

    // IDA_EA: 0x822C342
    // IDA_NAME: _ZN25Packet_Approve_Join_GuildC2Ev
    // DEMANGLED: Packet_Approve_Join_Guild::Packet_Approve_Join_Guild(void)
    Packet_Approve_Join_Guild();
};
#pragma pack(pop)

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(MSG_APPROVE_JOIN_GUILD) == 0x11u,
              "CMD351 request must retain its old packed extent");
static_assert(offsetof(MSG_APPROVE_JOIN_GUILD, applicant_charac_no) == 0x0du,
              "CMD351 applicant character must remain at +0x0d");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
static_assert(sizeof(Packet_Approve_Join_Guild) == 0x16u,
              "guild approval packet must retain its old 22-byte ABI");
static_assert(offsetof(Packet_Approve_Join_Guild, guild_key) == 0x0au &&
                  offsetof(Packet_Approve_Join_Guild, approver_charac_no) ==
                      0x0eu &&
                  offsetof(Packet_Approve_Join_Guild, applicant_charac_no) ==
                      0x12u,
              "guild approval packet offsets changed");

// CMD351 / APPROVE_JOIN_GUILD.  This provider remains deliberately absent
// from PacketDispatcher's 606-slot registration until that complete root is
// reconstructed as one transaction.
class Dispatcher_ApproveJoinGuild
    : public IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85974CE
    // IDA_NAME: _ZN27Dispatcher_ApproveJoinGuildC2Ev
    // DEMANGLED: Dispatcher_ApproveJoinGuild::Dispatcher_ApproveJoinGuild(void)
    Dispatcher_ApproveJoinGuild();

    // IDA_EA: 0x824515A / 0x824518A
    // IDA_NAME: _ZN27Dispatcher_ApproveJoinGuildD2Ev /
    //           _ZN27Dispatcher_ApproveJoinGuildD0Ev
    // DEMANGLED: Dispatcher_ApproveJoinGuild::~Dispatcher_ApproveJoinGuild()
    virtual ~Dispatcher_ApproveJoinGuild();

    // IDA_EA: 0x81D505E / 0x81D50B4
    // IDA_NAME: _ZN27Dispatcher_ApproveJoinGuild4readER9PacketBufR8MSG_BASE /
    //           _ZN27Dispatcher_ApproveJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ApproveJoinGuild::read(PacketBuf &,MSG_BASE &) /
    //            Dispatcher_ApproveJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ApproveJoinGuild) == 0x04u,
              "CMD351 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_APPROVE_JOIN_GUILD_DISPATCHER_H_
