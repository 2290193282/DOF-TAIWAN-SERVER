#ifndef OLDSRV_GLOBAL_GUILD_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_GUILD_SERVER_PROXY_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>

#include "old_data/db/core/db_conn_info.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/transport/network_endpoint.h"
#include "infra/network/transport/udp_handler.h"

struct Packet_Guild_Change_Power_War_Point;
class CItem;

// IDA unit: 99_misc_global/STGuildProxy.  The guild proxy cache is a 26-byte
// value.  LoadGuildInfoProxyFromDB @0x846E714 maps guild_name, lev,
// power_side and guild_agit_flag to +0x00/+0x17/+0x18/+0x19 respectively;
// CUser::SetGuildInfoProxy @0x865CD0A independently confirms that +0x18 is
// copied into STGuildDBInfoOnly::power_side.
#pragma pack(push, 1)
struct STGuildProxy {
    STGuildProxy();

    // IDA_EA: 0x823451C
    // IDA_NAME: _ZN12STGuildProxyC2EPKchh
    // The temporary-side update receiver copies only the first 0x16 guild
    // bytes and assigns +0x17/+0x18.  The untouched bytes remain exactly as
    // the original stack construction left them.
    STGuildProxy(const char* source,
                 uint8_t guild_level_value,
                 uint8_t power_side_value);

    char guild_name[23];
    uint8_t guild_level;       // +0x17
    uint8_t power_side;        // +0x18
    uint8_t guild_agit_flag;   // +0x19
};

struct Packet_Monitor_Call_Guild_Level_Up : PacketHeader {
    Packet_Monitor_Call_Guild_Level_Up();

    int32_t uid;
    uint32_t charac_no;
    uint32_t charac_guildkey;
    uint8_t raw_16;
    uint32_t levelup_price;
};

// IDA_EA: 0x846F2B0.  Both guild and monitor proxies send this exact
// 0x12-byte level/grow-type update packet.
struct Packet_Monitor_Char_Changable_Info : PacketHeader {
    Packet_Monitor_Char_Changable_Info();

    uint32_t account_id;       // +0x0a
    uint8_t character_no;      // +0x0e
    int16_t character_level;   // +0x0f
    uint8_t grow_type;         // +0x11
};

// IDA_EA: 0x846F376. The original C2 initializes only the packet header and
// byte +0x17; SendIncreaseGuildExp fills each transmitted payload field.
struct Packet_Monitor_Increase_Guild_Exp : PacketHeader {
    Packet_Monitor_Increase_Guild_Exp();

    uint32_t charac_no;
    uint32_t charac_guildkey;
    uint32_t gained_exp;
    uint8_t source_flag;
    uint8_t notify;
    uint8_t unused[4];
};

// IDA_EA: 0x846F5BE.  The proxy emits this compact TCP packet whenever a
// player's guild power-war balance changes locally.
struct Packet_Guild_Increase_Decrease_Power_War_Point : PacketHeader {
    Packet_Guild_Increase_Decrease_Power_War_Point();

    uint32_t charac_guildkey;  // +0x0a
    uint32_t charac_no;        // +0x0e
    int32_t point_delta;       // +0x12
};

// IDA_EA: 0x846F78C.  A temporary power-side character applies its original
// faction through this compact guild TCP packet before the local join state is
// committed.
struct Packet_Guild_Apply_Origial_Power_Side : PacketHeader {
    Packet_Guild_Apply_Origial_Power_Side();

    uint32_t charac_no;        // +0x0a
    uint32_t charac_guildkey;  // +0x0e
    uint8_t power_side;        // +0x12
};

// IDA_EA: 0x846F6FE.  SendGuildExpLimit copies the proven value_04 member of
// every GuildParameterScript::guild_levelup_params entry, with the original
// two duplicated positions retained at +0x0e and +0x12.
struct Packet_Guild_Exp_Limit : PacketHeader {
    Packet_Guild_Exp_Limit();

    int32_t channel_no;                          // +0x0a
    int32_t guild_levelup_param_09_value_04;     // +0x0e
    int32_t guild_levelup_param_17_value_04;     // +0x12
    int32_t guild_levelup_param_value_04[17];    // +0x16
};

// IDA unit: 99_misc_global/Packet_Add_Guild_Fund. The fund type is a byte on
// the TCP wire; the sender keeps the original nested enum ABI.
struct Packet_Add_Guild_Fund : PacketHeader {
    enum eFundType {
        kFundTypePartyReward = 0,
        kFundTypeUserDonation = 1,
        kFundTypeGameMaster = 2
    };

    uint8_t fund_type;          // +0x0a
    int32_t fund_amount;        // +0x0b
    uint32_t charac_no;         // +0x0f
    uint32_t charac_guildkey;   // +0x13

    // IDA_EA: 0x846FB20
    // IDA_NAME: _ZN21Packet_Add_Guild_FundC2Ev
    // DEMANGLED: Packet_Add_Guild_Fund::Packet_Add_Guild_Fund(void)
    Packet_Add_Guild_Fund();
};

// df_guild_r's 0x1BBD reply is the same total size as the request but has a
// different body layout.  Do not reuse Packet_Add_Guild_Fund here: +0x0B is
// the character number and +0x0F is the game-server routing UID.
struct Packet_Add_Guild_Fund_Result : PacketHeader {
    uint8_t result_code;       // +0x0a
    uint32_t charac_no;        // +0x0b
    int32_t routing_user_uid;  // +0x0f
    int32_t refund_amount;     // +0x13
};

// IDA unit: 08_quest_skill_growth/Packet_Monitor_Buy_Guild_Skill.
struct Packet_Monitor_Buy_Guild_Skill : PacketHeader {
    Packet_Monitor_Buy_Guild_Skill();

