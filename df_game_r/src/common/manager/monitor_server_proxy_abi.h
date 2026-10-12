#ifndef OLDSRV_GLOBAL_MONITOR_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_MONITOR_SERVER_PROXY_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "old_data/db/core/db_conn_info.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/transport/network_endpoint.h"
#include "infra/network/transport/udp_handler.h"

// IDA unit: 99_misc_global/CMonitorServerProxy and
// 99_misc_global/CServerProxyMgr<CMonitorServerProxy>.
// These old types live in the global namespace; keep them separate from the
// namespaced runtime transport helpers.
class CMonitorServerProxy {
public:
    // IDA_EA: 0x82A69CC / 0x847097C / 0x82B312C.
    CMonitorServerProxy();
    CMonitorServerProxy(std::string server_ip, int udp_port, int tcp_port);
    ~CMonitorServerProxy();

    // IDA_EA: 0x8470A00 / 0x82A6A4A / 0x8470A7C / 0x8470AD6.
    int Init();
    int Init(std::string server_ip, int udp_port, int tcp_port);
    int InitUdp();
    int InitTcp();
    int Disconnect();

    // IDA_EA: 0x8470C5E
    // IDA_NAME: _ZN19CMonitorServerProxy13SendHeartBeatEi
    // DEMANGLED: CMonitorServerProxy::SendHeartBeat(int)
    int SendHeartBeat(int channel_no);

    // IDA_EA: 0x8471124
    // IDA_NAME: _ZN19CMonitorServerProxy22SendExchangeServerInfoEiis
    // DEMANGLED: CMonitorServerProxy::SendExchangeServerInfo(int,int,short)
    int SendExchangeServerInfo(int channel_no,
                               int exchange_server_ip,
                               short exchange_server_port);

    // IDA_EA: 0x84714EC
    // IDA_NAME: _ZN19CMonitorServerProxy18SendGameServerInfoEPcthh
    char SendGameServerInfo(char* server_ip,
                            unsigned short udp_port,
                            unsigned char server_group,
                            unsigned char channel_no);

    // IDA_EA: 0x8470C94
    // IDA_NAME: _ZN19CMonitorServerProxy9SendLoginEijiPchS0_
    // DEMANGLED: CMonitorServerProxy::SendLogin(
    //             int,unsigned int,int,char *,unsigned char,char *)
    char SendLogin(int uid,
                   unsigned int account_id,
                   int channel_no,
                   char* peer_ip,
                   unsigned char sex,
                   char* ssn);

    // IDA_EA: 0x8470D68
    // IDA_NAME: _ZN19CMonitorServerProxy10SendLogoutEjiPcjjjhh
    // DEMANGLED: CMonitorServerProxy::SendLogout(unsigned int,int,char *,unsigned int,unsigned int,unsigned int,unsigned char,unsigned char)
    char SendLogout(unsigned int account_id,
                    int channel_no,
                    char* charac_name,
                    unsigned int charac_no,
                    unsigned int charac_guildkey,
                    unsigned int charac_memberkey,
                    unsigned char flag,
                    unsigned char cached);

    // Old UDP forwarding returns CUdpHandler::SendToServer's raw int, while
    // the TCP forwarding ABI returns the low byte of CNetwork::send_packet.
    // Do not normalize either result to bool: callers distinguish 0 from the
    // negative TCP failure result exactly as the old server did.
    int SendPacket(char* packet, int size);
    char SendTcpPacket(char* packet, int size);
    // IDA_EA: 0x8471170
    // IDA_NAME: _ZN19CMonitorServerProxy18SendNoticeCharLiveEj
    // DEMANGLED: CMonitorServerProxy::SendNoticeCharLive(unsigned int)
    int SendNoticeCharLive(uint32_t charac_no);
    // IDA_EA: 0x8471240
    // IDA_NAME: _ZN19CMonitorServerProxy15updateTowerRankEjjjj
    // DEMANGLED: CMonitorServerProxy::updateTowerRank(
    //             unsigned int,unsigned int,unsigned int,unsigned int)
    // Sends the fixed monitor UDP ranking frame for a completed Death Tower
    // record.  The four arguments map directly to the four u32 payload
    // fields following PacketHeader.
    int updateTowerRank(uint32_t charac_no,
                        uint32_t tower_index,
                        uint32_t member_count,
                        uint32_t virtual_rank);
    // IDA_EA: 0x8471072
    // IDA_NAME: _ZN19CMonitorServerProxy17SendNotifyNewMailEj
    // DEMANGLED: CMonitorServerProxy::SendNotifyNewMail(unsigned int)
    int SendNotifyNewMail(uint32_t receiver_charac_no);
    // IDA_EA: 0x8470C04
    // IDA_NAME: _ZN19CMonitorServerProxy21SendCharLevelGrowTypeEjjss
    // DEMANGLED: CMonitorServerProxy::SendCharLevelGrowType(
    //             unsigned int,unsigned int,short,short)
    int SendCharLevelGrowType(unsigned int account_id,
                              unsigned int character_no,
                              short character_level,
                              short grow_type);
    // IDA_EA: 0x8471288
    // IDA_NAME: _ZN19CMonitorServerProxy18SendChangeCharNameEjjjPc
    // DEMANGLED: CMonitorServerProxy::SendChangeCharName(unsigned int,unsigned int,unsigned int,char *)
    char SendChangeCharName(unsigned int account_id,
                            unsigned int charac_no,
                            unsigned int guild_id,
                            char* charac_name);
    int SendMemberPayTax(bool enabled,
                         unsigned int charac_no,
                         int money_tax,
                         int fatigue_tax);

