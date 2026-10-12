#ifndef OLDSRV_PROTOCOL_UDP_PACKETS_H_
#define OLDSRV_PROTOCOL_UDP_PACKETS_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// UDP login request sent to double-check server.
struct Packet_DoubleCheck_UDP_Login : public PacketHeader {
    uint32_t account_id;
    uint8_t channel_no;
    uint8_t server_group;

    Packet_DoubleCheck_UDP_Login();
};

// UDP logout request sent to double-check server.
struct Packet_DoubleCheck_UDP_Logout : public PacketHeader {
    uint32_t account_id;
    uint8_t channel_no;
    uint8_t server_group;

    Packet_DoubleCheck_UDP_Logout();
};

// Double-check server heartbeat.
struct Packet_Double_Check_Server_UDP_HeartBeat : public PacketHeader {
    uint8_t state;
    uint8_t reserved_tail;

    Packet_Double_Check_Server_UDP_HeartBeat();
};

// Hades UDP heartbeat packet.
struct Packet_Hades_Udp_HeartBeat : public PacketHeader {
    uint16_t server_id;
    uint16_t state;

    Packet_Hades_Udp_HeartBeat();
};

// Monitor UDP login packet, size 0x33 in old df_game_r.
struct Packet_Monitor_UDP_Login : public PacketHeader {
    // The login notifier writes these in this exact order.  They are the
    // CUser UID and account ID, not a character-number pair.
    uint32_t uid;
    uint32_t account_id;
    uint8_t channel_no;
    uint8_t peer_ip[16];
    uint8_t sex;
    uint8_t ssn[8];
    uint8_t tail[7];

    Packet_Monitor_UDP_Login();
};

static_assert(sizeof(Packet_Monitor_UDP_Login) == 0x33u,
              "monitor login packet must retain its old 0x33-byte ABI");

// IDA_EA: CDispatchServerPacket::dispatchPacket @0x847429F.  The monitor's
// reply reuses opcode 0x03e8 and its 0x33-byte frame length, but its body is
// not the outgoing Packet_Monitor_UDP_Login body.  It is queued unchanged to
// Inter_MonitorLoginOK, which consumes member_id at +0x0e, error_code at
// +0x23, and the last-login pair at +0x24/+0x28.
struct Packet_Monitor_LoginReply : public PacketHeader {
    int32_t uid;                      // +0x0a
    uint32_t member_id;               // +0x0e
    uint8_t reserved_12_22[0x11];     // +0x12
    uint8_t error_code;               // +0x23
    int32_t charac_no;                // +0x24
    int32_t channel_no;               // +0x28
    uint8_t reserved_2c_32[0x07];     // +0x2c
};

static_assert(sizeof(Packet_Monitor_LoginReply) == 0x33u &&
                  offsetof(Packet_Monitor_LoginReply, uid) == 0x0au &&
                  offsetof(Packet_Monitor_LoginReply, member_id) == 0x0eu &&
                  offsetof(Packet_Monitor_LoginReply, error_code) == 0x23u &&
                  offsetof(Packet_Monitor_LoginReply, charac_no) == 0x24u &&
                  offsetof(Packet_Monitor_LoginReply, channel_no) == 0x28u,
              "monitor login reply must retain the original 0x33-byte ABI");

// Monitor UDP logout packet, size 0x3b in old df_game_r.
struct Packet_Monitor_UDP_Logout : public PacketHeader {
    uint32_t account_id;
    uint8_t channel_no;
    uint32_t charac_no;
    uint32_t reason;
    uint8_t state;
    uint32_t play_time;
    uint8_t tail[30];
    uint8_t trailing_reserved;

    Packet_Monitor_UDP_Logout();
};

// Monitor UDP logout completion packet.
struct Packet_Monitor_UDP_Logout_Complete : public PacketHeader {
    // IDA_EA: Inter_UpdateLogout::dispatch_sig @0x84CE548 writes these
    // immediately after constructing the 10-byte PacketHeader: the DB reply
    // m_id at +0x0A and the local channel number at +0x0E.
    uint32_t account_id;
    uint8_t channel_no;

