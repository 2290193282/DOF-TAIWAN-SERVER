#include "old_data/Login/packet_dispatch/guild_attendance_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kGuildAttendanceReadSignature[] =
    "virtual int Dispatcher_GuildAttendanceInfo::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// This is the sole concrete owner of the old MSG_GUILD_ATTENDANCE dispatcher
// template/vtable.  The root PacketDispatcher registration remains untouched.
// IDA_EA: 0x82511F4
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8251344
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x825134E
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C424
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C454
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82513E0
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x8598AE6
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_GUILD_ATTENDANCE9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_GUILD_ATTENDANCE,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_GUILD_ATTENDANCE,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597522
// IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfoC2Ev
// DEMANGLED: Dispatcher_GuildAttendanceInfo::Dispatcher_GuildAttendanceInfo(void)
Dispatcher_GuildAttendanceInfo::Dispatcher_GuildAttendanceInfo()
    : IPacketDispatcher<MSG_GUILD_ATTENDANCE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8245022
// IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfoD2Ev
// DEMANGLED: Dispatcher_GuildAttendanceInfo::~Dispatcher_GuildAttendanceInfo()
// IDA_EA: 0x8245052
// IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfoD0Ev
// DEMANGLED: Dispatcher_GuildAttendanceInfo::~Dispatcher_GuildAttendanceInfo()
Dispatcher_GuildAttendanceInfo::~Dispatcher_GuildAttendanceInfo() {}

// IDA_EA: 0x81D543C
// IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildAttendanceInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildAttendanceInfo::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_GUILD_ATTENDANCE& request =
        static_cast<MSG_GUILD_ATTENDANCE&>(message);
    if (packet.get_byte(request.request_type) == 1) {
        return 0;
    }
    return LineFunc(13785, kGuildAttendanceReadSignature, 0, 0);
}

// IDA_EA: 0x81D5492
// IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildAttendanceInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildAttendanceInfo::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)param;
    if (user->get_state() <= 2 || user->get_charac_guildkey() == 0) {
        return 0;
    }

    const MSG_GUILD_ATTENDANCE& request =
        static_cast<const MSG_GUILD_ATTENDANCE&>(message);
    Packet_Guild_Attendance_Info packet;
    packet.dword_0a = static_cast<uint32_t>(user->get_charac_guildkey());
    packet.dword_0e = static_cast<uint32_t>(user->getCurCharacNo());
    packet.byte_12 = request.request_type;

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
    return 0;
}
