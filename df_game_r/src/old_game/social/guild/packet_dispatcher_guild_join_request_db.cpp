#include "old_game/social/guild/packet_dispatcher_guild_join_request_db.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/log/format/trace.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x822DD7C
// IDA_NAME: _ZN20MSG_CHECK_GUILD_JOINC2Ev
// DEMANGLED: MSG_CHECK_GUILD_JOIN::MSG_CHECK_GUILD_JOIN(void)
MSG_CHECK_GUILD_JOIN::MSG_CHECK_GUILD_JOIN()
    : MSG_BASE(), guild_name_length(0), guild_name() {}

// The three explicit specializations have one canonical C++ object/vtable
// owner.  CMD366 reuses the existing MSG_BASE state-0 specialization.
// IDA_EA: 0x8251BBA
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8251D16
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8251D20
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8251DB2
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x825C5AA
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C5DA
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598A5A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CHECK_GUILD_JOIN9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHECK_GUILD_JOIN,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CHECK_GUILD_JOIN,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x82517CA
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x825191A
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8251924
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82519B6
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x825C50E
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C53E
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598A92
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_CANCEL_JOIN_GUILD9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CANCEL_JOIN_GUILD,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x82513E6
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8251536
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8251540
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82515D2
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x825C472
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C4A2
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598ACA
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_DENY_JOIN_GUILD9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_DENY_JOIN_GUILD,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_DENY_JOIN_GUILD,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x859747A
// IDA_NAME: _ZN25Dispatcher_CheckJoinGuildC2Ev
// DEMANGLED: Dispatcher_CheckJoinGuild::Dispatcher_CheckJoinGuild(void)
Dispatcher_CheckJoinGuild::Dispatcher_CheckJoinGuild() {}

// IDA_EA: 0x8245244
// IDA_NAME: _ZN25Dispatcher_CheckJoinGuildD2Ev
// DEMANGLED: Dispatcher_CheckJoinGuild::~Dispatcher_CheckJoinGuild()
// IDA_EA: 0x8245274
// IDA_NAME: _ZN25Dispatcher_CheckJoinGuildD0Ev
// DEMANGLED: Dispatcher_CheckJoinGuild::~Dispatcher_CheckJoinGuild()
Dispatcher_CheckJoinGuild::~Dispatcher_CheckJoinGuild() {}

// IDA_EA: 0x81D4B4C
// IDA_NAME: _ZN25Dispatcher_CheckJoinGuild4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CheckJoinGuild::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CheckJoinGuild::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CHECK_GUILD_JOIN& input =
        static_cast<MSG_CHECK_GUILD_JOIN&>(message);
    if (packet.get_int(input.guild_name_length) != 1) {
        return LineFunc(13531,
                        "virtual int Dispatcher_CheckJoinGuild::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_str(input.guild_name,
                       static_cast<int>(sizeof(input.guild_name)),
                       input.guild_name_length) == 1) {
        return 0;
    }
    return LineFunc(13534,
                    "virtual int Dispatcher_CheckJoinGuild::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81D4BF6
// IDA_NAME: _ZN25Dispatcher_CheckJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CheckJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CheckJoinGuild::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)param;
    if (user->get_state() != 3) {
        return 0;
    }
    if (user->get_charac_guildkey()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHECK_JOIN_GUILD, 18);
        return 0;
    }

    const MSG_CHECK_GUILD_JOIN& input =
        static_cast<const MSG_CHECK_GUILD_JOIN&>(message);
    SIG_CHECK_JOIN_GUILD request = {{0}, 0};
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        request.server_id = user->GetServerGroup();
    } else {
        request.server_id = G_CEnvironment()->get_server_group();
    }
    std::strncpy(request.guild_name, input.guild_name, 22u);
    DB_CheckJoinGuild::makeRequest(user->GetUID(), &request);
    return 0;
}

// IDA_EA: 0x85974B2
// IDA_NAME: _ZN26Dispatcher_CancelJoinGuildC2Ev
// DEMANGLED: Dispatcher_CancelJoinGuild::Dispatcher_CancelJoinGuild(void)
Dispatcher_CancelJoinGuild::Dispatcher_CancelJoinGuild() {}

// IDA_EA: 0x82451A8
// IDA_NAME: _ZN26Dispatcher_CancelJoinGuildD2Ev
// DEMANGLED: Dispatcher_CancelJoinGuild::~Dispatcher_CancelJoinGuild()
// IDA_EA: 0x82451D8
// IDA_NAME: _ZN26Dispatcher_CancelJoinGuildD0Ev
// DEMANGLED: Dispatcher_CancelJoinGuild::~Dispatcher_CancelJoinGuild()
Dispatcher_CancelJoinGuild::~Dispatcher_CancelJoinGuild() {}

