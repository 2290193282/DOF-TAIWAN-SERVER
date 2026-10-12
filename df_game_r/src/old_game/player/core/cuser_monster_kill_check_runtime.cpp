#include "old_game/player/core/cuser.h"

#include <cstdlib>

// IDA_EA: 0x823049C
// IDA_NAME: _ZN5CUser14st_monstorkill5CheckEjttttj
// DEMANGLED: CUser::st_monstorkill::Check(unsigned int,unsigned short,unsigned short,unsigned short,unsigned short,unsigned int)
int CUser::st_monstorkill::Check(unsigned int tick,
                                 unsigned short reported_x,
                                 unsigned short reported_y,
                                 unsigned short maximum_span_x,
                                 unsigned short maximum_span_y,
                                 unsigned int reset_interval) {
    const int16_t x = static_cast<int16_t>(reported_x);
    const int16_t y = static_cast<int16_t>(reported_y);
    if (tick - last_tick > reset_interval) {
        span_x = 0;
        span_y = 0;
        report_count = 0;
        last_tick = tick;
        min_x = x;
        min_y = y;
        max_x = x;
        max_y = y;
        return 0;
    }

    ++report_count;
    if (min_x > x) min_x = x;
    if (min_y > y) min_y = y;
    if (max_x < x) max_x = x;
    if (max_y < y) max_y = y;

    span_x = static_cast<int16_t>(
        std::abs(static_cast<int>(max_x) - static_cast<int>(min_x)));
    span_y = static_cast<int16_t>(
        std::abs(static_cast<int>(max_y) - static_cast<int>(min_y)));
    return span_x >= static_cast<int>(maximum_span_x) ||
           span_y >= static_cast<int>(maximum_span_y);
}
