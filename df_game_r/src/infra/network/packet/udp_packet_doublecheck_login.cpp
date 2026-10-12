#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x846D58A
// IDA_NAME: _ZN28Packet_DoubleCheck_UDP_LoginC2Ev
// DEMANGLED: Packet_DoubleCheck_UDP_Login::Packet_DoubleCheck_UDP_Login(void)
Packet_DoubleCheck_UDP_Login::Packet_DoubleCheck_UDP_Login()
    : PacketHeader(0x0BB8u, 0x0010u),
      account_id(0),
      channel_no(0xFF),
      server_group(0xFF) {}