// IDA_EA: 0x81D4F76
// IDA_NAME: _ZN26Dispatcher_CancelJoinGuild4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CancelJoinGuild::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CancelJoinGuild::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CANCEL_JOIN_GUILD& input =
        static_cast<MSG_CANCEL_JOIN_GUILD&>(message);
    if (packet.get_int(input.guild_id) == 1) {
        return 0;
    }
    return LineFunc(13628,
                    "virtual int Dispatcher_CancelJoinGuild::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81D4FCC
// IDA_NAME: _ZN26Dispatcher_CancelJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CancelJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CancelJoinGuild::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)param;
    if (user->get_state() != 3) {
        return 0;
    }
    if (!user->get_charac_guildkey()) {
        const MSG_CANCEL_JOIN_GUILD& input =
            static_cast<const MSG_CANCEL_JOIN_GUILD&>(message);
        DB_CancelJoinGuild::makeRequest(
            user->GetUID(),
            input.guild_id,
            static_cast<unsigned int>(user->getCurCharacNo()));
        user->SendCmdOkPacket(ENUM_CMDPACKET_CANCEL_JOIN_GUILD);
    }
    return 0;
}

// IDA_EA: 0x85974EA
// IDA_NAME: _ZN24Dispatcher_DenyJoinGuildC2Ev
// DEMANGLED: Dispatcher_DenyJoinGuild::Dispatcher_DenyJoinGuild(void)
Dispatcher_DenyJoinGuild::Dispatcher_DenyJoinGuild() {}

// IDA_EA: 0x824510C
// IDA_NAME: _ZN24Dispatcher_DenyJoinGuildD2Ev
// DEMANGLED: Dispatcher_DenyJoinGuild::~Dispatcher_DenyJoinGuild()
// IDA_EA: 0x824513C
// IDA_NAME: _ZN24Dispatcher_DenyJoinGuildD0Ev
// DEMANGLED: Dispatcher_DenyJoinGuild::~Dispatcher_DenyJoinGuild()
Dispatcher_DenyJoinGuild::~Dispatcher_DenyJoinGuild() {}

// IDA_EA: 0x81D5190
// IDA_NAME: _ZN24Dispatcher_DenyJoinGuild4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DenyJoinGuild::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DenyJoinGuild::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DENY_JOIN_GUILD& input =
        static_cast<MSG_DENY_JOIN_GUILD&>(message);
    if (packet.get_int(input.charac_no) == 1) {
        return 0;
    }
    return LineFunc(13695,
                    "virtual int Dispatcher_DenyJoinGuild::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81D51E6
// IDA_NAME: _ZN24Dispatcher_DenyJoinGuild7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DenyJoinGuild::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DenyJoinGuild::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)param;
    if (user->get_state() != 3) {
        return 0;
    }
    if (user->IsGuildMaster() || user->IsSubGuildMaster()) {
        const MSG_DENY_JOIN_GUILD& input =
            static_cast<const MSG_DENY_JOIN_GUILD&>(message);
        DB_DenyJoinGuild::makeRequest(
            user->GetUID(),
            static_cast<unsigned int>(user->get_charac_guildkey()),
            input.charac_no);
        user->SendCmdOkPacket(ENUM_CMDPACKET_DENY_JOIN_GUILD);
    }
    return 0;
}

// IDA_EA: 0x8597592
// IDA_NAME: _ZN24Dispatcher_JoinGuildInfoC2Ev
// DEMANGLED: Dispatcher_JoinGuildInfo::Dispatcher_JoinGuildInfo(void)
Dispatcher_JoinGuildInfo::Dispatcher_JoinGuildInfo() {}

// IDA_EA: 0x8245070
// IDA_NAME: _ZN24Dispatcher_JoinGuildInfoD2Ev
// DEMANGLED: Dispatcher_JoinGuildInfo::~Dispatcher_JoinGuildInfo()
// IDA_EA: 0x82450A0
// IDA_NAME: _ZN24Dispatcher_JoinGuildInfoD0Ev
// DEMANGLED: Dispatcher_JoinGuildInfo::~Dispatcher_JoinGuildInfo()
Dispatcher_JoinGuildInfo::~Dispatcher_JoinGuildInfo() {}

// IDA_EA: 0x81D538E
// IDA_NAME: _ZN24Dispatcher_JoinGuildInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_JoinGuildInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_JoinGuildInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D5398
// IDA_NAME: _ZN24Dispatcher_JoinGuildInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_JoinGuildInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_JoinGuildInfo::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)message;
    (void)param;
    if (user->get_state() != 3 || user->get_charac_guildkey()) {
        return 0;
    }

    ENUM_SERVER_GROUP server_group;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        server_group =
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup());
    } else {
        server_group = static_cast<ENUM_SERVER_GROUP>(
            G_CEnvironment()->get_server_group());
    }
    DB_JoinGuildInfo::makeRequest(
        user->GetUID(),
        server_group,
        static_cast<unsigned int>(user->getCurCharacNo()));
    return 0;
}
