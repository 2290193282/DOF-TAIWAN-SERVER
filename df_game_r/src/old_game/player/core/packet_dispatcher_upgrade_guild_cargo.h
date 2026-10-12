#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_UPGRADE_GUILD_CARGO_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_UPGRADE_GUILD_CARGO_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 06_userinfo_stat_inventory/Packet_Guild_Guild_Cargo_Upgrade.
// CMD269 fills every five-word payload member before forwarding this TCP
// packet to the guild server.
#pragma pack(push, 1)
struct Packet_Guild_Guild_Cargo_Upgrade : PacketHeader {
    // IDA_EA: 0x822BFEA
    // IDA_NAME: _ZN32Packet_Guild_Guild_Cargo_UpgradeC2Ev
    // DEMANGLED: Packet_Guild_Guild_Cargo_Upgrade::Packet_Guild_Guild_Cargo_Upgrade(void)
    Packet_Guild_Guild_Cargo_Upgrade();

    uint32_t charac_guildkey;   // +0x0a
    uint32_t charac_no;         // +0x0e
    int32_t old_capacity;       // +0x12
    int32_t new_capacity;       // +0x16
    uint32_t guild_fund_cost;   // +0x1a
};
#pragma pack(pop)

static_assert(sizeof(Packet_Guild_Guild_Cargo_Upgrade) == 0x1eu,
              "guild-cargo upgrade packet must retain its 30-byte ABI");
static_assert(offsetof(Packet_Guild_Guild_Cargo_Upgrade, charac_guildkey) ==
                  0x0au &&
                  offsetof(Packet_Guild_Guild_Cargo_Upgrade, guild_fund_cost) ==
                      0x1au,
              "guild-cargo upgrade packet fields drifted");

// CMD269 / UPGRADE_GUILD_CARGO.  Its direct dispatcher stays unregistered
// while PacketDispatcher's complete original slot transaction remains absent.
class Dispatcher_UpgradeGuildCargo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596E54
    // IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargoC2Ev
    // DEMANGLED: Dispatcher_UpgradeGuildCargo::Dispatcher_UpgradeGuildCargo(void)
    Dispatcher_UpgradeGuildCargo();

    // IDA_EA: 0x8240D68
    // IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargoD2Ev
    // IDA_EA: 0x8240D98
    // IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargoD0Ev
    // DEMANGLED: Dispatcher_UpgradeGuildCargo::~Dispatcher_UpgradeGuildCargo()
    virtual ~Dispatcher_UpgradeGuildCargo();

    // IDA_EA: 0x8222958
    // IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargo12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_UpgradeGuildCargo::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233FE2
    // IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargo4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UpgradeGuildCargo::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233FEC
    // IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UpgradeGuildCargo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

static_assert(sizeof(void*) != 4 || sizeof(Dispatcher_UpgradeGuildCargo) ==
                  0x04u,
              "CMD269 dispatcher must be a base-only i386 object");

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_UPGRADE_GUILD_CARGO_H_
