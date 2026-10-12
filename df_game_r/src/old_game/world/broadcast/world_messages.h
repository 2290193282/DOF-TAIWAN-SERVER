#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_MESSAGES_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_MESSAGES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x81C91E6 in the companion source.


#include <stdint.h>

#include "infra/network/packet/message_base.h"


// IDA message body used by
// 05_world_enter_village/DisPatcher_SetArea::read.
#pragma pack(push, 1)
struct MSG_SET_USER_AREA : public MSG_BASE {
    char area;                   // +0x0d
    char before_area;            // +0x0e
    uint16_t village_index;      // +0x0f
    uint16_t move_index;         // +0x11
    char move_type;              // +0x13
    uint16_t move_arg0;          // +0x14
    uint16_t move_arg1;          // +0x16
};
#pragma pack(pop)

static_assert(sizeof(MSG_SET_USER_AREA) == 0x18u,
              "MSG_SET_USER_AREA must retain its old 24-byte extent");

// IDA message body used by
// 05_world_enter_village/DisPatcher_MoveMap::read.
#pragma pack(push, 1)
struct MSG_MOVE_MAP : public MSG_BASE {
    char map_x;                     // IDA +0x0d
    char map_y;                     // IDA +0x0e
    uint32_t dword_0f;              // IDA +0x0f
    uint32_t dword_13;              // IDA +0x13
    uint8_t layered_map;            // IDA +0x17
    uint16_t trap_bits;             // IDA +0x18
    uint8_t trap_bits_read;         // IDA +0x1a
    uint16_t clear_counts[8];       // native client +0x1b
    uint32_t map_play_times[8];     // native client +0x2b
    uint16_t map_clear_count;       // native client +0x4b
    int32_t dword_4d;               // native client +0x4d
    uint8_t party_seat;             // server result +0x51
};
#pragma pack(pop)

static_assert(sizeof(MSG_MOVE_MAP) == 0x52u,
              "CMD45 native move-map request must retain its 82-byte extent");


#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_MESSAGES_H_
