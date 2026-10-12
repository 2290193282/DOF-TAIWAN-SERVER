#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_SERVER_MATCH_DATA_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_SERVER_MATCH_DATA_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"


// IDA unit: 09_dungeon_pvp_party/Packet_Server_Match_data.
// The integrated-PvP reward path uses +0x0b/+0x0c as win/lose flags, while
// the leave-penalty path uses +0x0c as its leave flag.  They are the same
// original 13-byte statistic-server packet.
#pragma pack(push, 1)
struct Packet_Server_Match_data : PacketHeader {
    Packet_Server_Match_data();

    uint8_t server_group;        // +0x0a
    uint8_t win_flag;            // +0x0b
    uint8_t lose_or_leave_flag;  // +0x0c
};
#pragma pack(pop)

static_assert(sizeof(Packet_Server_Match_data) == 13u,
              "server-match packet must stay 13 bytes");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_SERVER_MATCH_DATA_H_
