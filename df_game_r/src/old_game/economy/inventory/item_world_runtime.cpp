#include "old_game/economy/inventory/item.h"

#include <set>

// IDA_EA: 0x822C87C
// IDA_NAME: _ZNK5CItem13IsEnableWorldE15ENUM_WORLD_TYPE
// DEMANGLED: CItem::IsEnableWorld(ENUM_WORLD_TYPE)const
bool CItem::IsEnableWorld(ENUM_WORLD_TYPE world_type) const {
    return enabled_worlds_.find(static_cast<int>(world_type)) !=
           enabled_worlds_.end();
}

// IDA_EA: 0x822C8EA
// IDA_NAME: _ZNK5CItem13IsEnableWorldEPKS_
// DEMANGLED: CItem::IsEnableWorld(CItem const*)const
bool CItem::IsEnableWorld(const CItem* item) const {
    for (std::set<int>::const_iterator world = item->enabled_worlds_.begin();
         world != item->enabled_worlds_.end();
         ++world) {
        if (!IsEnableWorld(static_cast<ENUM_WORLD_TYPE>(*world))) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x849D768
// IDA_NAME: _Z11EnableWorldPK5CItemS1_
// DEMANGLED: EnableWorld(CItem const*,CItem const*)
bool EnableWorld(const CItem* first, const CItem* second) {
    return first->IsEnableWorld(second) &&
           second->IsEnableWorld(first);
}

// IDA_EA: 0x850D292
// IDA_NAME: _ZNK5CItem37GetExpertJobCompoundMaterialVariationEv
// DEMANGLED: CItem::GetExpertJobCompoundMaterialVariation(void)const
float CItem::GetExpertJobCompoundMaterialVariation() const {
    return expert_job_material_variation_;
}

// IDA_EA: 0x850D2AA
// IDA_NAME: _ZNK5CItem33GetExpertJobCompoundRateVariationEv
// DEMANGLED: CItem::GetExpertJobCompoundRateVariation(void)const
float CItem::GetExpertJobCompoundRateVariation() const {
    return expert_job_compound_rate_variation_;
}

// IDA_EA: 0x850D2DE
// IDA_NAME: _ZNK5CItem34GetExpertJobSelfDisjointBigWinRateEv
float CItem::GetExpertJobSelfDisjointBigWinRate() const {
    return expert_job_disjoint_big_win_rate_;
}

// IDA_EA: 0x850D2F6
// IDA_NAME: _ZNK5CItem39GetExpertJobSelfDisjointResultVariationEv
float CItem::GetExpertJobSelfDisjointResultVariation() const {
    return expert_job_disjoint_result_variation_;
}

// IDA_EA: 0x850D30E
// IDA_NAME: _ZNK5CItem25GetExpertJobAdditionalExpERN12STItemScript10SEXPERTJOB24stExpertJobAdditionalExpE
// DEMANGLED: CItem::GetExpertJobAdditionalExp(STItemScript::SEXPERTJOB::stExpertJobAdditionalExp &)const
void CItem::GetExpertJobAdditionalExp(
    STItemScript::SEXPERTJOB::stExpertJobAdditionalExp& value) const {
    value = expert_job_additional_exp_;
}
