#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

// These records are database-business ABI, not Stream-owned opaque storage.
// Stream::GetOutBuffer specializations prove the exact sizes below:
//   0x8453398 / 0x860071E : SIG_INSERT_SERVER_LOAD_STATISTIC, 10 bytes
//   0x84523B8            : SIG_VALUE_STATISTIC, 128 bytes
//   0x8452FF4            : SIG_CIRCULATION_STATISTIC, 200 bytes
//   0x8453C64 / 0x85707E8: SIG_INSERT_RESPONSE_TIME, 2008 bytes
#pragma pack(push, 1)

struct SIG_INSERT_SERVER_LOAD_STATISTIC {
    int8_t server_id;       // +0x00
    int8_t channel_no;      // +0x01
    int16_t dbq_cnt;        // +0x02
    int16_t logdbq_cnt;     // +0x04
    int16_t netq_cnt;       // +0x06
    int16_t packetq_cnt;    // +0x08
};

// DB_InsertValueStatistic @0x842945E uses words 0..26 as the SQL values.
// The final five dwords are part of the original 128-byte Stream ABI but are
// not consumed by that dispatcher; their semantics remain intentionally
// unclaimed instead of being invented or collapsed into a byte buffer.
struct SIG_VALUE_STATISTIC {
    int32_t occ_time;                         // word 0, +0x00
    int32_t level;                            // word 1, +0x04
    uint32_t uv;                              // word 2, +0x08
    uint32_t drop_gold;                       // word 3, +0x0C
    uint32_t drop_item;                       // word 4, +0x10
    uint32_t result_card_gold;                // word 5, +0x14
    uint32_t result_card_item;                // word 6, +0x18
    uint32_t gold_card_item;                  // word 7, +0x1C
    uint32_t store_item_buy;                  // word 8, +0x20
    uint32_t jar_item;                        // word 9, +0x24
    uint32_t disjoint_create;                 // word 10, +0x28
    uint32_t upgrade_faild_forced_disjoint;   // word 11, +0x2C
    uint32_t quest_reward;                    // word 12, +0x30
    uint32_t deathtower_card_gold;            // word 13, +0x34
    uint32_t deathtower_card_item;            // word 14, +0x38
    uint32_t consume_store_item_buy;          // word 15, +0x3C
    uint32_t consume_upgrade_attempt;         // word 16, +0x40
    uint32_t consume_upgrade_faild;           // word 17, +0x44
    uint32_t consume_stamina_recovery;        // word 18, +0x48
    uint32_t consume_quest_consume;           // word 19, +0x4C
    uint32_t consume_auction_commision;       // word 20, +0x50
    uint32_t consume_item_disjoint;           // word 21, +0x54
    uint32_t consume_item_repair;             // word 22, +0x58
    uint32_t consume_item_use;                // word 23, +0x5C
    uint32_t consume_item_drop;               // word 24, +0x60
    uint32_t consume_gold_drop;               // word 25, +0x64
    uint32_t consume_gold_card_price;         // word 26, +0x68
    uint32_t unreported_27;                   // word 27, +0x6C
    uint32_t unreported_28;                   // word 28, +0x70
    uint32_t unreported_29;                   // word 29, +0x74
    uint32_t unreported_30;                   // word 30, +0x78
    uint32_t unreported_31;                   // word 31, +0x7C
};

