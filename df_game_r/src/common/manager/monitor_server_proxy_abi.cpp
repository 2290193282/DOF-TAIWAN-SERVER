#include "common/manager/monitor_server_proxy_abi.h"

#include "common/manager/global_data.h"
#include "core/config/env/environment.h"
#include "infra/network/transport/game_server_registration_packet.h"
#include "common/manager/gm_list_abi.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/player/core/character_packets.h"

#include <cstring>
#include <cerrno>
#include <new>

#include "infra/network/packet/udp_packets.h"

namespace {
// IDA unit: 04_character_select_create/
// Packet_Request_Charac_Tower_Update_Ranking.  The constructor at 0x84716D6
// creates this exact 0x1a-byte monitor UDP frame; the updateTowerRank body at
// 0x8471240 stores the four values in declaration order after the header.
#pragma pack(push, 1)
struct PacketMonitorTowerRankUpdate {
    PacketMonitorTowerRankUpdate(uint32_t charac_no_value,
                                 uint32_t tower_index_value,
                                 uint32_t member_count_value,
                                 uint32_t virtual_rank_value)
        : header(0x04CEu, 0x001Au),
          charac_no(charac_no_value),
          tower_index(tower_index_value),
          member_count(member_count_value),
          virtual_rank(virtual_rank_value) {}

    PacketHeader header;       // +0x00
    uint32_t charac_no;        // +0x0a
    uint32_t tower_index;      // +0x0e
    uint32_t member_count;     // +0x12
    uint32_t virtual_rank;     // +0x16
};
#pragma pack(pop)

static_assert(sizeof(PacketMonitorTowerRankUpdate) == 0x1au,
              "Death Tower monitor rank packet must remain 0x1a bytes");
static_assert(offsetof(PacketMonitorTowerRankUpdate, charac_no) == 0x0au &&
                  offsetof(PacketMonitorTowerRankUpdate, tower_index) == 0x0eu &&
                  offsetof(PacketMonitorTowerRankUpdate, member_count) == 0x12u &&
                  offsetof(PacketMonitorTowerRankUpdate, virtual_rank) == 0x16u,
              "Death Tower monitor rank packet field offsets must stay exact");

}  // namespace

// IDA_EA: 0x82A69CC
// IDA_NAME: _ZN19CMonitorServerProxyC2Ev
// DEMANGLED: CMonitorServerProxy::CMonitorServerProxy(void)
CMonitorServerProxy::CMonitorServerProxy()
    : server_ip_(), udp_port_(0), tcp_port_(0), tcp_handler_(), udp_handler_() {}

// IDA_EA: 0x847097C
// IDA_NAME: _ZN19CMonitorServerProxyC2ESsii
// DEMANGLED: CMonitorServerProxy::CMonitorServerProxy(std::string,int,int)
CMonitorServerProxy::CMonitorServerProxy(std::string server_ip,
                                           int udp_port,
                                           int tcp_port)
    : server_ip_(server_ip),
      udp_port_(udp_port),
      tcp_port_(tcp_port),
      tcp_handler_(),
      udp_handler_() {}

