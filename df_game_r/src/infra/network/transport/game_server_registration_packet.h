#ifndef OLDSRV_GLOBAL_GAME_SERVER_REGISTRATION_PACKET_H_
#define OLDSRV_GLOBAL_GAME_SERVER_REGISTRATION_PACKET_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

// IDA unit: 99_misc_global/Packet_Game_Server_Regist.  Both monitor and
// guild proxies use this exact TCP registration frame.
#pragma pack(push, 1)
struct Packet_Game_Server_Regist : public PacketHeader {
    uint8_t registration_kind;  // +0x0a, always 1 for game-server register
    uint8_t server_group;       // +0x0b
    uint8_t channel_no;         // +0x0c
    char server_ip[16];         // +0x0d
    uint16_t udp_port;          // +0x1d

    Packet_Game_Server_Regist();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Game_Server_Regist) == 0x1fu,
              "Packet_Game_Server_Regist must retain the 31-byte wire ABI");
static_assert(offsetof(Packet_Game_Server_Regist, registration_kind) == 0x0au &&
                  offsetof(Packet_Game_Server_Regist, server_group) == 0x0bu &&
                  offsetof(Packet_Game_Server_Regist, channel_no) == 0x0cu &&
                  offsetof(Packet_Game_Server_Regist, server_ip) == 0x0du &&
                  offsetof(Packet_Game_Server_Regist, udp_port) == 0x1du,
              "Packet_Game_Server_Regist field offsets changed");

#endif  // OLDSRV_GLOBAL_GAME_SERVER_REGISTRATION_PACKET_H_
