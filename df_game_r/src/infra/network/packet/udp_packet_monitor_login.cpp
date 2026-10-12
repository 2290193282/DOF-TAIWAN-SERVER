#include "infra/network/packet/udp_packets.h"

#include <string.h>


// IDA_EA: 0x846F24C
// IDA_NAME: _ZN24Packet_Monitor_UDP_LoginC2Ev
// DEMANGLED: Packet_Monitor_UDP_Login::Packet_Monitor_UDP_Login(void)
Packet_Monitor_UDP_Login::Packet_Monitor_UDP_Login()
    : PacketHeader(0x03E8u, 0x0033u) {
    uid = 0;
    account_id = 0;
    channel_no = 0xFF;
    sex = 0xFF;
    memset(tail, 0, sizeof(tail));
}
