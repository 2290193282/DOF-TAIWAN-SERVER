#include "old_data/channel/packet_dispatch/pvp_channel_info_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x822BD3C
// IDA_NAME: _ZN21Packet_PvPChannelInfoC2Ev
// DEMANGLED: Packet_PvPChannelInfo::Packet_PvPChannelInfo(void)
Packet_PvPChannelInfo::Packet_PvPChannelInfo()
    : header(kPacketType, kObjectBytes) {
    // The raw C2 calls only PacketHeader::C2.  Do not initialize
    // channel_count or entries: CMD198 transmits their raw stack state.
}

// IDA_EA: 0x859686C
// IDA_NAME: _ZN25Dispatcher_PvPChannelInfoC2Ev
// DEMANGLED: Dispatcher_PvPChannelInfo::Dispatcher_PvPChannelInfo(void)
Dispatcher_PvPChannelInfo::Dispatcher_PvPChannelInfo() {}

// IDA_EA: 0x824194A
// IDA_NAME: _ZN25Dispatcher_PvPChannelInfoD2Ev
// DEMANGLED: Dispatcher_PvPChannelInfo::~Dispatcher_PvPChannelInfo()
// IDA_EA: 0x824197A
// IDA_NAME: _ZN25Dispatcher_PvPChannelInfoD0Ev
// DEMANGLED: Dispatcher_PvPChannelInfo::~Dispatcher_PvPChannelInfo()
Dispatcher_PvPChannelInfo::~Dispatcher_PvPChannelInfo() {}

// IDA_EA: 0x82178C0
// IDA_NAME: _ZN25Dispatcher_PvPChannelInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PvPChannelInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PvPChannelInfo::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;

    if (user->get_state() <= 2) {
        return 0;
    }

    if (user->getCurCharacR() && user->isDisguiseCharac()) {
        user->setDisguiseCharac(false, 0, 0);
    }

    if (user->isCompetitionMercenary()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_PVP_CHANNEL_INFO, 21);
        return 0;
    }

    if (user->GetSchoolNo()) {
        // Default initialization invokes the raw C2 and deliberately leaves
        // channel_count and entries untouched.  Do not add {} here.
        Packet_PvPChannelInfo request;
        request.uid = static_cast<uint32_t>(user->GetUID());
        request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        request.request_value = 9;
        request.school_no = static_cast<uint32_t>(user->GetSchoolNo());
        request.header.size = Packet_PvPChannelInfo::kRequestBytes;

        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendTcpPacket(
            static_cast<char*>(static_cast<void*>(&request)),
            Packet_PvPChannelInfo::kRequestBytes);
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_PVP_CHANNEL_INFO);
    response.put_byte(1);
    response.put_int(0);
    response.put_byte(0);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233CEA
// IDA_NAME: _ZN25Dispatcher_PvPChannelInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PvPChannelInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PvPChannelInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233CF4
// IDA_NAME: _ZN25Dispatcher_PvPChannelInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PvPChannelInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PvPChannelInfo::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
