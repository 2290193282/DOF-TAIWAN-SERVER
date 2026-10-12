#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x846F34C
// IDA_NAME: _ZN28Packet_Monitor_UDP_HeartBeatC2Ev
// DEMANGLED: Packet_Monitor_UDP_HeartBeat::Packet_Monitor_UDP_HeartBeat(void)
Packet_Monitor_UDP_HeartBeat::Packet_Monitor_UDP_HeartBeat()
    : PacketHeader(0x03ECu, 0x000Bu), state(0xFF) {}