// IDA_EA: 0x84714EC
// IDA_NAME: _ZN19CMonitorServerProxy18SendGameServerInfoEPcthh
// DEMANGLED: CMonitorServerProxy::SendGameServerInfo(char *,unsigned short,unsigned char,unsigned char)
char CMonitorServerProxy::SendGameServerInfo(char* server_ip,
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

// IDA_EA: 0x82B312C
// IDA_NAME: _ZN19CMonitorServerProxyD2Ev
// DEMANGLED: CMonitorServerProxy::~CMonitorServerProxy()
CMonitorServerProxy::~CMonitorServerProxy() {}

// IDA_EA: 0x8470A7C
// IDA_NAME: _ZN19CMonitorServerProxy7InitUdpEv
// DEMANGLED: CMonitorServerProxy::InitUdp(void)
int CMonitorServerProxy::InitUdp() {
    if (udp_handler_.InitClientSocket() != -1) {
        return 1;
    }
    LogManager::logFormat(1,
                          "DF_MonitorServerProxy.cpp",
                          "bool CMonitorServerProxy::InitUdp()",
                          0x485,
                          "[CMonitorServerProxy::Init] Can't init udp client socket");
    return 0;
}

// IDA_EA: 0x8470AD6
// IDA_NAME: _ZN19CMonitorServerProxy7InitTcpEv
// DEMANGLED: CMonitorServerProxy::InitTcp(void)
int CMonitorServerProxy::InitTcp() {
    if (udp_port_ == 0) {
        return 1;
    }
    if (tcp_handler_.get_socket() < 0 &&
        CNetwork100000_100000::CreateConnectionSocket(
            tcp_handler_.socket_, server_ip_.c_str(), tcp_port_) != 0) {
        cMyTrace trace("bool CMonitorServerProxy::InitTcp()", 1179, 0);
        trace("CMonitorServerProxy CreateConnectionSocket() error");
        return 0;
    }
    cMyTrace trace("bool CMonitorServerProxy::InitTcp()", 1184, 9, true,
                   true);
    trace("[!] Connect To Monitor Server ... ");
    return 1;
}

// IDA_EA: 0x8470A00
// IDA_NAME: _ZN19CMonitorServerProxy4InitEv
// DEMANGLED: CMonitorServerProxy::Init(void)
int CMonitorServerProxy::Init() {
    if (udp_port_ == 0) {
        return 1;
    }
    if (InitUdp() != 1 || InitTcp() != 1) {
        return 0;
    }
    if (G_CEnvironment()->get_channel_no() == 1 && GlobalData::gmList) {
        GlobalData::gmList->SendGMList_To_UpperServer(0);
    }
    return 1;
}

// IDA_EA: 0x82A6A4A
// IDA_NAME: _ZN19CMonitorServerProxy4InitESsii
// DEMANGLED: CMonitorServerProxy::Init(std::string,int,int)
int CMonitorServerProxy::Init(std::string server_ip,
                              int udp_port,
                              int tcp_port) {
    server_ip_ = server_ip;
    udp_port_ = udp_port;
    tcp_port_ = tcp_port;
    return Init();
}

// IDA_EA: 0x8470BEE
// IDA_NAME: _ZN19CMonitorServerProxy10DisconnectEv
// DEMANGLED: CMonitorServerProxy::Disconnect(void)
int CMonitorServerProxy::Disconnect() {
    return tcp_handler_.disconnect();
}

// IDA_EA: 0x82B314E
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyEC2Ev
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::CServerProxyMgr(void)
CServerProxyMgr<CMonitorServerProxy>::CServerProxyMgr()
    : server_groups_(0),
      proxies_(0),
      proxy_count_(0),
      has_proxy_array_(0),
      reserved_0d_{0, 0, 0},
      default_proxy_() {}

// IDA_EA: 0x82B39FE
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyED2Ev
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::~CServerProxyMgr()
CServerProxyMgr<CMonitorServerProxy>::~CServerProxyMgr() {
    if (has_proxy_array_ == 1) {
        delete[] server_groups_;
        delete[] proxies_;
    }
    server_groups_ = 0;
    proxies_ = 0;
    proxy_count_ = 0;
    has_proxy_array_ = 0;
}

// IDA_EA: 0x82B3DC2
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE17createServerProxyEii17ENUM_SERVER_GROUPPcii
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::createServerProxy(int,int,ENUM_SERVER_GROUP,char *,int,int)
int CServerProxyMgr<CMonitorServerProxy>::createServerProxy(
    int server_count,
    int index,
    ENUM_SERVER_GROUP server_group,
    char* server_ip,
    int udp_port,
    int tcp_port) {
    const std::string endpoint(server_ip ? server_ip : "");
    if (server_group != SERVER_GROUP_DEFAULT) {
        if (has_proxy_array_ != 1) {
            if (server_count <= 0 || index < 0 || index >= server_count) {
                return 0;
            }
            server_groups_ = new (std::nothrow) ENUM_SERVER_GROUP[server_count];
            proxies_ = new (std::nothrow) CMonitorServerProxy[server_count];
            if (!server_groups_ || !proxies_) {
                delete[] server_groups_;
                delete[] proxies_;
                server_groups_ = 0;
                proxies_ = 0;
                return 0;
            }
            proxy_count_ = server_count;
            has_proxy_array_ = 1;
        }
        if (index < 0 || index >= proxy_count_) {
            return 0;
        }
        server_groups_[index] = server_group;
        return proxies_[index].Init(endpoint, udp_port, tcp_port);
    }

    proxies_ = &default_proxy_;
    proxy_count_ = 1;
    return default_proxy_.Init(endpoint, udp_port, tcp_port);
}

// IDA_EA: 0x8470C5E
// IDA_NAME: _ZN19CMonitorServerProxy13SendHeartBeatEi
// DEMANGLED: CMonitorServerProxy::SendHeartBeat(int)
int CMonitorServerProxy::SendHeartBeat(int channel_no) {
    Packet_Monitor_UDP_HeartBeat packet;
    // Official 0627 ELF 0x8470C6F..0x8470C72 stores the argument's low
    // byte at packet+0x0A after construction.  Packet_Monitor_UDP_HeartBeat
    // owns that exact byte as state.
    packet.state = static_cast<uint8_t>(channel_no);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x8471124
// IDA_NAME: _ZN19CMonitorServerProxy22SendExchangeServerInfoEiis
// DEMANGLED: CMonitorServerProxy::SendExchangeServerInfo(int,int,short)
int CMonitorServerProxy::SendExchangeServerInfo(int channel_no,
                                                int exchange_server_ip,
                                                short exchange_server_port) {
    Packet_Exchange_Server_Info packet;
    packet.channel_no = channel_no;
    packet.exchange_server_ip =
        static_cast<uint32_t>(exchange_server_ip);
    packet.exchange_server_port = exchange_server_port;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.header.size);
}

// IDA_EA: 0x8470C94
// IDA_NAME: _ZN19CMonitorServerProxy9SendLoginEijiPchS0_
// DEMANGLED: CMonitorServerProxy::SendLogin(int,unsigned int,int,char *,unsigned char,char *)
char CMonitorServerProxy::SendLogin(int uid,
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
            1, "DF_MonitorServerProxy.cpp",
            "void CMonitorServerProxy::SendLogin(int, memberIdentificationNumber_t, int, char*, unsigned char, char*)",
            0x4F3, "CMonitorServerProxy::SendLogin! DB ID(%u)", account_id);
    }
    packet.sex = sex;
    std::memcpy(packet.ssn, ssn, 6u);
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x811208A
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE14GetServerProxyE17ENUM_SERVER_GROUP
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::GetServerProxy(ENUM_SERVER_GROUP)
CMonitorServerProxy*
CServerProxyMgr<CMonitorServerProxy>::GetServerProxy(
    ENUM_SERVER_GROUP server_group) {
    if (!server_group) {
        return &default_proxy_;
    }

    for (int i = 0; proxy_count_ > i; ++i) {
        if (server_groups_ && server_groups_[i] == server_group) {
            return &proxies_[i];
        }
    }
    return &default_proxy_;
}

