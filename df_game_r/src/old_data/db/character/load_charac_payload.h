#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_LOAD_CHARAC_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_LOAD_CHARAC_PAYLOAD_H_

#include "common/config.h"
#include "common/manager/play_info_public_abi.h"
#include "old_data/db/character/create_charac_payload.h"

#include <stddef.h>
#include <stdint.h>


#pragma pack(push, 1)

// IDA_EA: CHARAC_DATA::CHARAC_DATA @0x822EC4C and
// CHARAC_DATA::Reset @0x822EC9E.  DB_LoadCharac fills this target 0x6BC
// record at SIG_LOAD_CHARAC+0x0C.  The anonymous union preserves the real
// _Additioal_info constructor ABI while exposing the same verified 0x52 DB
// columns through CREATE_CHARACTER_INITIAL_STATS.
struct CHARAC_DATA {
    CHARAC_DATA();

    CHARAC_DATA* Reset();

    int32_t character_no;                 // +0x000
    char character_name[30];              // +0x004
    char village;                         // +0x022
    char previous_village;                // +0x023
    char job;                             // +0x024
    int16_t level;                        // +0x025
    char grow_type;                       // +0x027
    uint32_t experience;                  // +0x028
    uint16_t fatigue;                     // +0x02C
    uint16_t maximum_fatigue;             // +0x02E
    uint16_t fatigue_grownup_buff;        // +0x030
    int16_t fatigue_battery_charging;     // +0x032
    char create_time[25];                 // +0x034
    uint32_t last_play_time;              // +0x04D

    // Live IDA Get/SetCharacInfoDetail and both DB readers expose no semantic
    // use for +0x51..+0x55.  Keep the two observed scalar slots explicit;
    // do not turn them into an opaque byte range.
    uint32_t ida_off_051;                 // +0x051
    uint8_t ida_off_055;                  // +0x055

    uint32_t guild_id;                    // +0x056
    uint8_t guild_right;                  // +0x05A
    uint16_t premium_fatigue;             // +0x05B
    uint16_t maximum_premium_fatigue;     // +0x05D
    uint8_t member_flag;                  // +0x05F
    union {
        _Additioal_info additional_info;  // +0x060, real constructed type
        CREATE_CHARACTER_INITIAL_STATS db_stats;
    };
    uint8_t hp;                           // +0x0B2
    uint16_t used_fatigue;                // +0x0B3
    uint32_t tutorial_flag;               // +0x0B5
    char escalade_tutorial_flag[16];      // +0x0B9
    uint32_t trade_gold_total;            // +0x0C9
    uint32_t trade_gold_total_billion;    // +0x0CD
    uint32_t trade_gold_daily;            // +0x0D1
    int32_t chaos_point;                  // +0x0D5
    int32_t chaos_experience;             // +0x0D9
    int32_t chaos_mode_count;             // +0x0DD
    int32_t chaos_kill_count;             // +0x0E1
    int32_t chaos_die_count;              // +0x0E5
    int32_t chaos_die_time;               // +0x0E9
    int32_t assault_count;                // +0x0ED
    int32_t chaos_kill_time;              // +0x0F1
    char open_flag;                       // +0x0F5
    int32_t luck_point;                   // +0x0F6
    int32_t dungeon_play_count;           // +0x0FA
    uint32_t help_abuse_ratio;            // +0x0FE
    uint32_t help_abuse_experience;       // +0x102
    int32_t expert_job;                   // +0x106
    int32_t expert_job_experience;        // +0x10A
    int16_t power_war_point;              // +0x10E
    uint32_t power_war_assault_count;     // +0x110
    uint32_t power_war_victory_count;     // +0x114
    uint32_t power_war_last_play_time;    // +0x118
    int16_t emotion;                      // +0x11C
    char skill_tree_index;                // +0x11E
    char add_slot_flag;                   // +0x11F
    // DB column event_charac_level carries the event grow-type code.
    char event_character_grow_type;       // +0x120
    char member_dungeon_flag;             // +0x121
    int16_t member_bonus_fatigue;         // +0x122
    char guild_secede;                    // +0x124
    uint32_t last_play_dungeon_index;     // +0x125
    char add_equipslot_flag;              // +0x129
    char channel_equipslot_switch;        // +0x12A
    char expand_equipslot_switch;         // +0x12B
    uint8_t event_bring_up_reward_flag;   // +0x12C
    uint8_t event_bring_up_material_count;// +0x12D
    char visible_flags;                   // +0x12E
    uint8_t growth_power_reward;          // +0x12F
    int32_t chaos_response_time;          // +0x130
    uint32_t member_id;                   // +0x134
    Inven_Item equipment[oldsrv::config::kCharacterEquipmentTypeCount];
                                             // +0x138
    uint32_t school_id;                      // +0x6B3
    uint32_t tag_character_no;               // +0x6B7
    uint8_t striker_skill_index;             // +0x6BB
};

