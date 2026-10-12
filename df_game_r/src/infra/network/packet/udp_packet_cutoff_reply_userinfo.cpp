#include "infra/network/packet/udp_packets.h"

#include <string.h>


// IDA_EA: 0x82A3FA2
// IDA_NAME: _ZN32Packet_CutOff_UDP_Reply_UserInfoC2Ev
// DEMANGLED: Packet_CutOff_UDP_Reply_UserInfo::Packet_CutOff_UDP_Reply_UserInfo(void)
Packet_CutOff_UDP_Reply_UserInfo::Packet_CutOff_UDP_Reply_UserInfo()
    : PacketHeader(0x03F9u, 0x0AFEu) {
    result = 0;
    channel_no = 0xFF;
    state = 0;
    memset(user_info_blob, 0, sizeof(user_info_blob));
}