    // IDA_EA: 0x847101A
    // IDA_NAME: _ZN19CMonitorServerProxy13SendUserRepelEjjPci
    // DEMANGLED: CMonitorServerProxy::SendUserRepel(
    //             unsigned int,unsigned int,char *,int)
    int SendUserRepel(uint32_t account_id,
                      uint32_t uid,
                      char* charac_name,
                      int charac_name_size);

    // IDA_EA: 0x84711A6
    // IDA_NAME: _ZN19CMonitorServerProxy18SendRequestUserMidEjjPci
    // DEMANGLED: CMonitorServerProxy::SendRequestUserMid(
    //             unsigned int,unsigned int,char *,int)
    int SendRequestUserMid(uint32_t account_id,
                           uint32_t uid,
                           char* charac_name,
                           int charac_name_size);

    // IDA_EA: 0x84712E0 / 0x847139C / 0x8471444 / 0x84714CE.
    // These are concrete monitor-side transport operations, not BaseServerProxy
    // virtuals: MonitorTcpThread calls them for every configured monitor
    // endpoint after select() reports readiness.
    int OnTcpRecv();
    int OnTcpSend();
    int OnTcpError();
    int Parsing(int received_bytes);

    CNetwork100000_100000* GetTcpHandler();

    std::string server_ip_;                         // +0x00000
    int udp_port_;                                  // +0x00004
    int tcp_port_;                                  // +0x00008
    CNetwork100000_100000 tcp_handler_;     // +0x0000C
    CUdpHandler udp_handler_;               // +0x30E04
};

#pragma pack(push, 1)
// IDA unit: 99_misc_global/Packet_Exchange_Server_Info.  The monitor uses
// this fixed frame to elect the exchange master and publish its endpoint.
struct Packet_Exchange_Server_Info {
    // IDA_EA: 0x8471670
    // IDA_NAME: _ZN27Packet_Exchange_Server_InfoC2Ev
    // DEMANGLED: Packet_Exchange_Server_Info::Packet_Exchange_Server_Info(void)
    Packet_Exchange_Server_Info();

    PacketHeader header;              // +0x00, type 0x09DE / size 0x14
    int32_t channel_no;                // +0x0A
    int16_t exchange_server_port;      // +0x0E
    uint32_t exchange_server_ip;       // +0x10, network byte order
};

struct Packet_Monitor_Member_Pay_Tax {
    Packet_Monitor_Member_Pay_Tax();

    uint16_t type;
    uint16_t size;
    uint16_t reserved;
    uint32_t checksum;
    uint32_t charac_no;
    int32_t money_tax;
    int32_t fatigue_tax;
};

// IDA unit: 99_misc_global/Packet_Monitor_Char_Info.  This is the fixed
// 0x3f-byte TCP frame sent to both monitor and guild servers immediately
// after CGameManager::insert_game_world succeeds.
struct Packet_Monitor_Char_Info {
    // IDA_EA: 0x82A3FFA
    // IDA_NAME: _ZN24Packet_Monitor_Char_InfoC2Ev
    // DEMANGLED: Packet_Monitor_Char_Info::Packet_Monitor_Char_Info(void)
    Packet_Monitor_Char_Info();

    PacketHeader header;                   // +0x00, type 0x03ef / size 0x3f
    uint32_t account_id;                   // +0x0a
    int8_t channel_no;                     // +0x0e, default -1
    uint32_t charac_no;                    // +0x0f
    uint32_t charac_guildkey;              // +0x13
    int8_t charac_job;                     // +0x17, default -1
    int8_t charac_grow_type;               // +0x18, default -1
    int16_t charac_level;                  // +0x19, default -1
    uint32_t charac_memberkey;             // +0x1b
    char charac_name[30];                  // +0x1f
    uint8_t channel_type;                  // +0x3d, set by world-entry caller
    uint8_t return_user_first_login;       // +0x3e, set by world-entry caller
};

