#include "old_game/pvp/battlefield/war_area_counter.h"

#include "infra/log/format/trace.h"

#include <string.h>
#include <time.h>

// IDA_EA: 0x891A866
// IDA_NAME: _ZN10stWarPointC2Ev
// DEMANGLED: stWarPoint::stWarPoint(void)
stWarPoint::stWarPoint() {
    clear();
}

// IDA_EA: 0x891A87A
// IDA_NAME: _ZN10stWarPoint5clearEv
// DEMANGLED: stWarPoint::clear(void)
void* stWarPoint::clear() {
    return ::memset(this, 0, 0x10u);
}

// IDA_EA: 0x83708C6
// IDA_NAME: _ZN14WarAreaCounterC2Ev
// DEMANGLED: WarAreaCounter::WarAreaCounter(void)
WarAreaCounter::WarAreaCounter() : hp_to_wp_points() {
    clear();
}

// IDA_EA: 0x837D3B2
// IDA_NAME: _ZN14WarAreaCounterD2Ev
// DEMANGLED: WarAreaCounter::~WarAreaCounter()
WarAreaCounter::~WarAreaCounter() {}

// IDA_EA: 0x83707A4
// IDA_NAME: _ZN14WarAreaCounter5clearEv
// DEMANGLED: WarAreaCounter::clear(void)
WarAreaCounter* WarAreaCounter::clear() {
    ::memset(counter, 0, 0x60u);
    ::memset(time_table, 0, 0x60u);
    ::memset(door_fee, 0, 0x28u);
    peak_count_index = 0;
    ban_interval = 0;
    kick_point = 0;
    ready_count = 0;
    ::memset(dungeon_ids, 0, 0x28u);
    hp_to_wp_points.clear();
    time_table_count = 0;
    ::memset(war_room_count, 0, 0x3C0u);
    wp_penalty = 0;
    monster_exp_const = 0.0f;
    return this;
}

// IDA_EA: 0x89024C4
// IDA_NAME: _ZN14WarAreaCounter18GetCurrenTimeTableEv
// DEMANGLED: WarAreaCounter::GetCurrenTimeTable(void)
int WarAreaCounter::GetCurrenTimeTable() {
    struct tm current_tm;
    time_t timer;

    ::time(&timer);
    struct tm* local_time = ::localtime_r(&timer, &current_tm);
    const int current_hour = local_time->tm_hour;

    int index = 0;
    for (; time_table_count - 1 > index &&
           (time_table[index] > current_hour ||
            time_table[index + 1] <= current_hour);
         ++index) {
    }

    return index;
}

// IDA_EA: 0x82A3D80
// IDA_NAME: _ZN14WarAreaCounter25GetWarRoomCountAtPeekTimeEi
// DEMANGLED: WarAreaCounter::GetWarRoomCountAtPeekTime(int)
int WarAreaCounter::GetWarRoomCountAtPeekTime(int level_grid) {
    if (static_cast<unsigned int>(level_grid) < 0x0Au) {
        return war_room_count[peak_count_index][level_grid];
    }

    LogManager::logFormat(
        1,
        "../../DNFShared/GameScript/RDARScriptEtcParameter.h",
        "GetWarRoomCountAtPeekTime",
        0x317,
        "GetWarRoomCountAtPeekTime Invalid Parameter levelGrid<%d>",
        level_grid);
    return 0;
}

// IDA_EA: 0x82A3DF4
// IDA_NAME: _ZN14WarAreaCounter25GetWarRoomCountFirstIndexEi
// DEMANGLED: WarAreaCounter::GetWarRoomCountFirstIndex(int)
int WarAreaCounter::GetWarRoomCountFirstIndex(int level_grid) {
    if (static_cast<unsigned int>(level_grid) < 0x0Au) {
        int count = 0;
        for (int index = 0; index < level_grid; ++index) {
            count += war_room_count[peak_count_index][index];
        }
        return count;
    }

    LogManager::logFormat(
        1,
        "../../DNFShared/GameScript/RDARScriptEtcParameter.h",
        "GetWarRoomCountFirstIndex",
        0x320,
        "GetWarRoomCountLastIndex Invalid Parameter levelGrid<%d>",
        level_grid);
    return 0;
}

// IDA_EA: 0x82A3E90
// IDA_NAME: _ZN14WarAreaCounter24GetWarRoomCountLastIndexEi
// DEMANGLED: WarAreaCounter::GetWarRoomCountLastIndex(int)
int WarAreaCounter::GetWarRoomCountLastIndex(int level_grid) {
    if (static_cast<unsigned int>(level_grid) < 0x0Au) {
        int count = 0;
        for (int index = 0; index <= level_grid; ++index) {
            count += war_room_count[peak_count_index][index];
        }
        return count - 1;
    }

    LogManager::logFormat(
        1,
        "../../DNFShared/GameScript/RDARScriptEtcParameter.h",
        "GetWarRoomCountLastIndex",
        0x32F,
        "GetWarRoomCountLastIndex Invalid Parameter levelGrid<%d>",
        level_grid);
    return 0;
}