    uint32_t charac_no;
    uint32_t charac_guildkey;
    int32_t skill_index;
    int32_t skill_level;
    int16_t guild_skill_point;
    uint32_t gold_cost;
};

// IDA_EA: 0x846FBDE.  This is the 0x90-byte TCP request used for a guild-board
// notice when a separate upgrade crosses the original +5 threshold.
struct Packet_Guild_Request_Guild_Board_Write : PacketHeader {
    Packet_Guild_Request_Guild_Board_Write();

    uint32_t charac_guildkey;
    uint32_t raw_0e;
    uint32_t charac_no;
    int8_t raw_16;
    int8_t raw_17;
    char message[0x78];
};

// IDA_EA: 0x846FBA0.  CMD341's gateway request.
struct Packet_Guild_Request_Guild_Board_Open : PacketHeader {
    Packet_Guild_Request_Guild_Board_Open();

    uint8_t request_kind;
    uint32_t charac_no;
    uint32_t charac_guildkey;
};

// IDA_EA: 0x846FC4C.  CMD343's gateway request.
struct Packet_Guild_Request_Guild_Board_Delete : PacketHeader {
    Packet_Guild_Request_Guild_Board_Delete();

    uint32_t board_index;
    uint32_t charac_no;
    uint32_t charac_guildkey;
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Increase_Guild_Exp) == 0x1cu,
              "guild EXP monitor packet must retain its 0x1c-byte ABI");
static_assert(sizeof(Packet_Guild_Increase_Decrease_Power_War_Point) == 0x16u,
              "guild power-war delta packet must retain its 0x16-byte ABI");
static_assert(sizeof(Packet_Guild_Apply_Origial_Power_Side) == 0x13u,
              "guild original-power-side packet must retain its 0x13-byte ABI");
static_assert(sizeof(Packet_Guild_Exp_Limit) == 0x5au,
              "guild EXP-limit packet must retain its old 0x5a-byte ABI");
static_assert(sizeof(Packet_Add_Guild_Fund) == 0x17u,
              "guild-fund TCP packet must retain its old 0x17-byte ABI");
static_assert(sizeof(Packet_Add_Guild_Fund_Result) == 0x17u,
              "guild-fund result must retain its old 0x17-byte ABI");
static_assert(sizeof(Packet_Monitor_Buy_Guild_Skill) == 0x20u,
              "guild-skill purchase packet must retain the old 0x20-byte ABI");

// IDA unit: 99_misc_global/CGuildServerProxy.  The old i386 object is a
// 4-byte COW string, UDP port, std::map<unsigned, STGuildProxy>, TCP port,
// followed by CNetwork<100000,100000> at +0x24.  Keeping the actual map
// instead of raw bytes is required for ModifyGuildInfoProxy to preserve the
// cache's construction/destruction and lookup semantics.
class CGuildServerProxy {
public:
    CGuildServerProxy();
    ~CGuildServerProxy();

    // IDA_EA: 0x846F0F2
    // IDA_NAME: _ZN17CGuildServerProxy18SendGameServerInfoEPcthh
    char SendGameServerInfo(char* server_ip,
                            unsigned short udp_port,
                            unsigned char server_group,
                            unsigned char channel_no);

    // IDA_EA: 0x846D8C2 / 0x82A6B3A
    // IDA_NAME: _ZN17CGuildServerProxy4InitEv / _ZN17CGuildServerProxy4InitESsii
    bool Init();
    bool Init(std::string server_ip, int udp_port, int tcp_port);
    bool InitUdp();
    bool InitTcp();
    int Disconnect();

    // IDA_EA: 0x846E53C / 0x846E5CC / 0x846E66C / 0x846E6F6
    // IDA_NAME: _ZN17CGuildServerProxy9OnTcpRecvEv /
    //           _ZN17CGuildServerProxy9OnTcpSendEv /
    //           _ZN17CGuildServerProxy10OnTcpErrorEv /
    //           _ZN17CGuildServerProxy7ParsingEi
    // MonitorTcpThread calls the concrete guild transport methods after its
    // select() pass; these are not BaseServerProxy virtuals.
    int OnTcpRecv();
    int OnTcpSend();
    int OnTcpError();
    int Parsing(int received_bytes);

    // IDA_EA: 0x846DAF4
    // IDA_NAME: _ZN17CGuildServerProxy13SendHeartBeatEi
    // DEMANGLED: CGuildServerProxy::SendHeartBeat(int)
    int SendHeartBeat(int channel_no);

    // IDA_EA: 0x846DB2A
    // IDA_NAME: _ZN17CGuildServerProxy9SendLoginEijiPchS0_
    // DEMANGLED: CGuildServerProxy::SendLogin(
    //             int,unsigned int,int,char *,unsigned char,char *)
    char SendLogin(int uid,
                   unsigned int account_id,
                   int channel_no,
                   char* peer_ip,
                   unsigned char sex,
                   char* ssn);

    // IDA_EA: 0x846DBFE
    // IDA_NAME: _ZN17CGuildServerProxy10SendLogoutEjiPcjjjh
    // DEMANGLED: CGuildServerProxy::SendLogout(unsigned int,int,char *,unsigned int,unsigned int,unsigned int,unsigned char)
    char SendLogout(unsigned int account_id,
                    int channel_no,
                    char* charac_name,
                    unsigned int charac_no,
                    unsigned int charac_guildkey,
                    unsigned int charac_memberkey,
                    unsigned char flag);

    // IDA_EA: 0x846DC7E
    // IDA_NAME: _ZN17CGuildServerProxy10SendPacketEPci
    int SendPacket(char* packet, int size);