// IDA unit: 99_misc_global/Packet_Monitor_Member_Enter_Reply.
// Inter_MonitorNoticeRequestMemberEnterToResponser sends this exact TCP
// monitor frame when the target is in solo mode or has opted out.
struct Packet_Monitor_Member_Enter_Reply {
    // IDA_EA: 0x822BA10
    // IDA_NAME: _ZN33Packet_Monitor_Member_Enter_ReplyC2Ev
    Packet_Monitor_Member_Enter_Reply();

    PacketHeader header;       // +0x00, type 0x04B9 / size 0x13
    uint32_t uid;              // +0x0a
    uint32_t charac_no;        // +0x0e
    uint8_t reply_kind;        // +0x12
};

// IDA unit: 99_misc_global/Packet_Monitor_User_Repel_ByCharName.
// SendUserRepel initializes only the header and name buffer before filling
// uid/account_id and copying caller-owned bytes.
struct Packet_Monitor_User_Repel_ByCharName {
    // IDA_EA: 0x847160A
    // IDA_NAME: _ZN36Packet_Monitor_User_Repel_ByCharNameC2Ev
    // DEMANGLED: Packet_Monitor_User_Repel_ByCharName::
    //             Packet_Monitor_User_Repel_ByCharName(void)
    Packet_Monitor_User_Repel_ByCharName();

    PacketHeader header;       // +0x00, type 0x04d0 / size 0x30
    uint32_t uid;              // +0x0a
    uint32_t account_id;       // +0x0e
    char charac_name[30];      // +0x12
};

// IDA unit: 08_quest_skill_growth/Packet_GM_Request_Mid.
// The original constructor zeroes only charac_name.  Its caller fills uid and
// account_id, while +0x12 remains caller-stack data and is intentionally not
// initialized here.
struct Packet_GM_Request_Mid {
    // IDA_EA: 0x8471694
    // IDA_NAME: _ZN21Packet_GM_Request_MidC2Ev
    // DEMANGLED: Packet_GM_Request_Mid::Packet_GM_Request_Mid(void)
    Packet_GM_Request_Mid();

    PacketHeader header;       // +0x00, type 0x076c / size 0x34
    uint32_t uid;              // +0x0a
    uint32_t account_id;       // +0x0e
    uint32_t opaque_12;        // +0x12, deliberately uninitialized by C2
    char charac_name[30];      // +0x16
};
#pragma pack(pop)

template <typename T>
class CServerProxyMgr;

template <>
class CServerProxyMgr<CMonitorServerProxy> {
public:
    // IDA_EA: 0x82B314E / 0x82B39FE.
    CServerProxyMgr();
    ~CServerProxyMgr();

    CMonitorServerProxy* GetServerProxy(ENUM_SERVER_GROUP server_group);
    // IDA_EA: 0x82B3DC2.
    int createServerProxy(int server_count,
                          int index,
                          ENUM_SERVER_GROUP server_group,
                          char* server_ip,
                          int udp_port,
                          int tcp_port);
    // IDA_EA: 0x82B2AAA / 0x82B2AD6 / 0x82B2AE2.
    // These methods iterate configured server groups, rather than physical
    // proxy-array positions. MainLoop uses the returned group as the lookup
    // key for GetServerProxy.
    int GetStartIndex();
    int GetEndIndex();
    int GetNextIndex(int& index);

    // IDA_EA: 0x857F8C2 / 0x857F8CE.  MonitorTcpThread iterates physical
    // proxy-array entries, whereas GetServerProxy() looks up a server group.
    int GetServerProxyArraySize();
    CMonitorServerProxy* GetServerProxyArrayAt(int index);

    ENUM_SERVER_GROUP* server_groups_;              // +0x00000
    CMonitorServerProxy* proxies_;                  // +0x00004
    int proxy_count_;                               // +0x00008
    uint8_t has_proxy_array_;                       // +0x0000C
    uint8_t reserved_0d_[3];                        // +0x0000D
    CMonitorServerProxy default_proxy_;             // +0x00010
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ENUM_SERVER_GROUP) == 0x04u,
              "ENUM_SERVER_GROUP IDA ABI size must stay 0x04");
static_assert(sizeof(CMonitorServerProxy) == 200204u,
              "CMonitorServerProxy IDA ABI size must stay 200204");