    Packet_Monitor_UDP_Logout_Complete();
};

static_assert(sizeof(Packet_Monitor_UDP_Logout_Complete) == 0x0Fu,
              "monitor logout-complete packet must retain its old 15-byte ABI");

// One complete record in Packet_Monitor_UDP_Reply_UserInfo.  IDA_EA:
// CGameManager::Send_userinfos_to_upper_server @0x82961BE writes all fields
// below in this exact order before it forwards the packet to the guild or
// monitor proxy.  The final byte is cleared by the packet C2/helper after
// every send; no consumer assigns it another protocol meaning.
struct MonitorUpperServerUserInfo {
    uint32_t account_id;          // +0x00
    uint32_t charac_no;           // +0x04
    uint32_t user_uid;            // +0x08
    uint32_t charac_guildkey;     // +0x0c
    uint8_t job;                  // +0x10
    uint8_t grow_type;            // +0x11
    uint16_t level;               // +0x12
    char charac_name[30];         // +0x14
    uint32_t charac_memberkey;    // +0x32
    char peer_ip[16];             // +0x36
    uint8_t sex;                  // +0x46
    char ssn[6];                  // +0x47
    uint8_t legacy_c2_zero;       // +0x4d
};

static_assert(sizeof(MonitorUpperServerUserInfo) == 0x4eu,
              "upper-server user-info record must retain the old 78-byte ABI");
static_assert(offsetof(MonitorUpperServerUserInfo, charac_name) == 0x14u &&
                  offsetof(MonitorUpperServerUserInfo, charac_memberkey) == 0x32u &&
                  offsetof(MonitorUpperServerUserInfo, peer_ip) == 0x36u &&
                  offsetof(MonitorUpperServerUserInfo, sex) == 0x46u &&
                  offsetof(MonitorUpperServerUserInfo, ssn) == 0x47u,
              "upper-server user-info record offsets must match IDA");

// Monitor UDP reply user info packet, size 0x16e6.
struct Packet_Monitor_UDP_Reply_UserInfo : public PacketHeader {
    uint8_t result;
    uint8_t channel_no;
    MonitorUpperServerUserInfo users[75];

    Packet_Monitor_UDP_Reply_UserInfo();
};

static_assert(sizeof(Packet_Monitor_UDP_Reply_UserInfo) == 0x16e6u,
              "monitor user-info packet must retain the old 5862-byte ABI");

// Monitor UDP heartbeat packet.
struct Packet_Monitor_UDP_HeartBeat : public PacketHeader {
    uint8_t state;

    Packet_Monitor_UDP_HeartBeat();
};

// Monitor notification emitted when a character reaches the maximum level.
// IDA constructs only the 10-byte server header and clears the remaining
// 0x1e-byte payload; fields are filled by the level-up path afterwards.
struct Packet_Monitor_Max_Level_BroadCast : public PacketHeader {
    uint8_t payload[0x1E];

    // IDA_EA: 0x8694560
    // IDA_NAME: _ZN34Packet_Monitor_Max_Level_BroadCastC2Ev
    // DEMANGLED: Packet_Monitor_Max_Level_BroadCast::Packet_Monitor_Max_Level_BroadCast(void)
    Packet_Monitor_Max_Level_BroadCast();
};

static_assert(sizeof(Packet_Monitor_Max_Level_BroadCast) == 0x28u,
              "Packet_Monitor_Max_Level_BroadCast IDA ABI size must stay 0x28");

// Cut-off UDP user info reply, size 0xafe.
struct Packet_CutOff_UDP_Reply_UserInfo : public PacketHeader {
    uint16_t result;
    uint8_t channel_no;
    uint8_t state;
    uint8_t user_info_blob[0xAF0];

    Packet_CutOff_UDP_Reply_UserInfo();
};

#pragma pack(pop)


#endif  // OLDSRV_PROTOCOL_UDP_PACKETS_H_
