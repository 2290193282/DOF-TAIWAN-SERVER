#ifndef OLDSRV_OLD_DATA_LOGIN_PACKETS_HADES_LOGIN_PACKET_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKETS_HADES_LOGIN_PACKET_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

#pragma pack(push, 1)

// IDA unit: 02_login_account/Packet_Hades_Login.  The member layout is
// confirmed by XNuclear::CHades::Send_Login @0x84B9200.
struct Packet_Hades_Login {
    Packet_Hades_Login();

    PacketHeader header;
    int32_t acc_id;
    char ip[16];
    char mac_addr[65];
    uint8_t channel_no;
    int32_t uid;
};

#pragma pack(pop)

static_assert(sizeof(Packet_Hades_Login) == 0x64u,
              "Packet_Hades_Login IDA ABI size must stay 0x64");

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKETS_HADES_LOGIN_PACKET_H_
