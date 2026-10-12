#include "old_game/social/guild/guild_server_proxy_abi.h"

#include "infra/network/transport/game_server_registration_packet.h"
#include "old_game/economy/mail/mail_trade_packets.h"

#include <errno.h>
#include <cstring>
#include <stdint.h>

#include "old_game/pvp/battlefield/assault_power_war_packet_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/economy/inventory/item.h"
#include "core/config/env/environment.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/udp_packets.h"

// IDA_EA: 0x82344DE
// IDA_NAME: _ZN12STGuildProxyC2Ev
// DEMANGLED: STGuildProxy::STGuildProxy(void)
STGuildProxy::STGuildProxy() {
    std::memset(guild_name, 0, sizeof(guild_name));
    guild_level = 0;
    power_side = 0;
    guild_agit_flag = 0;
}

// IDA_EA: 0x823451C
// IDA_NAME: _ZN12STGuildProxyC2EPKchh
// DEMANGLED: STGuildProxy::STGuildProxy(char const*,unsigned char,unsigned char)
STGuildProxy::STGuildProxy(const char* source,
                           uint8_t guild_level_value,
                           uint8_t power_side_value) {
    // The original C2 writes the two changed bytes before copying exactly the
    // old 22-byte guild-name prefix.  It deliberately does not clear the
    // final name byte or guild_agit_flag, so a delegating/default construction
    // would alter the ABI.
    guild_level = guild_level_value;
    power_side = power_side_value;
    std::memcpy(guild_name, source, sizeof(guild_name) - 1u);
}

// IDA_EA: 0x846F420
// IDA_NAME: _ZN34Packet_Monitor_Call_Guild_Level_UpC2Ev
// DEMANGLED: Packet_Monitor_Call_Guild_Level_Up::Packet_Monitor_Call_Guild_Level_Up(void)
Packet_Monitor_Call_Guild_Level_Up::Packet_Monitor_Call_Guild_Level_Up()
    : PacketHeader(0x0408u, 0x001bu),
      uid(-1),
      charac_no(0),
      charac_guildkey(0),
      raw_16(0),
      levelup_price(0) {}

// IDA_EA: 0x846F376
// IDA_NAME: _ZN33Packet_Monitor_Increase_Guild_ExpC2Ev
// DEMANGLED: Packet_Monitor_Increase_Guild_Exp::Packet_Monitor_Increase_Guild_Exp(void)
Packet_Monitor_Increase_Guild_Exp::Packet_Monitor_Increase_Guild_Exp()
    : PacketHeader(0x040eu, 0x001cu), notify(0) {}

// IDA_EA: 0x846F5BE
// IDA_NAME: _ZN46Packet_Guild_Increase_Decrease_Power_War_PointC2Ev
// DEMANGLED: Packet_Guild_Increase_Decrease_Power_War_Point::Packet_Guild_Increase_Decrease_Power_War_Point(void)
Packet_Guild_Increase_Decrease_Power_War_Point::
    Packet_Guild_Increase_Decrease_Power_War_Point()
    : PacketHeader(0x06dbu, 0x0016u),
      charac_guildkey(0),
      charac_no(0),
      point_delta(0) {}

// IDA_EA: 0x846F78C
// IDA_NAME: _ZN37Packet_Guild_Apply_Origial_Power_SideC2Ev
// DEMANGLED: Packet_Guild_Apply_Origial_Power_Side::Packet_Guild_Apply_Origial_Power_Side(void)
Packet_Guild_Apply_Origial_Power_Side::
    Packet_Guild_Apply_Origial_Power_Side()
    : PacketHeader(0x06b2u, 0x0013u),
      charac_no(0),
      charac_guildkey(0),
      power_side(0) {}

// IDA_EA: 0x846F6FE
// IDA_NAME: _ZN22Packet_Guild_Exp_LimitC2Ev
// DEMANGLED: Packet_Guild_Exp_Limit::Packet_Guild_Exp_Limit(void)
Packet_Guild_Exp_Limit::Packet_Guild_Exp_Limit()
    : PacketHeader(0x06e5u, 0x005au),
      channel_no(0),
      guild_levelup_param_09_value_04(0),
      guild_levelup_param_17_value_04(0),
      guild_levelup_param_value_04() {}

// IDA_EA: 0x846FB20
// IDA_NAME: _ZN21Packet_Add_Guild_FundC2Ev
// DEMANGLED: Packet_Add_Guild_Fund::Packet_Add_Guild_Fund(void)
Packet_Add_Guild_Fund::Packet_Add_Guild_Fund()
    : PacketHeader(0x1BBCu, 0x0017u),
      fund_type(0),
      fund_amount(0),
      charac_no(0),
      charac_guildkey(0) {}

// IDA_EA: 0x846F3FC
// IDA_NAME: _ZN30Packet_Monitor_Buy_Guild_SkillC2Ev
Packet_Monitor_Buy_Guild_Skill::Packet_Monitor_Buy_Guild_Skill()
    : PacketHeader(0x0429u, 0x0020u),
      charac_no(0),
      charac_guildkey(0),
      skill_index(0),
      skill_level(0),
      guild_skill_point(0),
      gold_cost(0) {}

// IDA_EA: 0x846E460
// IDA_NAME: _ZN17CGuildServerProxy17SendPowerWarPointER35Packet_Guild_Change_Power_War_Point
// DEMANGLED: CGuildServerProxy::SendPowerWarPoint(Packet_Guild_Change_Power_War_Point &)
char CGuildServerProxy::SendPowerWarPoint(
    Packet_Guild_Change_Power_War_Point& packet) {
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(30);
    const char eventing = static_cast<char>(repeat_event->IsEventing(0));
    if (eventing) {
        return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
    }
    return eventing;
}

