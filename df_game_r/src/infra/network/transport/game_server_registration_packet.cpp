#include "infra/network/transport/game_server_registration_packet.h"

#include <cstring>

// IDA_EA: 0x846FAC0
// IDA_NAME: _ZN25Packet_Game_Server_RegistC2Ev
// DEMANGLED: Packet_Game_Server_Regist::Packet_Game_Server_Regist(void)
Packet_Game_Server_Regist::Packet_Game_Server_Regist()
    : PacketHeader(0x1f41u, 0x1fu), registration_kind(0), server_group(0),
      channel_no(0), udp_port(0) {
    std::memset(server_ip, 0, sizeof(server_ip));
}
