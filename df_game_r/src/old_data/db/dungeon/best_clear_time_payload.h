#ifndef DNF_DB_BEST_CLEAR_TIME_PAYLOAD_H_
#define DNF_DB_BEST_CLEAR_TIME_PAYLOAD_H_

#include <stdint.h>


// IDA_EA: 0x80C8C72. Five difficulty records follow every dungeon id.
struct SIG_BEST_CLEAR_TIME_DIFFICULTY {
    SIG_BEST_CLEAR_TIME_DIFFICULTY();

    int32_t clear_time[5];  // +0x00
};

// IDA_EA: 0x848FAE2.
struct SIG_BEST_CLEAR_TIME_DATA {
    SIG_BEST_CLEAR_TIME_DATA();

    int32_t dungeon_id;                           // +0x00
    SIG_BEST_CLEAR_TIME_DIFFICULTY difficulty;    // +0x04
};

// IDA_EA: 0x848FB02; Stream payload size @0x80C9032/@0x84545D2.
struct SIG_BEST_CLEAR_TIME {
    SIG_BEST_CLEAR_TIME();

    uint32_t charac_no;                    // +0x0000
    uint32_t count;                        // +0x0004
    SIG_BEST_CLEAR_TIME_DATA records[1000]; // +0x0008
};

static_assert(sizeof(SIG_BEST_CLEAR_TIME_DIFFICULTY) == 0x14,
              "SIG_BEST_CLEAR_TIME_DIFFICULTY must match IDA");
static_assert(sizeof(SIG_BEST_CLEAR_TIME_DATA) == 0x18,
              "SIG_BEST_CLEAR_TIME_DATA must match IDA");
static_assert(sizeof(SIG_BEST_CLEAR_TIME) == 0x5DC8,
              "SIG_BEST_CLEAR_TIME must match the IDA stream ABI");

#endif  // DNF_DB_BEST_CLEAR_TIME_PAYLOAD_H_
