#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_PACKETS_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_PACKETS_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// IDA unit: 04_character_select_create/Pakcet_FindUserName_use_Charac_no.
struct Pakcet_FindUserName_use_Charac_no : public PacketHeader {
    Pakcet_FindUserName_use_Charac_no();

    uint32_t requester_uid;
    uint32_t charac_no;
    uint32_t charac_name_length;
    char charac_name[30];
};

// IDA unit: 04_character_select_create/Packet_Monitor_Request_Charac_Info.
struct Packet_Monitor_Request_Charac_Info : public PacketHeader {
    Packet_Monitor_Request_Charac_Info();

    char target_charac_name[21];
    uint32_t requester_charac_no;
};

// IDA unit: 04_character_select_create/Packet_Monitor_Notice_Charac_Live_On_Ten_Min.
struct Packet_Monitor_Notice_Charac_Live_On_Ten_Min : public PacketHeader {
    Packet_Monitor_Notice_Charac_Live_On_Ten_Min();

    uint32_t charac_no;  // +0x0a
};

// IDA unit: 04_character_select_create/Packet_Monitor_Charac_Delete.
struct Packet_Monitor_Charac_Delete : public PacketHeader {
    Packet_Monitor_Charac_Delete();

    uint32_t account_id;  // +0x0a
    uint32_t charac_no;   // +0x0e
    uint32_t guild_id;    // +0x12
};

// IDA unit: 04_character_select_create/Packet_Dungeon_Statistic_Party_Charac.
struct Packet_Dungeon_Statistic_Party_Charac : public PacketHeader {
    Packet_Dungeon_Statistic_Party_Charac();

    uint8_t raw_0a[0x3d];
};

// IDA unit: 04_character_select_create/Packet_Request_Charac_Tower_Update_Ranking.
struct Packet_Request_Charac_Tower_Update_Ranking : public PacketHeader {
    Packet_Request_Charac_Tower_Update_Ranking();

    uint8_t raw_0a[0x10];
};

#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Charac_Delete) == 0x16u,
              "monitor character-delete packet size changed");
static_assert(sizeof(Pakcet_FindUserName_use_Charac_no) == 0x34u,
              "find-character-name packet size changed");
static_assert(offsetof(Pakcet_FindUserName_use_Charac_no, requester_uid) ==
                      0x0au &&
                  offsetof(Pakcet_FindUserName_use_Charac_no, charac_no) ==
                      0x0eu &&
                  offsetof(Pakcet_FindUserName_use_Charac_no,
                           charac_name_length) == 0x12u &&
                  offsetof(Pakcet_FindUserName_use_Charac_no, charac_name) ==
                      0x16u,
              "find-character-name packet fields moved");
static_assert(sizeof(Packet_Monitor_Request_Charac_Info) == 0x23u,
              "monitor character-info request size changed");
static_assert(offsetof(Packet_Monitor_Request_Charac_Info,
                       target_charac_name) == 0x0au &&
                  offsetof(Packet_Monitor_Request_Charac_Info,
                           requester_charac_no) == 0x1fu,
              "monitor character-info request fields moved");
static_assert(offsetof(Packet_Monitor_Charac_Delete, account_id) == 0x0au &&
                  offsetof(Packet_Monitor_Charac_Delete, charac_no) == 0x0eu &&
                  offsetof(Packet_Monitor_Charac_Delete, guild_id) == 0x12u,
              "monitor character-delete fields moved");


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_PACKETS_H_
