#ifndef OLDSRV_DB_DB_SAVE_CHARAC_H_
#define OLDSRV_DB_DB_SAVE_CHARAC_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"

// IDA wire body consumed by DB_SaveCharac @0x8415C98.  This is a packed DB
// signal record, not a projection over CUser or _Charac_info.
#pragma pack(push, 1)
struct SIG_SAVE_CHARAC_ADDITIONAL_INFO {
    uint32_t max_hp;                  // +0x00
    uint32_t max_mp;                  // +0x04
    uint16_t physical_attack;         // +0x08
    uint16_t physical_defense;        // +0x0a
    uint16_t magical_attack;          // +0x0c
    uint16_t magical_defense;         // +0x0e
    uint16_t element_resist[4];       // +0x10
    uint16_t spec_property[17];       // +0x18
    int32_t inventory_weight;         // +0x3a
    int16_t hp_regen_speed;           // +0x3e
    int16_t mp_regen_speed;           // +0x40
    uint32_t move_speed;              // +0x42
    uint16_t attack_speed;            // +0x46
    uint16_t cast_speed;              // +0x48
    int16_t hit_recovery;             // +0x4a
    int16_t jump_power;               // +0x4c
    int32_t character_weight;         // +0x4e
};

struct SIG_SAVE_CHARAC {
    uint32_t charac_no;                       // +0x000
    uint8_t save_charac_info;                 // +0x004
    uint8_t save_charac_stat;                 // +0x005
    uint8_t save_charac_etc;                  // +0x006
    uint8_t save_charac_manage_info;          // +0x007
    int16_t character_level;                  // +0x008
    int8_t grow_type;                         // +0x00a
    SIG_SAVE_CHARAC_ADDITIONAL_INFO stats;    // +0x00b
    int8_t expert_job_type;                   // +0x05d
    int32_t experience;                       // +0x05e
    int8_t village;                           // +0x062
    int16_t fatigue;                          // +0x063
    uint16_t premium_fatigue;                 // +0x065
    uint8_t stamina;                          // +0x067
    uint16_t used_fatigue;                    // +0x068
    int32_t tutorial_flag;                    // +0x06a
    uint8_t escalade_tutorial_flag[16];       // +0x06e
    int32_t trade_gold_total;                 // +0x07e
    int32_t trade_gold_total_billion;         // +0x082
    int32_t trade_gold_daily;                 // +0x086
    int32_t help_abuse_ratio;                 // +0x08a
    int32_t help_abuse_experience;            // +0x08e
    int32_t chaos_point;                      // +0x092
    int32_t chaos_experience;                 // +0x096
    int32_t chaos_mode_count;                 // +0x09a
    int32_t chaos_kill_count;                 // +0x09e
    int32_t chaos_die_count;                  // +0x0a2
    int32_t chaos_die_time;                   // +0x0a6
    int8_t open_coin_flag;                    // +0x0aa
    int32_t assault_count;                    // +0x0ab
    int32_t chaos_kill_time;                  // +0x0af
    int32_t luck_point;                       // +0x0b3
    int32_t dungeon_play_count;               // +0x0b7
    int32_t expert_job_experience;            // +0x0bb
    int16_t fatigue_battery_charging;         // +0x0bf
    int16_t power_war_point;                  // +0x0c1
    int8_t previous_village;                  // +0x0c3
    int32_t power_war_assault_count;          // +0x0c4
    int32_t power_war_victory_count;          // +0x0c8
    uint16_t fatigue_grownup_buff;            // +0x0cc
    uint8_t member_dungeon_flag;              // +0x0ce
    int16_t member_bonus_fatigue;             // +0x0cf
    uint32_t last_play_dungeon_index;         // +0x0d1
    uint8_t growth_power_reward;              // +0x0d5
    int32_t chaos_response_time;              // +0x0d6
    uint8_t guild_right;                      // +0x0da
    uint16_t maximum_premium_fatigue;         // +0x0db
    // DB column event_charac_level carries the event grow-type code.
    int8_t event_character_grow_type;         // +0x0dd
    uint32_t last_play_time;                  // +0x0de
    uint32_t last_play_time_power_war;        // +0x0e2
    uint32_t account_id;                      // +0x0e6
    int32_t report_mannerless_count;           // +0x0ea
    int32_t over_trade_gold_count;             // +0x0ee
    int32_t dungeon_gained_gold;               // +0x0f2
    float rating;                              // +0x0f6
    int8_t tutorial_skipable;                  // +0x0fa
    int16_t emotion;                           // +0x0fb
    int8_t skill_tree_index;                   // +0x0fd
    int8_t expanded_slot_flag;                 // +0x0fe
    int8_t equipslot_flag;                     // +0x0ff
    int8_t channel_equipslot_switch;           // +0x100
    int8_t expand_equipslot_switch;            // +0x101
    int8_t visible_flags;                      // +0x102
    int8_t striker_skill_index;                // +0x103
    uint32_t tag_charac_no;                    // +0x104
    uint16_t maximum_equip_level;              // +0x108
    uint8_t save_cube_premium_info;            // +0x10a
    uint8_t cube_premium_selected;             // +0x10b
    uint8_t cube_premium_type;                 // +0x10c
};
#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_CHARAC_ADDITIONAL_INFO) == 0x52u,
              "SIG_SAVE_CHARAC additional-info ABI drift");
