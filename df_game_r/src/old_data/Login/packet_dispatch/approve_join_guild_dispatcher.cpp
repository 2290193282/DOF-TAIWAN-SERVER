#include "old_data/Login/packet_dispatch/approve_join_guild_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kApproveJoinGuildReadSignature[] =
    "virtual int Dispatcher_ApproveJoinGuild::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// CMD351 owns the original concrete template/vtable family.  It is not wired
// into PacketDispatcher before the entire 606-slot root has a real owner.
// IDA_EA: 0x82515D8
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8251728
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8251732
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C4C0
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C4F0
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82517C4
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x8598AAE
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_APPROVE_JOIN_GUILD9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x822C342
// IDA_NAME: _ZN25Packet_Approve_Join_GuildC2Ev
// DEMANGLED: Packet_Approve_Join_Guild::Packet_Approve_Join_Guild(void)
Packet_Approve_Join_Guild::Packet_Approve_Join_Guild()
    : PacketHeader(0x1BC2u, 0x0016u),
      guild_key(0),
      approver_charac_no(0),
      applicant_charac_no(0) {}

// IDA_EA: 0x85974CE
// IDA_NAME: _ZN27Dispatcher_ApproveJoinGuildC2Ev
// DEMANGLED: Dispatcher_ApproveJoinGuild::Dispatcher_ApproveJoinGuild(void)
Dispatcher_ApproveJoinGuild::Dispatcher_ApproveJoinGuild()
    : IPacketDispatcher<MSG_APPROVE_JOIN_GUILD,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824515A
// IDA_NAME: _ZN27Dispatcher_ApproveJoinGuildD2Ev
// DEMANGLED: Dispatcher_ApproveJoinGuild::~Dispatcher_ApproveJoinGuild()
// IDA_EA: 0x824518A
// IDA_NAME: _ZN27Dispatcher_ApproveJoinGuildD0Ev
// DEMANGLED: Dispatcher_ApproveJoinGuild::~Dispatcher_ApproveJoinGuild()
Dispatcher_ApproveJoinGuild::~Dispatcher_ApproveJoinGuild() {}

// IDA_EA: 0x81D505E
// IDA_NAME: _ZN27Dispatcher_ApproveJoinGuild4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ApproveJoinGuild::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ApproveJoinGuild::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_APPROVE_JOIN_GUILD& request =
        static_cast<MSG_APPROVE_JOIN_GUILD&>(message);
    if (packet.get_int(request.applicant_charac_no) == 1) {
        return 0;
    }
    return LineFunc(13658, kApproveJoinGuildReadSignature, 0, 0);
}

// IDA_EA: 0x81D50B4
// IDA_NAME: _ZN27Dispatcher_ApproveJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ApproveJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ApproveJoinGuild::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    if (user->get_state() != 3 ||
        (user->IsGuildMaster() != 1 && user->IsSubGuildMaster() != 1)) {
        return 0;
    }

    const MSG_APPROVE_JOIN_GUILD& request =
        static_cast<const MSG_APPROVE_JOIN_GUILD&>(message);
    Packet_Approve_Join_Guild packet;
    packet.guild_key = static_cast<uint32_t>(user->get_charac_guildkey());
    packet.approver_charac_no =
        static_cast<uint32_t>(user->getCurCharacNo());
    packet.applicant_charac_no = request.applicant_charac_no;

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
    return 0;
}
