#include "old_data/Login/packet_dispatch/request_join_guild_dispatcher.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_tools.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kRequestJoinGuildReadSignature[] =
    "virtual int Dispatcher_RequestJoinGuild::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// CMD349 owns this old concrete state-0 template/vtable family.  Its root
// PacketDispatcher slot remains intentionally unregistered until the full
// 606-entry root table has a single real owner.
// IDA_EA: 0x82519BC
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8251B18
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8251B22
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C55C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C58C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8251BB4
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x8598A76
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_REQUEST_GUILD_JOIN9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597496
// IDA_NAME: _ZN27Dispatcher_RequestJoinGuildC2Ev
// DEMANGLED: Dispatcher_RequestJoinGuild::Dispatcher_RequestJoinGuild(void)
Dispatcher_RequestJoinGuild::Dispatcher_RequestJoinGuild()
    : IPacketDispatcher<MSG_REQUEST_GUILD_JOIN,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82451F6
// IDA_NAME: _ZN27Dispatcher_RequestJoinGuildD2Ev
// DEMANGLED: Dispatcher_RequestJoinGuild::~Dispatcher_RequestJoinGuild()
// IDA_EA: 0x8245226
// IDA_NAME: _ZN27Dispatcher_RequestJoinGuildD0Ev
// DEMANGLED: Dispatcher_RequestJoinGuild::~Dispatcher_RequestJoinGuild()
Dispatcher_RequestJoinGuild::~Dispatcher_RequestJoinGuild() {}

// IDA_EA: 0x81D4CE2
// IDA_NAME: _ZN27Dispatcher_RequestJoinGuild4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestJoinGuild::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestJoinGuild::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_REQUEST_GUILD_JOIN& request =
        static_cast<MSG_REQUEST_GUILD_JOIN&>(message);
    if (packet.get_int(request.guild_name_length) != 1) {
        return LineFunc(13574, kRequestJoinGuildReadSignature, 0, 0);
    }
    if (packet.get_str(request.guild_name,
                       static_cast<int>(sizeof(request.guild_name)),
                       request.guild_name_length) != 1) {
        return LineFunc(13577, kRequestJoinGuildReadSignature, 0, 0);
    }
    if (packet.get_int(request.memo_length) != 1) {
        return LineFunc(13581, kRequestJoinGuildReadSignature, 0, 0);
    }
    if (packet.get_str(request.memo,
                       static_cast<int>(sizeof(request.memo)),
                       request.memo_length) == 1) {
        return 0;
    }
    return LineFunc(13584, kRequestJoinGuildReadSignature, 0, 0);
}

// IDA_EA: 0x81D4E28
// IDA_NAME: _ZN27Dispatcher_RequestJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestJoinGuild::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    if (user->get_state() != 3) {
        return 0;
    }
    if (user->get_charac_guildkey()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_JOIN_GUILD, 18);
        return 0;
    }

    const MSG_REQUEST_GUILD_JOIN& request =
        static_cast<const MSG_REQUEST_GUILD_JOIN&>(message);
    SIG_REQUEST_JOIN_GUILD db_request;
    std::memset(&db_request, 0, sizeof(db_request));
    std::strncpy(db_request.guild_name, request.guild_name, 0x16u);
    db_request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    db_request.m_id = static_cast<uint32_t>(user->get_acc_id());
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        db_request.server_group = user->GetServerGroup();
    } else {
        db_request.server_group = G_CEnvironment()->get_server_group();
    }
    std::memcpy(db_request.born_year, user->GetSsnString(), 2u);
    std::strncpy(db_request.memo, request.memo, 0x19u);
    DB_RequestJoinGuild::makeRequest(user->GetUID(), &db_request);
    return 0;
}
