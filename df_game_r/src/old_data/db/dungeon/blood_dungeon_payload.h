#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_BLOOD_DUNGEON_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_BLOOD_DUNGEON_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

// Old DB signal 0x197 payload.  The 0x28-byte layout is fixed by
// CCharacBloodDungeon::_saveData @0x832A330,
// DB_SaveBloodDungeon::SaveBloodDungeonReward @0x84374F8, and all three
// SIG_BLOOD_DUNGEON stream specializations.
struct SIG_BLOOD_DUNGEON {
    uint32_t charac_no;                    // +0x00
    uint32_t week_point;                   // +0x04
    uint32_t week_enter_count;             // +0x08
    uint32_t week_use_gold;                // +0x0c
    int32_t last_play_date;                // +0x10
    uint32_t enter_count;                  // +0x14
    uint32_t previous_rank;                // +0x18
    uint8_t previous_reward_claimed;       // +0x1c
    uint32_t previous_reward_item_id;      // +0x20
    uint32_t previous_reward_gold;         // +0x24
};

static_assert(sizeof(SIG_BLOOD_DUNGEON) == 0x28u,
              "SIG_BLOOD_DUNGEON IDA ABI size must stay 0x28");
static_assert(offsetof(SIG_BLOOD_DUNGEON, previous_reward_claimed) == 0x1cu,
              "SIG_BLOOD_DUNGEON reward flag offset must stay 0x1c");
static_assert(offsetof(SIG_BLOOD_DUNGEON, previous_reward_item_id) == 0x20u,
              "SIG_BLOOD_DUNGEON reward item offset must stay 0x20");

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_BLOOD_DUNGEON_PAYLOAD_H_
