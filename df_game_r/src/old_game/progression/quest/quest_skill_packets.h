#ifndef OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_PACKETS_H_
#define OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_PACKETS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x846F75E in the companion source.


#include <stdint.h>

#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// IDA unit: 08_quest_skill_growth/Packet_Request_Power_War_Start_Info.
struct Packet_Request_Power_War_Start_Info : public PacketHeader {
    Packet_Request_Power_War_Start_Info();

    uint32_t dword_0a;
};

// IDA unit: 08_quest_skill_growth/Packet_Request_Join_Power.
struct Packet_Request_Join_Power : public PacketHeader {
    Packet_Request_Join_Power();

    uint8_t raw_0a[0x09];
};

// IDA unit: 08_quest_skill_growth/Packet_Request_Secede_Power.
struct Packet_Request_Secede_Power : public PacketHeader {
    Packet_Request_Secede_Power();

    uint8_t raw_0a[0x09];
};

// IDA unit: 08_quest_skill_growth/Packet_Request_PvP_Buddy_Conn_List.
struct Packet_Request_PvP_Buddy_Conn_List : public PacketHeader {
    Packet_Request_PvP_Buddy_Conn_List();

    uint32_t dword_0a;
    uint32_t dword_0e;
};

// IDA unit: 08_quest_skill_growth/Packet_Request_Add_PvP_Buddy.
struct Packet_Request_Add_PvP_Buddy : public PacketHeader {
    Packet_Request_Add_PvP_Buddy();

    uint32_t acc_id;
    uint32_t charac_no;
    char request_kind;
    char target_name[30];
};

// IDA unit: 08_quest_skill_growth/Packet_Request_Remove_PvP_Buddy.
struct Packet_Request_Remove_PvP_Buddy : public PacketHeader {
    Packet_Request_Remove_PvP_Buddy();

    uint32_t dword_0a;
    uint32_t dword_0e;
    uint8_t byte_12;
    uint8_t raw_13[0x1e];
};

#pragma pack(pop)


#endif  // OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_PACKETS_H_