// IDA_EA: 0x82B2AAA
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE13GetStartIndexEv
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::GetStartIndex(void)
int CServerProxyMgr<CMonitorServerProxy>::GetStartIndex() {
    if (proxy_count_ > 0 && has_proxy_array_ == 1) {
        return static_cast<int>(server_groups_[0]);
    }
    return 0;
}

// IDA_EA: 0x82B2AD6
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE11GetEndIndexEv
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::GetEndIndex(void)
int CServerProxyMgr<CMonitorServerProxy>::GetEndIndex() {
    return proxy_count_;
}

// IDA_EA: 0x857F8C2
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE23GetServerProxyArraySizeEv
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::GetServerProxyArraySize(void)
int CServerProxyMgr<CMonitorServerProxy>::GetServerProxyArraySize() {
    return proxy_count_;
}

// IDA_EA: 0x857F8CE
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE21GetServerProxyArrayAtEi
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::GetServerProxyArrayAt(int)
CMonitorServerProxy*
CServerProxyMgr<CMonitorServerProxy>::GetServerProxyArrayAt(int index) {
    if (index < 0 || index >= proxy_count_) {
        return 0;
    }
    return proxies_ + index;
}

// IDA_EA: 0x82B2AE2
// IDA_NAME: _ZN15CServerProxyMgrI19CMonitorServerProxyE12GetNextIndexERi
// DEMANGLED: CServerProxyMgr<CMonitorServerProxy>::GetNextIndex(int &)
int CServerProxyMgr<CMonitorServerProxy>::GetNextIndex(int& index) {
    ++index;
    if (has_proxy_array_ != 1 || proxy_count_ <= index) {
        return 0;
    }
    return static_cast<int>(server_groups_[index]);
}

