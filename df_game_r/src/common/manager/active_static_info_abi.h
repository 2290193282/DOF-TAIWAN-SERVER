#ifndef OLDSRV_GLOBAL_ACTIVE_STATIC_INFO_ABI_H_
#define OLDSRV_GLOBAL_ACTIVE_STATIC_INFO_ABI_H_

#include <stdint.h>
#include <stddef.h>

struct ActiveStaticValueRate {
    int32_t value;
    float rate;
};

// The four element slots use the old server order: fire, water, dark, light.
// They are grouped because the 0627 script loader and monster ability code
// iterate the four real slots as one physical table.
struct ActiveStaticElementStats {
    int32_t tolerance[4];
    float tolerance_rate[4];
    int32_t attack[4];
    float attack_rate[4];
};

// The 17 slots use the order frozen by STMonsterScript's 0627 tag table:
// slow, freeze, poison, stun, curse, blind, lightning, stone, sleep, burn,
// weapon-break, bleeding, piercing, stuck, confuse, hold, armor-break.
struct ActiveStaticStatusStats {
    int32_t resistance[17];
    float resistance_rate[17];
};

// Dwords 64..95 are real value/rate pairs.  Naming the whole physical block
// prevents callers from treating ActiveStaticInfo as a flat dword array.
struct ActiveStaticMotionStats {
    ActiveStaticValueRate general_recovery;
    ActiveStaticValueRate hp_regeneration;
    ActiveStaticValueRate mp_regeneration;
    ActiveStaticValueRate move_speed;
    ActiveStaticValueRate attack_speed;
    ActiveStaticValueRate cast_speed;
    ActiveStaticValueRate hit_recovery;
    ActiveStaticValueRate rigidity;
    ActiveStaticValueRate jump_power;
    ActiveStaticValueRate jump_speed;
    ActiveStaticValueRate weight;
    ActiveStaticValueRate equipment_physical_attack;
    ActiveStaticValueRate equipment_physical_defense;
    ActiveStaticValueRate equipment_magical_attack;
    ActiveStaticValueRate equipment_magical_defense;
    ActiveStaticValueRate auxiliary_combat;
};

// The three 18-entry tables are the physical active-status effect channels
// initialized together by ActiveStaticInfo::clear().
struct ActiveStaticStatusEffectStats {
    int32_t level[18];
    int32_t duration[18];
    int32_t probability[18];
};

// IDA unit: 99_misc_global/ActiveStaticInfo.
// Every physical region used by the 0627 monster runtime is represented by a
// named member or a named homogeneous sub-table.  There is deliberately no
// flat words/raw storage view.
struct ActiveStaticInfo {
    ActiveStaticInfo();
    ActiveStaticInfo(const ActiveStaticInfo& other);
    ~ActiveStaticInfo();

    ActiveStaticInfo& operator=(const ActiveStaticInfo& other);

    ActiveStaticInfo* clear();
    int getHpMax();
    int getMpMax();

    int32_t initial_state;
    int32_t hp_max_base;
    float hp_max_rate;
    int32_t mp_max_base;
    float mp_max_rate;
    uint8_t primary_state_enabled;
    uint8_t primary_state_auxiliary_a;
    uint8_t primary_state_auxiliary_b;
    uint8_t primary_state_auxiliary_c;

    ActiveStaticValueRate physical_attack;
    ActiveStaticValueRate physical_defense;
    ActiveStaticValueRate magical_attack;
    ActiveStaticValueRate magical_defense;
    ActiveStaticElementStats elements;
    ActiveStaticStatusStats active_status;
    ActiveStaticMotionStats motion;

    // Dwords 96..119 are distinct post-motion combat channels.  The one
    // fractional value at dword 106 is consumed by the monster base table.
    int32_t combat_adjustments[10];
    float base_table_fractional_bonus;
    int32_t combat_modes[3];
    float combat_adjustment_rate;
    int32_t behavior_adjustments[4];
    float behavior_adjustment_rate;
    int32_t attack_cancel_value;
    int32_t attack_cancel_state;
    float attack_cancel_rate;
    int32_t attack_cancel_count;

    ActiveStaticStatusEffectStats status_effects;
    int32_t terminal_adjustments[9];
    float terminal_primary_rate;
    float terminal_secondary_rate;
    int32_t terminal_primary_flag;
    int32_t terminal_secondary_flag;
    float terminal_final_rate;
};

extern int g_ActiveStaticInfoCount;
extern int g_ActiveStaticInfoSize;

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ActiveStaticValueRate) == 0x08u,
              "ActiveStaticValueRate ABI size must stay 0x08");
static_assert(sizeof(ActiveStaticElementStats) == 0x40u,
              "ActiveStaticElementStats ABI size must stay 0x40");
static_assert(sizeof(ActiveStaticStatusStats) == 0x88u,
              "ActiveStaticStatusStats ABI size must stay 0x88");
static_assert(sizeof(ActiveStaticMotionStats) == 0x80u,
              "ActiveStaticMotionStats ABI size must stay 0x80");
static_assert(sizeof(ActiveStaticStatusEffectStats) == 0xD8u,
              "ActiveStaticStatusEffectStats ABI size must stay 0xD8");
static_assert(sizeof(ActiveStaticInfo) == 0x2F0u,
              "ActiveStaticInfo IDA ABI size must stay 0x2F0");
static_assert(offsetof(ActiveStaticInfo, hp_max_base) == 0x04u &&
                  offsetof(ActiveStaticInfo, hp_max_rate) == 0x08u &&
                  offsetof(ActiveStaticInfo, mp_max_base) == 0x0cu &&
                  offsetof(ActiveStaticInfo, mp_max_rate) == 0x10u &&
                  offsetof(ActiveStaticInfo, primary_state_enabled) == 0x14u &&
                  offsetof(ActiveStaticInfo, physical_attack) == 0x18u &&
                  offsetof(ActiveStaticInfo, elements) == 0x38u &&
                  offsetof(ActiveStaticInfo, active_status) == 0x78u &&
                  offsetof(ActiveStaticInfo, motion) == 0x100u &&
                  offsetof(ActiveStaticInfo, combat_adjustments) == 0x180u &&
                  offsetof(ActiveStaticInfo, base_table_fractional_bonus) ==
                      0x1a8u &&
                  offsetof(ActiveStaticInfo, status_effects) == 0x1e0u &&
                  offsetof(ActiveStaticInfo, terminal_adjustments) == 0x2b8u &&
                  offsetof(ActiveStaticInfo, terminal_final_rate) == 0x2ecu,
              "ActiveStaticInfo named prefix ABI changed");
#endif

#endif  // OLDSRV_GLOBAL_ACTIVE_STATIC_INFO_ABI_H_
