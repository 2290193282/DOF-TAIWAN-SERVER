#include "old_game/player/core/packet_dispatcher_upgrade_guild_cargo.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kUpgradeGuildCargoDispatch[] =
    "virtual int Dispatcher_UpgradeGuildCargo::dispatch_sig(CUser*, PacketBuf&)";
const INVEN_TYPE kInventory = static_cast<INVEN_TYPE>(1);
const int kGuildCargoMaterialItemId = 3261;

}  // namespace

// IDA_EA: 0x822BFEA
// IDA_NAME: _ZN32Packet_Guild_Guild_Cargo_UpgradeC2Ev
// DEMANGLED: Packet_Guild_Guild_Cargo_Upgrade::Packet_Guild_Guild_Cargo_Upgrade(void)
Packet_Guild_Guild_Cargo_Upgrade::Packet_Guild_Guild_Cargo_Upgrade()
    : PacketHeader(0x0713u, 0x001eu),
      charac_guildkey(0),
      charac_no(0),
      old_capacity(0),
      new_capacity(0),
      guild_fund_cost(0) {}

// IDA_EA: 0x8596E54
// IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargoC2Ev
// DEMANGLED: Dispatcher_UpgradeGuildCargo::Dispatcher_UpgradeGuildCargo(void)
Dispatcher_UpgradeGuildCargo::Dispatcher_UpgradeGuildCargo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240D68
// IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargoD2Ev
// IDA_EA: 0x8240D98
// IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargoD0Ev
// DEMANGLED: Dispatcher_UpgradeGuildCargo::~Dispatcher_UpgradeGuildCargo()
Dispatcher_UpgradeGuildCargo::~Dispatcher_UpgradeGuildCargo() {}

// IDA_EA: 0x8222958
// IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpgradeGuildCargo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeGuildCargo::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(56444, kUpgradeGuildCargoDispatch, 0, 0);
    }
    if (!user->get_charac_guildkey()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 100);
        return 0;
    }
    if (!user->IsExistGuildAgit()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 181);
        return 0;
    }
    if (!user->IsGuildMaster()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 36);
        return 0;
    }

    const int capacity = user->GetGuildCargoCapacity();
    STGuildStorageUpgradeInfo* const upgrade =
        G_CDataManager()->GetGuildParameterScriptForBuild()
            ->getGuildCargoUpradeInfo(capacity);
    if (!upgrade) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 1);
        return 0;
    }

    const uint32_t required_fund = static_cast<uint32_t>(upgrade->values[1]);
    const uint32_t required_power = static_cast<uint32_t>(upgrade->values[2]);
    const int required_material_count = upgrade->values[3];
    if (required_power > static_cast<uint32_t>(user->GetGuildPowerWarPoint())) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 179);
        return 0;
    }

    Inven_Item material;
    const int material_slot = user->getCurCharacInvenR()->GetInvenData(
        kGuildCargoMaterialItemId, material);
    if (required_material_count &&
        (material_slot == -1 || material.add_info_ < required_material_count)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 185);
        return 0;
    }
    if (required_fund > user->GetGuildDBInfo()->guild_fund) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 186);
        return 0;
    }

    if (required_material_count &&
        user->getCurCharacInvenW()->delete_item(
            kInventory,
            material_slot,
            required_material_count,
            static_cast<eItemDelReason>(32),
            true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_GUILD_CARGO, 185);
        return 0;
    }

    PacketGuard item_packet;
    item_packet.put_header(0, 14);
    item_packet.put_byte(0);
    item_packet.put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(kInventory,
                                               material_slot,
                                               item_packet);
    item_packet.finalize(true);
    user->Send(item_packet);

    user->DecreaseGuildPowerWarPoint(required_power);
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendIncreaseDecreasePowerWarPoint(
        static_cast<unsigned int>(user->get_charac_guildkey()),
        static_cast<unsigned int>(user->get_charac_no(-1)),
        -static_cast<int>(required_power));

    Packet_Guild_Guild_Cargo_Upgrade request;
    request.charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    request.charac_no = static_cast<unsigned int>(user->getCurCharacNo());
    request.old_capacity = capacity;
    request.new_capacity = capacity + upgrade->values[4];
    request.guild_fund_cost = required_fund;
    proxy->SendTcpPacket(reinterpret_cast<char*>(&request), request.size);
    return 0;
}

// IDA_EA: 0x8233FE2
// IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeGuildCargo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeGuildCargo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233FEC
// IDA_NAME: _ZN28Dispatcher_UpgradeGuildCargo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeGuildCargo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeGuildCargo::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