// IDA_EA: 0x8470D68
// IDA_NAME: _ZN19CMonitorServerProxy10SendLogoutEjiPcjjjhh
// DEMANGLED: CMonitorServerProxy::SendLogout(unsigned int,int,char *,unsigned int,unsigned int,unsigned int,unsigned char,unsigned char)
char CMonitorServerProxy::SendLogout(unsigned int account_id,
                                     int channel_no,
                                     char* charac_name,
                                     unsigned int charac_no,
                                     unsigned int charac_guildkey,
                                     unsigned int charac_memberkey,
                                     unsigned char flag,
                                     unsigned char cached) {
    if (!account_id) {
        return static_cast<char>(cached);
    }

    Packet_Monitor_UDP_Logout packet;
    packet.account_id = account_id;
    packet.channel_no = static_cast<uint8_t>(channel_no);
    packet.charac_no = charac_no;
    packet.reason = charac_guildkey;
    packet.state = flag;
    packet.play_time = charac_memberkey;
    packet.trailing_reserved = cached;
    if (charac_name) {
        std::strcpy(reinterpret_cast<char*>(packet.tail), charac_name);
    }
    return static_cast<char>(
        SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size));
}

// IDA_EA: 0x8470DF4
// IDA_NAME: _ZN19CMonitorServerProxy10SendPacketEPci
// DEMANGLED: CMonitorServerProxy::SendPacket(char *,int)
int CMonitorServerProxy::SendPacket(char* packet, int size) {
    if (!udp_port_) {
        return 0;
    }

    return udp_handler_.SendToServer(packet,
                                     static_cast<size_t>(size),
                                     static_cast<uint16_t>(udp_port_),
                                     server_ip_.c_str());
}

