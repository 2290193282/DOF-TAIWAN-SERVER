#include "old_data/Login/packet_dispatch/donate_guild_fund_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kDonateGuildFundReadSignature[] =
    "virtual int Dispatcher_DonateGuildFund::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// CMD347 owns this concrete template/vtable family. The 606-slot root remains
// deliberately untouched.
// IDA_EA: 0x8251DB8
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8251F08
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8251F12
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C5F8
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C628
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8251FA6
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::send(CUser *,ParamBase&)
// IDA_EA: 0x8598A3E
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_DONATE_GUILD_FUND9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_DONATE_GUILD_FUND,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_DONATE_GUILD_FUND,
                                 ParamBase,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x859745E
// IDA_NAME: _ZN26Dispatcher_DonateGuildFundC2Ev
// DEMANGLED: Dispatcher_DonateGuildFund::Dispatcher_DonateGuildFund(void)
Dispatcher_DonateGuildFund::Dispatcher_DonateGuildFund()
    : IPacketDispatcher<MSG_DONATE_GUILD_FUND,
                        ParamBase,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245292
// IDA_NAME: _ZN26Dispatcher_DonateGuildFundD2Ev
// DEMANGLED: Dispatcher_DonateGuildFund::~Dispatcher_DonateGuildFund()
// IDA_EA: 0x82452C2
// IDA_NAME: _ZN26Dispatcher_DonateGuildFundD0Ev
// DEMANGLED: Dispatcher_DonateGuildFund::~Dispatcher_DonateGuildFund()
Dispatcher_DonateGuildFund::~Dispatcher_DonateGuildFund() {}

// IDA_EA: 0x81D4920
// IDA_NAME: _ZN26Dispatcher_DonateGuildFund4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DonateGuildFund::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DonateGuildFund::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DONATE_GUILD_FUND& request =
        static_cast<MSG_DONATE_GUILD_FUND&>(message);
    if (packet.get_int(request.fund_amount) == 1) {
        return 0;
    }
    return LineFunc(13457, kDonateGuildFundReadSignature, 0, 0);
}

// IDA_EA: 0x81D4970
// IDA_NAME: _ZN26Dispatcher_DonateGuildFund7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DonateGuildFund::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DonateGuildFund::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)param;
    if (user->get_state() <= 2) {
        return 0;
    }

    const int protection_error =
        GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(10));
    if (protection_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DONATE_GUILD_FUND,
                                 static_cast<unsigned char>(protection_error));
        return 0;
    }
    if (user->get_charac_guildkey() == 0) {
        return 0;
    }

    const MSG_DONATE_GUILD_FUND& request =
        static_cast<const MSG_DONATE_GUILD_FUND&>(message);
    if (request.fund_amount > user->getCurCharacMoney()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DONATE_GUILD_FUND, 10);
        return 0;
    }
    if (user->IsAddableGuildFund(request.fund_amount) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DONATE_GUILD_FUND, 95);
        return 0;
    }
    if (user->getCurCharacInvenW()->use_money(
            request.fund_amount,
            static_cast<eMoneySubReason>(42),
            true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DONATE_GUILD_FUND, 10);
        return 0;
    }

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendAddGuildFund(
        static_cast<unsigned int>(user->get_charac_no(-1)),
        static_cast<unsigned int>(user->get_charac_guildkey()),
        request.fund_amount,
        Packet_Add_Guild_Fund::kFundTypeUserDonation);
    return 0;
}
