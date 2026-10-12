#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"

// Migration marker: expert_job::CExpertJob::GetType (0x8234796) is defined
// only by src/old_game/quest_skill/expert_job_runtime.cpp.

typedef char STExpertJobEtcScript_Size_Must_Be_0x30[
    (sizeof(STExpertJobEtcScript) == 0x30) ? 1 : -1];
typedef char CExpertJobList_Size_Must_Be_0x48[
    (sizeof(CExpertJobList) == 0x48) ? 1 : -1];
typedef char ExpertJobExp_Size_Must_Be_0x0c[
    (sizeof(ExpertJobExp) == 0x0c) ? 1 : -1];
typedef char STExpertJobScript_stSelfDisjoinMaterialInfo_Size_Must_Be_0x0c[
    (sizeof(STExpertJobScript::stSelfDisjoinMaterialInfo) == 0x0c) ? 1 : -1];
typedef char STExpertJobScript_stMonsterCardBindRate_Size_Must_Be_0x04[
    (sizeof(STExpertJobScript::stMonsterCardBindRate) == 0x04) ? 1 : -1];
typedef char STExpertJobScript_stMonsterCardBindResult_Size_Must_Be_0x08[
    (sizeof(STExpertJobScript::stMonsterCardBindResult) == 0x08) ? 1 : -1];
typedef char STExpertJobScript_stMonsterCardBindInfo_Size_Must_Be_0x40[
    (sizeof(STExpertJobScript::stMonsterCardBindInfo) == 0x40) ? 1 : -1];
typedef char STExpertJobScript_stAlchemistExtractionResultInfo_Size_Must_Be_0x30[
    (sizeof(STExpertJobScript::stAlchemistExtractionResultInfo) == 0x30)
        ? 1
        : -1];
typedef char STExpertJobScript_stEnchanterExtractionResultInfo_Size_Must_Be_0x10[
    (sizeof(STExpertJobScript::stEnchanterExtractionResultInfo) == 0x10)
        ? 1
        : -1];
typedef char STExpertJobScript_stDollControllerResult_Size_Must_Be_0x0c[
    (sizeof(STExpertJobScript::stDollControllerResult) == 0x0c) ? 1 : -1];
typedef char STExpertJobScript_stDollControllerResultInfo_Size_Must_Be_0x30[
    (sizeof(STExpertJobScript::stDollControllerResultInfo) == 0x30) ? 1 : -1];
typedef char STExpertJobScript_Size_Must_Be_0x290[
    (sizeof(STExpertJobScript) == 0x290) ? 1 : -1];

// IDA_EA: 0x896E72C
// IDA_NAME: _ZN12ExpertJobExpC2Ev
// DEMANGLED: ExpertJobExp::ExpertJobExp(void)
ExpertJobExp::ExpertJobExp() {}

// IDA_EA: 0x896E742
// IDA_NAME: _ZN12ExpertJobExpD2Ev
// DEMANGLED: ExpertJobExp::~ExpertJobExp()
ExpertJobExp::~ExpertJobExp() {}

// IDA_EA: 0x8970BBE
// IDA_NAME: _ZN12ExpertJobExpC2ERKS_
// DEMANGLED: ExpertJobExp::ExpertJobExp(ExpertJobExp const&)
ExpertJobExp::ExpertJobExp(const ExpertJobExp& other)
    : exp(other.exp),
      level(other.level),
      text(other.text) {}

// IDA_EA: 0x8970C7A
// IDA_NAME: _ZN12ExpertJobExpaSERKS_
// DEMANGLED: ExpertJobExp::operator=(ExpertJobExp const&)
ExpertJobExp& ExpertJobExp::operator=(const ExpertJobExp& other) {
    if (this != &other) {
        exp = other.exp;
        level = other.level;
        text = other.text;
    }
    return *this;
}

