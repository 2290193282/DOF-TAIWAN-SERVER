#ifndef OLDSRV_GLOBAL_MONSTER_SCRIPT_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_MONSTER_SCRIPT_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA unit: 99_misc_global/MonsterBaseParameter.
// IDA_EA: clear @0x8A3CE4A, ctor @0x8A3CF2C, dtor @0x8A4AF16.
struct MonsterBaseParameter {
    MonsterBaseParameter();
    ~MonsterBaseParameter();

    MonsterBaseParameter* clear();

    int32_t monster_level;
    int32_t hp_max;
    int32_t mp_max;
    int32_t physical_attack;
    int32_t physical_defense;
    int32_t magical_attack;
    int32_t magical_defense;
    int32_t equipment_physical_attack;
    int32_t equipment_physical_defense;
    int32_t equipment_magical_attack;
    int32_t equipment_magical_defense;
    int32_t fractional_combat_bonus_x100;
    int32_t stuck_resistance_bonus_x10;
    // Old table order: fire, water, dark, light.
    int32_t element_tolerance[4];
    int32_t element_attack[4];
    int32_t hit_recovery;
    int32_t rigidity;
    int32_t active_status_resistance;
};

static_assert(sizeof(MonsterBaseParameter) == 0x60u,
              "MonsterBaseParameter IDA ABI size must stay 0x60");
static_assert(offsetof(MonsterBaseParameter, hp_max) == 0x04u &&
                  offsetof(MonsterBaseParameter, physical_attack) == 0x0cu &&
                  offsetof(MonsterBaseParameter, element_tolerance) == 0x34u &&
                  offsetof(MonsterBaseParameter, element_attack) == 0x44u &&
                  offsetof(MonsterBaseParameter, hit_recovery) == 0x54u,
              "MonsterBaseParameter named ABI changed");

// IDA unit: 99_misc_global/stLimitSlowDown.
// IDA_EA: clear @0x8A3DF02, ctor @0x8A3DF1A, dtor @0x8A4BADC.
struct stLimitSlowDown {
    stLimitSlowDown();
    ~stLimitSlowDown();

    stLimitSlowDown* clear();

    int32_t raw_00;
    int32_t raw_04;
};

static_assert(sizeof(stLimitSlowDown) == 0x08u,
              "stLimitSlowDown IDA ABI size must stay 0x08");

extern std::vector<std::vector<MonsterBaseParameter> > g_monsterBaseParameters
    __asm__("g_monsterBaseParameters");
extern float DIFFICULTY_OF_DUNGEON_BONUS[13 * 5]
    __asm__("DIFFICULTY_OF_DUNGEON_BONUS");
extern float DEATH_TOWER_BONUS[13 * 4] __asm__("DEATH_TOWER_BONUS");
extern float BLOOD_SYSTEM_BONUS[13 * 5] __asm__("BLOOD_SYSTEM_BONUS");
extern std::vector<std::vector<float> > MONSTER_DIFFICULTY_BONUS[5]
    __asm__("MONSTER_DIFFICULTY_BONUS");
extern std::vector<DungeonPartyBalance> DUNGEON_PARTY_BALANCE_TABLE
    __asm__("DUNGEON_PARTY_BALANCE_TABLE");
extern std::vector<DungeonHoldBalance> NORMAL_MONSTER_HOLD_BALANCE_TABLE
    __asm__("NORMAL_MONSTER_HOLD_BALANCE_TABLE");
extern std::vector<DungeonHoldBalance> NAMED_MONSTER_HOLD_BALANCE_TABLE
    __asm__("NAMED_MONSTER_HOLD_BALANCE_TABLE");
extern std::vector<DungeonHoldBalance> BOSS_MONSTER_HOLD_BALANCE_TABLE
    __asm__("BOSS_MONSTER_HOLD_BALANCE_TABLE");
extern std::vector<stLimitSlowDown> NORMAL_MONSER_LIMIT_SLOWDOWN_TABLE
    __asm__("NORMAL_MONSER_LIMIT_SLOWDOWN_TABLE");
extern std::vector<stLimitSlowDown> NAMED_MONSER_LIMIT_SLOWDOWN_TABLE
    __asm__("NAMED_MONSER_LIMIT_SLOWDOWN_TABLE");
extern std::vector<stLimitSlowDown> BOSS_MONSER_LIMIT_SLOWDOWN_TABLE
    __asm__("BOSS_MONSER_LIMIT_SLOWDOWN_TABLE");
extern float NORMAL_MONSTER_GAUGE_ADJUST
    __asm__("NORMAL_MONSTER_GAUGE_ADJUST");
extern float NAMED_MONSTER_GAUGE_ADJUST
    __asm__("NAMED_MONSTER_GAUGE_ADJUST");
extern float BOSS_MONSTER_GAUGE_ADJUST
    __asm__("BOSS_MONSTER_GAUGE_ADJUST");
extern const char* const MONSTER_BASIS_PARAMETER_TABLE_NAMES[5]
    __asm__("MONSTER_BASIS_PARAMETER_TABLE_NAMES");

// IDA_EA: 0x8A11311
int InitMonsterScript(const char* src,
                      const char* list_file,
                      const char* bonus_table)
    __asm__("_Z17InitMonsterScriptPKcS0_S0_");


extern char g_szMonsterScriptBaseDirectory[256]
    __asm__("g_szMonsterScriptBaseDirectory");


#endif  // OLDSRV_GLOBAL_MONSTER_SCRIPT_PUBLIC_ABI_H_