    // IDA_EA: 0x846DA9A
    // IDA_NAME: _ZN17CGuildServerProxy21SendCharLevelGrowTypeEjjss
    // DEMANGLED: CGuildServerProxy::SendCharLevelGrowType(
    //             unsigned int,unsigned int,short,short)
    int SendCharLevelGrowType(unsigned int account_id,
                              unsigned int character_no,
                              short character_level,
                              short grow_type);

    // IDA_EA: 0x846DE80
    // IDA_NAME: _ZN17CGuildServerProxy17SendPowerWarStartEj
    // DEMANGLED: CGuildServerProxy::SendPowerWarStart(unsigned int)
    int SendPowerWarStart(unsigned int account_id);

    // IDA_EA: 0x846DECE
    // IDA_NAME: _ZN17CGuildServerProxy15SendPowerWarEndEj
    // DEMANGLED: CGuildServerProxy::SendPowerWarEnd(unsigned int)
    int SendPowerWarEnd(unsigned int account_id);

    // IDA_EA: 0x846DF1C
    // IDA_NAME: _ZN17CGuildServerProxy23SendPowerWarProcessInfoEj
    // DEMANGLED: CGuildServerProxy::SendPowerWarProcessInfo(unsigned int)
    int SendPowerWarProcessInfo(unsigned int charac_no);

    // IDA_EA: 0x846EEDA
    // IDA_NAME: _ZN17CGuildServerProxy26SendApplyOriginalPowerSideEjjh
    // DEMANGLED: CGuildServerProxy::SendApplyOriginalPowerSide(
    //             unsigned int,unsigned int,unsigned char)
    char SendApplyOriginalPowerSide(unsigned int charac_no,
                                   unsigned int charac_guildkey,
                                   unsigned char power_side);

    // IDA_EA: 0x846E026
    // IDA_NAME: _ZN17CGuildServerProxy17SendBuyGuildSkillEjjiisj
    char SendBuyGuildSkill(unsigned int charac_no,
                           unsigned int charac_guildkey,
                           int skill_index,
                           int skill_level,
                           short guild_skill_point,
                           unsigned int gold_cost);

    // IDA_EA: 0x846E084
    // IDA_NAME: _ZN17CGuildServerProxy13SendGuildMailEjjiPc
    // DEMANGLED: CGuildServerProxy::SendGuildMail(
    //             unsigned int,unsigned int,int,char *)
    int SendGuildMail(unsigned int charac_no,
                      unsigned int charac_guildkey,
                      int text_length,
                      char* text);

    // IDA_EA: 0x846ED36
    // IDA_NAME: _ZN17CGuildServerProxy33SendIncreaseDecreasePowerWarPointEjji
    // DEMANGLED: CGuildServerProxy::SendIncreaseDecreasePowerWarPoint(
    //             unsigned int,unsigned int,int)
    char SendIncreaseDecreasePowerWarPoint(unsigned int charac_guildkey,
                                           unsigned int charac_no,
                                           int point_delta);

    // IDA_EA: 0x846EF24
    // IDA_NAME: _ZN17CGuildServerProxy9OnBuyItemEP5CUserPK5CItemjj
    // DEMANGLED: CGuildServerProxy::OnBuyItem(CUser *,CItem const*,
    //             unsigned int,unsigned int)
    char OnBuyItem(CUser* user,
                   const CItem* item,
                   unsigned int guild_power_cost,
                   unsigned int user_power_cost);

    // IDA_EA: 0x846E460
    // IDA_NAME: _ZN17CGuildServerProxy17SendPowerWarPointER35Packet_Guild_Change_Power_War_Point
    // DEMANGLED: CGuildServerProxy::SendPowerWarPoint(Packet_Guild_Change_Power_War_Point &)
    char SendPowerWarPoint(Packet_Guild_Change_Power_War_Point& packet);

    // IDA_EA: 0x846DD12
    // IDA_NAME: _ZN17CGuildServerProxy26SendPacketCallGuildMembersEjj
    // DEMANGLED: CGuildServerProxy::SendPacketCallGuildMembers(unsigned int,unsigned int)
    int SendPacketCallGuildMembers(unsigned int charac_no,
                                   unsigned int charac_guildkey);

    // IDA_EA: 0x846DDD2
    // IDA_NAME: _ZN17CGuildServerProxy17SendCallGuildInfoEjjhPj
    // DEMANGLED: CGuildServerProxy::SendCallGuildInfo(unsigned int,unsigned int,unsigned char,unsigned int *)
    int SendCallGuildInfo(unsigned int charac_no,
                          unsigned int uid,
                          unsigned char guild_count,
                          unsigned int* guild_keys);

    // IDA_EA: 0x846DD4E
    // IDA_NAME: _ZN17CGuildServerProxy29SendPacketCallGuildAllMembersEjj
    // DEMANGLED: CGuildServerProxy::SendPacketCallGuildAllMembers(unsigned int,unsigned int)
    int SendPacketCallGuildAllMembers(unsigned int charac_no,
                                      unsigned int charac_guildkey);

    // IDA_EA: 0x846E0F2
    // IDA_NAME: _ZN17CGuildServerProxy19SendCallGuildInviteEjjPc
    // DEMANGLED: CGuildServerProxy::SendCallGuildInvite(unsigned int,unsigned int,char *)
    int SendCallGuildInvite(unsigned int charac_no,
                            unsigned int charac_guildkey,
                            char* target_name);

    // IDA_EA: 0x846E14C
    // IDA_NAME: _ZN17CGuildServerProxy20SendReplyGuildInviteEjh
    // DEMANGLED: CGuildServerProxy::SendReplyGuildInvite(unsigned int,unsigned char)
    int SendReplyGuildInvite(unsigned int charac_no,
                             unsigned char reply_code);

