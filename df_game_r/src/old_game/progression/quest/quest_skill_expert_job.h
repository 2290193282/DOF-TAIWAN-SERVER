#ifndef OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_EXPERT_JOB_H_
#define OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_EXPERT_JOB_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x896E72C in the companion source.


#include <map>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include "old_game/progression/career/expert_job_disjoint_types.h"


// IDA unit: 08_quest_skill_growth/ExpertJobExp.
struct ExpertJobExp {
    ExpertJobExp();
    ~ExpertJobExp();
    ExpertJobExp(const ExpertJobExp& other);

    ExpertJobExp& operator=(const ExpertJobExp& other);

    uint32_t exp;
    uint8_t level;
    uint8_t pad_05[3];
    std::string text;
};

// IDA unit: 08_quest_skill_growth/STExpertJobScript.
struct STExpertJobScript {
    struct stSelfDisjoinMaterialInfo {
        stSelfDisjoinMaterialInfo();

        stSelfDisjoinMaterialInfo* clear();

        int item_index;
        int item_count;
        int gold;
    };

    struct stMonsterCardBindRate {
        stMonsterCardBindRate();

        uint16_t common_rate;
        uint16_t special_rate;
    };

    struct stMonsterCardBindResult {
        stMonsterCardBindResult();
        ~stMonsterCardBindResult();

        uint32_t item_index;
        uint16_t probability;
        uint8_t pad_06[2];
    };

    struct stMonsterCardBindInfo {
        stMonsterCardBindInfo();
        ~stMonsterCardBindInfo();

        bool clear();

        int group_counts[4];
        std::vector<stMonsterCardBindResult> results[4];
    };

    struct stAlchemistExtractionResultInfo {
        stAlchemistExtractionResultInfo();
        ~stAlchemistExtractionResultInfo();

        bool clear();

        int group_counts[3];
        std::vector<stMonsterCardBindResult> results[3];
    };

    struct stEnchanterExtractionResultInfo {
        stEnchanterExtractionResultInfo();
        ~stEnchanterExtractionResultInfo();

        int clear();

        int group_count;
        std::vector<stMonsterCardBindResult> results;
    };

    struct stDollControllerResult {
        stDollControllerResult();
        ~stDollControllerResult();

        uint32_t item_index;
        uint16_t grade;
        uint16_t rate;
        uint16_t count;
        uint8_t pad_0a[2];
    };

    struct stDollControllerResultInfo {
        stDollControllerResultInfo();
        ~stDollControllerResultInfo();

        bool clear();

        int group_counts[3];
        std::vector<stDollControllerResult> results[3];
    };

    STExpertJobScript();
    ~STExpertJobScript();

    int GetPortableDisjointPossibleLevel(int index);
    int GetUnsealScrollPossibleLevel(int index);
    int GetMonsterCardBindRate(int index, stMonsterCardBindRate& rate);
    stMonsterCardBindInfo* GetSpecialBindInfo();
    stMonsterCardBindInfo* GetCommonBindInfo();
    stRecipeInfo* GetRecipeInfo(int recipe_item_index);
    int GetRecipeInfoIndex(unsigned long recipe_result_item_index);
    stCompoundRate* GetCompoundRate(int value);
    int isBoundaryExpValue(int value);
    int GetAlchemistExtractionPossibleLevel(int index);
    int GetEnchanterExtractionPossibleLevel(int index);
    int GetDollControllerExtractionPossibleLevel(int index);
    stAlchemistExtractionResultInfo* GetAlchemistExtractionResultInfo();
    stDollControllerResultInfo* GetDollControllerExtractionResultInfo();
    stEnchanterExtractionResultInfo* GetEnchanterExtractionResultInfo();
    int GetShopDollSlot(int index);
    int GetDisjointMachineMaxGrade();
    stEnduranceRepairCost* _GetEnduranceRepairCost(unsigned int grade);
    int GetDisjointMachineRepairCost(int grade, int disjoint_machine_endurance);
    int GetDisjointMachineRepair(int grade, int cur_charac_money,
                                 int& usable_money);
    int GetUpgradeCost(unsigned int grade);
    uint32_t GetLevel(uint32_t exp_value);
    bool Clear();

    std::vector<ExpertJobExp> expert_job_exp;
    std::vector<std::pair<int, int> > unknown_exp_pairs;
    std::map<int, uint16_t> portable_disjoint_possible_levels;
    stSelfDisjoinMaterialInfo self_disjoin_material_info;
    std::map<int, uint8_t> shop_doll_slots;
    std::map<int, uint16_t> unseal_scroll_possible_levels;
    std::map<int, stMonsterCardBindRate> monster_card_bind_rates;
    stMonsterCardBindInfo common_bind_info;
    stMonsterCardBindInfo special_bind_info;
    std::map<int, uint16_t> alchemist_extraction_possible_levels;
    std::map<int, uint16_t> enchanter_extraction_possible_levels;
    std::map<int, uint16_t> doll_controller_extraction_possible_levels;
    stAlchemistExtractionResultInfo alchemist_extraction_result_info;
    stEnchanterExtractionResultInfo enchanter_extraction_result_info;
    stDollControllerResultInfo doll_controller_extraction_result_info;
    std::map<DisjointMachineGrade, stDisjointResult> disjoint_results;
    uint32_t dword_1D4;
    uint32_t dword_1D8;
    std::vector<int> dword_vector_1DC;
    std::vector<int> dword_vector_1E8;
    std::vector<stCompoundRate> compound_rates;
    std::map<int, stRecipeInfo> recipe_infos;
    std::map<unsigned long, int> recipe_info_indices;
    std::map<int, int> int_map_230;
    std::map<unsigned long, unsigned long> ulong_map_248;
    std::vector<stEnduranceRepairCost> endurance_repair_costs;
    uint32_t dword_26C;
    uint32_t dword_270;
    uint32_t dword_274;
    std::vector<int> upgrade_costs;
    uint32_t dword_284;
    uint32_t dword_288;
    // IDA +0x28c, read as the upper bound for a player-chosen disjoint
    // machine cost by CDisjointer::OnCreateDisjointStore @0x85D2D08.
    uint32_t max_disjoint_machine_cost;
};

// IDA unit: 08_quest_skill_growth/STExpertJobEtcScript.
struct STExpertJobEtcScript {
    STExpertJobEtcScript();

    int Clear();
    int isExpertJobLimitLevel(int charac_level);

    std::map<unsigned char, short> expert_job_limit_levels;
    std::map<unsigned char, unsigned char> expert_job_unknown_flags;
};

#if defined(__i386__)
static_assert(offsetof(STExpertJobScript, self_disjoin_material_info) == 0x30u,
              "STExpertJobScript self-disjoint material ABI drift");
static_assert(offsetof(STExpertJobScript::stSelfDisjoinMaterialInfo,
                       item_index) == 0x00u &&
                  offsetof(STExpertJobScript::stSelfDisjoinMaterialInfo,
                           item_count) == 0x04u &&
                  offsetof(STExpertJobScript::stSelfDisjoinMaterialInfo,
                           gold) == 0x08u,
              "self-disjoint material field layout drift");
#endif

// IDA unit: 08_quest_skill_growth/CExpertJobList.
class CExpertJobList {
public:
    CExpertJobList();

    STExpertJobScript* get(int expert_job_index);
    STExpertJobEtcScript* get_etc();
    bool init();
    bool destroy();

private:
    std::map<int, STExpertJobScript*> expert_job_scripts;
    STExpertJobEtcScript etc_script;
};


#endif  // OLDSRV_08_QUEST_SKILL_GROWTH_QUEST_SKILL_EXPERT_JOB_H_
