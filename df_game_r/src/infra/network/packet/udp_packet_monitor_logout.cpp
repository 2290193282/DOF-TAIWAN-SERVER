#include "infra/network/packet/udp_packets.h"

#include <string.h>


// IDA_EA: 0x846F2D4
// IDA_NAME: _ZN25Packet_Monitor_UDP_LogoutC2Ev
// DEMANGLED: Packet_Monitor_UDP_Logout::Packet_Monitor_UDP_Logout(void)
Packet_Monitor_UDP_Logout::Packet_Monitor_UDP_Logout()
    : PacketHeader(0x03E9u, 0x003Bu) {
    account_id = 0;
    channel_no = 0xFF;
    charac_no = 0;
    reason = 0;
    state = 0;
    play_time = 0;
    memset(tail, 0, sizeof(tail));
}
