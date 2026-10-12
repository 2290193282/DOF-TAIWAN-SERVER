#ifndef OLDSRV_DB_BLOOD_BEST_RECORD_PAYLOAD_H_
#define OLDSRV_DB_BLOOD_BEST_RECORD_PAYLOAD_H_

#include <stdint.h>


// IDA unit: 13_database_dispatch/DB_LoadBloodBestRecord.
struct SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD {
    uint32_t charac_no;      // +0x00
    uint32_t best_time;      // +0x04
    int32_t best_round;      // +0x08
    int32_t dungeon_index;   // +0x0C
};

// IDA unit: 13_database_dispatch/DB_UpdateBloodBestRecord.
struct SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD {
    uint32_t charac_no;      // +0x00
    uint32_t best_time;      // +0x04
    int32_t best_round;      // +0x08
    int32_t dungeon_index;   // +0x0C
};

static_assert(sizeof(SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD) == 0x10u,
              "SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD IDA ABI size must stay 0x10");
static_assert(sizeof(SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD) == 0x10u,
              "SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD IDA ABI size must stay 0x10");

#endif  // OLDSRV_DB_BLOOD_BEST_RECORD_PAYLOAD_H_
