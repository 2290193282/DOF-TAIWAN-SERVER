#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x84E933C
// IDA_NAME: _ZN34Packet_Monitor_UDP_Logout_CompleteC2Ev
// DEMANGLED: Packet_Monitor_UDP_Logout_Complete::Packet_Monitor_UDP_Logout_Complete(void)
Packet_Monitor_UDP_Logout_Complete::Packet_Monitor_UDP_Logout_Complete()
    : PacketHeader(0x03F4u, 0x000Fu), account_id(0), channel_no(0) {}