static_assert(offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO, element_resist) ==
                  0x10u &&
                  offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO, spec_property) ==
                      0x18u &&
                  offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO,
                           inventory_weight) == 0x3au &&
                  offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO,
                           hp_regen_speed) == 0x3eu &&
                  offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO, move_speed) ==
                      0x42u &&
                  offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO, attack_speed) ==
                      0x46u &&
                  offsetof(SIG_SAVE_CHARAC_ADDITIONAL_INFO,
                           character_weight) == 0x4eu,
              "SIG_SAVE_CHARAC additional-info field offsets moved");
static_assert(sizeof(SIG_SAVE_CHARAC) == 0x10du,
              "SIG_SAVE_CHARAC must retain the old 269-byte wire extent");
static_assert(offsetof(SIG_SAVE_CHARAC, save_charac_info) == 0x004u &&
                  offsetof(SIG_SAVE_CHARAC, save_charac_stat) == 0x005u &&
                  offsetof(SIG_SAVE_CHARAC, save_charac_etc) == 0x006u &&
                  offsetof(SIG_SAVE_CHARAC, save_charac_manage_info) ==
                      0x007u &&
                  offsetof(SIG_SAVE_CHARAC, character_level) == 0x008u &&
                  offsetof(SIG_SAVE_CHARAC, grow_type) == 0x00au &&
                  offsetof(SIG_SAVE_CHARAC, stats) == 0x00bu &&
                  offsetof(SIG_SAVE_CHARAC, expert_job_type) == 0x05du,
              "SIG_SAVE_CHARAC character-info field offsets moved");
static_assert(offsetof(SIG_SAVE_CHARAC, experience) == 0x05eu &&
                  offsetof(SIG_SAVE_CHARAC, village) == 0x062u &&
                  offsetof(SIG_SAVE_CHARAC, fatigue) == 0x063u &&
                  offsetof(SIG_SAVE_CHARAC, premium_fatigue) == 0x065u &&
                  offsetof(SIG_SAVE_CHARAC, stamina) == 0x067u &&
                  offsetof(SIG_SAVE_CHARAC, used_fatigue) == 0x68u &&
                  offsetof(SIG_SAVE_CHARAC, tutorial_flag) == 0x06au &&
                  offsetof(SIG_SAVE_CHARAC, escalade_tutorial_flag) ==
                      0x06eu &&
                  offsetof(SIG_SAVE_CHARAC, trade_gold_total) == 0x07eu &&
                  offsetof(SIG_SAVE_CHARAC, trade_gold_total_billion) ==
                      0x082u &&
                  offsetof(SIG_SAVE_CHARAC, trade_gold_daily) == 0x086u,
              "SIG_SAVE_CHARAC primary-stat field offsets moved");
static_assert(offsetof(SIG_SAVE_CHARAC, help_abuse_ratio) == 0x08au &&
                  offsetof(SIG_SAVE_CHARAC, help_abuse_experience) ==
                      0x08eu &&
                  offsetof(SIG_SAVE_CHARAC, chaos_point) == 0x092u &&
                  offsetof(SIG_SAVE_CHARAC, chaos_experience) == 0x096u &&
                  offsetof(SIG_SAVE_CHARAC, chaos_mode_count) == 0x09au &&
                  offsetof(SIG_SAVE_CHARAC, chaos_kill_count) == 0x09eu &&
                  offsetof(SIG_SAVE_CHARAC, chaos_die_count) == 0x0a2u &&
                  offsetof(SIG_SAVE_CHARAC, chaos_die_time) == 0x0a6u &&
                  offsetof(SIG_SAVE_CHARAC, open_coin_flag) == 0x0aau &&
                  offsetof(SIG_SAVE_CHARAC, assault_count) == 0x0abu &&
                  offsetof(SIG_SAVE_CHARAC, chaos_kill_time) == 0x0afu,
              "SIG_SAVE_CHARAC dungeon-stat field offsets moved");
