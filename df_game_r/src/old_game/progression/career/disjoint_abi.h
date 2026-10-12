#ifndef OLDSRV_GLOBAL_DISJOINT_ABI_H_
#define OLDSRV_GLOBAL_DISJOINT_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "common/config.h"
#include "old_game/activity/schedule/event_value_types.h"

class CItem;

// IDA unit: 08_quest_skill_growth/DisjointSkillScript.
struct DisjointSkillScript {
    DisjointSkillScript();
    ~DisjointSkillScript();

    DisjointSkillScript* clear();

    int32_t character_job;  // +0x00
    int32_t skill_index;    // +0x04
    int32_t skill_level;    // +0x08
    int32_t element;        // +0x0c
};

static_assert(sizeof(DisjointSkillScript) == 0x10u,
              "DisjointSkillScript IDA ABI size must stay 0x10");

// IDA unit: 99_misc_global/DisJointResult.
struct DisJointResult {
    DisJointResult();

    int32_t values[10];  // +0x00
};

static_assert(sizeof(DisJointResult) == 0x28u,
              "DisJointResult IDA ABI size must stay 0x28");

// IDA unit: 12_pvf_datamanager_scripts/DisjointScript.
struct DisjointScript {
    struct AdditionalResult {
        int32_t count;               // +0x00
        std::vector<int32_t> items;  // +0x04
    };

    DisjointScript();
    ~DisjointScript();

    bool clear();

    std::vector<int32_t> cube_indexes[5];              // +0x000
    LotteryInfo legacy_infos[4];               // +0x03c
    int32_t skill_info_count;                           // +0x0bc
    std::vector<DisjointSkillScript> skill_infos;       // +0x0c0
    float skill_prob[2];                                // +0x0cc
    int32_t avatar_disjoint_info
        [oldsrv::config::kCharacterJobCount][4][5];     // +0x0d4
    int32_t avatar_parts_disjoint_info[4][10];          // +0x494
    uint8_t reserved_534[0x50];                         // +0x534
    int32_t cube_creation_const;                        // +0x584
    float cube_creation_rates[6];                       // +0x588
    float additional_result_grade_a[6];                 // +0x5a0
    uint8_t reserved_5b8[0x04];                         // +0x5b8
    int32_t additional_result_const[17];                // +0x5bc
    float additional_result_grade_b[6];                 // +0x600
    AdditionalResult additional_results[6];             // +0x618
};

static_assert(sizeof(DisjointScript) == 0x678u,
              "DisjointScript 0828 ABI size must stay 0x678");
static_assert(offsetof(DisjointScript, legacy_infos) == 0x03cu,
              "DisjointScript legacy_infos offset must stay 0x3c");
static_assert(offsetof(DisjointScript, skill_info_count) == 0x0bcu,
              "DisjointScript skill_info_count offset must stay 0xbc");
static_assert(offsetof(DisjointScript, skill_infos) == 0x0c0u,
              "DisjointScript skill_infos offset must stay 0xc0");
static_assert(offsetof(DisjointScript, cube_creation_const) == 0x584u,
              "DisjointScript cube_creation_const offset must stay 0x584");
static_assert(offsetof(DisjointScript, additional_results) == 0x618u,
              "DisjointScript additional_results offset must stay 0x618");

// IDA unit: 99_misc_global/DisJoint.
struct DisJoint : public DisjointScript {
    DisJoint();
    ~DisJoint();

    int LoadDisJoint();
    std::vector<int32_t>* GetPredictResult(int rarity);
    int getPredictResultItemTypeCount(int rarity) const;
    int* GetAddtionalSkill(int character_job,
                           int skill_index,
                           int& skill_level,
                           int& element);
    int _GetAdditionalItemInfo(int rarity);
    char _GetAdditionalItem(const CItem* item,
                            int& item_index,
                            int& grade);
    int GetResult(int item_index, DisJointResult& result);

    std::vector<int32_t> predict_results[6];  // +0x678
};

static_assert(sizeof(DisJoint) == 0x6c0u,
              "DisJoint 0828 ABI size must stay 0x6c0");
static_assert(offsetof(DisJoint, predict_results) == 0x678u,
              "DisJoint predict_results offset must stay 0x678");

// IDA_EA: 0x88E4D88
// IDA_NAME: _Z20importDisjointScriptP14DisjointScriptPKc
// DEMANGLED: importDisjointScript(DisjointScript *,char const*)
int importDisjointScript(DisjointScript* script, const char* filename)
    __asm__("_Z20importDisjointScriptP14DisjointScriptPKc");


using ::DisJoint;
using ::DisJointResult;
using ::DisjointScript;
using ::DisjointSkillScript;
using ::importDisjointScript;


#endif  // OLDSRV_GLOBAL_DISJOINT_ABI_H_