// IDA_EA: 0x896DBE6
// IDA_NAME: _ZN17STExpertJobScript25stSelfDisjoinMaterialInfo5clearEv
// DEMANGLED: STExpertJobScript::stSelfDisjoinMaterialInfo::clear(void)
STExpertJobScript::stSelfDisjoinMaterialInfo*
STExpertJobScript::stSelfDisjoinMaterialInfo::clear() {
    gold = 0;
    item_count = gold;
    item_index = item_count;
    return this;
}

// IDA_EA: 0x896DC0C
// IDA_NAME: _ZN17STExpertJobScript25stSelfDisjoinMaterialInfoC2Ev
// DEMANGLED: STExpertJobScript::stSelfDisjoinMaterialInfo::stSelfDisjoinMaterialInfo(void)
STExpertJobScript::stSelfDisjoinMaterialInfo::stSelfDisjoinMaterialInfo() {
    clear();
}

// IDA_EA: 0x822B1DE
// IDA_NAME: _ZN17STExpertJobScript21stMonsterCardBindRateC2Ev
// DEMANGLED: STExpertJobScript::stMonsterCardBindRate::stMonsterCardBindRate(void)
STExpertJobScript::stMonsterCardBindRate::stMonsterCardBindRate()
    : common_rate(0),
      special_rate(0) {}

// IDA_EA: 0x896DC20
// IDA_NAME: _ZN17STExpertJobScript23stMonsterCardBindResultC2Ev
// DEMANGLED: STExpertJobScript::stMonsterCardBindResult::stMonsterCardBindResult(void)
STExpertJobScript::stMonsterCardBindResult::stMonsterCardBindResult()
    : item_index(0),
      probability(0) {}

// IDA_EA: 0x8974FAC
// IDA_NAME: _ZN17STExpertJobScript23stMonsterCardBindResultD2Ev
// DEMANGLED: STExpertJobScript::stMonsterCardBindResult::~stMonsterCardBindResult()
STExpertJobScript::stMonsterCardBindResult::~stMonsterCardBindResult() {}

// IDA_EA: 0x896DC38
// IDA_NAME: _ZN17STExpertJobScript21stMonsterCardBindInfo5clearEv
// DEMANGLED: STExpertJobScript::stMonsterCardBindInfo::clear(void)
bool STExpertJobScript::stMonsterCardBindInfo::clear() {
    for (int index = 0; index <= 3; ++index) {
        group_counts[index] = 0;
        results[index].clear();
    }
    return false;
}

// IDA_EA: 0x896DC80
// IDA_NAME: _ZN17STExpertJobScript21stMonsterCardBindInfoC2Ev
// DEMANGLED: STExpertJobScript::stMonsterCardBindInfo::stMonsterCardBindInfo(void)
STExpertJobScript::stMonsterCardBindInfo::stMonsterCardBindInfo() {
    clear();
}

// IDA_EA: 0x896E218
// IDA_NAME: _ZN17STExpertJobScript21stMonsterCardBindInfoD2Ev
// DEMANGLED: STExpertJobScript::stMonsterCardBindInfo::~stMonsterCardBindInfo()
STExpertJobScript::stMonsterCardBindInfo::~stMonsterCardBindInfo() {}

// IDA_EA: 0x896DD3E
// IDA_NAME: _ZN17STExpertJobScript31stAlchemistExtractionResultInfo5clearEv
// DEMANGLED: STExpertJobScript::stAlchemistExtractionResultInfo::clear(void)
bool STExpertJobScript::stAlchemistExtractionResultInfo::clear() {
    for (int index = 0; index <= 2; ++index) {
        group_counts[index] = 0;
        results[index].clear();
    }
    return false;
}

// IDA_EA: 0x896DD86
// IDA_NAME: _ZN17STExpertJobScript31stAlchemistExtractionResultInfoC2Ev
// DEMANGLED: STExpertJobScript::stAlchemistExtractionResultInfo::stAlchemistExtractionResultInfo(void)
STExpertJobScript::stAlchemistExtractionResultInfo::
    stAlchemistExtractionResultInfo() {
    clear();
}

