#ifndef OLDSRV_LOGIN_LOGIN_PACKETS_H_
#define OLDSRV_LOGIN_LOGIN_PACKETS_H_

#include "infra/network/packet/PacketHeader.h"
#include "old_data/Login/packets/hades_login_packet.h"


#pragma pack(push, 1)

struct Packet_Community_Login {
    Packet_Community_Login();

    PacketHeader header;
    unsigned char payload[0x5A2 - sizeof(PacketHeader)];
};

#pragma pack(pop)

#endif  // OLDSRV_LOGIN_LOGIN_PACKETS_H_