// DB_InsertCirculationStatistic @0x84327BA consumes the labelled words in
// the original log_gold_stat INSERT. Unreported slots are individually
// preserved because the 200-byte Stream ABI proves their positions, while no
// old consumer recovered here establishes a business meaning for them.
struct SIG_CIRCULATION_STATISTIC {
    int32_t occ_time;                         // word 0, +0x00
    int32_t level;                            // word 1, +0x04
    uint32_t dungeon_drop;                    // word 2, +0x08
    uint32_t result_card;                     // word 3, +0x0C
    uint32_t sell_store;                      // word 4, +0x10
    uint32_t quest_reward;                    // word 5, +0x14
    uint32_t death_tower_reward;              // word 6, +0x18
    uint32_t illusion_tower_reward;           // word 7, +0x1C
    uint32_t war_area_drop;                   // word 8, +0x20
    uint32_t member_tax;                      // word 9, +0x24
    uint32_t unreported_10;                   // word 10, +0x28
    uint32_t unreported_11;                   // word 11, +0x2C
    uint32_t unreported_12;                   // word 12, +0x30
    uint32_t blood_dungeon_reward;            // word 13, +0x34
    uint32_t unreported_14;                   // word 14, +0x38
    uint32_t blood_dungeon_lotto;             // word 15, +0x3C
    uint32_t power_dungeon_drop;              // word 16, +0x40
    uint32_t power_dungeon_result_card;       // word 17, +0x44
    uint32_t unreported_18;                   // word 18, +0x48
    uint32_t unreported_19;                   // word 19, +0x4C
    uint32_t buy_store;                       // word 20, +0x50
    uint32_t stamina_recovery;                // word 21, +0x54
    uint32_t repair_item;                     // word 22, +0x58
    uint32_t private_store_commission;        // word 23, +0x5C
    uint32_t gold_card;                       // word 24, +0x60
    uint32_t unreported_25;                   // word 25, +0x64
    uint32_t gold_drop;                       // word 26, +0x68
    uint32_t upgrade;                         // word 27, +0x6C
    uint32_t quest_use;                       // word 28, +0x70
    uint32_t mail_commission;                 // word 29, +0x74
    uint32_t punish_user;                     // word 30, +0x78
    uint32_t restrict_trade;                  // word 31, +0x7C
    uint32_t guild_level_up;                  // word 32, +0x80
    uint32_t guild_skill;                     // word 33, +0x84
    uint32_t guild_mail;                      // word 34, +0x88
    uint32_t item_compound;                   // word 35, +0x8C
    uint32_t unreported_36;                   // word 36, +0x90
    uint32_t war_area_enter;                  // word 37, +0x94
    uint32_t assault_gold;                    // word 38, +0x98
    uint32_t blood_dungeon_enter;             // word 39, +0x9C
    uint32_t buy_cerashop;                    // word 40, +0xA0
    uint32_t upgrade_guild_agit;              // word 41, +0xA4
    uint32_t upgrade_guild_cargo;             // word 42, +0xA8
    uint32_t unreported_43;                   // word 43, +0xAC
    uint32_t unreported_44;                   // word 44, +0xB0
    uint32_t unreported_45;                   // word 45, +0xB4
    uint32_t unreported_46;                   // word 46, +0xB8
    uint32_t unreported_47;                   // word 47, +0xBC
    uint32_t unreported_48;                   // word 48, +0xC0
    uint32_t unreported_49;                   // word 49, +0xC4
};

struct SIG_RESPONSE_TIME_ENTRY {
    uint32_t packet_id;                       // +0x00
    uint32_t packet_count;                    // +0x04
    uint32_t total_response_time_low;         // +0x08
    uint32_t total_response_time_high;        // +0x0C
    uint32_t avg_response_time;               // +0x10
};

struct SIG_INSERT_RESPONSE_TIME {
    int32_t occ_time;                         // word 0, +0x000
    int32_t entry_count;                      // word 1, +0x004
    SIG_RESPONSE_TIME_ENTRY entries[100];     // word 2, +0x008
};

#pragma pack(pop)

static_assert(sizeof(SIG_INSERT_SERVER_LOAD_STATISTIC) == 10u,
              "server-load statistic ABI must stay 10 bytes");
static_assert(sizeof(SIG_VALUE_STATISTIC) == 128u,
              "value statistic ABI must stay 128 bytes");
static_assert(sizeof(SIG_CIRCULATION_STATISTIC) == 200u,
              "circulation statistic ABI must stay 200 bytes");
static_assert(sizeof(SIG_RESPONSE_TIME_ENTRY) == 20u,
              "response-time entry ABI must stay 20 bytes");
static_assert(sizeof(SIG_INSERT_RESPONSE_TIME) == 2008u,
              "response-time statistic ABI must stay 2008 bytes");
static_assert(offsetof(SIG_INSERT_RESPONSE_TIME, entries) == 8u,
              "response-time entries must begin at word 2");
static_assert(offsetof(SIG_CIRCULATION_STATISTIC, blood_dungeon_reward) ==
                  0x34u,
              "circulation ABI word 13 must stay stable");
static_assert(offsetof(SIG_CIRCULATION_STATISTIC, war_area_enter) == 0x94u,
              "circulation ABI word 37 must stay stable");
static_assert(offsetof(SIG_VALUE_STATISTIC, consume_gold_card_price) ==
                  0x68u,
              "value statistic SQL tail must stay at word 26");

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_PAYLOADS_H_
