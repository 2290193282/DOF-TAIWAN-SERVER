#ifndef OLDSRV_DB_DB_LOAD_NO_CACHE_DATA_H_
#define OLDSRV_DB_DB_LOAD_NO_CACHE_DATA_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


// IDA_EA: CStreamGuard::GetInBuffer<SIG_LOAD_NO_CACHE_DATA> @0x8437EB4.
// Inter_LoadNoCacheData @0x84DD1EE passes the 40-byte tail to
// CCharacBloodDungeon::loadData @0x832A4C6; its field order is also fixed by
// the two charac_blood_dungeon_reward SELECT lists at 0x8437F58.
struct SIG_LOAD_NO_CACHE_DATA {
    uint32_t charac_no;                    // +0x00, outer identity
    uint32_t serialized_charac_no;         // +0x04, CData word 0
    uint32_t week_point;                   // +0x08
    uint32_t week_enter_count;             // +0x0C
    uint32_t week_use_gold;                // +0x10
    int32_t last_play_date;                 // +0x14
    uint32_t enter_count;                  // +0x18
    uint32_t previous_rank;                // +0x1C
    uint8_t previous_reward_available;     // +0x20
    uint32_t previous_reward_item_id;      // +0x24
    uint32_t previous_reward_gold;         // +0x28
};

static_assert(sizeof(SIG_LOAD_NO_CACHE_DATA) == 0x2Cu,
              "SIG_LOAD_NO_CACHE_DATA IDA ABI size must stay 0x2C");
static_assert(offsetof(SIG_LOAD_NO_CACHE_DATA, week_point) == 0x08u,
              "blood-dungeon week point offset must stay 0x08");
static_assert(offsetof(SIG_LOAD_NO_CACHE_DATA, previous_reward_available) ==
                  0x20u,
              "blood-dungeon reward flag offset must stay 0x20");
static_assert(offsetof(SIG_LOAD_NO_CACHE_DATA, previous_reward_item_id) ==
                  0x24u,
              "blood-dungeon reward item offset must stay 0x24");


// IDA unit: 13_database_dispatch/DB_LoadNoCacheData.
class DB_LoadNoCacheData : public DBDispatcher {
public:
    DB_LoadNoCacheData();
    virtual ~DB_LoadNoCacheData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool LoadBloodDungeonReward(SIG_LOAD_NO_CACHE_DATA* payload);
};

#endif  // OLDSRV_DB_DB_LOAD_NO_CACHE_DATA_H_
