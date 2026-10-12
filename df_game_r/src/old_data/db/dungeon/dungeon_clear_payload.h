#ifndef OLDSRV_DB_DUNGEON_CLEAR_PAYLOAD_H_
#define OLDSRV_DB_DUNGEON_CLEAR_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>


// The two layouts are fixed by the original Stream template instantiations:
// Stream::GetOutBuffer<SIG_LOAD_DUNGEON_CLEAR> @0x8450F74 uses 0x1810
// bytes and Stream::GetOutBuffer<SIG_SAVE_DUNGEON_CLEAR> @0x8450FC8 uses
// 0x180C bytes.  The field offsets are also observed at the request producer,
// DB dispatcher, and Inter_LoadDungeonClear consumer.
struct SIG_LOAD_DUNGEON_CLEAR {
    uint32_t charac_no;          // +0x0000
    uint32_t account_id;         // +0x0004
    uint8_t needs_member_save;   // +0x0008; aggregate fallback sets this to 1
    uint32_t result;             // +0x000C; 0 success, 1 load failure
    char dungeon[0x1800];        // +0x0010
};

struct SIG_SAVE_DUNGEON_CLEAR {
    uint32_t charac_no;          // +0x0000
    uint32_t account_id;         // +0x0004
    uint32_t producer_zero;      // +0x0008; original producer zero-initializes it
    char dungeon[0x1800];        // +0x000C
};

static_assert(sizeof(SIG_LOAD_DUNGEON_CLEAR) == 0x1810u,
              "SIG_LOAD_DUNGEON_CLEAR IDA ABI size must stay 0x1810");
static_assert(offsetof(SIG_LOAD_DUNGEON_CLEAR, needs_member_save) == 0x08u,
              "SIG_LOAD_DUNGEON_CLEAR save flag offset must stay 0x08");
static_assert(offsetof(SIG_LOAD_DUNGEON_CLEAR, result) == 0x0Cu,
              "SIG_LOAD_DUNGEON_CLEAR result offset must stay 0x0C");
static_assert(offsetof(SIG_LOAD_DUNGEON_CLEAR, dungeon) == 0x10u,
              "SIG_LOAD_DUNGEON_CLEAR dungeon offset must stay 0x10");
static_assert(sizeof(SIG_SAVE_DUNGEON_CLEAR) == 0x180Cu,
              "SIG_SAVE_DUNGEON_CLEAR IDA ABI size must stay 0x180C");
static_assert(offsetof(SIG_SAVE_DUNGEON_CLEAR, dungeon) == 0x0Cu,
              "SIG_SAVE_DUNGEON_CLEAR dungeon offset must stay 0x0C");

#endif  // OLDSRV_DB_DUNGEON_CLEAR_PAYLOAD_H_
