#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_SDC_DAMAGE_COMPUTE_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_SDC_DAMAGE_COMPUTE_H_

#include <stddef.h>
#include <stdint.h>

// IDA_EA: 0x8095193..0x80966F1.
//
// The client sends the first 0x1f4 bytes.  The old CMD183 dispatcher places
// its second, compressed 500-byte buffer immediately after that destination;
// computeHpDamageSDC intentionally consumes +0x1f4/+0x1f8 from that adjacent
// storage.  The complete calculation view is therefore 0x1fc bytes, while
// the wire payload remains exactly 0x1f4 bytes.
#pragma pack(push, 1)
struct STComputeDamageVariable {
    float base_damage;                         // +0x000
    uint8_t use_ability_bonus;                 // +0x004
    uint8_t attack_type;                       // +0x005 (0 physical, 1 magical, 2 fixed)
    float attack_rate_percent;                 // +0x006
    uint8_t attack_element_enabled[4];         // +0x00a
    uint8_t suppress_defense_element;          // +0x00e
    float skill_base_damage;                   // +0x00f
    float critical_rate;                       // +0x013
    float additional_critical_rate;            // +0x017
    float final_damage_percent;                // +0x01b
    float conditional_rate_1;                  // +0x01f
    float conditional_rate_2;                  // +0x023
    float conditional_rate_3;                  // +0x027
    float creature_damage_multiplier;          // +0x02b
    float physical_ability_bonus;              // +0x02f
    float magical_ability_bonus;               // +0x033
    float physical_attack_percent;             // +0x037
    float magical_attack_percent;              // +0x03b
    float physical_attack_power;               // +0x03f
    float magical_attack_power;                // +0x043
    float collision_damage_multiplier;         // +0x047
    float physical_critical_rate;              // +0x04b
    float magical_critical_rate;               // +0x04f
    float physical_extra_critical_rate;        // +0x053
    float magical_extra_critical_rate;         // +0x057
    float creature_critical_rate;              // +0x05b
    float target_defense_rate;                 // +0x05f
    int32_t physical_ability;                  // +0x063
    int32_t magical_ability;                   // +0x067
    float final_damage_multiplier;             // +0x06b
    float critical_damage_multiplier;          // +0x06f
    float critical_damage_add;                 // +0x073
    float actor_damage_multiplier;             // +0x077
    int32_t actor_damage_add;                  // +0x07b
    int32_t element_resistance[4];             // +0x07f
    float element_resistance_rate;             // +0x08f
    int32_t physical_defense;                  // +0x093
    int32_t physical_defense_percent;          // +0x097
    float physical_defense_reduction;          // +0x09b
    int32_t magical_defense;                   // +0x09f
    int32_t magical_defense_percent;           // +0x0a3
    float magical_defense_reduction;           // +0x0a7
    float physical_defense_ability;            // +0x0ab
    float magical_defense_ability;             // +0x0af
    float ability_level_rate;                  // +0x0b3
    int32_t physical_defense_add;              // +0x0b7
    int32_t magical_defense_add;               // +0x0bb
    float physical_damage_reduction;           // +0x0bf
    float magical_damage_reduction;            // +0x0c3
    int32_t physical_damage_add;               // +0x0c7
    int32_t magical_damage_add;                // +0x0cb
    float target_critical_resistance;          // +0x0cf
    uint8_t conditional_rate_enabled[3];       // +0x0d3
    float critical_damage_rate[2][26];         // +0x0d6 (normal, critical)
    float post_damage_multiplier;              // +0x1a6
    double creature_damage_basis;              // +0x1aa
    int8_t module_type;                        // +0x1b2
    int8_t attacker_level;                     // +0x1b3
    int8_t alternate_attacker_level;           // +0x1b4
    uint8_t has_creature_damage;               // +0x1b5
    uint8_t use_creature_damage;               // +0x1b6
    int8_t creature_level;                     // +0x1b7
    int8_t creature_level_bonus;               // +0x1b8
    int8_t creature_index;                     // +0x1b9
    uint32_t source_object_type_mask;          // +0x1ba
    uint32_t actor_object_type_mask;           // +0x1be
    uint32_t target_object_type_mask;          // +0x1c2
    uint32_t damage_rate_object_type_mask;     // +0x1c6
    int32_t monster_unique_id;                 // +0x1ca
    uint8_t is_critical;                       // +0x1ce
    uint8_t has_critical_bonus;                // +0x1cf
    int32_t critical_bonus_damage;             // +0x1d0
    uint8_t raw_1d4;                           // +0x1d4
    int32_t damage_percent;                    // +0x1d5
    uint32_t random_seed;                      // +0x1d9
    uint8_t attack_element;                    // +0x1dd
    uint8_t use_additional_critical;           // +0x1de
    uint8_t use_quarter_damage_bonus;          // +0x1df
    uint8_t use_fixed_damage_add;              // +0x1e0
    int32_t fixed_damage_add;                  // +0x1e1
    uint8_t double_defense_rate;               // +0x1e5
    int32_t computed_damage;                   // +0x1e6
    uint8_t raw_1ea[5];                        // +0x1ea
    uint8_t use_adjacent_multiplier;           // +0x1ef
    float adjacent_multiplier;                 // +0x1f0; last wire field
    float adjacent_damage_add;                 // +0x1f4; first float of compressed buffer
    float adjacent_damage_percent;             // +0x1f8; second float of compressed buffer
};
#pragma pack(pop)

// IDA_EA: 0x808BD3F
// IDA_NAME: _Z8SDC_RandPj
int SDC_Rand(uint32_t* state);

// IDA_EA: 0x8095C32
// IDA_NAME: _Z18computeHpDamageSDCP23STComputeDamageVariablePvS1_
int computeHpDamageSDC(STComputeDamageVariable* variable,
                       void* unused_a,
                       void* unused_b);

static_assert(offsetof(STComputeDamageVariable, monster_unique_id) == 0x1cau,
              "CMD183 monster id ABI offset");
static_assert(offsetof(STComputeDamageVariable, computed_damage) == 0x1e6u,
              "CMD183 client/computed damage ABI offset");
static_assert(offsetof(STComputeDamageVariable, adjacent_multiplier) == 0x1f0u,
              "CMD183 wire payload must end at 0x1f4");
static_assert(offsetof(STComputeDamageVariable, adjacent_damage_add) == 0x1f4u,
              "CMD183 adjacent compressed-buffer ABI offset");
static_assert(sizeof(STComputeDamageVariable) == 0x1fcu,
              "CMD183 full calculation view ABI size");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_SDC_DAMAGE_COMPUTE_H_
