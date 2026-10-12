#ifndef OLDSRV_OLD_DATA_CHANNEL_CHANNEL_DB_PAYLOADS_H_
#define OLDSRV_OLD_DATA_CHANNEL_CHANNEL_DB_PAYLOADS_H_

#include <stdint.h>

#include "common/config.h"

#pragma pack(push, 1)

// IDA wire type: SIG_INSERT_CHANNEL (0x5c).
struct SIG_INSERT_CHANNEL {
    int32_t gc_no;
    uint8_t reserved_04[2];
    char gc_ip[16];
    uint16_t gc_port;
    int16_t gc_max;
    char gc_game;
    char gc_channel[30];
    int16_t gc_ch_group;
    char gc_channeltype[25];
    int32_t gc_type;
    int32_t server_group;
};

// IDA wire type: SIG_UPDATE_CHANNEL (0x3a).
struct SIG_UPDATE_CHANNEL {
    int32_t gc_no;
    int16_t gc_now;
    int32_t swordman_count;
    int32_t fighter_count;
    int32_t gunner_count;
    int32_t mage_count;
    int32_t priest_count;
    int32_t at_gunner_count;
    int32_t thief_count;
    uint8_t reserved_22[16];
    int32_t hangame_count;
    int32_t nexon_count;
};

// IDA wire type: SIG_DELETE_CHANNEL (0x0c).
struct SIG_DELETE_CHANNEL {
    int32_t gc_no;
    int32_t server_group;
    int32_t reserved_result;
};

// IDA wire type: SIG_UPDATE_DETAIL_CHANNEL (0x10).
struct SIG_UPDATE_DETAIL_CHANNEL {
    int32_t player_status;
    int32_t dungeon_index;
    int32_t channel_no;
    int32_t occ_count;
};

// IDA wire type: SIG_UPDATE_STATISTIC4LEV (0x124).  Index zero is the
// channel number; the original dispatcher reads the configured legacy level
// 2..71 and the occurrence time from dword 72.
struct SIG_UPDATE_STATISTIC4LEV {
    int32_t channel_no;
    int32_t reserved_dword_04;
    int32_t level_counts[oldsrv::config::kChannelLevelStatisticCount];
    int32_t occ_time;
};

#pragma pack(pop)

typedef char SIG_INSERT_CHANNEL_Size_Must_Be_0x5c[
    (sizeof(SIG_INSERT_CHANNEL) == 0x5c) ? 1 : -1];
typedef char SIG_UPDATE_CHANNEL_Size_Must_Be_0x3a[
    (sizeof(SIG_UPDATE_CHANNEL) == 0x3a) ? 1 : -1];
typedef char SIG_DELETE_CHANNEL_Size_Must_Be_0x0c[
    (sizeof(SIG_DELETE_CHANNEL) == 0x0c) ? 1 : -1];
typedef char SIG_UPDATE_DETAIL_CHANNEL_Size_Must_Be_0x10[
    (sizeof(SIG_UPDATE_DETAIL_CHANNEL) == 0x10) ? 1 : -1];
typedef char SIG_UPDATE_STATISTIC4LEV_Size_Must_Match_Config[
    (sizeof(SIG_UPDATE_STATISTIC4LEV) ==
     sizeof(int32_t) *
         (3 + oldsrv::config::kChannelLevelStatisticCount))
        ? 1
        : -1];

#endif  // OLDSRV_OLD_DATA_CHANNEL_CHANNEL_DB_PAYLOADS_H_