// IDA_EA: 0x896E250
// IDA_NAME: _ZN17STExpertJobScript31stAlchemistExtractionResultInfoD2Ev
// DEMANGLED: STExpertJobScript::stAlchemistExtractionResultInfo::~stAlchemistExtractionResultInfo()
STExpertJobScript::stAlchemistExtractionResultInfo::
    ~stAlchemistExtractionResultInfo() {}

// IDA_EA: 0x896DE44
// IDA_NAME: _ZN17STExpertJobScript31stEnchanterExtractionResultInfo5clearEv
// DEMANGLED: STExpertJobScript::stEnchanterExtractionResultInfo::clear(void)
int STExpertJobScript::stEnchanterExtractionResultInfo::clear() {
    group_count = 0;
    results.clear();
    return 0;
}

// IDA_EA: 0x896DE64
// IDA_NAME: _ZN17STExpertJobScript31stEnchanterExtractionResultInfoC2Ev
// DEMANGLED: STExpertJobScript::stEnchanterExtractionResultInfo::stEnchanterExtractionResultInfo(void)
STExpertJobScript::stEnchanterExtractionResultInfo::
    stEnchanterExtractionResultInfo() {
    clear();
}

// IDA_EA: 0x896E288
// IDA_NAME: _ZN17STExpertJobScript31stEnchanterExtractionResultInfoD2Ev
// DEMANGLED: STExpertJobScript::stEnchanterExtractionResultInfo::~stEnchanterExtractionResultInfo()
STExpertJobScript::stEnchanterExtractionResultInfo::
    ~stEnchanterExtractionResultInfo() {}

// IDA_EA: 0x896DEAC
// IDA_NAME: _ZN17STExpertJobScript22stDollControllerResultC2Ev
// DEMANGLED: STExpertJobScript::stDollControllerResult::stDollControllerResult(void)
STExpertJobScript::stDollControllerResult::stDollControllerResult()
    : item_index(0),
      grade(0),
      rate(0),
      count(0) {}

// IDA_EA: 0x89751FA
// IDA_NAME: _ZN17STExpertJobScript22stDollControllerResultD2Ev
// DEMANGLED: STExpertJobScript::stDollControllerResult::~stDollControllerResult()
STExpertJobScript::stDollControllerResult::~stDollControllerResult() {}

// IDA_EA: 0x896DEE6
// IDA_NAME: _ZN17STExpertJobScript26stDollControllerResultInfo5clearEv
// DEMANGLED: STExpertJobScript::stDollControllerResultInfo::clear(void)
bool STExpertJobScript::stDollControllerResultInfo::clear() {
    for (int index = 0; index <= 2; ++index) {
        group_counts[index] = 0;
        results[index].clear();
    }
    return false;
}

// IDA_EA: 0x896DF2E
// IDA_NAME: _ZN17STExpertJobScript26stDollControllerResultInfoC2Ev
// DEMANGLED: STExpertJobScript::stDollControllerResultInfo::stDollControllerResultInfo(void)
STExpertJobScript::stDollControllerResultInfo::stDollControllerResultInfo() {
    clear();
}

// IDA_EA: 0x896E29E
// IDA_NAME: _ZN17STExpertJobScript26stDollControllerResultInfoD2Ev
// DEMANGLED: STExpertJobScript::stDollControllerResultInfo::~stDollControllerResultInfo()
STExpertJobScript::stDollControllerResultInfo::~stDollControllerResultInfo() {}

