#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_CHARACTER_RETURN_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_CHARACTER_RETURN_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/server_dispatch_packets.h"


// Complete 0627 server-to-server replies consumed by the character Inter
// dispatchers.  These are transport records, so every field is packed behind
// the original ten-byte PacketHeader and is addressed by its protocol name.
#pragma pack(push, 1)

struct Packet_Today_Guild_Member_Reply {
    PacketHeader header;
    uint32_t routing_uid;
    uint32_t requester_charac_no;
    uint32_t guild_key;
    uint32_t member_charac_no;
    char member_charac_name[30];
    uint8_t job;
    uint8_t grow_type;
    uint8_t level;
    uint8_t guild_member_grade;
    uint8_t sex;
};

struct Packet_Approve_Join_Guild_Reply {
    PacketHeader header;
    uint32_t result_code;
    uint32_t approver_charac_no;
    uint32_t guild_key;
    uint32_t applicant_charac_no;
};

struct Packet_Monitor_Reply_Charac_Info {
    PacketHeader header;
    uint32_t result_code;
    uint32_t requester_charac_no;
    uint32_t target_user_uid;
    uint8_t channel_no;
    char target_charac_name[30];
    uint16_t level;
    uint8_t job;
    uint8_t grow_type;
};

#pragma pack(pop)

static_assert(sizeof(Packet_Today_Guild_Member_Reply) == 0x3du,
              "today-guild-member reply must retain its 0627 extent");
static_assert(offsetof(Packet_Today_Guild_Member_Reply, routing_uid) == 0x0au &&
                  offsetof(Packet_Today_Guild_Member_Reply,
                           requester_charac_no) == 0x0eu &&
                  offsetof(Packet_Today_Guild_Member_Reply, guild_key) ==
                      0x12u &&
                  offsetof(Packet_Today_Guild_Member_Reply,
                           member_charac_no) == 0x16u &&
                  offsetof(Packet_Today_Guild_Member_Reply,
                           member_charac_name) == 0x1au &&
                  offsetof(Packet_Today_Guild_Member_Reply, job) == 0x38u &&
                  offsetof(Packet_Today_Guild_Member_Reply, grow_type) ==
                      0x39u &&
                  offsetof(Packet_Today_Guild_Member_Reply, level) == 0x3au &&
                  offsetof(Packet_Today_Guild_Member_Reply,
                           guild_member_grade) == 0x3bu &&
                  offsetof(Packet_Today_Guild_Member_Reply, sex) == 0x3cu,
              "today-guild-member reply fields moved");

static_assert(sizeof(Packet_Approve_Join_Guild_Reply) == 0x1au,
              "guild-approval reply must retain its 0627 extent");
static_assert(offsetof(Packet_Approve_Join_Guild_Reply, result_code) == 0x0au &&
                  offsetof(Packet_Approve_Join_Guild_Reply,
                           approver_charac_no) == 0x0eu &&
                  offsetof(Packet_Approve_Join_Guild_Reply, guild_key) ==
                      0x12u &&
                  offsetof(Packet_Approve_Join_Guild_Reply,
                           applicant_charac_no) == 0x16u,
              "guild-approval reply fields moved");

static_assert(sizeof(Packet_Monitor_Reply_Charac_Info) == 0x39u,
              "monitor character-info reply must retain its 0627 extent");
static_assert(offsetof(Packet_Monitor_Reply_Charac_Info, result_code) ==
                      0x0au &&
                  offsetof(Packet_Monitor_Reply_Charac_Info,
                           requester_charac_no) == 0x0eu &&
                  offsetof(Packet_Monitor_Reply_Charac_Info,
                           target_user_uid) == 0x12u &&
                  offsetof(Packet_Monitor_Reply_Charac_Info, channel_no) ==
                      0x16u &&
                  offsetof(Packet_Monitor_Reply_Charac_Info,
                           target_charac_name) == 0x17u &&
                  offsetof(Packet_Monitor_Reply_Charac_Info, level) == 0x35u &&
                  offsetof(Packet_Monitor_Reply_Charac_Info, job) == 0x37u &&
                  offsetof(Packet_Monitor_Reply_Charac_Info, grow_type) ==
                      0x38u,
              "monitor character-info reply fields moved");

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_CHARACTER_RETURN_PAYLOADS_H_