    // IDA_EA: 0x846E190
    // IDA_NAME: _ZN17CGuildServerProxy22SendRequestGuildSecedeEjjiPc
    // DEMANGLED: CGuildServerProxy::SendRequestGuildSecede(unsigned int,unsigned int,int,char *)
    int SendRequestGuildSecede(unsigned int charac_guildkey,
                               unsigned int charac_no,
                               int text_length,
                               char* text);

    // IDA_EA: 0x846E1F4
    // IDA_NAME: _ZN17CGuildServerProxy18SendMessageToGuildEjiPc
    // DEMANGLED: CGuildServerProxy::SendMessageToGuild(unsigned int,int,char *)
    int SendMessageToGuild(unsigned int charac_guildkey,
                           int message_length,
                           char* message);

    // IDA_EA: 0x846E254
    // IDA_NAME: _ZN17CGuildServerProxy15SendMemoToGuildEjiiPc
    // DEMANGLED: CGuildServerProxy::SendMemoToGuild(unsigned int,int,int,char *)
    int SendMemoToGuild(unsigned int charac_guildkey,
                        int charac_no,
                        int memo_length,
                        char* memo);

    // IDA_EA: 0x846E2AC
    // IDA_NAME: _ZN17CGuildServerProxy23SendGuildMasterDelegateEjjiPc
    // DEMANGLED: CGuildServerProxy::SendGuildMasterDelegate(unsigned int,unsigned int,int,char *)
    int SendGuildMasterDelegate(unsigned int charac_guildkey,
                                unsigned int charac_no,
                                int target_name_length,
                                char* target_name);

    // IDA_EA: 0x846E376
    // IDA_NAME: _ZN17CGuildServerProxy19SendGuildNameChangeEjjPc
    // DEMANGLED: CGuildServerProxy::SendGuildNameChange(unsigned int,unsigned int,char *)
    char SendGuildNameChange(unsigned int charac_no,
                             unsigned int charac_guildkey,
                             char* guild_name);

    // IDA_EA: 0x846E304
    // IDA_NAME: _ZN17CGuildServerProxy15SendGuildCreateEjPcS0_
    // DEMANGLED: CGuildServerProxy::SendGuildCreate(unsigned int,char *,char *)
    char SendGuildCreate(unsigned int charac_no,
                         char* guild_name,
                         char* guild_address);

    // IDA_EA: 0x846E3C8
    // IDA_NAME: _ZN17CGuildServerProxy18SendChangeCharNameEjjjPc
    // DEMANGLED: CGuildServerProxy::SendChangeCharName(unsigned int,unsigned int,unsigned int,char *)
    char SendChangeCharName(unsigned int account_id,
                            unsigned int charac_no,
                            unsigned int guild_id,
                            char* charac_name);

    // IDA_EA: 0x846DD8A
    // IDA_NAME: _ZN17CGuildServerProxy16SendGuildLevelupEijjj
    // DEMANGLED: CGuildServerProxy::SendGuildLevelup(int,unsigned int,unsigned int,unsigned int)
    char SendGuildLevelup(int uid,
                          unsigned int charac_no,
                          unsigned int charac_guildkey,
                          unsigned int levelup_price);

    // IDA_EA: 0x846ECE2
    // IDA_NAME: _ZN17CGuildServerProxy20SendIncreaseGuildExpEjjjc
    // DEMANGLED: CGuildServerProxy::SendIncreaseGuildExp(
    //             unsigned int,unsigned int,unsigned int,char)
    char SendIncreaseGuildExp(unsigned int charac_guildkey,
                              unsigned int charac_no,
                              unsigned int gained_exp,
                              char source_flag);

    // IDA_EA: 0x846EE44
    // IDA_NAME: _ZN17CGuildServerProxy17SendGuildExpLimitEv
    // DEMANGLED: CGuildServerProxy::SendGuildExpLimit(void)
    char SendGuildExpLimit();

    // IDA_EA: 0x846E95E
    // IDA_NAME: _ZN17CGuildServerProxy20ModifyGuildInfoProxyEjR12STGuildProxy
    // DEMANGLED: CGuildServerProxy::ModifyGuildInfoProxy(unsigned int,STGuildProxy &)
    int ModifyGuildInfoProxy(unsigned int charac_guildkey,
                             STGuildProxy& guild_info);

    // IDA_EA: 0x846EA3E
    // IDA_NAME: _ZN17CGuildServerProxy22ChangeGuildNameInProxyEjPc
    // DEMANGLED: CGuildServerProxy::ChangeGuildNameInProxy(unsigned int,char *)
    // The old source declaration is void (also retained in its
    // __PRETTY_FUNCTION__ literal).  Hex-Rays inferred void* only because
    // the successful tail ends in memcpy, whose incidental EAX value is not
    // part of the ABI contract.
    void ChangeGuildNameInProxy(unsigned int charac_guildkey,
                                char* guild_name);

    // IDA_EA: 0x846EC88
    // IDA_NAME: _ZN17CGuildServerProxy20InsertGuildInfoProxyEjR12STGuildProxy
    // DEMANGLED: CGuildServerProxy::InsertGuildInfoProxy(unsigned int,STGuildProxy &)
    int InsertGuildInfoProxy(unsigned int charac_guildkey,
                             STGuildProxy& guild_info);

    // IDA_EA: 0x846EB0E
    // IDA_NAME: _ZN17CGuildServerProxy26ChangeGuildAgitFlagInProxyEjh
    // DEMANGLED: CGuildServerProxy::ChangeGuildAgitFlagInProxy(
    //             unsigned int,unsigned char)
    void ChangeGuildAgitFlagInProxy(unsigned int charac_guildkey,
                                    unsigned char guild_agit_flag);

    // IDA_EA: 0x846DCCE
    // IDA_NAME: _ZN17CGuildServerProxy13SendTcpPacketEPci
    char SendTcpPacket(char* packet, int size);