// IDA_EA: 0x896E312
// IDA_NAME: _ZN17STExpertJobScriptC2Ev
// DEMANGLED: STExpertJobScript::STExpertJobScript(void)
STExpertJobScript::STExpertJobScript()
    : expert_job_exp(),
      unknown_exp_pairs(),
      portable_disjoint_possible_levels(),
      self_disjoin_material_info(),
      shop_doll_slots(),
      unseal_scroll_possible_levels(),
      monster_card_bind_rates(),
      common_bind_info(),
      special_bind_info(),
      alchemist_extraction_possible_levels(),
      enchanter_extraction_possible_levels(),
      doll_controller_extraction_possible_levels(),
      alchemist_extraction_result_info(),
      enchanter_extraction_result_info(),
      doll_controller_extraction_result_info(),
      disjoint_results(),
      dword_1D4(0),
      dword_1D8(0),
      dword_vector_1DC(),
      dword_vector_1E8(),
      compound_rates(),
      recipe_infos(),
      recipe_info_indices(),
      int_map_230(),
      ulong_map_248(),
      endurance_repair_costs(),
      dword_26C(0),
      dword_270(0),
      dword_274(0),
      upgrade_costs(),
      dword_284(0),
      dword_288(0),
      max_disjoint_machine_cost(0) {
    Clear();
}

// IDA_EA: 0x896E81C
// IDA_NAME: _ZN17STExpertJobScriptD2Ev
// DEMANGLED: STExpertJobScript::~STExpertJobScript()
STExpertJobScript::~STExpertJobScript() {}

// IDA_EA: 0x822B1F4
// IDA_NAME: _ZN17STExpertJobScript32GetPortableDisjointPossibleLevelEi
// DEMANGLED: STExpertJobScript::GetPortableDisjointPossibleLevel(int)
int STExpertJobScript::GetPortableDisjointPossibleLevel(int index) {
    std::map<int, uint16_t>::iterator it =
        portable_disjoint_possible_levels.find(index);
    if (it == portable_disjoint_possible_levels.end()) {
        return -1;
    }
    return it->second;
}

// IDA_EA: 0x822B262
// IDA_NAME: _ZN17STExpertJobScript28GetUnsealScrollPossibleLevelEi
// DEMANGLED: STExpertJobScript::GetUnsealScrollPossibleLevel(int)
int STExpertJobScript::GetUnsealScrollPossibleLevel(int index) {
    std::map<int, uint16_t>::iterator it =
        unseal_scroll_possible_levels.find(index);
    if (it == unseal_scroll_possible_levels.end()) {
        return -1;
    }
    return it->second;
}

// IDA_EA: 0x822B2D0
// IDA_NAME: _ZN17STExpertJobScript22GetMonsterCardBindRateEiRNS_21stMonsterCardBindRateE
// DEMANGLED: STExpertJobScript::GetMonsterCardBindRate(int,STExpertJobScript::stMonsterCardBindRate &)
int STExpertJobScript::GetMonsterCardBindRate(
    int index,
    STExpertJobScript::stMonsterCardBindRate& rate) {
    std::map<int, stMonsterCardBindRate>::iterator it =
        monster_card_bind_rates.find(index);
    if (it == monster_card_bind_rates.end()) {
        rate = stMonsterCardBindRate();
        return 0;
    }
    rate = it->second;
    return 1;
}

// IDA_EA: 0x822B358
// IDA_NAME: _ZN17STExpertJobScript18GetSpecialBindInfoEv
// DEMANGLED: STExpertJobScript::GetSpecialBindInfo(void)
STExpertJobScript::stMonsterCardBindInfo*
STExpertJobScript::GetSpecialBindInfo() {
    return &special_bind_info;
}

// IDA_EA: 0x822B366
// IDA_NAME: _ZN17STExpertJobScript17GetCommonBindInfoEv
// DEMANGLED: STExpertJobScript::GetCommonBindInfo(void)
STExpertJobScript::stMonsterCardBindInfo* STExpertJobScript::GetCommonBindInfo() {
    return &common_bind_info;
}

// IDA_EA: 0x832B6B6
// IDA_NAME: _ZN17STExpertJobScript13GetRecipeInfoEi
// DEMANGLED: STExpertJobScript::GetRecipeInfo(int)
stRecipeInfo* STExpertJobScript::GetRecipeInfo(int recipe_item_index) {
    std::map<int, stRecipeInfo>::iterator it =
        recipe_infos.find(recipe_item_index);
    if (it != recipe_infos.end()) {
        return &it->second;
    }
    return 0;
}