// IDA_EA: 0x846EEDA
// IDA_NAME: _ZN17CGuildServerProxy26SendApplyOriginalPowerSideEjjh
// DEMANGLED: CGuildServerProxy::SendApplyOriginalPowerSide(
//             unsigned int,unsigned int,unsigned char)
char CGuildServerProxy::SendApplyOriginalPowerSide(
    unsigned int charac_no,
    unsigned int charac_guildkey,
    unsigned char power_side) {
    Packet_Guild_Apply_Origial_Power_Side packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    packet.power_side = power_side;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x82A6A96
// IDA_NAME: _ZN17CGuildServerProxyC2Ev
// DEMANGLED: CGuildServerProxy::CGuildServerProxy(void)
CGuildServerProxy::CGuildServerProxy()
    : server_ip_(),
      udp_port_(0),
      guild_map_(),
      tcp_port_(0),
      tcp_handler_(),
      udp_handler_() {}

// IDA_EA: 0x82B31BE
// IDA_NAME: _ZN17CGuildServerProxyD2Ev
// DEMANGLED: CGuildServerProxy::~CGuildServerProxy()
CGuildServerProxy::~CGuildServerProxy() {}

// IDA_EA: 0x846D8C2
// IDA_NAME: _ZN17CGuildServerProxy4InitEv
// DEMANGLED: CGuildServerProxy::Init(void)
bool CGuildServerProxy::Init() {
    if (!udp_port_) {
        return true;
    }
    return InitUdp() && InitTcp();
}

// IDA_EA: 0x82A6B3A
// IDA_NAME: _ZN17CGuildServerProxy4InitESsii
// DEMANGLED: CGuildServerProxy::Init(std::string,int,int)
bool CGuildServerProxy::Init(std::string server_ip,
                             int udp_port,
                             int tcp_port) {
    server_ip_ = server_ip;
    udp_port_ = static_cast<unsigned int>(udp_port);
    tcp_port_ = static_cast<unsigned int>(tcp_port);
    return Init();
}

// IDA_EA: 0x846D912
// IDA_NAME: _ZN17CGuildServerProxy7InitUdpEv
// DEMANGLED: CGuildServerProxy::InitUdp(void)
bool CGuildServerProxy::InitUdp() {
    if (udp_handler_.InitClientSocket() != -1) {
        return true;
    }
    LogManager::logFormat(1,
                          "DF_GuildServerProxy.cpp",
                          "bool CGuildServerProxy::InitUdp()",
                          0x4D4,
                          "[CGuildServerProxy::Init] Can't init udp client socket\n");
    return false;
}

// IDA_EA: 0x846D96C
// IDA_NAME: _ZN17CGuildServerProxy7InitTcpEv
// DEMANGLED: CGuildServerProxy::InitTcp(void)
bool CGuildServerProxy::InitTcp() {
    if (!udp_port_) {
        return true;
    }
    if (tcp_handler_.get_socket() < 0) {
        int& socket = *reinterpret_cast<int*>(&tcp_handler_);
        const int result = CNetwork100000_100000::CreateConnectionSocket(
            socket, server_ip_.c_str(), static_cast<int>(tcp_port_));
        if (result != 0) {
            cMyTrace trace("bool CGuildServerProxy::InitTcp()", 1258, 0);
            trace("CGuildServerProxy CreateConnectionSocket() error(%d:%d:%s)",
                  result,
                  errno,
                  std::strerror(errno));
            return false;
        }
    }
    cMyTrace trace("bool CGuildServerProxy::InitTcp()", 1263, 9, true, true);
    trace("[!] Connect To Guild Server ...");
    return true;
}

// IDA_EA: 0x846DA84
// IDA_NAME: _ZN17CGuildServerProxy10DisconnectEv
// DEMANGLED: CGuildServerProxy::Disconnect(void)
int CGuildServerProxy::Disconnect() {
    return tcp_handler_.disconnect();
}

// IDA_EA: 0x846E53C
// IDA_NAME: _ZN17CGuildServerProxy9OnTcpRecvEv
// DEMANGLED: CGuildServerProxy::OnTcpRecv(void)
int CGuildServerProxy::OnTcpRecv() {
    const int received_bytes = tcp_handler_.recv_packet();
    if (received_bytes > 0) {
        return Parsing(received_bytes) == 1 ? 1 : 0;
    }
    if (received_bytes >= 0) {
        return 1;
    }

    LogManager::logFormat(1,
                          "DF_GuildServerProxy.cpp",
                          "bool CGuildServerProxy::OnTcpRecv()",
                          0x71F,
                          "Guild Server maybe Die!!!");
    Disconnect();
    return 0;
}

// IDA_EA: 0x846E5CC
// IDA_NAME: _ZN17CGuildServerProxy9OnTcpSendEv
// DEMANGLED: CGuildServerProxy::OnTcpSend(void)
int CGuildServerProxy::OnTcpSend() {
    if (tcp_handler_.send_packet() >= 0) {
        return tcp_handler_.GetRemainSendSize();
    }

    const char* const error_text = std::strerror(errno);
    const int socket_fd = GetTcpHandler()->get_socket();
    cMyTrace trace("int CGuildServerProxy::OnTcpSend()", 1843, 0);
    trace("OnSend() Socket[%d] Error(%s)", socket_fd, error_text);
    Disconnect();
    return -1;
}

// IDA_EA: 0x846E66C
// IDA_NAME: _ZN17CGuildServerProxy10OnTcpErrorEv
// DEMANGLED: CGuildServerProxy::OnTcpError(void)
int CGuildServerProxy::OnTcpError() {
    if (errno == 0) {
        return 1;
    }

    const char* const error_text = std::strerror(errno);
    const int socket_fd = GetTcpHandler()->get_socket();
    cMyTrace trace("bool CGuildServerProxy::OnTcpError()", 1861, 0);
    trace("GuildServer OnError() Socket[%d] Error(%s)", socket_fd, error_text);
    return 0;
}

// IDA_EA: 0x846E6F6
// IDA_NAME: _ZN17CGuildServerProxy7ParsingEi
// DEMANGLED: CGuildServerProxy::Parsing(int)
int CGuildServerProxy::Parsing(int received_bytes) {
    return tcp_handler_.Parsing_ServerPacket(received_bytes);
}

// IDA_EA: 0x846DAF4
// IDA_NAME: _ZN17CGuildServerProxy13SendHeartBeatEi
// DEMANGLED: CGuildServerProxy::SendHeartBeat(int)
int CGuildServerProxy::SendHeartBeat(int channel_no) {
    Packet_Monitor_UDP_HeartBeat packet;
    // Official 0627 ELF 0x846DB05..0x846DB08 stores the argument's low
    // byte at packet+0x0A, matching the monitor-proxy heartbeat ABI.
    packet.state = static_cast<uint8_t>(channel_no);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846DB2A
// IDA_NAME: _ZN17CGuildServerProxy9SendLoginEijiPchS0_
// DEMANGLED: CGuildServerProxy::SendLogin(int,unsigned int,int,char *,unsigned char,char *)
char CGuildServerProxy::SendLogin(int uid,
                                  unsigned int account_id,
                                  int channel_no,
                                  char* peer_ip,
                                  unsigned char sex,
                                  char* ssn) {
    Packet_Monitor_UDP_Login packet;
    packet.uid = static_cast<uint32_t>(uid);
    packet.account_id = account_id;
    packet.channel_no = static_cast<uint8_t>(channel_no);
    if (peer_ip) {
        std::strncpy(reinterpret_cast<char*>(packet.peer_ip), peer_ip,
                     sizeof(packet.peer_ip));
    } else {
        LogManager::logFormat(
            1, "DF_GuildServerProxy.cpp",
            "void CGuildServerProxy::SendLogin(int, memberIdentificationNumber_t, int, char*, unsigned char, char*)",
            0x533, "CGuildServerProxy::SendLogin! DB ID(%u)", account_id);
    }
    packet.sex = sex;
    std::memcpy(packet.ssn, ssn, 6u);
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA unit: 99_misc_global/Packet_Refresh_Guild_Info.  PacketHeader is packed
// to 10 bytes, followed at +0x0A/+0x0E by two 32-bit values.
#pragma pack(push, 1)
struct Packet_Refresh_Guild_Info : PacketHeader {
    unsigned int charac_no;
    unsigned int charac_guildkey;

    // IDA_EA: 0x846FB68
    // IDA_NAME: _ZN25Packet_Refresh_Guild_InfoC2Ev
    Packet_Refresh_Guild_Info();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Refresh_Guild_Info) == 0x12u,
              "Packet_Refresh_Guild_Info must retain the old 18-byte wire ABI");
static_assert(sizeof(Packet_Request_Guild_Create) == 0x32u,
              "guild-create TCP request must retain the old 0x32-byte ABI");

// IDA_EA: 0x82B3212
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyEC2Ev
// DEMANGLED: CServerProxyMgr<CGuildServerProxy>::CServerProxyMgr(void)
CServerProxyMgr<CGuildServerProxy>::CServerProxyMgr()
    : server_groups_(0),
      proxies_(0),
      proxy_count_(0),
      has_proxy_array_(0),
      reserved_0d_{0, 0, 0},
      default_proxy_() {
    default_proxy_.Init();
}

// IDA_EA: 0x82B3AC6
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyED2Ev
// DEMANGLED: CServerProxyMgr<CGuildServerProxy>::~CServerProxyMgr()
CServerProxyMgr<CGuildServerProxy>::~CServerProxyMgr() {
    if (has_proxy_array_) {
        delete[] server_groups_;
        server_groups_ = 0;
        delete[] proxies_;
        proxies_ = 0;
    }
    proxy_count_ = 0;
}

// IDA_EA: 0x811D3B8
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyE14GetServerProxyE17ENUM_SERVER_GROUP
// DEMANGLED: CServerProxyMgr<CGuildServerProxy>::GetServerProxy(ENUM_SERVER_GROUP)
CGuildServerProxy*
CServerProxyMgr<CGuildServerProxy>::GetServerProxy(
    ENUM_SERVER_GROUP server_group) {
    if (!server_group) {
        return &default_proxy_;
    }
    for (int index = 0; index < proxy_count_; ++index) {
        if (server_groups_ && server_groups_[index] == server_group) {
            return proxies_ + index;
        }
    }
    return &default_proxy_;
}

// IDA_EA: 0x82B3C50
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyE13GetStartIndexEv
int CServerProxyMgr<CGuildServerProxy>::GetStartIndex() {
    if (proxy_count_ > 0 && has_proxy_array_ == 1) {
        return static_cast<int>(server_groups_[0]);
    }
    return 0;
}

// IDA_EA: 0x82B3C7C
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyE11GetEndIndexEv
int CServerProxyMgr<CGuildServerProxy>::GetEndIndex() {
    return proxy_count_;
}

// IDA_EA: 0x82B3C88
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyE12GetNextIndexERi
int CServerProxyMgr<CGuildServerProxy>::GetNextIndex(int& index) {
    ++index;
    if (has_proxy_array_ != 1 || proxy_count_ <= index) {
        return 0;
    }
    return static_cast<int>(server_groups_[index]);
}

// IDA_EA: 0x82B4018
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyE17createServerProxyEii17ENUM_SERVER_GROUPPcii
int CServerProxyMgr<CGuildServerProxy>::createServerProxy(
    int server_count,
    int index,
    ENUM_SERVER_GROUP server_group,
    char* server_ip,
    int udp_port,
    int tcp_port) {
    if (server_group) {
        if (has_proxy_array_ != 1) {
            has_proxy_array_ = 1;
            server_groups_ = new ENUM_SERVER_GROUP[server_count];
            proxies_ = new CGuildServerProxy[server_count];
            proxy_count_ = server_count;
        }
        server_groups_[index] = server_group;
        return proxies_[index].Init(server_ip ? server_ip : "",
                                    udp_port,
                                    tcp_port);
    }

    proxies_ = &default_proxy_;
    proxy_count_ = 1;
    return default_proxy_.Init(server_ip ? server_ip : "", udp_port, tcp_port);
}

// IDA_EA: 0x857F8FE
// IDA_NAME: _ZN15CServerProxyMgrI17CGuildServerProxyE21GetServerProxyArrayAtEi
CGuildServerProxy*
CServerProxyMgr<CGuildServerProxy>::GetServerProxyArrayAt(int index) {
    if (index < 0 || index >= proxy_count_) {
        return 0;
    }
    return proxies_ + index;
}

// IDA_EA: 0x846DBFE
// IDA_NAME: _ZN17CGuildServerProxy10SendLogoutEjiPcjjjh
// DEMANGLED: CGuildServerProxy::SendLogout(unsigned int,int,char *,unsigned int,unsigned int,unsigned int,unsigned char)
char CGuildServerProxy::SendLogout(unsigned int account_id,
                                   int channel_no,
                                   char* charac_name,
                                   unsigned int charac_no,
                                   unsigned int charac_guildkey,
                                   unsigned int charac_memberkey,
                                   unsigned char flag) {
    if (!account_id) {
        return static_cast<char>(flag);
    }

    Packet_Monitor_UDP_Logout packet;
    packet.account_id = account_id;
    packet.channel_no = static_cast<uint8_t>(channel_no);
    packet.charac_no = charac_no;
    packet.reason = charac_guildkey;
    packet.state = flag;
    packet.play_time = charac_memberkey;
    if (charac_name) {
        std::strcpy(reinterpret_cast<char*>(packet.tail), charac_name);
    }
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846DC7E
// IDA_NAME: _ZN17CGuildServerProxy10SendPacketEPci
// DEMANGLED: CGuildServerProxy::SendPacket(char *,int)
int CGuildServerProxy::SendPacket(char* packet, int size) {
    if (!udp_port_) {
        return 0;
    }
    return udp_handler_.SendToServer(
        packet,
        static_cast<unsigned int>(size),
        static_cast<uint16_t>(udp_port_),
        server_ip_.c_str());
}

// IDA_EA: 0x846DA9A
// IDA_NAME: _ZN17CGuildServerProxy21SendCharLevelGrowTypeEjjss
// DEMANGLED: CGuildServerProxy::SendCharLevelGrowType(
//             unsigned int,unsigned int,short,short)
int CGuildServerProxy::SendCharLevelGrowType(unsigned int account_id,
                                             unsigned int character_no,
                                             short character_level,
                                             short grow_type) {
    Packet_Monitor_Char_Changable_Info packet;
    packet.account_id = account_id;
    packet.character_no = static_cast<uint8_t>(character_no);
    packet.character_level = character_level;
    packet.grow_type = static_cast<uint8_t>(grow_type);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F684
// IDA_NAME: _ZN35Packet_Guild_Power_War_Process_InfoC2Ev
// DEMANGLED: Packet_Guild_Power_War_Process_Info::Packet_Guild_Power_War_Process_Info(void)
Packet_Guild_Power_War_Process_Info::Packet_Guild_Power_War_Process_Info()
    : PacketHeader(0x06E0u, 0x000Eu), charac_no(0) {}

// IDA_EA: 0x846F836
// IDA_NAME: _ZN24Packet_GM_PowerWar_StartC2Ev
// DEMANGLED: Packet_GM_PowerWar_Start::Packet_GM_PowerWar_Start(void)
Packet_GM_PowerWar_Start::Packet_GM_PowerWar_Start()
    : PacketHeader(0x06A6u, 0x000Au) {}

// IDA_EA: 0x846F85A
// IDA_NAME: _ZN22Packet_GM_PowerWar_EndC2Ev
// DEMANGLED: Packet_GM_PowerWar_End::Packet_GM_PowerWar_End(void)
Packet_GM_PowerWar_End::Packet_GM_PowerWar_End()
    : PacketHeader(0x06A7u, 0x000Au) {}

// IDA_EA: 0x846DE80
// IDA_NAME: _ZN17CGuildServerProxy17SendPowerWarStartEj
// DEMANGLED: CGuildServerProxy::SendPowerWarStart(unsigned int)
int CGuildServerProxy::SendPowerWarStart(unsigned int account_id) {
    if (GlobalData::s_pGMAccounts_->isGM(account_id)) {
        return 0;
    }

    Packet_GM_PowerWar_Start packet;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846DECE
// IDA_NAME: _ZN17CGuildServerProxy15SendPowerWarEndEj
// DEMANGLED: CGuildServerProxy::SendPowerWarEnd(unsigned int)
int CGuildServerProxy::SendPowerWarEnd(unsigned int account_id) {
    if (GlobalData::s_pGMAccounts_->isGM(account_id)) {
        return 0;
    }

    Packet_GM_PowerWar_End packet;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846DF1C
// IDA_NAME: _ZN17CGuildServerProxy23SendPowerWarProcessInfoEj
// DEMANGLED: CGuildServerProxy::SendPowerWarProcessInfo(unsigned int)
int CGuildServerProxy::SendPowerWarProcessInfo(unsigned int charac_no) {
    Packet_Guild_Power_War_Process_Info packet;
    packet.charac_no = charac_no;

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendPacket(wire_bytes, packet.size);
}

// IDA_EA: 0x846E026
// IDA_NAME: _ZN17CGuildServerProxy17SendBuyGuildSkillEjjiisj
char CGuildServerProxy::SendBuyGuildSkill(unsigned int charac_no,
                                          unsigned int charac_guildkey,
                                          int skill_index,
                                          int skill_level,
                                          short guild_skill_point,
                                          unsigned int gold_cost) {
    Packet_Monitor_Buy_Guild_Skill packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    packet.skill_index = skill_index;
    packet.skill_level = skill_level;
    packet.guild_skill_point = guild_skill_point;
    packet.gold_cost = gold_cost;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846E084
// IDA_NAME: _ZN17CGuildServerProxy13SendGuildMailEjjiPc
// DEMANGLED: CGuildServerProxy::SendGuildMail(
//             unsigned int,unsigned int,int,char *)
int CGuildServerProxy::SendGuildMail(unsigned int charac_no,
                                     unsigned int charac_guildkey,
                                     int text_length,
                                     char* text) {
    Packet_Monitor_Send_Guild_Mail packet;
    std::memcpy(packet.raw_0a, &charac_no, sizeof(charac_no));
    std::memcpy(packet.raw_0a + sizeof(charac_no),
                &charac_guildkey,
                sizeof(charac_guildkey));
    std::memcpy(packet.body_12, text, static_cast<size_t>(text_length));
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846ED36
// IDA_NAME: _ZN17CGuildServerProxy33SendIncreaseDecreasePowerWarPointEjji
// DEMANGLED: CGuildServerProxy::SendIncreaseDecreasePowerWarPoint(
//             unsigned int,unsigned int,int)
char CGuildServerProxy::SendIncreaseDecreasePowerWarPoint(
    unsigned int charac_guildkey,
    unsigned int charac_no,
    int point_delta) {
    Packet_Guild_Increase_Decrease_Power_War_Point packet;
    packet.charac_guildkey = charac_guildkey;
    packet.charac_no = charac_no;
    packet.point_delta = point_delta;
    return static_cast<char>(
        SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size));
}

// IDA_EA: 0x846EF24
// IDA_NAME: _ZN17CGuildServerProxy9OnBuyItemEP5CUserPK5CItemjj
// DEMANGLED: CGuildServerProxy::OnBuyItem(CUser *,CItem const*,
//             unsigned int,unsigned int)
char CGuildServerProxy::OnBuyItem(CUser* user,
                                  const CItem* item,
                                  unsigned int guild_power_cost,
                                  unsigned int user_power_cost) {
    if (item->vtable_->get_guild_power_war_point(item) != 0) {
        user->DecreaseGuildPowerWarPoint(guild_power_cost);
        SendIncreaseDecreasePowerWarPoint(
            static_cast<unsigned int>(user->get_charac_guildkey()),
            static_cast<unsigned int>(user->get_charac_no(-1)),
            -static_cast<int>(guild_power_cost));
    }
    if (item->vtable_->get_user_power_war_point(item) != 0) {
        return static_cast<char>(user->DecreasePowerWarPoint(
            static_cast<int16_t>(user_power_cost)));
    }
    return 0;
}

// IDA_EA: 0x846F3A0
// IDA_NAME: _ZN33Packet_Monitor_Call_Guild_MembersC2Ev
// DEMANGLED: Packet_Monitor_Call_Guild_Members::Packet_Monitor_Call_Guild_Members(void)
Packet_Monitor_Call_Guild_Members::Packet_Monitor_Call_Guild_Members()
    : PacketHeader(0x0401u, 0x0012u), charac_guildkey(0) {}

// IDA_EA: 0x846DD12
// IDA_NAME: _ZN17CGuildServerProxy26SendPacketCallGuildMembersEjj
// DEMANGLED: CGuildServerProxy::SendPacketCallGuildMembers(unsigned int,unsigned int)
int CGuildServerProxy::SendPacketCallGuildMembers(
    unsigned int charac_no,
    unsigned int charac_guildkey) {
    Packet_Monitor_Call_Guild_Members packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendPacket(wire_bytes, packet.size);
}

// IDA_EA: 0x846F472
// IDA_NAME: _ZN30Packet_Monitor_Call_Guild_InfoC2Ev
// DEMANGLED: Packet_Monitor_Call_Guild_Info::Packet_Monitor_Call_Guild_Info(void)
Packet_Monitor_Call_Guild_Info::Packet_Monitor_Call_Guild_Info()
    : PacketHeader(0x040Du, 0x018Fu), charac_no(0), uid(0), guild_count(0) {
    std::memset(guild_keys, 0, sizeof(guild_keys));
}

// IDA_EA: 0x846DDD2
// IDA_NAME: _ZN17CGuildServerProxy17SendCallGuildInfoEjjhPj
// DEMANGLED: CGuildServerProxy::SendCallGuildInfo(unsigned int,unsigned int,unsigned char,unsigned int *)
int CGuildServerProxy::SendCallGuildInfo(unsigned int charac_no,
                                         unsigned int uid,
                                         unsigned char guild_count,
                                         unsigned int* guild_keys) {
    Packet_Monitor_Call_Guild_Info packet;
    packet.charac_no = charac_no;
    packet.uid = uid;
    packet.guild_count = guild_count;
    for (unsigned int index = 0; index < guild_count; ++index) {
        packet.guild_keys[index] = guild_keys[index];
    }

    char wire_bytes[sizeof(packet)];
    std::memcpy(wire_bytes, &packet, sizeof(wire_bytes));
    return SendPacket(wire_bytes, 19 + 4 * static_cast<int>(guild_count));
}

// IDA_EA: 0x822BA9A
// IDA_NAME: _ZN22ST_Guild_War_Rank_InfoC2Ev
// DEMANGLED: ST_Guild_War_Rank_Info::ST_Guild_War_Rank_Info(void)
ST_Guild_War_Rank_Info::ST_Guild_War_Rank_Info()
    : raw_00(0), raw_04(0), raw_08(0) {
    std::memset(raw_0a, 0, sizeof(raw_0a));
}

// IDA_EA: 0x822BADC
// IDA_NAME: _ZN28Packet_Notice_Guild_War_InfoC2Ev
// DEMANGLED: Packet_Notice_Guild_War_Info::Packet_Notice_Guild_War_Info(void)
Packet_Notice_Guild_War_Info::Packet_Notice_Guild_War_Info()
    : PacketHeader(0x0414u, 0x015Cu) {}

// IDA_EA: 0x846F3CE
// IDA_NAME: _ZN37Packet_Monitor_Call_Guild_All_MembersC2Ev
// DEMANGLED: Packet_Monitor_Call_Guild_All_Members::Packet_Monitor_Call_Guild_All_Members(void)
Packet_Monitor_Call_Guild_All_Members::Packet_Monitor_Call_Guild_All_Members()
    : PacketHeader(0x0424u, 0x0012u), charac_guildkey(0) {}

// IDA_EA: 0x846DD4E
// IDA_NAME: _ZN17CGuildServerProxy29SendPacketCallGuildAllMembersEjj
// DEMANGLED: CGuildServerProxy::SendPacketCallGuildAllMembers(unsigned int,unsigned int)
int CGuildServerProxy::SendPacketCallGuildAllMembers(
    unsigned int charac_no,
    unsigned int charac_guildkey) {
    Packet_Monitor_Call_Guild_All_Members packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F8C0
// IDA_NAME: _ZN30Packet_Guild_Call_Guild_InviteC2Ev
// DEMANGLED: Packet_Guild_Call_Guild_Invite::Packet_Guild_Call_Guild_Invite(void)
Packet_Guild_Call_Guild_Invite::Packet_Guild_Call_Guild_Invite()
    : PacketHeader(0x0434u, 0x0030u) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x846E0F2
// IDA_NAME: _ZN17CGuildServerProxy19SendCallGuildInviteEjjPc
// DEMANGLED: CGuildServerProxy::SendCallGuildInvite(unsigned int,unsigned int,char *)
int CGuildServerProxy::SendCallGuildInvite(unsigned int charac_no,
                                           unsigned int charac_guildkey,
                                           char* target_name) {
    Packet_Guild_Call_Guild_Invite packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    std::memcpy(packet.target_name, target_name, 0x1Du);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F902
// IDA_NAME: _ZN44Packet_Guild_Reply_Guild_Invite_From_InvitedC2Ev
// DEMANGLED: Packet_Guild_Reply_Guild_Invite_From_Invited::Packet_Guild_Reply_Guild_Invite_From_Invited(void)
Packet_Guild_Reply_Guild_Invite_From_Invited::
    Packet_Guild_Reply_Guild_Invite_From_Invited()
    : PacketHeader(0x0436u, 0x000Fu) {}

// IDA_EA: 0x846E14C
// IDA_NAME: _ZN17CGuildServerProxy20SendReplyGuildInviteEjh
// DEMANGLED: CGuildServerProxy::SendReplyGuildInvite(unsigned int,unsigned char)
int CGuildServerProxy::SendReplyGuildInvite(unsigned int charac_no,
                                            unsigned char reply_code) {
    Packet_Guild_Reply_Guild_Invite_From_Invited packet;
    packet.charac_no = charac_no;
    packet.reply_code = reply_code;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F926
// IDA_NAME: _ZN33Packet_Guild_Request_Guild_SecedeC2Ev
// DEMANGLED: Packet_Guild_Request_Guild_Secede::Packet_Guild_Request_Guild_Secede(void)
Packet_Guild_Request_Guild_Secede::Packet_Guild_Request_Guild_Secede()
    : PacketHeader(0x0439u, 0x0034u),
      charac_guildkey(0),
      charac_no(0),
      text_length(0) {
    std::memset(text, 0, sizeof(text));
}

// IDA_EA: 0x846E190
// IDA_NAME: _ZN17CGuildServerProxy22SendRequestGuildSecedeEjjiPc
// DEMANGLED: CGuildServerProxy::SendRequestGuildSecede(unsigned int,unsigned int,int,char *)
int CGuildServerProxy::SendRequestGuildSecede(unsigned int charac_guildkey,
                                               unsigned int charac_no,
                                               int text_length,
                                               char* text) {
    Packet_Guild_Request_Guild_Secede packet;
    packet.charac_guildkey = charac_guildkey;
    packet.charac_no = charac_no;
    packet.text_length = text_length;
    if (text_length > 0) {
        std::memcpy(packet.text, text, static_cast<size_t>(text_length));
    }
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F524
// IDA_NAME: _ZN34Packet_Web_Notify_Message_To_GuildC2Ev
// DEMANGLED: Packet_Web_Notify_Message_To_Guild::Packet_Web_Notify_Message_To_Guild(void)
Packet_Web_Notify_Message_To_Guild::Packet_Web_Notify_Message_To_Guild()
    : PacketHeader(0x042Du, 0x0074u), charac_guildkey(0), message_kind(0) {
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x846E1F4
// IDA_NAME: _ZN17CGuildServerProxy18SendMessageToGuildEjiPc
// DEMANGLED: CGuildServerProxy::SendMessageToGuild(unsigned int,int,char *)
int CGuildServerProxy::SendMessageToGuild(unsigned int charac_guildkey,
                                          int message_length,
                                          char* message) {
    Packet_Web_Notify_Message_To_Guild packet;
    packet.charac_guildkey = charac_guildkey;
    packet.message_kind = 2;
    if (message_length <= 100) {
        std::memcpy(packet.message, message, message_length);
    }
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F4CE
// IDA_NAME: _ZN36Packet_Guild_Write_Guild_Member_MemoC2Ev
// DEMANGLED: Packet_Guild_Write_Guild_Member_Memo::Packet_Guild_Write_Guild_Member_Memo(void)
Packet_Guild_Write_Guild_Member_Memo::Packet_Guild_Write_Guild_Member_Memo()
    : PacketHeader(0x04D1u, 0x0027u), charac_guildkey(0), charac_no(0) {
    std::memset(memo, 0, sizeof(memo));
}

// IDA_EA: 0x846E254
// IDA_NAME: _ZN17CGuildServerProxy15SendMemoToGuildEjiiPc
// DEMANGLED: CGuildServerProxy::SendMemoToGuild(unsigned int,int,int,char *)
int CGuildServerProxy::SendMemoToGuild(unsigned int charac_guildkey,
                                       int charac_no,
                                       int memo_length,
                                       char* memo) {
    Packet_Guild_Write_Guild_Member_Memo packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    std::memcpy(packet.memo, memo, memo_length);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F968
// IDA_NAME: _ZN42Packet_Guild_Request_Guild_Master_DelegateC2Ev
// DEMANGLED: Packet_Guild_Request_Guild_Master_Delegate::Packet_Guild_Request_Guild_Master_Delegate(void)
Packet_Guild_Request_Guild_Master_Delegate::
    Packet_Guild_Request_Guild_Master_Delegate()
    : PacketHeader(0x043Bu, 0x0030u) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x846E2AC
// IDA_NAME: _ZN17CGuildServerProxy23SendGuildMasterDelegateEjjiPc
// DEMANGLED: CGuildServerProxy::SendGuildMasterDelegate(unsigned int,unsigned int,int,char *)
int CGuildServerProxy::SendGuildMasterDelegate(unsigned int charac_guildkey,
                                               unsigned int charac_no,
                                               int target_name_length,
                                               char* target_name) {
    Packet_Guild_Request_Guild_Master_Delegate packet;
    packet.charac_guildkey = charac_guildkey;
    packet.charac_no = charac_no;
    std::memcpy(packet.target_name, target_name, target_name_length);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x822B978
// IDA_NAME: _ZN35Packet_Monitor_Set_Sub_Guild_MasterC2Ev
// DEMANGLED: Packet_Monitor_Set_Sub_Guild_Master::Packet_Monitor_Set_Sub_Guild_Master(void)
Packet_Monitor_Set_Sub_Guild_Master::Packet_Monitor_Set_Sub_Guild_Master()
    : PacketHeader(0x0430u, 0x0031u), charac_no(0), charac_guildkey(0) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x846F9AA
// IDA_NAME: _ZN24Packet_Change_Guild_NameC2Ev
// DEMANGLED: Packet_Change_Guild_Name::Packet_Change_Guild_Name(void)
Packet_Change_Guild_Name::Packet_Change_Guild_Name()
    : PacketHeader(0x0444u, 0x0029u), charac_no(0), charac_guildkey(0) {
    std::memset(guild_name, 0, sizeof(guild_name));
}

// IDA_EA: 0x846E376
// IDA_NAME: _ZN17CGuildServerProxy19SendGuildNameChangeEjjPc
// DEMANGLED: CGuildServerProxy::SendGuildNameChange(unsigned int,unsigned int,char *)
char CGuildServerProxy::SendGuildNameChange(unsigned int charac_no,
                                             unsigned int charac_guildkey,
                                             char* guild_name) {
    Packet_Change_Guild_Name packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    std::strcpy(packet.guild_name, guild_name);
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846FA60
// IDA_NAME: _ZN27Packet_Request_Guild_CreateC2Ev
// DEMANGLED: Packet_Request_Guild_Create::Packet_Request_Guild_Create(void)
Packet_Request_Guild_Create::Packet_Request_Guild_Create()
    : PacketHeader(0x043Fu, 0x0032u), charac_no(0) {
    std::memset(guild_name, 0, sizeof(guild_name));
    std::memset(guild_address, 0, sizeof(guild_address));
}

// IDA_EA: 0x846E304
// IDA_NAME: _ZN17CGuildServerProxy15SendGuildCreateEjPcS0_
// DEMANGLED: CGuildServerProxy::SendGuildCreate(unsigned int,char *,char *)
char CGuildServerProxy::SendGuildCreate(unsigned int charac_no,
                                        char* guild_name,
                                        char* guild_address) {
    (void)guild_address;

    Packet_Request_Guild_Create request;
    request.charac_no = charac_no;
    std::memcpy(request.guild_name, guild_name, sizeof(request.guild_name));
    return SendTcpPacket(reinterpret_cast<char*>(&request), request.size);
}

// IDA_EA: 0x846DD8A
// IDA_NAME: _ZN17CGuildServerProxy16SendGuildLevelupEijjj
// DEMANGLED: CGuildServerProxy::SendGuildLevelup(int,unsigned int,unsigned int,unsigned int)
char CGuildServerProxy::SendGuildLevelup(int uid,
                                         unsigned int charac_no,
                                         unsigned int charac_guildkey,
                                         unsigned int levelup_price) {
    Packet_Monitor_Call_Guild_Level_Up packet;
    packet.uid = uid;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    packet.levelup_price = levelup_price;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846ECE2
// IDA_NAME: _ZN17CGuildServerProxy20SendIncreaseGuildExpEjjjc
// DEMANGLED: CGuildServerProxy::SendIncreaseGuildExp(
//             unsigned int,unsigned int,unsigned int,char)
char CGuildServerProxy::SendIncreaseGuildExp(unsigned int charac_guildkey,
                                             unsigned int charac_no,
                                             unsigned int gained_exp,
                                             char source_flag) {
    Packet_Monitor_Increase_Guild_Exp packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    packet.gained_exp = gained_exp;
    packet.source_flag = static_cast<uint8_t>(source_flag);
    packet.notify = 1;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846EE44
// IDA_NAME: _ZN17CGuildServerProxy17SendGuildExpLimitEv
// DEMANGLED: CGuildServerProxy::SendGuildExpLimit(void)
char CGuildServerProxy::SendGuildExpLimit() {
    Packet_Guild_Exp_Limit packet;
    const GuildParameterScript* const guild_parameters =
        G_CDataManager()->GetGuildParameterScriptForBuild();

    packet.channel_no = G_CEnvironment()->get_channel_no();
    // The original reads DataManager+0xa5ac/+0xa5ec and then +0xa56c+8*i.
    // Those are exactly the value_04 fields of the embedded 17-entry table.
    packet.guild_levelup_param_09_value_04 =
        guild_parameters->guild_levelup_params[8].value_04;
    packet.guild_levelup_param_17_value_04 =
        guild_parameters->guild_levelup_params[16].value_04;
    for (int level = 0; level <= 16; ++level) {
        packet.guild_levelup_param_value_04[level] =
            guild_parameters->guild_levelup_params[level].value_04;
    }
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846E95E
// IDA_NAME: _ZN17CGuildServerProxy20ModifyGuildInfoProxyEjR12STGuildProxy
// DEMANGLED: CGuildServerProxy::ModifyGuildInfoProxy(unsigned int,STGuildProxy &)
int CGuildServerProxy::ModifyGuildInfoProxy(unsigned int charac_guildkey,
                                            STGuildProxy& guild_info) {
    std::map<unsigned int, STGuildProxy>::iterator found =
        guild_map_.find(charac_guildkey);
    if (found == guild_map_.end()) {
        LogManager::logFormat(
            1,
            "DF_GuildServerProxy.cpp",
            "bool CGuildServerProxy::ModifyGuildInfoProxy(unsigned int, STGuildProxy&)",
            0x852,
            "Find fail from guild info proxy(guild_id:%d)",
            charac_guildkey);
        return 0;
    }

    // The original writes six dwords followed by the final word, i.e. the
    // complete packed 26-byte STGuildProxy value at the map node.
    std::memcpy(&found->second, &guild_info, sizeof(guild_info));
    return 1;
}

// IDA_EA: 0x846EA3E
// IDA_NAME: _ZN17CGuildServerProxy22ChangeGuildNameInProxyEjPc
// DEMANGLED: CGuildServerProxy::ChangeGuildNameInProxy(unsigned int,char *)
void CGuildServerProxy::ChangeGuildNameInProxy(unsigned int charac_guildkey,
                                               char* guild_name) {
    std::map<unsigned int, STGuildProxy>::iterator found =
        guild_map_.find(charac_guildkey);
    if (found == guild_map_.end()) {
        LogManager::logFormat(
            1, "DF_GuildServerProxy.cpp",
            "void CGuildServerProxy::ChangeGuildNameInProxy(unsigned int, char*)",
            0x860, "Find fail from guild info proxy(guild_id:%d)",
            charac_guildkey);
        return;
    }
    std::memset(found->second.guild_name,
                0,
                sizeof(found->second.guild_name));
    std::memcpy(found->second.guild_name,
                guild_name,
                sizeof(found->second.guild_name) - 1u);
}

// IDA_EA: 0x846EC88
// IDA_NAME: _ZN17CGuildServerProxy20InsertGuildInfoProxyEjR12STGuildProxy
// DEMANGLED: CGuildServerProxy::InsertGuildInfoProxy(unsigned int,STGuildProxy &)
int CGuildServerProxy::InsertGuildInfoProxy(unsigned int charac_guildkey,
                                            STGuildProxy& guild_info) {
    return guild_map_.insert(std::make_pair(charac_guildkey, guild_info)).second
               ? 1
               : 0;
}

// IDA_EA: 0x846FC8E
// IDA_NAME: _ZN17CGuildServerProxy13GetTcpHandlerEv
// DEMANGLED: CGuildServerProxy::GetTcpHandler(void)
CNetwork100000_100000* CGuildServerProxy::GetTcpHandler() {
    return &tcp_handler_;
}

// IDA_EA: 0x846DCCE
// IDA_NAME: _ZN17CGuildServerProxy13SendTcpPacketEPci
// DEMANGLED: CGuildServerProxy::SendTcpPacket(char *,int)
char CGuildServerProxy::SendTcpPacket(char* packet, int size) {
    CNetwork100000_100000* const tcp_handler = GetTcpHandler();
    const char disconnected = tcp_handler->get_socket() == -1;
    if (!disconnected) {
        return static_cast<char>(tcp_handler->send_packet(packet, size));
    }
    return disconnected;
}

// IDA_EA: 0x846F0F2
// IDA_NAME: _ZN17CGuildServerProxy18SendGameServerInfoEPcthh
// DEMANGLED: CGuildServerProxy::SendGameServerInfo(char *,unsigned short,unsigned char,unsigned char)
char CGuildServerProxy::SendGameServerInfo(char* server_ip,
                                           unsigned short udp_port,
                                           unsigned char server_group,
                                           unsigned char channel_no) {
    Packet_Game_Server_Regist packet;
    packet.registration_kind = 1;
    packet.server_group = server_group;
    packet.channel_no = channel_no;
    std::strncpy(packet.server_ip, server_ip, sizeof(packet.server_ip));
    packet.udp_port = udp_port;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

// IDA_EA: 0x846FBDE
// IDA_NAME: _ZN38Packet_Guild_Request_Guild_Board_WriteC2Ev
// DEMANGLED: Packet_Guild_Request_Guild_Board_Write::Packet_Guild_Request_Guild_Board_Write(void)
Packet_Guild_Request_Guild_Board_Write::
    Packet_Guild_Request_Guild_Board_Write()
    : PacketHeader(0x232du, 0x90u),
      charac_guildkey(0),
      raw_0e(0),
      charac_no(0),
      raw_16(-1),
      raw_17(-1) {
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x846FBA0
// IDA_NAME: _ZN37Packet_Guild_Request_Guild_Board_OpenC2Ev
// DEMANGLED: Packet_Guild_Request_Guild_Board_Open::Packet_Guild_Request_Guild_Board_Open(void)
Packet_Guild_Request_Guild_Board_Open::Packet_Guild_Request_Guild_Board_Open()
    : PacketHeader(0x2329u, 0x13u),
      request_kind(0),
      charac_no(0),
      charac_guildkey(0) {}

// IDA_EA: 0x846FC4C
// IDA_NAME: _ZN39Packet_Guild_Request_Guild_Board_DeleteC2Ev
// DEMANGLED: Packet_Guild_Request_Guild_Board_Delete::Packet_Guild_Request_Guild_Board_Delete(void)
Packet_Guild_Request_Guild_Board_Delete::Packet_Guild_Request_Guild_Board_Delete()
    : PacketHeader(0x2331u, 0x16u),
      board_index(0),
      charac_no(0),
      charac_guildkey(0) {}

// IDA_EA: 0x846EFCE
// IDA_NAME: _ZN17CGuildServerProxy18SendGuildBoardOpenEjjc
// DEMANGLED: CGuildServerProxy::SendGuildBoardOpen(unsigned int,unsigned int,char)
char CGuildServerProxy::SendGuildBoardOpen(unsigned int charac_no_value,
                                           unsigned int charac_guildkey_value,
                                           char request_kind_value) {
    Packet_Guild_Request_Guild_Board_Open packet;
    packet.charac_no = charac_no_value;
    packet.charac_guildkey = charac_guildkey_value;
    packet.request_kind = request_kind_value != 0;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F01A
// IDA_NAME: _ZN17CGuildServerProxy19SendGuildBoardWriteEPcjjicj
// DEMANGLED: CGuildServerProxy::SendGuildBoardWrite(char *,unsigned int,unsigned int,int,char,unsigned int)
char CGuildServerProxy::SendGuildBoardWrite(char* message,
                                            unsigned int raw_0e_value,
                                            unsigned int charac_no_value,
                                            int raw_16_value,
                                            char raw_17_value,
                                            unsigned int charac_guildkey_value) {
    Packet_Guild_Request_Guild_Board_Write packet;
    packet.charac_guildkey = charac_guildkey_value;
    packet.raw_0e = raw_0e_value;
    packet.charac_no = charac_no_value;
    packet.raw_16 = static_cast<int8_t>(raw_16_value);
    packet.raw_17 = static_cast<int8_t>(raw_17_value);
    std::memcpy(packet.message, message, sizeof(packet.message));
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F0B0
// IDA_NAME: _ZN17CGuildServerProxy20SendGuildBoardDeleteEjjj
// DEMANGLED: CGuildServerProxy::SendGuildBoardDelete(unsigned int,unsigned int,unsigned int)
char CGuildServerProxy::SendGuildBoardDelete(unsigned int board_index_value,
                                             unsigned int charac_no_value,
                                             unsigned int charac_guildkey_value) {
    Packet_Guild_Request_Guild_Board_Delete packet;
    packet.board_index = board_index_value;
    packet.charac_no = charac_no_value;
    packet.charac_guildkey = charac_guildkey_value;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846FB68
// IDA_NAME: _ZN25Packet_Refresh_Guild_InfoC2Ev
// DEMANGLED: Packet_Refresh_Guild_Info::Packet_Refresh_Guild_Info(void)
Packet_Refresh_Guild_Info::Packet_Refresh_Guild_Info()
    : PacketHeader(0x1BBEu, 0x12u), charac_no(0), charac_guildkey(0) {}

// IDA_EA: 0x846F16C
// IDA_NAME: _ZN17CGuildServerProxy20SendRefreshGuildInfoEjj
// DEMANGLED: CGuildServerProxy::SendRefreshGuildInfo(unsigned int,unsigned int)
char CGuildServerProxy::SendRefreshGuildInfo(unsigned int charac_no,
                                              unsigned int charac_guildkey) {
    Packet_Refresh_Guild_Info packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x846F1A8
// IDA_NAME: _ZN17CGuildServerProxy16SendAddGuildFundEjjiN21Packet_Add_Guild_Fund9eFundTypeE
// DEMANGLED: CGuildServerProxy::SendAddGuildFund(unsigned int,unsigned int,int,Packet_Add_Guild_Fund::eFundType)
char CGuildServerProxy::SendAddGuildFund(
    unsigned int charac_no,
    unsigned int charac_guildkey,
    int fund_amount,
    Packet_Add_Guild_Fund::eFundType fund_type) {
    Packet_Add_Guild_Fund packet;
    packet.charac_no = charac_no;
    packet.charac_guildkey = charac_guildkey;
    packet.fund_amount = fund_amount;
    packet.fund_type = static_cast<uint8_t>(fund_type);
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}
// IDA_EA: 0x846F2B0
// IDA_NAME: _ZN34Packet_Monitor_Char_Changable_InfoC2Ev
// DEMANGLED: Packet_Monitor_Char_Changable_Info::
//             Packet_Monitor_Char_Changable_Info(void)
Packet_Monitor_Char_Changable_Info::
Packet_Monitor_Char_Changable_Info()
    : PacketHeader(0x03f5u, 0x0012u),
      account_id(0),
      character_no(0),
      character_level(0),
      grow_type(0) {}
