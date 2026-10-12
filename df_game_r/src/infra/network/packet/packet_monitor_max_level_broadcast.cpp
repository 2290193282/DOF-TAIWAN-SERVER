#include "infra/network/packet/udp_packets.h"

#include <string.h>

// IDA_EA: 0x8694560
// IDA_NAME: _ZN34Packet_Monitor_Max_Level_BroadCastC2Ev
// DEMANGLED: Packet_Monitor_Max_Level_BroadCast::Packet_Monitor_Max_Level_BroadCast(void)
Packet_Monitor_Max_Level_BroadCast::Packet_Monitor_Max_Level_BroadCast()
    : PacketHeader(0x177Eu, 0x0028u) {
    memset(payload, 0, sizeof(payload));
}
