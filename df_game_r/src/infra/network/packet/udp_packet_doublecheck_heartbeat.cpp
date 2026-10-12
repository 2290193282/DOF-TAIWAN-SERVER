#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x846D602
// IDA_NAME: _ZN40Packet_Double_Check_Server_UDP_HeartBeatC2Ev
// DEMANGLED: Packet_Double_Check_Server_UDP_HeartBeat::Packet_Double_Check_Server_UDP_HeartBeat(void)
Packet_Double_Check_Server_UDP_HeartBeat::
    Packet_Double_Check_Server_UDP_HeartBeat()
    : PacketHeader(0x0BBAu, 0x000Cu), state(0xFF), reserved_tail(0) {}
