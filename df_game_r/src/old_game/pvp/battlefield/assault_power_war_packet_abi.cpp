#include "old_game/pvp/battlefield/assault_power_war_packet_abi.h"

#include <cstring>

// IDA_EA: 0x82F0780
// IDA_NAME: _ZN20Packet_Broadcast_MsgC2Ev
// DEMANGLED: Packet_Broadcast_Msg::Packet_Broadcast_Msg(void)
Packet_Broadcast_Msg::Packet_Broadcast_Msg()
    : PacketHeader(0x2711u, 0x010eu) {}

// IDA_EA: 0x82F070C
// IDA_NAME: _ZN35Packet_Guild_Change_Power_War_PointC2Ev
// DEMANGLED: Packet_Guild_Change_Power_War_Point::Packet_Guild_Change_Power_War_Point(void)
Packet_Guild_Change_Power_War_Point::Packet_Guild_Change_Power_War_Point()
    : PacketHeader(0x06a4u, 0x002du),
      power_side(0),
      winner_point(0),
      loser_point(0) {
    std::memset(winner_charac_nos, 0, sizeof(winner_charac_nos));
    std::memset(loser_charac_nos, 0, sizeof(loser_charac_nos));
}