    // IDA_EA: 0x846F01A
    // IDA_NAME: _ZN17CGuildServerProxy19SendGuildBoardWriteEPcjjicj
    // DEMANGLED: CGuildServerProxy::SendGuildBoardWrite(char *,unsigned int,unsigned int,int,char,unsigned int)
    char SendGuildBoardWrite(char* message,
                             unsigned int raw_0a,
                             unsigned int charac_no,
                             int raw_12,
                             char raw_16,
                             unsigned int charac_guildkey);

    // IDA_EA: 0x846EFCE / 0x846F0B0.
    char SendGuildBoardOpen(unsigned int charac_no,
                            unsigned int charac_guildkey,
                            char request_kind);
    char SendGuildBoardDelete(unsigned int board_index,
                              unsigned int charac_no,
                              unsigned int charac_guildkey);

    // IDA_EA: 0x846F16C
    // IDA_NAME: _ZN17CGuildServerProxy20SendRefreshGuildInfoEjj
    char SendRefreshGuildInfo(unsigned int charac_no,
                              unsigned int charac_guildkey);

    // IDA_EA: 0x846F1A8
    // IDA_NAME: _ZN17CGuildServerProxy16SendAddGuildFundEjjiN21Packet_Add_Guild_Fund9eFundTypeE
    // DEMANGLED: CGuildServerProxy::SendAddGuildFund(unsigned int,unsigned int,int,Packet_Add_Guild_Fund::eFundType)
    char SendAddGuildFund(unsigned int charac_no,
                          unsigned int charac_guildkey,
                          int fund_amount,
                          Packet_Add_Guild_Fund::eFundType fund_type);

    // IDA_EA: 0x846FC8E
    // IDA_NAME: _ZN17CGuildServerProxy13GetTcpHandlerEv
    CNetwork100000_100000* GetTcpHandler();

private:
    std::string server_ip_;
    unsigned int udp_port_;
    std::map<unsigned int, STGuildProxy> guild_map_;
    unsigned int tcp_port_;
    CNetwork100000_100000 tcp_handler_;
    CUdpHandler udp_handler_;
};

static_assert(sizeof(Packet_Monitor_Char_Changable_Info) == 0x12u,
              "monitor character change packet ABI");

// IDA unit: 99_misc_global/CServerProxyMgr<CGuildServerProxy>.  This is the
// concrete owner used by GlobalData, not a void* facade.  The default proxy is
// embedded and is returned for group zero or a group that has no configured
// array entry.
template <typename T>
class CServerProxyMgr;

template <>
class CServerProxyMgr<CGuildServerProxy> {
public:
    CServerProxyMgr();
    ~CServerProxyMgr();

    CGuildServerProxy* GetServerProxy(ENUM_SERVER_GROUP server_group);
    int GetStartIndex();
    int GetEndIndex();
    int GetNextIndex(int& index);
    int createServerProxy(int server_count,
                          int index,
                          ENUM_SERVER_GROUP server_group,
                          char* server_ip,
                          int udp_port,
                          int tcp_port);
    CGuildServerProxy* GetServerProxyArrayAt(int index);

    ENUM_SERVER_GROUP* server_groups_;              // +0x00000
    CGuildServerProxy* proxies_;                     // +0x00004
    int proxy_count_;                                // +0x00008
    uint8_t has_proxy_array_;                        // +0x0000C
    uint8_t reserved_0d_[3];                         // +0x0000D
    CGuildServerProxy default_proxy_;                // +0x00010
};

// IDA unit: 99_misc_global/Packet_Monitor_Call_Guild_All_Members. The exact
// packed 18-byte UDP payload carries the caller's character and guild keys.
#pragma pack(push, 1)
// IDA unit: 08_quest_skill_growth/Packet_Request_Guild_Create.  The guild
// endpoint consumes all 0x32 bytes, including the explicitly cleared address
// field at +0x25.
struct Packet_Request_Guild_Create : PacketHeader {
    uint32_t charac_no;
    char guild_name[23];
    char guild_address[13];

    // IDA_EA: 0x846FA60
    // IDA_NAME: _ZN27Packet_Request_Guild_CreateC2Ev
    // DEMANGLED: Packet_Request_Guild_Create::Packet_Request_Guild_Create(void)
    Packet_Request_Guild_Create();
};

struct Packet_Monitor_Call_Guild_All_Members : PacketHeader {
    uint32_t charac_no;
    uint32_t charac_guildkey;

    // IDA_EA: 0x846F3CE
    // IDA_NAME: _ZN37Packet_Monitor_Call_Guild_All_MembersC2Ev
    // DEMANGLED: Packet_Monitor_Call_Guild_All_Members::Packet_Monitor_Call_Guild_All_Members(void)
    Packet_Monitor_Call_Guild_All_Members();
};

// IDA unit: 99_misc_global/Packet_Monitor_Call_Guild_Members.
// The raw constructor initializes only charac_guildkey; the caller supplies
// charac_no before transmission.
struct Packet_Monitor_Call_Guild_Members : PacketHeader {
    uint32_t charac_no;
    uint32_t charac_guildkey;

    // IDA_EA: 0x846F3A0
    // IDA_NAME: _ZN33Packet_Monitor_Call_Guild_MembersC2Ev
    // DEMANGLED: Packet_Monitor_Call_Guild_Members::Packet_Monitor_Call_Guild_Members(void)
    Packet_Monitor_Call_Guild_Members();
};

// IDA unit: 99_misc_global/Packet_Monitor_Call_Guild_Info. The physical
// packet carries up to 95 guild keys, while the old sender transmits only
// the 19-byte prefix plus four bytes for every requested key.
struct Packet_Monitor_Call_Guild_Info : PacketHeader {
    uint32_t charac_no;
    uint32_t uid;
    uint8_t guild_count;
    uint32_t guild_keys[95];