static_assert(sizeof(Packet_Exchange_Server_Info) == 0x14u,
              "Packet_Exchange_Server_Info IDA ABI size must stay 0x14");
static_assert(offsetof(Packet_Exchange_Server_Info, channel_no) == 0x0au &&
                  offsetof(Packet_Exchange_Server_Info,
                           exchange_server_port) == 0x0eu &&
                  offsetof(Packet_Exchange_Server_Info,
                           exchange_server_ip) == 0x10u,
              "Packet_Exchange_Server_Info offsets must stay exact");
static_assert(sizeof(Packet_Monitor_Member_Pay_Tax) == 0x16u,
              "Packet_Monitor_Member_Pay_Tax IDA ABI size must stay 0x16");
static_assert(sizeof(Packet_Monitor_Char_Info) == 0x3fu,
              "Packet_Monitor_Char_Info IDA ABI size must stay 0x3f");
static_assert(offsetof(Packet_Monitor_Char_Info, account_id) == 0x0au &&
                  offsetof(Packet_Monitor_Char_Info, channel_no) == 0x0eu &&
                  offsetof(Packet_Monitor_Char_Info, charac_no) == 0x0fu &&
                  offsetof(Packet_Monitor_Char_Info, charac_guildkey) == 0x13u &&
                  offsetof(Packet_Monitor_Char_Info, charac_job) == 0x17u &&
                  offsetof(Packet_Monitor_Char_Info, charac_grow_type) == 0x18u &&
                  offsetof(Packet_Monitor_Char_Info, charac_level) == 0x19u &&
                  offsetof(Packet_Monitor_Char_Info, charac_memberkey) == 0x1bu &&
                  offsetof(Packet_Monitor_Char_Info, charac_name) == 0x1fu &&
                  offsetof(Packet_Monitor_Char_Info, channel_type) == 0x3du &&
                  offsetof(Packet_Monitor_Char_Info, return_user_first_login) == 0x3eu,
              "Packet_Monitor_Char_Info offsets must stay exact");
static_assert(sizeof(Packet_Monitor_Member_Enter_Reply) == 0x13u,
              "Packet_Monitor_Member_Enter_Reply IDA ABI size must stay 0x13");
static_assert(offsetof(Packet_Monitor_Member_Enter_Reply, uid) == 0x0au &&
                  offsetof(Packet_Monitor_Member_Enter_Reply, charac_no) == 0x0eu &&
                  offsetof(Packet_Monitor_Member_Enter_Reply, reply_kind) == 0x12u,
              "Packet_Monitor_Member_Enter_Reply offsets must stay exact");
static_assert(sizeof(Packet_Monitor_User_Repel_ByCharName) == 0x30u,
              "Packet_Monitor_User_Repel_ByCharName IDA ABI size must stay 0x30");
static_assert(offsetof(Packet_Monitor_User_Repel_ByCharName, uid) == 0x0au &&
                  offsetof(Packet_Monitor_User_Repel_ByCharName, account_id) == 0x0eu &&
                  offsetof(Packet_Monitor_User_Repel_ByCharName, charac_name) == 0x12u,
              "Packet_Monitor_User_Repel_ByCharName offsets must stay exact");
static_assert(sizeof(Packet_GM_Request_Mid) == 0x34u,
              "Packet_GM_Request_Mid IDA ABI size must stay 0x34");
static_assert(offsetof(Packet_GM_Request_Mid, uid) == 0x0au &&
                  offsetof(Packet_GM_Request_Mid, account_id) == 0x0eu &&
                  offsetof(Packet_GM_Request_Mid, opaque_12) == 0x12u &&
                  offsetof(Packet_GM_Request_Mid, charac_name) == 0x16u,
              "Packet_GM_Request_Mid offsets must stay exact");
static_assert(offsetof(CMonitorServerProxy, tcp_handler_) == 0x0Cu,
              "CMonitorServerProxy tcp handler offset must stay 0x0C");
static_assert(offsetof(CMonitorServerProxy, udp_handler_) == 200196u,
              "CMonitorServerProxy UDP handler offset must stay 200196");
static_assert(sizeof(CServerProxyMgr<CMonitorServerProxy>) == 0x30E1Cu,
              "CServerProxyMgr<CMonitorServerProxy> size must stay 0x30E1C");
static_assert(offsetof(CServerProxyMgr<CMonitorServerProxy>, default_proxy_) ==
                  0x10u,
              "CServerProxyMgr<CMonitorServerProxy> default proxy offset must stay 0x10");
#endif

#endif  // OLDSRV_GLOBAL_MONITOR_SERVER_PROXY_ABI_H_
