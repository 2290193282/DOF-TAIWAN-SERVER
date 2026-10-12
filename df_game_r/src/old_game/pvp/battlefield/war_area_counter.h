#ifndef OLDSRV_GLOBAL_WAR_AREA_COUNTER_H_
#define OLDSRV_GLOBAL_WAR_AREA_COUNTER_H_

#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <map>

// IDA unit: 99_misc_global/stWarPoint.
struct stWarPoint {
    stWarPoint();

    void* clear();

    int32_t values[4];
};

static_assert(sizeof(stWarPoint) == 0x10u,
              "stWarPoint IDA ABI size must stay 0x10");

// IDA unit: 05_world_enter_village/WarAreaCounter. B/PVF owns
// ImportWarAreaCounter and real Etc/WarArea.etc row population.
struct WarAreaCounter {
    WarAreaCounter();
    ~WarAreaCounter();

    WarAreaCounter* clear();
    int GetCurrenTimeTable();
    int GetWarRoomCountAtPeekTime(int level_grid);
    int GetWarRoomCountFirstIndex(int level_grid);
    int GetWarRoomCountLastIndex(int level_grid);

    int32_t counter[24];       // +0x000
    int32_t time_table[24];    // +0x060
    int32_t door_fee[10];      // +0x0c0
    int32_t peak_count_index;  // +0x0e8
    int32_t ban_interval;      // +0x0ec
    int32_t kick_point;        // +0x0f0
    int32_t ready_count;       // +0x0f4
    int32_t dungeon_ids[10];   // +0x0f8
    std::map<int, stWarPoint, std::greater<int> > hp_to_wp_points;  // +0x120
    int32_t wp_penalty;        // +0x138
    float monster_exp_const;   // +0x13c
    int32_t time_table_count;  // +0x140
    int32_t war_room_count[24][10];  // +0x144
};

static_assert(sizeof(WarAreaCounter) == 0x504u,
              "WarAreaCounter IDA ABI size must stay 0x504");
static_assert(offsetof(WarAreaCounter, door_fee) == 0x0c0u,
              "WarAreaCounter door-fee table must stay at +0x0c0");
static_assert(offsetof(WarAreaCounter, ban_interval) == 0x0ecu,
              "WarAreaCounter ban interval must stay at +0x0ec");
static_assert(offsetof(WarAreaCounter, kick_point) == 0x0f0u,
              "WarAreaCounter kick-point threshold must stay at +0x0f0");
static_assert(offsetof(WarAreaCounter, ready_count) == 0x0f4u,
              "WarAreaCounter ready-count delay must stay at +0x0f4");
static_assert(offsetof(WarAreaCounter, hp_to_wp_points) == 0x120u,
              "WarAreaCounter hp_to_wp_points offset must stay 0x120");
static_assert(offsetof(WarAreaCounter, wp_penalty) == 0x138u,
              "WarAreaCounter wp_penalty offset must stay 0x138");
static_assert(offsetof(WarAreaCounter, time_table_count) == 0x140u,
              "WarAreaCounter time_table_count offset must stay 0x140");
static_assert(offsetof(WarAreaCounter, war_room_count) == 0x144u,
              "WarAreaCounter war_room_count offset must stay 0x144");

// IDA_EA: 0x89026F3
// IDA_NAME: _Z20ImportWarAreaCounterP14WarAreaCounter
// DEMANGLED: ImportWarAreaCounter(WarAreaCounter *)
int ImportWarAreaCounter(WarAreaCounter* counter)
    __asm__("_Z20ImportWarAreaCounterP14WarAreaCounter");


using ::ImportWarAreaCounter;
using ::WarAreaCounter;
using ::stWarPoint;


#endif  // OLDSRV_GLOBAL_WAR_AREA_COUNTER_H_