    // IDA_EA: 0x846F472
    // IDA_NAME: _ZN30Packet_Monitor_Call_Guild_InfoC2Ev
    // DEMANGLED: Packet_Monitor_Call_Guild_Info::Packet_Monitor_Call_Guild_Info(void)
    Packet_Monitor_Call_Guild_Info();
};

// IDA unit: 99_misc_global/ST_Guild_War_Rank_Info. Ten contiguous records
// begin at Packet_Notice_Guild_War_Info +0x12.
struct ST_Guild_War_Rank_Info {
    uint32_t raw_00;
    uint32_t raw_04;
    uint16_t raw_08;
    char raw_0a[23];

    // IDA_EA: 0x822BA9A
    // IDA_NAME: _ZN22ST_Guild_War_Rank_InfoC2Ev
    // DEMANGLED: ST_Guild_War_Rank_Info::ST_Guild_War_Rank_Info(void)
    ST_Guild_War_Rank_Info();
};

// IDA unit: 99_misc_global/Packet_Notice_Guild_War_Info.
struct Packet_Notice_Guild_War_Info : PacketHeader {
    uint32_t uid;
    uint32_t charac_no;
    ST_Guild_War_Rank_Info ranks[10];

    // IDA_EA: 0x822BADC
    // IDA_NAME: _ZN28Packet_Notice_Guild_War_InfoC2Ev
    // DEMANGLED: Packet_Notice_Guild_War_Info::Packet_Notice_Guild_War_Info(void)
    Packet_Notice_Guild_War_Info();
};

// IDA units: 99_misc_global/Packet_Guild_Call_Guild_Invite and
// Packet_Guild_Reply_Guild_Invite_From_Invited. The old proxy stack allocates
// both packets locally and fills exactly the fields below before UDP send.
struct Packet_Guild_Call_Guild_Invite : PacketHeader {
    uint32_t charac_no;
    uint32_t charac_guildkey;
    char target_name[30];

    // IDA_EA: 0x846F8C0
    // IDA_NAME: _ZN30Packet_Guild_Call_Guild_InviteC2Ev
    // DEMANGLED: Packet_Guild_Call_Guild_Invite::Packet_Guild_Call_Guild_Invite(void)
    Packet_Guild_Call_Guild_Invite();
};

struct Packet_Guild_Reply_Guild_Invite_From_Invited : PacketHeader {
    uint32_t charac_no;
    uint8_t reply_code;

    // IDA_EA: 0x846F902
    // IDA_NAME: _ZN44Packet_Guild_Reply_Guild_Invite_From_InvitedC2Ev
    // DEMANGLED: Packet_Guild_Reply_Guild_Invite_From_Invited::Packet_Guild_Reply_Guild_Invite_From_Invited(void)
    Packet_Guild_Reply_Guild_Invite_From_Invited();
};

struct Packet_Web_Notify_Message_To_Guild : PacketHeader {
    uint32_t charac_guildkey;
    uint8_t message_kind;
    char message[101];

    // IDA_EA: 0x846F524
    // IDA_NAME: _ZN34Packet_Web_Notify_Message_To_GuildC2Ev
    // DEMANGLED: Packet_Web_Notify_Message_To_Guild::Packet_Web_Notify_Message_To_Guild(void)
    Packet_Web_Notify_Message_To_Guild();
};

struct Packet_Guild_Write_Guild_Member_Memo : PacketHeader {
    uint32_t charac_guildkey;
    int32_t charac_no;
    char memo[21];

    // IDA_EA: 0x846F4CE
    // IDA_NAME: _ZN36Packet_Guild_Write_Guild_Member_MemoC2Ev
    // DEMANGLED: Packet_Guild_Write_Guild_Member_Memo::Packet_Guild_Write_Guild_Member_Memo(void)
    Packet_Guild_Write_Guild_Member_Memo();
};

struct Packet_Guild_Request_Guild_Master_Delegate : PacketHeader {
    uint32_t charac_guildkey;
    uint32_t charac_no;
    char target_name[30];

    // IDA_EA: 0x846F968
    // IDA_NAME: _ZN42Packet_Guild_Request_Guild_Master_DelegateC2Ev
    // DEMANGLED: Packet_Guild_Request_Guild_Master_Delegate::Packet_Guild_Request_Guild_Master_Delegate(void)
    Packet_Guild_Request_Guild_Master_Delegate();
};

// IDA unit: 99_misc_global/Packet_Guild_Request_Guild_Secede.  The sender
// writes the two keys and length before copying at most the 30-byte text tail.
struct Packet_Guild_Request_Guild_Secede : PacketHeader {
    uint32_t charac_guildkey;
    uint32_t charac_no;
    int32_t text_length;
    char text[30];

    // IDA_EA: 0x846F926
    // IDA_NAME: _ZN33Packet_Guild_Request_Guild_SecedeC2Ev
    Packet_Guild_Request_Guild_Secede();
};

struct Packet_Monitor_Set_Sub_Guild_Master : PacketHeader {
    uint32_t charac_no;
    uint32_t charac_guildkey;
    char target_name[30];
    uint8_t request_flag;

    // IDA_EA: 0x822B978
    // IDA_NAME: _ZN35Packet_Monitor_Set_Sub_Guild_MasterC2Ev
    // DEMANGLED: Packet_Monitor_Set_Sub_Guild_Master::Packet_Monitor_Set_Sub_Guild_Master(void)
    Packet_Monitor_Set_Sub_Guild_Master();
};

struct Packet_Change_Guild_Name : PacketHeader {
    uint32_t charac_no;
    uint32_t charac_guildkey;
    char guild_name[23];

