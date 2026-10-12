#ifndef DB_BREAK_AWAY_PAYLOAD_H_
#define DB_BREAK_AWAY_PAYLOAD_H_

#include <stdint.h>

// IDA wire type: SIG_BREAK_AWAY_UV_LIST (0x1f).
struct SIG_BREAK_AWAY_UV_LIST {
    char play_info[31];
};

// IDA wire type: SIG_BREAK_AWAY_PREVENT_MESSAGE (0x08).
struct SIG_BREAK_AWAY_PREVENT_MESSAGE {
    uint32_t next_reward_time;
    uint32_t reward_order;
};

// DB_LoadBreakAwayPreventData @0x84389FE builds this before copying it to
// the event-owned SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER signal.
struct DBBreakAwayPreventRewardRow {
    uint8_t quest_id;
    uint8_t min_day;
    uint8_t max_day;
    uint8_t reserved_03;
    int32_t min_value;
    int32_t max_value;
    uint32_t item_id;
    int32_t add_info;
    int32_t luck_point;
};

struct DBBreakAwayPreventSystemPayload {
    int16_t weekday_var_a;
    int16_t weekday_var_b;
    int16_t weekday_var_c;
    int16_t weekend_var_x;
    int16_t weekend_var_y;
    int16_t weekend_var_z;
    int16_t next_reward_day;
    uint8_t state_flag;
    uint8_t reserved_0f;
    DBBreakAwayPreventRewardRow rewards[30];
    uint32_t reward_count;
};

typedef char SIG_BREAK_AWAY_UV_LIST_Size_Must_Be_0x1f[
    (sizeof(SIG_BREAK_AWAY_UV_LIST) == 0x1f) ? 1 : -1];
typedef char SIG_BREAK_AWAY_PREVENT_MESSAGE_Size_Must_Be_0x08[
    (sizeof(SIG_BREAK_AWAY_PREVENT_MESSAGE) == 0x08) ? 1 : -1];
typedef char DBBreakAwayPreventRewardRow_Size_Must_Be_0x18[
    (sizeof(DBBreakAwayPreventRewardRow) == 0x18) ? 1 : -1];
typedef char DBBreakAwayPreventSystemPayload_Size_Must_Be_0x2e4[
    (sizeof(DBBreakAwayPreventSystemPayload) == 0x2e4) ? 1 : -1];

#endif  // DB_BREAK_AWAY_PAYLOAD_H_