static_assert(offsetof(SIG_SAVE_CHARAC, luck_point) == 0x0b3u &&
                  offsetof(SIG_SAVE_CHARAC, dungeon_play_count) == 0x0b7u &&
                  offsetof(SIG_SAVE_CHARAC, expert_job_experience) ==
                      0x0bbu &&
                  offsetof(SIG_SAVE_CHARAC, fatigue_battery_charging) ==
                      0x0bfu &&
                  offsetof(SIG_SAVE_CHARAC, power_war_point) == 0x0c1u &&
                  offsetof(SIG_SAVE_CHARAC, previous_village) == 0x0c3u &&
                  offsetof(SIG_SAVE_CHARAC, power_war_assault_count) ==
                      0x0c4u &&
                  offsetof(SIG_SAVE_CHARAC, power_war_victory_count) ==
                      0x0c8u &&
                  offsetof(SIG_SAVE_CHARAC, fatigue_grownup_buff) == 0x0ccu &&
                  offsetof(SIG_SAVE_CHARAC, member_dungeon_flag) == 0x0ceu &&
                  offsetof(SIG_SAVE_CHARAC, member_bonus_fatigue) == 0x0cfu,
              "SIG_SAVE_CHARAC extended-stat field offsets moved");
static_assert(offsetof(SIG_SAVE_CHARAC, last_play_dungeon_index) == 0x0d1u &&
                  offsetof(SIG_SAVE_CHARAC, growth_power_reward) == 0x0d5u &&
                  offsetof(SIG_SAVE_CHARAC, chaos_response_time) == 0x0d6u &&
                  offsetof(SIG_SAVE_CHARAC, guild_right) == 0x0dau &&
                  offsetof(SIG_SAVE_CHARAC, maximum_premium_fatigue) ==
                      0x0dbu &&
                  offsetof(SIG_SAVE_CHARAC, event_character_grow_type) ==
                      0x0ddu &&
                  offsetof(SIG_SAVE_CHARAC, last_play_time) == 0x0deu &&
                  offsetof(SIG_SAVE_CHARAC, last_play_time_power_war) ==
                      0x0e2u &&
                  offsetof(SIG_SAVE_CHARAC, account_id) == 0xe6u &&
                  offsetof(SIG_SAVE_CHARAC, report_mannerless_count) ==
                      0x0eau &&
                  offsetof(SIG_SAVE_CHARAC, over_trade_gold_count) == 0x0eeu &&
                  offsetof(SIG_SAVE_CHARAC, dungeon_gained_gold) == 0x0f2u &&
                  offsetof(SIG_SAVE_CHARAC, rating) == 0xf6u &&
                  offsetof(SIG_SAVE_CHARAC, tutorial_skipable) == 0x0fau,
              "SIG_SAVE_CHARAC account-state field offsets moved");
static_assert(offsetof(SIG_SAVE_CHARAC, emotion) == 0x0fbu &&
                  offsetof(SIG_SAVE_CHARAC, skill_tree_index) == 0x0fdu &&
                  offsetof(SIG_SAVE_CHARAC, expanded_slot_flag) == 0x0feu &&
                  offsetof(SIG_SAVE_CHARAC, equipslot_flag) == 0x0ffu &&
                  offsetof(SIG_SAVE_CHARAC, channel_equipslot_switch) ==
                      0x100u &&
                  offsetof(SIG_SAVE_CHARAC, expand_equipslot_switch) ==
                      0x101u &&
                  offsetof(SIG_SAVE_CHARAC, visible_flags) == 0x102u &&
                  offsetof(SIG_SAVE_CHARAC, striker_skill_index) == 0x103u &&
                  offsetof(SIG_SAVE_CHARAC, tag_charac_no) == 0x104u &&
                  offsetof(SIG_SAVE_CHARAC, maximum_equip_level) == 0x108u &&
                  offsetof(SIG_SAVE_CHARAC, save_cube_premium_info) ==
                      0x10au &&
                  offsetof(SIG_SAVE_CHARAC, cube_premium_selected) == 0x10bu &&
                  offsetof(SIG_SAVE_CHARAC, cube_premium_type) == 0x10cu,
              "SIG_SAVE_CHARAC tail field offsets moved");


// IDA unit: 13_database_dispatch/DB_SaveCharac.
class DB_SaveCharac : public DBDispatcher {
public:
    DB_SaveCharac();
    virtual ~DB_SaveCharac();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    bool _SaveContractOfCubePremiumInfo(const SIG_SAVE_CHARAC& payload);
    bool _SaveCharacInfo(SIG_SAVE_CHARAC* payload);
    bool _SaveCharacStat(SIG_SAVE_CHARAC* payload);
    bool SendResult(int signal, int user_idx, SIG_SAVE_CHARAC* payload);
    bool _SaveCharacManageInfo(SIG_SAVE_CHARAC* payload);
};

#endif  // OLDSRV_DB_DB_SAVE_CHARAC_H_