    // IDA_EA: 0x846F9AA
    // IDA_NAME: _ZN24Packet_Change_Guild_NameC2Ev
    // DEMANGLED: Packet_Change_Guild_Name::Packet_Change_Guild_Name(void)
    Packet_Change_Guild_Name();
};

// IDA unit: 99_misc_global/Packet_Guild_Power_War_Process_Info.  The sender
// writes the requesting character number at +0x0a after this exact 14-byte
// guild-service header.
struct Packet_Guild_Power_War_Process_Info : PacketHeader {
    uint32_t charac_no;

    // IDA_EA: 0x846F684
    // IDA_NAME: _ZN35Packet_Guild_Power_War_Process_InfoC2Ev
    // DEMANGLED: Packet_Guild_Power_War_Process_Info::Packet_Guild_Power_War_Process_Info(void)
    Packet_Guild_Power_War_Process_Info();
};

// IDA unit: 99_misc_global/Packet_GM_PowerWar_Start.  The original packet
// contains only the 10-byte guild-service header.
struct Packet_GM_PowerWar_Start : PacketHeader {
    // IDA_EA: 0x846F836
    // IDA_NAME: _ZN24Packet_GM_PowerWar_StartC2Ev
    // DEMANGLED: Packet_GM_PowerWar_Start::Packet_GM_PowerWar_Start(void)
    Packet_GM_PowerWar_Start();
};

// IDA unit: 99_misc_global/Packet_GM_PowerWar_End.  It has the same physical
// extent as the start packet but a distinct old-server packet type.
struct Packet_GM_PowerWar_End : PacketHeader {
    // IDA_EA: 0x846F85A
    // IDA_NAME: _ZN22Packet_GM_PowerWar_EndC2Ev
    // DEMANGLED: Packet_GM_PowerWar_End::Packet_GM_PowerWar_End(void)
    Packet_GM_PowerWar_End();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Call_Guild_All_Members) == 0x12u,
              "Guild all-members packet must retain the old 18-byte ABI");
static_assert(offsetof(Packet_Monitor_Call_Guild_All_Members, charac_no) ==
                  0x0Au,
              "Guild all-members character offset changed");
static_assert(offsetof(Packet_Monitor_Call_Guild_All_Members,
                       charac_guildkey) == 0x0Eu,
              "Guild all-members key offset changed");
static_assert(sizeof(Packet_Monitor_Call_Guild_Members) == 0x12u,
              "Guild members packet must retain the old 18-byte ABI");
static_assert(offsetof(Packet_Monitor_Call_Guild_Members, charac_no) ==
                  0x0Au &&
                  offsetof(Packet_Monitor_Call_Guild_Members,
                           charac_guildkey) == 0x0Eu,
              "Guild members packet offsets changed");
static_assert(sizeof(Packet_Monitor_Call_Guild_Info) == 0x18Fu,
              "Guild info packet must retain the old 399-byte ABI");
static_assert(offsetof(Packet_Monitor_Call_Guild_Info, charac_no) == 0x0Au &&
                  offsetof(Packet_Monitor_Call_Guild_Info, uid) == 0x0Eu &&
                  offsetof(Packet_Monitor_Call_Guild_Info, guild_count) ==
                      0x12u &&
                  offsetof(Packet_Monitor_Call_Guild_Info, guild_keys) ==
                      0x13u,
              "Guild info packet offsets changed");
static_assert(sizeof(ST_Guild_War_Rank_Info) == 0x21u,
              "Guild-war rank record must retain the old 33-byte ABI");
static_assert(sizeof(Packet_Notice_Guild_War_Info) == 0x15Cu,
              "Guild-war info packet must retain the old 348-byte ABI");
static_assert(offsetof(Packet_Notice_Guild_War_Info, uid) == 0x0Au &&
                  offsetof(Packet_Notice_Guild_War_Info, charac_no) ==
                      0x0Eu &&
                  offsetof(Packet_Notice_Guild_War_Info, ranks) == 0x12u,
              "Guild-war info packet offsets changed");
static_assert(sizeof(Packet_Guild_Call_Guild_Invite) == 0x30u,
              "Guild invite packet must retain the old 48-byte ABI");
static_assert(offsetof(Packet_Guild_Call_Guild_Invite, charac_no) == 0x0Au &&
                  offsetof(Packet_Guild_Call_Guild_Invite, charac_guildkey) ==
                      0x0Eu &&
                  offsetof(Packet_Guild_Call_Guild_Invite, target_name) ==
                      0x12u,
              "Guild invite packet field offsets changed");
static_assert(sizeof(Packet_Guild_Reply_Guild_Invite_From_Invited) == 0x0Fu,
              "Guild invite reply packet must retain the old 15-byte ABI");
static_assert(offsetof(Packet_Guild_Reply_Guild_Invite_From_Invited,
                       charac_no) == 0x0Au &&
                  offsetof(Packet_Guild_Reply_Guild_Invite_From_Invited,
                       reply_code) == 0x0Eu,
              "Guild invite reply packet field offsets changed");
static_assert(sizeof(Packet_Web_Notify_Message_To_Guild) == 0x74u,
              "Guild message packet must retain the old 116-byte ABI");
static_assert(offsetof(Packet_Web_Notify_Message_To_Guild, charac_guildkey) ==
                  0x0Au &&
                  offsetof(Packet_Web_Notify_Message_To_Guild, message_kind) ==
                      0x0Eu &&
                  offsetof(Packet_Web_Notify_Message_To_Guild, message) ==
                      0x0Fu,
              "Guild message packet field offsets changed");
static_assert(sizeof(Packet_Guild_Write_Guild_Member_Memo) == 0x27u,
              "Guild memo packet must retain the old 39-byte ABI");
