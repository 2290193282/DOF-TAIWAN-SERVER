#include "infra/network/packet/udp_packets.h"

#include <string.h>


// IDA_EA: 0x82A3F52
// IDA_NAME: _ZN33Packet_Monitor_UDP_Reply_UserInfoC2Ev
// DEMANGLED: Packet_Monitor_UDP_Reply_UserInfo::Packet_Monitor_UDP_Reply_UserInfo(void)
Packet_Monitor_UDP_Reply_UserInfo::Packet_Monitor_UDP_Reply_UserInfo()
    : PacketHeader(0x03EAu, 0x16E6u) {
    result = 0;
    channel_no = 0xFF;
    memset(users, 0, sizeof(users));
}