// IDA_EA: TAG_CHARAC_DATA::clear @0x822ED84,
// TAG_CHARAC_DATA::TAG_CHARAC_DATA @0x822EDAE, and
// DB_LoadCharac::_GetTagCharacInfo @0x8409FA6.
struct TAG_CHARAC_DATA {
    TAG_CHARAC_DATA();

    TAG_CHARAC_DATA* clear();

    int32_t character_no;                 // +0x00
    char character_name[29];              // +0x04
    char job;                             // +0x21
    int16_t level;                        // +0x22
    char grow_type;                       // +0x24
    char skill_tree_index;                // +0x25
    union {
        _Additioal_info additional_info;  // +0x26, real constructed type
        CREATE_CHARACTER_INITIAL_STATS db_stats;
    };
};

// IDA_EA: SIG_LOAD_CHARAC::SIG_LOAD_CHARAC @0x8491C62,
// Stream::GetOutBuffer<SIG_LOAD_CHARAC> @0x845079E,
// Stream::GetInBuffer<SIG_LOAD_CHARAC> @0x82396C8, and
// CStreamGuard::GetInBuffer<SIG_LOAD_CHARAC> @0x8236D1A.
struct SIG_LOAD_CHARAC {
    SIG_LOAD_CHARAC();

    uint32_t member_id;                   // +0x000
    uint32_t character_no;                // +0x004
    uint32_t result;                      // +0x008
    CHARAC_DATA charac_data;              // +0x00C
    TAG_CHARAC_DATA tag_charac_data;      // +0x6C8
};

#pragma pack(pop)

static_assert(sizeof(CHARAC_DATA) == 0x6BCu,
              "CHARAC_DATA target ABI size must stay 0x6BC");
static_assert(offsetof(CHARAC_DATA, db_stats) == 0x060u,
              "CHARAC_DATA DB stat offset must stay 0x60");
static_assert(offsetof(CHARAC_DATA, last_play_time) == 0x04Du,
              "CHARAC_DATA last-play offset must stay 0x4D");
static_assert(offsetof(CHARAC_DATA, guild_id) == 0x056u,
              "CHARAC_DATA guild offset must stay 0x56");
static_assert(offsetof(CHARAC_DATA, expert_job) == 0x106u,
              "CHARAC_DATA expert-job offset must stay 0x106");
static_assert(offsetof(CHARAC_DATA, member_id) == 0x134u,
              "CHARAC_DATA member id offset must stay 0x134");
static_assert(offsetof(CHARAC_DATA, equipment) == 0x138u,
              "CHARAC_DATA equipment offset must stay 0x138");
static_assert(offsetof(CHARAC_DATA, school_id) == 0x6B3u,
              "CHARAC_DATA school id offset must stay 0x6B3");
static_assert(offsetof(CHARAC_DATA, tag_character_no) == 0x6B7u,
              "CHARAC_DATA tag character offset must stay 0x6B7");
static_assert(offsetof(CHARAC_DATA, striker_skill_index) == 0x6BBu,
              "CHARAC_DATA striker skill offset must stay 0x6BB");
static_assert(sizeof(TAG_CHARAC_DATA) == 0x78u,
              "TAG_CHARAC_DATA IDA ABI size must stay 0x78");
static_assert(offsetof(TAG_CHARAC_DATA, skill_tree_index) == 0x25u,
              "TAG_CHARAC_DATA skill-tree offset must stay 0x25");
static_assert(offsetof(TAG_CHARAC_DATA, db_stats) == 0x26u,
              "TAG_CHARAC_DATA DB stat offset must stay 0x26");
static_assert(sizeof(SIG_LOAD_CHARAC) == 0x740u,
              "SIG_LOAD_CHARAC target ABI size must stay 0x740");
static_assert(offsetof(SIG_LOAD_CHARAC, charac_data) == 0x00Cu,
              "SIG_LOAD_CHARAC character data offset must stay 0x0C");
static_assert(offsetof(SIG_LOAD_CHARAC, tag_charac_data) == 0x6C8u,
              "SIG_LOAD_CHARAC tag data offset must stay 0x6C8");


#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_LOAD_CHARAC_PAYLOAD_H_
