#include "old_game/progression/career/expert_job_disjoint_types.h"


// IDA_EA: 0x896DBCC
// IDA_NAME: _ZN20DisjointMachineGradeC2Ev
// DEMANGLED: DisjointMachineGrade::DisjointMachineGrade(void)
DisjointMachineGrade::DisjointMachineGrade()
    : disjoint_machine_grade(0),
      rarity(0),
      unidentified(0) {}

// IDA_EA: 0x85D3B9E
// IDA_NAME: _ZN20DisjointMachineGradeC2Ehhh
// DEMANGLED: DisjointMachineGrade::DisjointMachineGrade(unsigned char,unsigned char,unsigned char)
DisjointMachineGrade::DisjointMachineGrade(
    uint8_t rarity_value,
    uint8_t disjoint_machine_grade_value,
    uint8_t unidentified_value)
    : disjoint_machine_grade(disjoint_machine_grade_value),
      rarity(rarity_value),
      unidentified(unidentified_value) {}

// IDA_EA: 0x85D3B28
// IDA_NAME: _ZNK20DisjointMachineGradeltERKS_
// DEMANGLED: DisjointMachineGrade::operator<(DisjointMachineGrade const&) const
bool DisjointMachineGrade::operator<(
    const DisjointMachineGrade& other) const {
    if (rarity < other.rarity) {
        return true;
    }
    if (rarity != other.rarity) {
        return false;
    }
    if (disjoint_machine_grade < other.disjoint_machine_grade) {
        return true;
    }
    return disjoint_machine_grade == other.disjoint_machine_grade &&
           unidentified < other.unidentified;
}

// IDA_EA: 0x85D3BD6
// IDA_NAME: _ZN16stItemSelectRateC2Ev
// DEMANGLED: stItemSelectRate::stItemSelectRate(void)
stItemSelectRate::stItemSelectRate()
    : item_index(0),
      select_rate(0),
      grade_divisor(0.0f),
      min_grade(0),
      max_grade(0) {}

// IDA_EA: 0x897553E
// IDA_NAME: _ZN16stItemSelectRateD2Ev
// DEMANGLED: stItemSelectRate::~stItemSelectRate()
stItemSelectRate::~stItemSelectRate() {}

// IDA_EA: 0x896E758
// IDA_NAME: _ZN16stDisjointResultC2Ev
// DEMANGLED: stDisjointResult::stDisjointResult(void)
stDisjointResult::stDisjointResult() {}

// IDA_EA: 0x896E7A4
// IDA_NAME: _ZN16stDisjointResultD2Ev
// DEMANGLED: stDisjointResult::~stDisjointResult()
stDisjointResult::~stDisjointResult() {}

// IDA_EA: 0x896F5AA
// IDA_NAME: _ZN16stDisjointResultC2ERKS_
// DEMANGLED: stDisjointResult::stDisjointResult(stDisjointResult const&)
stDisjointResult::stDisjointResult(const stDisjointResult& other)
    : result_item_index(other.result_item_index),
      base_cost_rate(other.base_cost_rate),
      additional_items(other.additional_items),
      big_win_items(other.big_win_items),
      big_win_rate(other.big_win_rate) {}

// IDA_EA: 0x89743B4
// IDA_NAME: _ZN14stCompoundRateD2Ev
// DEMANGLED: stCompoundRate::~stCompoundRate()
stCompoundRate::~stCompoundRate() {}

// IDA_EA: 0x8974932
// IDA_NAME: _ZN21stEnduranceRepairCostD2Ev
// DEMANGLED: stEnduranceRepairCost::~stEnduranceRepairCost()
stEnduranceRepairCost::~stEnduranceRepairCost() {}
