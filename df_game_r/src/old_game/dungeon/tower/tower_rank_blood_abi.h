#ifndef OLDSRV_GLOBAL_TOWER_RANK_BLOOD_ABI_H_
#define OLDSRV_GLOBAL_TOWER_RANK_BLOOD_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "old_game/world/region/world_value_types.h"


struct STUltimateRewardProb;

// CDataManager old embedded-slot offsets used by the Tower/Rank/Blood PVF
// boundary. Runtime script population remains DataManager/PVF owner work.
// IDA unit: 05_world_enter_village/stClearTimeAverageMapKey. The active
// definition and old function bodies live in world_value_types; this public
// Tower/Rank ABI consumes the same key instead of defining a duplicate type.
static_assert(sizeof(stClearTimeAverageMapKey) == 0x08u,
              "stClearTimeAverageMapKey IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/stClearTimeAverage.
struct stClearTimeAverage {
    stClearTimeAverage();

    stClearTimeAverage* clear();

    int32_t values[5];
};

static_assert(sizeof(stClearTimeAverage) == 0x14u,
              "stClearTimeAverage IDA ABI size must stay 0x14");

// IDA unit: 99_misc_global/stBonusPointValues.
struct stBonusPointValues {
    stBonusPointValues();
    ~stBonusPointValues();

    stBonusPointValues* clear();

    int32_t threshold;
    int32_t bonus;
};

static_assert(sizeof(stBonusPointValues) == 0x08u,
              "stBonusPointValues IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/stRankSystemInfo plus nested value records used by
// the Tower/Rank public DataManager slots.
struct stRankSystemInfo {
    struct stDungeonClearTimeData {
        stDungeonClearTimeData();
        ~stDungeonClearTimeData();

        void* clear();

        int32_t values[7];
    };

    struct stComboIntervalBoundary {
        stComboIntervalBoundary();
        ~stComboIntervalBoundary();

        void* clear();

        int32_t values[4];
    };

    struct stDungeonRankSystem_t {
        stDungeonRankSystem_t();
        ~stDungeonRankSystem_t();

        void* clear();

        std::map<stClearTimeAverageMapKey, stClearTimeAverage>
            clear_time_averages;
        std::vector<stBonusPointValues> bonus_point_values;
        uint16_t rank_grade[9];
        uint8_t pad_36[2];
    };

    stRankSystemInfo();
    ~stRankSystemInfo();

    stRankSystemInfo* clear();

    stDungeonRankSystem_t dungeon_rank_system;
    std::vector<stDungeonClearTimeData> dungeon_clear_time_data;
    std::map<int, int> dungeon_clear_time_reduction_bonus;
    std::vector<int> rank_grade_values;
    std::vector<stComboIntervalBoundary> combo_interval_boundaries;
    int32_t combo_practice_bonus_interval;
};

static_assert(sizeof(stRankSystemInfo::stDungeonClearTimeData) == 0x1Cu,
              "stDungeonClearTimeData IDA ABI size must stay 0x1c");
static_assert(sizeof(stRankSystemInfo::stComboIntervalBoundary) == 0x10u,
              "stComboIntervalBoundary IDA ABI size must stay 0x10");
static_assert(sizeof(stRankSystemInfo::stDungeonRankSystem_t) == 0x38u,
              "stDungeonRankSystem_t IDA ABI size must stay 0x38");
static_assert(sizeof(stRankSystemInfo) == 0x78u,
              "stRankSystemInfo IDA ABI size must stay 0x78");

// IDA unit: 09_dungeon_pvp_party/stDeathTower_t. Public value ABI needed by
// CDataManager and importTowerInfo; dungeon behavior remains owner work.
struct stDeathTower_t {
    stDeathTower_t();

    float apc_exp_weight;
    float reward_exp_weight[200];
    int32_t reward_card_num[200];
    float illusion_reward_exp_weight[200];
    int32_t illusion_reward_card_num[200];
    float monster_exp_weight;
    int32_t reward_item_prob_cumulative[3];
    float reward_gold_weight;
};

static_assert(sizeof(stDeathTower_t) == 0xC98u,
              "stDeathTower_t IDA ABI size must stay 0xc98");
static_assert(offsetof(stDeathTower_t, apc_exp_weight) == 0x000u &&
                  offsetof(stDeathTower_t, reward_exp_weight) == 0x004u &&
                  offsetof(stDeathTower_t, reward_card_num) == 0x324u &&
                  offsetof(stDeathTower_t, illusion_reward_exp_weight) ==
                      0x644u &&
                  offsetof(stDeathTower_t, illusion_reward_card_num) ==
                      0x964u &&
                  offsetof(stDeathTower_t, monster_exp_weight) == 0xc84u &&
                  offsetof(stDeathTower_t, reward_item_prob_cumulative) ==
                      0xc88u &&
                  offsetof(stDeathTower_t, reward_gold_weight) == 0xc94u,
              "stDeathTower_t named fields moved from the 0627 ABI");

// IDA unit: 09_dungeon_pvp_party/stBloodDungeon_t. This consumes the active
// C-owned STUltimateRewardProb value without duplicating its definition.
struct stBloodDungeon_t {
    stBloodDungeon_t();
    ~stBloodDungeon_t();

    void clear();

    float reward_exp_weight[200];
    int32_t exp_standard_monster_count;
    int32_t reward_item_prob_cumulative[3];
    float reward_gold_weight;
    std::vector<float> reward_exp_inout_count_weight;
    // IDA importBloodDungeonScript writes dwords 1 and 2 at +0x344/+0x348;
    // CBloodClearRewardData indexes the same dword table from +0x340.
    int32_t ultimate_difficulty_points[6];
    std::map<int, STUltimateRewardProb> ultimate_reward_prob;
};

static_assert(sizeof(stBloodDungeon_t) == 0x370u,
              "stBloodDungeon_t IDA ABI size must stay 0x370");
static_assert(offsetof(stBloodDungeon_t, reward_exp_weight) == 0x000u &&
                  offsetof(stBloodDungeon_t, exp_standard_monster_count) ==
                      0x320u &&
                  offsetof(stBloodDungeon_t, reward_item_prob_cumulative) ==
                      0x324u &&
                  offsetof(stBloodDungeon_t, reward_gold_weight) == 0x330u &&
                  offsetof(stBloodDungeon_t,
                           reward_exp_inout_count_weight) == 0x334u &&
                  offsetof(stBloodDungeon_t, ultimate_difficulty_points) ==
                      0x340u &&
                  offsetof(stBloodDungeon_t, ultimate_reward_prob) == 0x358u,
              "stBloodDungeon_t named fields moved from the 0627 ABI");

// IDA_EA: 0x89089F3
// IDA_NAME: _Z22importDeathTowerScriptPKcR14stDeathTower_t
// DEMANGLED: importDeathTowerScript(char const*,stDeathTower_t &)
int importDeathTowerScript(const char* filename, stDeathTower_t& script);

// IDA_EA: 0x8909016
// IDA_NAME: _Z24importBloodDungeonScriptPKcR16stBloodDungeon_t
// DEMANGLED: importBloodDungeonScript(char const*,stBloodDungeon_t &)
int importBloodDungeonScript(const char* filename, stBloodDungeon_t& script);

// IDA_EA: 0x890B8D0
// IDA_NAME: _Z15importTowerInfoPKcS0_RSt3mapIj14stDeathTower_tSt4lessIjESaISt4pairIKjS2_EEE
// DEMANGLED: importTowerInfo(char const*,char const*,std::map<uint,stDeathTower_t> &)
int importTowerInfo(const char* dir,
                    const char* filename,
                    std::map<unsigned int, stDeathTower_t>& tower_info);

// IDA_EA: 0x890ED7F
// IDA_NAME: _Z22importRankSystemScriptPKcP16stRankSystemInfo
// DEMANGLED: importRankSystemScript(char const*,stRankSystemInfo *)
int importRankSystemScript(const char* filename, stRankSystemInfo* info);


#endif  // OLDSRV_GLOBAL_TOWER_RANK_BLOOD_ABI_H_