// IDA_EA: 0x8471170
// IDA_NAME: _ZN19CMonitorServerProxy18SendNoticeCharLiveEj
// DEMANGLED: CMonitorServerProxy::SendNoticeCharLive(unsigned int)
int CMonitorServerProxy::SendNoticeCharLive(uint32_t charac_no) {
    Packet_Monitor_Notice_Charac_Live_On_Ten_Min packet;
    packet.charac_no = charac_no;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x8471240
// IDA_NAME: _ZN19CMonitorServerProxy15updateTowerRankEjjjj
// DEMANGLED: CMonitorServerProxy::updateTowerRank(
//             unsigned int,unsigned int,unsigned int,unsigned int)
int CMonitorServerProxy::updateTowerRank(uint32_t charac_no,
                                         uint32_t tower_index,
                                         uint32_t member_count,
                                         uint32_t virtual_rank) {
    PacketMonitorTowerRankUpdate packet(charac_no,
                                        tower_index,
                                        member_count,
                                        virtual_rank);
    return SendPacket(reinterpret_cast<char*>(&packet), packet.header.size);
}

// IDA_EA: 0x8471072
// IDA_NAME: _ZN19CMonitorServerProxy17SendNotifyNewMailEj
// DEMANGLED: CMonitorServerProxy::SendNotifyNewMail(unsigned int)
int CMonitorServerProxy::SendNotifyNewMail(unsigned int receiver_charac_no) {
    Packet_Monitor_Notify_New_Mail packet;
    packet.receiver_charac_no = receiver_charac_no;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x8470C04
// IDA_NAME: _ZN19CMonitorServerProxy21SendCharLevelGrowTypeEjjss
// DEMANGLED: CMonitorServerProxy::SendCharLevelGrowType(
//             unsigned int,unsigned int,short,short)
int CMonitorServerProxy::SendCharLevelGrowType(unsigned int account_id,
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

// IDA_EA: 0x8470E44
// IDA_NAME: _ZN19CMonitorServerProxy13SendTcpPacketEPci
// DEMANGLED: CMonitorServerProxy::SendTcpPacket(char *,int)
char CMonitorServerProxy::SendTcpPacket(char* packet, int size) {
    CNetwork100000_100000* tcp_handler = GetTcpHandler();
    if (tcp_handler->get_socket() == -1) {
        return 1;
    }
    return static_cast<char>(tcp_handler_.send_packet(packet, size));
}

// IDA_EA: 0x84712E0
// IDA_NAME: _ZN19CMonitorServerProxy9OnTcpRecvEv
// DEMANGLED: CMonitorServerProxy::OnTcpRecv(void)
int CMonitorServerProxy::OnTcpRecv() {
    const int received_bytes = tcp_handler_.recv_packet();
    if (received_bytes <= 0) {
        if (received_bytes >= 0) {
            return 1;
        }

        LogManager::logFormat(1,
                              "DF_MonitorServerProxy.cpp",
                              "bool CMonitorServerProxy::OnTcpRecv()",
                              0x68E,
                              "Monitor Server maybe Die!!!");
        Disconnect();
        return 0;
    }

    if (Parsing(received_bytes) == 1) {
        return 1;
    }

    LogManager::logFormat(1,
                          "DF_MonitorServerProxy.cpp",
                          "bool CMonitorServerProxy::OnTcpRecv()",
                          0x685,
                          "Monitor Recv Parsing Err!!!");
    return 0;
}

// IDA_EA: 0x847139C
// IDA_NAME: _ZN19CMonitorServerProxy9OnTcpSendEv
// DEMANGLED: CMonitorServerProxy::OnTcpSend(void)
int CMonitorServerProxy::OnTcpSend() {
    if (tcp_handler_.send_packet() >= 0) {
        return tcp_handler_.GetRemainSendSize();
    }

    cMyTrace trace("int CMonitorServerProxy::OnTcpSend()", 1699, 0);
    trace("OnSend() Socket[%d] Error(%s)", tcp_handler_.get_socket(),
          std::strerror(errno));
    Disconnect();
    return -1;
}

// IDA_EA: 0x8471444
// IDA_NAME: _ZN19CMonitorServerProxy10OnTcpErrorEv
// DEMANGLED: CMonitorServerProxy::OnTcpError(void)
int CMonitorServerProxy::OnTcpError() {
    if (errno == 0) {
        return 1;
    }

    cMyTrace trace("bool CMonitorServerProxy::OnTcpError()", 1718, 0);
    trace("Monitor Server OnError() Socket[%d] Error(%s)",
          tcp_handler_.get_socket(), std::strerror(errno));
    return 0;
}

// IDA_EA: 0x84714CE
// IDA_NAME: _ZN19CMonitorServerProxy7ParsingEi
// DEMANGLED: CMonitorServerProxy::Parsing(int)
int CMonitorServerProxy::Parsing(int received_bytes) {
    return tcp_handler_.Parsing_ServerPacket(received_bytes);
}

// IDA_EA: 0x847173C
// IDA_NAME: _ZN19CMonitorServerProxy13GetTcpHandlerEv
// DEMANGLED: CMonitorServerProxy::GetTcpHandler(void)
CNetwork100000_100000* CMonitorServerProxy::GetTcpHandler() {
    return &tcp_handler_;
}

// IDA_EA: 0x8470FB4
// IDA_NAME: _ZN19CMonitorServerProxy16SendMemberPayTaxEbjii
// DEMANGLED: CMonitorServerProxy::SendMemberPayTax(bool,unsigned int,int,int)
int CMonitorServerProxy::SendMemberPayTax(bool enabled,
                                          unsigned int charac_no,
                                          int money_tax,
                                          int fatigue_tax) {
    if (!enabled) {
        return 1;
    }
    if (!money_tax && !fatigue_tax) {
        return 0;
    }

    Packet_Monitor_Member_Pay_Tax packet;
    packet.charac_no = charac_no;
    packet.money_tax = money_tax;
    packet.fatigue_tax = fatigue_tax;
    return SendPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x847160A
// IDA_NAME: _ZN36Packet_Monitor_User_Repel_ByCharNameC2Ev
// DEMANGLED: Packet_Monitor_User_Repel_ByCharName::
//             Packet_Monitor_User_Repel_ByCharName(void)
Packet_Monitor_User_Repel_ByCharName::Packet_Monitor_User_Repel_ByCharName()
    : header(0x04D0u, 0x0030u) {
    std::memset(charac_name, 0, sizeof(charac_name));
}

// IDA_EA: 0x8471694
// IDA_NAME: _ZN21Packet_GM_Request_MidC2Ev
// DEMANGLED: Packet_GM_Request_Mid::Packet_GM_Request_Mid(void)
Packet_GM_Request_Mid::Packet_GM_Request_Mid() : header(0x076Cu, 0x0034u) {
    std::memset(charac_name, 0, sizeof(charac_name));
}

// IDA_EA: 0x847101A
// IDA_NAME: _ZN19CMonitorServerProxy13SendUserRepelEjjPci
// DEMANGLED: CMonitorServerProxy::SendUserRepel(unsigned int,unsigned int,char *,int)
int CMonitorServerProxy::SendUserRepel(uint32_t account_id,
                                       uint32_t uid,
                                       char* charac_name,
                                       int charac_name_size) {
    Packet_Monitor_User_Repel_ByCharName packet;
    packet.uid = uid;
    packet.account_id = account_id;
    std::memcpy(packet.charac_name,
                charac_name,
                static_cast<size_t>(charac_name_size));
    return SendPacket(reinterpret_cast<char*>(&packet), packet.header.size);
}

// IDA_EA: 0x84711A6
// IDA_NAME: _ZN19CMonitorServerProxy18SendRequestUserMidEjjPci
// DEMANGLED: CMonitorServerProxy::SendRequestUserMid(unsigned int,unsigned int,char *,int)
int CMonitorServerProxy::SendRequestUserMid(uint32_t account_id,
                                            uint32_t uid,
                                            char* charac_name,
                                            int charac_name_size) {
    Packet_GM_Request_Mid packet;
    packet.uid = uid;
    packet.account_id = account_id;
    std::memcpy(packet.charac_name,
                charac_name,
                static_cast<size_t>(charac_name_size));
    return SendPacket(reinterpret_cast<char*>(&packet), packet.header.size);
}

// IDA_EA: 0x84715C2
// IDA_NAME: _ZN29Packet_Monitor_Member_Pay_TaxC2Ev
// DEMANGLED: Packet_Monitor_Member_Pay_Tax::Packet_Monitor_Member_Pay_Tax(void)
Packet_Monitor_Member_Pay_Tax::Packet_Monitor_Member_Pay_Tax()
    : type(0x04C0), size(0x0016), reserved(0), checksum(0), charac_no(0),
      money_tax(0), fatigue_tax(0) {}

// IDA_EA: 0x8471670
// IDA_NAME: _ZN27Packet_Exchange_Server_InfoC2Ev
// DEMANGLED: Packet_Exchange_Server_Info::Packet_Exchange_Server_Info(void)
Packet_Exchange_Server_Info::Packet_Exchange_Server_Info()
    : header(0x09DEu, 0x0014u),
      channel_no(0),
      exchange_server_port(0),
      exchange_server_ip(0) {}

// IDA_EA: 0x82A3FFA
// IDA_NAME: _ZN24Packet_Monitor_Char_InfoC2Ev
// DEMANGLED: Packet_Monitor_Char_Info::Packet_Monitor_Char_Info(void)
Packet_Monitor_Char_Info::Packet_Monitor_Char_Info()
    : header(0x03EFu, 0x003Fu),
      account_id(0),
      channel_no(-1),
      charac_no(0),
      charac_guildkey(0),
      charac_job(-1),
      charac_grow_type(-1),
      charac_level(-1),
      charac_memberkey(0),
      return_user_first_login(0) {
    // The old constructor clears exactly +0x1f..+0x3c and byte +0x3e.  It
    // deliberately leaves channel_type (+0x3d) for CGameManager::
    // insert_game_world to populate before the frame is sent.
    std::memset(charac_name, 0, sizeof(charac_name));
}

// IDA_EA: 0x822BA10
// IDA_NAME: _ZN33Packet_Monitor_Member_Enter_ReplyC2Ev
// DEMANGLED: Packet_Monitor_Member_Enter_Reply::Packet_Monitor_Member_Enter_Reply(void)
Packet_Monitor_Member_Enter_Reply::Packet_Monitor_Member_Enter_Reply()
    : header(0x04B9u, 0x0013u) {}
