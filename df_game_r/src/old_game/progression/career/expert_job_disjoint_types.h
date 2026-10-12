#ifndef OLDSRV_GLOBAL_EXPERT_JOB_DISJOINT_TYPES_H_
#define OLDSRV_GLOBAL_EXPERT_JOB_DISJOINT_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>


// IDA_EA: DisjointMachineGrade @0x896DBCC; stItemSelectRate @0x85D3B9E;
// stDisjointResult @0x896E758/0x896E7A4.
// IDA unit: 99_misc_global/DisjointMachineGrade.
struct DisjointMachineGrade {
    DisjointMachineGrade();
    DisjointMachineGrade(uint8_t rarity_value,
                         uint8_t disjoint_machine_grade_value,
                         uint8_t unidentified_value);

    bool operator<(const DisjointMachineGrade& other) const;

    uint8_t disjoint_machine_grade;  // +0x00
    uint8_t rarity;                  // +0x01
    uint8_t unidentified;            // +0x02
};

static_assert(sizeof(DisjointMachineGrade) == 0x03u,
              "DisjointMachineGrade IDA ABI size must stay 0x03");
static_assert(offsetof(DisjointMachineGrade, disjoint_machine_grade) == 0x00u,
              "DisjointMachineGrade grade offset must stay 0x00");
static_assert(offsetof(DisjointMachineGrade, rarity) == 0x01u,
              "DisjointMachineGrade rarity offset must stay 0x01");
static_assert(offsetof(DisjointMachineGrade, unidentified) == 0x02u,
              "DisjointMachineGrade unidentified offset must stay 0x02");

// IDA unit: 99_misc_global/stItemSelectRate.
struct stItemSelectRate {
    stItemSelectRate();
    ~stItemSelectRate();

    int32_t item_index;     // +0x00
    int32_t select_rate;    // +0x04
    float grade_divisor;    // +0x08
    int32_t min_grade;      // +0x0C
    int32_t max_grade;      // +0x10
};

static_assert(sizeof(stItemSelectRate) == 0x14u,
              "stItemSelectRate IDA ABI size must stay 0x14");
static_assert(offsetof(stItemSelectRate, grade_divisor) == 0x08u,
              "stItemSelectRate grade_divisor offset must stay 0x08");

// IDA unit: 99_misc_global/stDisjointResult.
struct stDisjointResult {
    stDisjointResult();
    ~stDisjointResult();
    stDisjointResult(const stDisjointResult& other);

    int32_t result_item_index;                       // +0x00
    float base_cost_rate;                            // +0x04
    std::vector<stItemSelectRate> additional_items;  // +0x08
    std::vector<stItemSelectRate> big_win_items;     // +0x14
    int32_t big_win_rate;                            // +0x20
};

static_assert(sizeof(stDisjointResult) == 0x24u,
              "stDisjointResult IDA ABI size must stay 0x24");
static_assert(offsetof(stDisjointResult, additional_items) == 0x08u,
              "stDisjointResult additional_items offset must stay 0x08");
static_assert(offsetof(stDisjointResult, big_win_items) == 0x14u,
              "stDisjointResult big_win_items offset must stay 0x14");
static_assert(offsetof(stDisjointResult, big_win_rate) == 0x20u,
              "stDisjointResult big_win_rate offset must stay 0x20");

// IDA unit: 99_misc_global/stCompoundRate.
struct stCompoundRate {
    ~stCompoundRate();

    int32_t boundary;   // +0x00
    int32_t rates[3];   // +0x04
};

static_assert(sizeof(stCompoundRate) == 0x10u,
              "stCompoundRate IDA ABI size must stay 0x10");

// IDA unit: 99_misc_global/stEnduranceRepairCost.
struct stEnduranceRepairCost {
    ~stEnduranceRepairCost();

    int32_t full_repair_cost;  // +0x00
    int32_t max_endurance;     // +0x04
};

static_assert(sizeof(stEnduranceRepairCost) == 0x08u,
              "stEnduranceRepairCost IDA ABI size must stay 0x08");

// IDA unit: 08_quest_skill_growth/STExpertJobScript tail value record.
// Stored in STExpertJobScript's map at +0x200; importer semantics remain
// B/C owner work.
struct stRecipeInfo {
    uint32_t recipe_item_index;   // +0x00
    uint32_t result_item_index;   // +0x04
    uint32_t required_level;      // +0x08, read as uint16_t by old callers
};

static_assert(sizeof(stRecipeInfo) == 0x0Cu,
              "stRecipeInfo IDA ABI size must stay 0x0C");
static_assert(offsetof(stRecipeInfo, required_level) == 0x08u,
              "stRecipeInfo required_level offset must stay 0x08");


#endif  // OLDSRV_GLOBAL_EXPERT_JOB_DISJOINT_TYPES_H_