// IDA_EA: 0x832B726
// IDA_NAME: _ZN17STExpertJobScript18GetRecipeInfoIndexEm
// DEMANGLED: STExpertJobScript::GetRecipeInfoIndex(unsigned long)
int STExpertJobScript::GetRecipeInfoIndex(
    unsigned long recipe_result_item_index) {
    std::map<unsigned long, int>::iterator it =
        recipe_info_indices.find(recipe_result_item_index);
    if (it != recipe_info_indices.end()) {
        return it->second;
    }
    return -1;
}

// IDA_EA: 0x849F366
// IDA_NAME: _ZN17STExpertJobScript15GetCompoundRateEi
// DEMANGLED: STExpertJobScript::GetCompoundRate(int)
stCompoundRate* STExpertJobScript::GetCompoundRate(int value) {
    if (compound_rates.empty()) {
        return 0;
    }
    for (std::vector<stCompoundRate>::iterator it = compound_rates.begin();
         it != compound_rates.end(); ++it) {
        if (it->boundary >= value) {
            return &*it;
        }
    }
    return &compound_rates.back();
}

// IDA_EA: 0x849F420
// IDA_NAME: _ZN17STExpertJobScript18isBoundaryExpValueEi
// DEMANGLED: STExpertJobScript::isBoundaryExpValue(int)
int STExpertJobScript::isBoundaryExpValue(int value) {
    for (uint32_t index = 0; index < expert_job_exp.size(); ++index) {
        if (value == static_cast<int>(expert_job_exp[index].exp)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x84A44B6
// IDA_NAME: _ZN17STExpertJobScript35GetAlchemistExtractionPossibleLevelEi
// DEMANGLED: STExpertJobScript::GetAlchemistExtractionPossibleLevel(int)
int STExpertJobScript::GetAlchemistExtractionPossibleLevel(int index) {
    std::map<int, uint16_t>::iterator it =
        alchemist_extraction_possible_levels.find(index);
    if (it == alchemist_extraction_possible_levels.end()) {
        return -1;
    }
    return it->second;
}

// IDA_EA: 0x84A452A
// IDA_NAME: _ZN17STExpertJobScript35GetEnchanterExtractionPossibleLevelEi
// DEMANGLED: STExpertJobScript::GetEnchanterExtractionPossibleLevel(int)
int STExpertJobScript::GetEnchanterExtractionPossibleLevel(int index) {
    std::map<int, uint16_t>::iterator it =
        enchanter_extraction_possible_levels.find(index);
    if (it == enchanter_extraction_possible_levels.end()) {
        return -1;
    }
    return it->second;
}

// IDA_EA: 0x84A459E
// IDA_NAME: _ZN17STExpertJobScript40GetDollControllerExtractionPossibleLevelEi
// DEMANGLED: STExpertJobScript::GetDollControllerExtractionPossibleLevel(int)
int STExpertJobScript::GetDollControllerExtractionPossibleLevel(int index) {
    std::map<int, uint16_t>::iterator it =
        doll_controller_extraction_possible_levels.find(index);
    if (it == doll_controller_extraction_possible_levels.end()) {
        return -1;
    }
    return it->second;
}

// IDA_EA: 0x84A4612
// IDA_NAME: _ZN17STExpertJobScript32GetAlchemistExtractionResultInfoEv
// DEMANGLED: STExpertJobScript::GetAlchemistExtractionResultInfo(void)
STExpertJobScript::stAlchemistExtractionResultInfo*
STExpertJobScript::GetAlchemistExtractionResultInfo() {
    return &alchemist_extraction_result_info;
}

// IDA_EA: 0x84A4620
// IDA_NAME: _ZN17STExpertJobScript37GetDollControllerExtractionResultInfoEv
// DEMANGLED: STExpertJobScript::GetDollControllerExtractionResultInfo(void)
STExpertJobScript::stDollControllerResultInfo*
STExpertJobScript::GetDollControllerExtractionResultInfo() {
    return &doll_controller_extraction_result_info;
}

// IDA_EA: 0x84A462E
// IDA_NAME: _ZN17STExpertJobScript32GetEnchanterExtractionResultInfoEv
// DEMANGLED: STExpertJobScript::GetEnchanterExtractionResultInfo(void)
STExpertJobScript::stEnchanterExtractionResultInfo*
STExpertJobScript::GetEnchanterExtractionResultInfo() {
    return &enchanter_extraction_result_info;
}

// IDA_EA: 0x85CB306
// IDA_NAME: _ZN17STExpertJobScript15GetShopDollSlotEi
// DEMANGLED: STExpertJobScript::GetShopDollSlot(int)
int STExpertJobScript::GetShopDollSlot(int index) {
    std::map<int, uint8_t>::iterator it = shop_doll_slots.find(index);
    if (it != shop_doll_slots.end()) {
        return it->second;
    }
    return -1;
}

// IDA_EA: 0x85D3C0E
// IDA_NAME: _ZN17STExpertJobScript26GetDisjointMachineMaxGradeEv
// DEMANGLED: STExpertJobScript::GetDisjointMachineMaxGrade(void)
int STExpertJobScript::GetDisjointMachineMaxGrade() {
    return static_cast<int>(endurance_repair_costs.size());
}

// IDA_EA: 0x85D3C26
// IDA_NAME: _ZN17STExpertJobScript23_GetEnduranceRepairCostEj
// DEMANGLED: STExpertJobScript::_GetEnduranceRepairCost(unsigned int)
stEnduranceRepairCost* STExpertJobScript::_GetEnduranceRepairCost(
    unsigned int grade) {
    if (endurance_repair_costs.size() > grade - 1) {
        return &endurance_repair_costs.at(grade - 1);
    }
    return 0;
}

// IDA_EA: 0x85D3C74
// IDA_NAME: _ZN17STExpertJobScript28GetDisjointMachineRepairCostEii
// DEMANGLED: STExpertJobScript::GetDisjointMachineRepairCost(int,int)
int STExpertJobScript::GetDisjointMachineRepairCost(
    int grade,
    int disjoint_machine_endurance) {
    stEnduranceRepairCost* cost =
        _GetEnduranceRepairCost(static_cast<unsigned int>(grade));
    if (!cost) {
        return 0;
    }
    return cost->full_repair_cost -
           disjoint_machine_endurance * cost->full_repair_cost /
               cost->max_endurance;
}

// IDA_EA: 0x85D3CC8
// IDA_NAME: _ZN17STExpertJobScript24GetDisjointMachineRepairEiiRi
// DEMANGLED: STExpertJobScript::GetDisjointMachineRepair(int,int,int &)
int STExpertJobScript::GetDisjointMachineRepair(int grade,
                                                int cur_charac_money,
                                                int& usable_money) {
    stEnduranceRepairCost* cost =
        _GetEnduranceRepairCost(static_cast<unsigned int>(grade));
    if (!cost) {
        return 0;
    }
    const int unit_cost = cost->full_repair_cost / cost->max_endurance;
    if (unit_cost <= 0) {
        return 0;
    }
    usable_money = cur_charac_money - cur_charac_money % unit_cost;
    return cur_charac_money / unit_cost;
}

// IDA_EA: 0x85D3D3A
// IDA_NAME: _ZN17STExpertJobScript14GetUpgradeCostEj
// DEMANGLED: STExpertJobScript::GetUpgradeCost(unsigned int)
int STExpertJobScript::GetUpgradeCost(unsigned int grade) {
    if (upgrade_costs.size() > grade) {
        return upgrade_costs.at(grade);
    }
    return 0;
}

// IDA_EA: 0x8693BA0
// IDA_NAME: _ZN17STExpertJobScript8GetLevelEj
// DEMANGLED: STExpertJobScript::GetLevel(unsigned int)
uint32_t STExpertJobScript::GetLevel(uint32_t exp_value) {
    uint32_t index = 0;
    for (; index < expert_job_exp.size(); ++index) {
        if (expert_job_exp[index].exp > exp_value) {
            return index + 1;
        }
    }
    return index;
}

// IDA_EA: 0x896DFEC
// IDA_NAME: _ZN17STExpertJobScript5ClearEv
// DEMANGLED: STExpertJobScript::Clear(void)
bool STExpertJobScript::Clear() {
    expert_job_exp.clear();
    unknown_exp_pairs.clear();
    disjoint_results.clear();
    dword_1D4 = 0;
    dword_1D8 = 0;
    dword_vector_1DC.clear();
    dword_vector_1E8.clear();
    compound_rates.clear();
    recipe_infos.clear();
    recipe_info_indices.clear();
    ulong_map_248.clear();
    int_map_230.clear();
    endurance_repair_costs.clear();
    dword_26C = 0;
    dword_270 = 0;
    dword_274 = 0;
    upgrade_costs.clear();
    dword_284 = 0;
    dword_288 = 0;
    max_disjoint_machine_cost = 0;
    portable_disjoint_possible_levels.clear();
    self_disjoin_material_info.clear();
    shop_doll_slots.clear();
    unseal_scroll_possible_levels.clear();
    monster_card_bind_rates.clear();
    common_bind_info.clear();
    special_bind_info.clear();
    alchemist_extraction_possible_levels.clear();
    enchanter_extraction_possible_levels.clear();
    doll_controller_extraction_possible_levels.clear();
    alchemist_extraction_result_info.clear();
    enchanter_extraction_result_info.clear();
    return doll_controller_extraction_result_info.clear();
}

// IDA_EA: 0x8371796
// IDA_NAME: _ZN20STExpertJobEtcScriptC2Ev
// DEMANGLED: STExpertJobEtcScript::STExpertJobEtcScript(void)
STExpertJobEtcScript::STExpertJobEtcScript() {
    Clear();
}

// IDA_EA: 0x83717FE
// IDA_NAME: _ZN20STExpertJobEtcScript5ClearEv
// DEMANGLED: STExpertJobEtcScript::Clear(void)
int STExpertJobEtcScript::Clear() {
    expert_job_limit_levels.clear();
    expert_job_unknown_flags.clear();
    return 0;
}

// IDA_EA: 0x849F47C
// IDA_NAME: _ZN20STExpertJobEtcScript21isExpertJobLimitLevelEi
// DEMANGLED: STExpertJobEtcScript::isExpertJobLimitLevel(int)
int STExpertJobEtcScript::isExpertJobLimitLevel(int charac_level) {
    for (std::map<unsigned char, short>::const_iterator it =
             expert_job_limit_levels.begin();
         it != expert_job_limit_levels.end(); ++it) {
        if (it->second == charac_level) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x822B374
// IDA_NAME: _ZN14CExpertJobList3getEi
// DEMANGLED: CExpertJobList::get(int)
STExpertJobScript* CExpertJobList::get(int expert_job_index) {
    std::map<int, STExpertJobScript*>::iterator it =
        expert_job_scripts.find(expert_job_index);
    if (it != expert_job_scripts.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x8371834
// IDA_NAME: _ZN14CExpertJobListC2Ev
// DEMANGLED: CExpertJobList::CExpertJobList(void)
CExpertJobList::CExpertJobList() {}

// IDA_EA: 0x849F50A
// IDA_NAME: _ZN14CExpertJobList7get_etcEv
// DEMANGLED: CExpertJobList::get_etc(void)
STExpertJobEtcScript* CExpertJobList::get_etc() {
    return &etc_script;
}
