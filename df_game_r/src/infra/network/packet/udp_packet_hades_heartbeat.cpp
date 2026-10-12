#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x84708E4
// IDA_NAME: _ZN26Packet_Hades_Udp_HeartBeatC2Ev
// DEMANGLED: Packet_Hades_Udp_HeartBeat::Packet_Hades_Udp_HeartBeat(void)
Packet_Hades_Udp_HeartBeat::Packet_Hades_Udp_HeartBeat()
    : PacketHeader(0x0FD8u, 0x000Eu), server_id(0), state(0) {}