static_assert(offsetof(Packet_Guild_Write_Guild_Member_Memo,
                       charac_guildkey) == 0x0Au &&
                  offsetof(Packet_Guild_Write_Guild_Member_Memo, charac_no) ==
                      0x0Eu &&
                  offsetof(Packet_Guild_Write_Guild_Member_Memo, memo) ==
                      0x12u,
              "Guild memo packet field offsets changed");
static_assert(sizeof(Packet_Guild_Request_Guild_Master_Delegate) == 0x30u,
              "Guild delegate packet must retain the old 48-byte ABI");
static_assert(offsetof(Packet_Guild_Request_Guild_Master_Delegate,
                       charac_guildkey) == 0x0Au &&
                  offsetof(Packet_Guild_Request_Guild_Master_Delegate,
                           charac_no) == 0x0Eu &&
                  offsetof(Packet_Guild_Request_Guild_Master_Delegate,
                           target_name) == 0x12u,
                  "Guild delegate packet field offsets changed");
static_assert(sizeof(Packet_Guild_Request_Guild_Secede) == 0x34u,
              "Guild secede packet must retain the old 52-byte ABI");
static_assert(offsetof(Packet_Guild_Request_Guild_Secede, charac_guildkey) ==
                  0x0Au &&
                  offsetof(Packet_Guild_Request_Guild_Secede, charac_no) ==
                      0x0Eu &&
                  offsetof(Packet_Guild_Request_Guild_Secede, text_length) ==
                      0x12u &&
                  offsetof(Packet_Guild_Request_Guild_Secede, text) == 0x16u,
              "Guild secede packet field offsets changed");
static_assert(sizeof(Packet_Guild_Request_Guild_Board_Write) == 0x90u,
              "Guild board write packet must retain the old 0x90-byte ABI");
static_assert(sizeof(Packet_Guild_Request_Guild_Board_Open) == 0x13u &&
                  sizeof(Packet_Guild_Request_Guild_Board_Delete) == 0x16u,
              "Guild board open/delete packet extent changed");
static_assert(offsetof(Packet_Guild_Request_Guild_Board_Write, charac_no) ==
                  0x12u &&
                  offsetof(Packet_Guild_Request_Guild_Board_Write, message) ==
                      0x18u,
              "Guild board write request field offsets changed");
static_assert(sizeof(Packet_Monitor_Set_Sub_Guild_Master) == 0x31u,
              "Sub-guild-master packet must retain the old 49-byte ABI");
static_assert(offsetof(Packet_Monitor_Set_Sub_Guild_Master, charac_no) ==
                  0x0Au &&
                  offsetof(Packet_Monitor_Set_Sub_Guild_Master,
                           charac_guildkey) == 0x0Eu &&
                  offsetof(Packet_Monitor_Set_Sub_Guild_Master, target_name) ==
                      0x12u &&
                  offsetof(Packet_Monitor_Set_Sub_Guild_Master, request_flag) ==
                      0x30u,
              "Sub-guild-master packet field offsets changed");
static_assert(sizeof(Packet_Change_Guild_Name) == 0x29u,
              "Guild-name packet must retain the old 41-byte ABI");
static_assert(offsetof(Packet_Change_Guild_Name, charac_no) == 0x0Au &&
                  offsetof(Packet_Change_Guild_Name, charac_guildkey) ==
                      0x0Eu &&
                  offsetof(Packet_Change_Guild_Name, guild_name) == 0x12u,
              "Guild-name packet field offsets changed");
static_assert(sizeof(Packet_Guild_Power_War_Process_Info) == 0x0Eu,
              "Power-war process packet must retain the old 14-byte ABI");
static_assert(offsetof(Packet_Guild_Power_War_Process_Info, charac_no) ==
                  0x0Au,
              "Power-war process character offset changed");
static_assert(sizeof(Packet_GM_PowerWar_Start) == 0x0Au,
              "Power-war start packet must retain the old 10-byte ABI");
static_assert(sizeof(Packet_GM_PowerWar_End) == 0x0Au,
              "Power-war end packet must retain the old 10-byte ABI");

#if defined(__i386__)
static_assert(sizeof(STGuildProxy) == 0x1au,
              "STGuildProxy must retain the old 26-byte ABI");
static_assert(offsetof(STGuildProxy, guild_name) == 0x00u &&
                  offsetof(STGuildProxy, guild_level) == 0x17u &&
                  offsetof(STGuildProxy, power_side) == 0x18u &&
                  offsetof(STGuildProxy, guild_agit_flag) == 0x19u,
              "STGuildProxy named guild fields changed");
static_assert(sizeof(Packet_Monitor_Call_Guild_Level_Up) == 0x1bu,
              "guild-levelup packet must retain the old 27-byte ABI");
static_assert(offsetof(Packet_Monitor_Call_Guild_Level_Up, uid) == 0x0au &&
                  offsetof(Packet_Monitor_Call_Guild_Level_Up, charac_no) == 0x0eu &&
                  offsetof(Packet_Monitor_Call_Guild_Level_Up, charac_guildkey) == 0x12u &&
                  offsetof(Packet_Monitor_Call_Guild_Level_Up, levelup_price) == 0x17u,
              "guild-levelup packet offsets changed");
static_assert(sizeof(CNetwork100000_100000) == 200184u,
              "Guild proxy TCP endpoint must retain the old 0x30E18 extent");
static_assert(sizeof(CGuildServerProxy) == 200228u,
              "Guild proxy must retain the old i386 extent");
static_assert(sizeof(CServerProxyMgr<CGuildServerProxy>) == 0x30E34u,
              "Guild proxy manager must retain the old i386 extent");
static_assert(offsetof(CServerProxyMgr<CGuildServerProxy>, default_proxy_) ==
                  0x10u,
              "Guild proxy manager default proxy offset changed");
#endif

#endif  // OLDSRV_GLOBAL_GUILD_SERVER_PROXY_ABI_H_
