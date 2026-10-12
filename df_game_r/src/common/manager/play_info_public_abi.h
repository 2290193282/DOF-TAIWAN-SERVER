#ifndef OLDSRV_GLOBAL_PLAY_INFO_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_PLAY_INFO_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>


// IDA unit: 99_misc_global/PlayInfo. Old i386 size: 0x3c.
struct PlayInfo {
    PlayInfo();

    PlayInfo* Reset();
    PlayInfo& operator+=(const PlayInfo& rhs);

    uint32_t clear_time_seconds_;             // +0x00
    uint32_t die_count_;                      // +0x04
    uint32_t hp_consume_;                     // +0x08
    uint32_t mp_consume_;                     // +0x0c
    uint32_t hit_count_;                      // +0x10
    uint32_t hit_per_average_damage_;         // +0x14
    uint32_t hp_recovery_;                    // +0x18
    uint32_t mp_recovery_;                    // +0x1c
    uint32_t party_member_count_;             // +0x20
    uint32_t character_level_;                // +0x24
    uint32_t dungeon_exp_;                    // +0x28
    uint8_t value_2c_;                        // +0x2c
    uint8_t alignment_2d_2f_[3];              // +0x2d
    uint32_t dungeon_bonus_exp_;              // +0x30
    uint32_t reason_one_or_two_exp_;          // +0x34
    uint8_t value_38_;                        // +0x38
    uint8_t alignment_39_3b_[3];              // +0x39
};

static_assert(sizeof(PlayInfo) == 0x3cu,
              "PlayInfo IDA ABI size must stay 0x3c");
static_assert(offsetof(PlayInfo, clear_time_seconds_) == 0x00u,
              "PlayInfo clear time offset must stay at +0x00");
static_assert(offsetof(PlayInfo, die_count_) == 0x04u,
              "PlayInfo die count offset must stay at +0x04");
static_assert(offsetof(PlayInfo, hp_consume_) == 0x08u,
              "PlayInfo HP consume offset must stay at +0x08");
static_assert(offsetof(PlayInfo, mp_consume_) == 0x0cu,
              "PlayInfo MP consume offset must stay at +0x0c");
static_assert(offsetof(PlayInfo, hit_count_) == 0x10u,
              "PlayInfo hit count offset must stay at +0x10");
static_assert(offsetof(PlayInfo, hit_per_average_damage_) == 0x14u,
              "PlayInfo average hit damage offset must stay at +0x14");
static_assert(offsetof(PlayInfo, hp_recovery_) == 0x18u,
              "PlayInfo HP recovery offset must stay at +0x18");
static_assert(offsetof(PlayInfo, mp_recovery_) == 0x1cu,
              "PlayInfo MP recovery offset must stay at +0x1c");
static_assert(offsetof(PlayInfo, party_member_count_) == 0x20u,
              "PlayInfo party member count offset must stay at +0x20");
static_assert(offsetof(PlayInfo, character_level_) == 0x24u,
              "PlayInfo character level offset must stay at +0x24");
static_assert(offsetof(PlayInfo, dungeon_exp_) == 0x28u,
              "PlayInfo dungeon EXP offset must stay at +0x28");
static_assert(offsetof(PlayInfo, value_2c_) == 0x2cu,
              "PlayInfo byte value offset must stay at +0x2c");
static_assert(offsetof(PlayInfo, dungeon_bonus_exp_) == 0x30u,
              "PlayInfo dungeon bonus EXP offset must stay at +0x30");
static_assert(offsetof(PlayInfo, reason_one_or_two_exp_) == 0x34u,
              "PlayInfo reason EXP offset must stay at +0x34");
static_assert(offsetof(PlayInfo, value_38_) == 0x38u,
              "PlayInfo final byte value offset must stay at +0x38");

// IDA unit: 99_misc_global/IData. Marker base object with empty constructor.
struct IData {
    IData();
};

// IDA unit: 99_misc_global/_Additioal_info. Old i386 size: 0x52.
#pragma pack(push, 1)
struct _Additioal_info {
    _Additioal_info();

    void* clear();
    _Additioal_info* add(const _Additioal_info& rhs);
    _Additioal_info& operator+=(const _Additioal_info& rhs);
    _Additioal_info* subtract(const _Additioal_info& rhs);

    uint32_t max_hp;
    uint32_t max_mp;
    uint16_t physical_attack;
    uint16_t physical_defense;
    uint16_t magical_attack;
    uint16_t magical_defense;
    uint16_t element_resist[4];
    uint16_t spec_property[17];
    int32_t inventory_weight;
    int16_t hp_regen_speed;
    int16_t mp_regen_speed;
    uint32_t move_speed;
    uint16_t attack_speed;
    uint16_t cast_speed;
    int16_t hit_recovery;
    int16_t jump_power;
    int32_t character_weight;
};
#pragma pack(pop)

static_assert(sizeof(_Additioal_info) == 0x52u,
              "_Additioal_info IDA ABI size must stay 0x52");
static_assert(alignof(_Additioal_info) == 0x01u,
              "_Additioal_info IDA ABI alignment must stay 1");
static_assert(offsetof(_Additioal_info, max_hp) == 0x00u,
              "_Additioal_info::max_hp must stay at +0x00");
static_assert(offsetof(_Additioal_info, max_mp) == 0x04u,
              "_Additioal_info::max_mp must stay at +0x04");
static_assert(offsetof(_Additioal_info, physical_attack) == 0x08u,
              "_Additioal_info::physical_attack must stay at +0x08");
static_assert(offsetof(_Additioal_info, physical_defense) == 0x0Au,
              "_Additioal_info::physical_defense must stay at +0x0a");
static_assert(offsetof(_Additioal_info, magical_attack) == 0x0Cu,
              "_Additioal_info::magical_attack must stay at +0x0c");
static_assert(offsetof(_Additioal_info, magical_defense) == 0x0Eu,
              "_Additioal_info::magical_defense must stay at +0x0e");
static_assert(offsetof(_Additioal_info, element_resist) == 0x10u,
              "_Additioal_info::element_resist must stay at +0x10");
static_assert(offsetof(_Additioal_info, spec_property) == 0x18u,
              "_Additioal_info::spec_property must stay at +0x18");
static_assert(offsetof(_Additioal_info, inventory_weight) == 0x3Au,
              "_Additioal_info::inventory_weight must stay at +0x3a");
static_assert(offsetof(_Additioal_info, hp_regen_speed) == 0x3Eu,
              "_Additioal_info::hp_regen_speed must stay at +0x3e");
static_assert(offsetof(_Additioal_info, mp_regen_speed) == 0x40u,
              "_Additioal_info::mp_regen_speed must stay at +0x40");
static_assert(offsetof(_Additioal_info, move_speed) == 0x42u,
              "_Additioal_info::move_speed must stay at +0x42");
static_assert(offsetof(_Additioal_info, attack_speed) == 0x46u,
              "_Additioal_info::attack_speed must stay at +0x46");
static_assert(offsetof(_Additioal_info, cast_speed) == 0x48u,
              "_Additioal_info::cast_speed must stay at +0x48");
static_assert(offsetof(_Additioal_info, hit_recovery) == 0x4Au,
              "_Additioal_info::hit_recovery must stay at +0x4a");
static_assert(offsetof(_Additioal_info, jump_power) == 0x4Cu,
              "_Additioal_info::jump_power must stay at +0x4c");
static_assert(offsetof(_Additioal_info, character_weight) == 0x4Eu,
              "_Additioal_info::character_weight must stay at +0x4e");


#endif  // OLDSRV_GLOBAL_PLAY_INFO_PUBLIC_ABI_H_
