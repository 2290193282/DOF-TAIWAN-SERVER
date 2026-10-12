#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x846D5C6
// IDA_NAME: _ZN29Packet_DoubleCheck_UDP_LogoutC2Ev
// DEMANGLED: Packet_DoubleCheck_UDP_Logout::Packet_DoubleCheck_UDP_Logout(void)
Packet_DoubleCheck_UDP_Logout::Packet_DoubleCheck_UDP_Logout()
    : PacketHeader(0x0BB9u, 0x0010u),
      account_id(0),
      channel_no(0xFF),
      server_group(0xFF) {}
