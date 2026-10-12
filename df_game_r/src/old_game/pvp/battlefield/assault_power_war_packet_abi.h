#ifndef OLDSRV_GLOBAL_ASSAULT_POWER_WAR_PACKET_ABI_H_
#define OLDSRV_GLOBAL_ASSAULT_POWER_WAR_PACKET_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

// IDA unit: 99_misc_global/Packet_Broadcast_Msg.
// The original publisher writes a zero control word at +0x0a, then places a
// one-byte text length and its text payload at +0x0e/+0x0f before transport.
#pragma pack(push, 1)
struct Packet_Broadcast_Msg : PacketHeader {
    // IDA_EA: 0x82F0780
    // IDA_NAME: _ZN20Packet_Broadcast_MsgC2Ev
    // DEMANGLED: Packet_Broadcast_Msg::Packet_Broadcast_Msg(void)
    Packet_Broadcast_Msg();

    int32_t raw_0a;
    uint8_t text_length;
    char text[0xff];
};

// IDA unit: 99_misc_global/Packet_Guild_Change_Power_War_Point.
// The guild-server payload is a side discriminator, four winner IDs, four
// loser IDs, and their point deltas.  All nine IDs/point fields are cleared by
// C2 before CAssaultPlace fills the selected winner/loser slots.
struct Packet_Guild_Change_Power_War_Point : PacketHeader {
    // IDA_EA: 0x82F070C
    // IDA_NAME: _ZN35Packet_Guild_Change_Power_War_PointC2Ev
    // DEMANGLED: Packet_Guild_Change_Power_War_Point::Packet_Guild_Change_Power_War_Point(void)
    Packet_Guild_Change_Power_War_Point();

    uint8_t power_side;
    uint32_t winner_charac_nos[4];
    uint32_t loser_charac_nos[4];
    uint8_t winner_point;
    uint8_t loser_point;
};
#pragma pack(pop)

static_assert(sizeof(Packet_Broadcast_Msg) == 0x10eu,
              "Packet_Broadcast_Msg IDA ABI size must stay 0x10e");
static_assert(offsetof(Packet_Broadcast_Msg, raw_0a) == 0x0au &&
                  offsetof(Packet_Broadcast_Msg, text_length) == 0x0eu &&
                  offsetof(Packet_Broadcast_Msg, text) == 0x0fu,
              "Packet_Broadcast_Msg field offsets changed");
static_assert(sizeof(Packet_Guild_Change_Power_War_Point) == 0x2du,
              "Packet_Guild_Change_Power_War_Point IDA ABI size must stay 0x2d");
static_assert(offsetof(Packet_Guild_Change_Power_War_Point, power_side) == 0x0au &&
                  offsetof(Packet_Guild_Change_Power_War_Point, winner_charac_nos) == 0x0bu &&
                  offsetof(Packet_Guild_Change_Power_War_Point, loser_charac_nos) == 0x1bu &&
                  offsetof(Packet_Guild_Change_Power_War_Point, winner_point) == 0x2bu &&
                  offsetof(Packet_Guild_Change_Power_War_Point, loser_point) == 0x2cu,
              "Packet_Guild_Change_Power_War_Point field offsets changed");

#endif  // OLDSRV_GLOBAL_ASSAULT_POWER_WAR_PACKET_ABI_H_
