#include "infra/security/policy/police_one_to_one_chat_disable_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketHeader.h"

namespace {

// The original packet C1 is outside the six-member dispatcher claim. Keep its
// proven stack-only layout local to this sole consumer.
#pragma pack(push, 1)
struct DisableUserOneToOneChatPolicePacket {
    DisableUserOneToOneChatPolicePacket()
        : header(0x1f43u, 0x0030u), charac_no(0) {
        std::memset(target_name, 0, sizeof(target_name));
    }

    PacketHeader header;
    int charac_no;
    int target_value;
    char target_name[30];
};
#pragma pack(pop)

static_assert(sizeof(DisableUserOneToOneChatPolicePacket) == 0x30u,
              "CMD330 monitor packet must retain its old 48-byte layout");

}  // namespace

// IDA_EA: 0x859739A
// IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disableC2Ev
// DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::Dispatcer_polic_one_to_one_chat_disable(void)
Dispatcer_polic_one_to_one_chat_disable::Dispatcer_polic_one_to_one_chat_disable()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82402BE
// IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disableD2Ev
// DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::~Dispatcer_polic_one_to_one_chat_disable()
// IDA_EA: 0x82402EE
// IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disableD0Ev
// DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::~Dispatcer_polic_one_to_one_chat_disable()
Dispatcer_polic_one_to_one_chat_disable::~Dispatcer_polic_one_to_one_chat_disable() {}

// IDA_EA: 0x82272CE
// IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disable12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::dispatch_sig(CUser *,PacketBuf &)
int Dispatcer_polic_one_to_one_chat_disable::dispatch_sig(CUser* user,
                                                           PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }

    DisableUserOneToOneChatPolicePacket monitor_packet;
    packet.get_int(monitor_packet.target_value);
    packet.get_str(monitor_packet.target_name,
                   sizeof(monitor_packet.target_name),
                   monitor_packet.target_value);

    if (user->isGMUser()) {
        monitor_packet.charac_no = user->getCurCharacNo();
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendTcpPacket(reinterpret_cast<char*>(&monitor_packet), 0x30);
    }
    return 0;
}

// IDA_EA: 0x823429E
// IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disable4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::read(PacketBuf &,MSG_BASE &)
int Dispatcer_polic_one_to_one_chat_disable::read(PacketBuf& packet,
                                                   MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82342A8
// IDA_NAME: _ZN39Dispatcer_polic_one_to_one_chat_disable7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcer_polic_one_to_one_chat_disable::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcer_polic_one_to_one_chat_disable::process(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
