#include "old_game/economy/inventory/item.h"

#include <string>
#include <vector>

#include "old_data/PVF/equipment/datamanager/equipment_script.h"

// IDA_EA: 0x8694658
// IDA_NAME: _ZNK5CItem24GetIncreaseStatusIntDataEiRi
// DEMANGLED: CItem::GetIncreaseStatusIntData(int,int &)const
int CItem::GetIncreaseStatusIntData(int index, int& value) const {
    const std::vector<int>& values = increase_status_.values;
    if (index < 0 || values.size() <= static_cast<size_t>(index)) {
        return 0;
    }
    value = values.at(static_cast<size_t>(index));
    return 1;
}

// IDA_EA: 0x86946B6
// IDA_NAME: _ZNK5CItem21GetIncreaseStatusTypeEv
// DEMANGLED: CItem::GetIncreaseStatusType(void)const
int CItem::GetIncreaseStatusType() const {
    return increase_status_.type;
}

// IDA_EA: 0x86946C4
// IDA_NAME: _ZNK5CItem16GetUsablePvPRankEv
// DEMANGLED: CItem::GetUsablePvPRank(void)const
int CItem::GetUsablePvPRank() const {
    return usable_pvp_rank_;
}

// IDA_EA: 0x80F12D6
// IDA_NAME: _ZNK5CItem10get_rarityEv
// DEMANGLED: CItem::get_rarity(void)const
int CItem::get_rarity() const {
    return rarity_;
}

// IDA_EA: 0x80F12EE
// IDA_NAME: _ZNK5CItem14getUsableLevelEv
// DEMANGLED: CItem::getUsableLevel(void)const
int CItem::getUsableLevel() const {
    return usable_level_;
}

// IDA_EA: 0x80F12E2
// IDA_NAME: _ZNK5CItem13GetAttachTypeEv
// DEMANGLED: CItem::GetAttachType(void)const
int CItem::GetAttachType() const {
    return attach_type_;
}

// IDA_EA: 0x80F12FA
// IDA_NAME: _ZNK5CItem12is_stackableEv
// DEMANGLED: CItem::is_stackable(void)const
int CItem::is_stackable() const {
    return stackable_;
}

// IDA_EA: 0x8150F18
// IDA_NAME: _ZNK5CItem13IsRoutingItemEv
// DEMANGLED: CItem::IsRoutingItem(void)const
int CItem::IsRoutingItem() const {
    return routing_item_;
}

// IDA_EA: 0x85BEEC8
// IDA_NAME: _ZN5CItem13IsPriorityJobEii
// DEMANGLED: CItem::IsPriorityJob(int,int)
int CItem::IsPriorityJob(int charac_job, int grow_type) {
    typedef std::pair<int, int> PriorityJob;
    typedef std::vector<PriorityJob> PriorityJobList;
    const PriorityJobList& priority_jobs = priority_jobs_;

    if (priority_jobs.empty()) {
        return 0;
    }

    int grow_category = 0;
    switch (grow_type) {
    case 1:
    case 17:
        grow_category = 1;
        break;
    case 2:
    case 18:
        grow_category = 2;
        break;
    case 3:
    case 19:
        grow_category = 3;
        break;
    case 4:
    case 20:
        grow_category = 4;
        break;
    default:
        break;
    }

    for (PriorityJobList::const_iterator it = priority_jobs.begin();
         it != priority_jobs.end();
         ++it) {
        if (it->first == charac_job && it->second == grow_category) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8110C48
// IDA_NAME: _ZNK5CItem9get_indexEv
// DEMANGLED: CItem::get_index(void)const
int32_t CItem::get_index() const {
    return item_index;
}

// IDA_EA: 0x80F1312
// IDA_NAME: _ZNK5CItem16getItemGroupNameEv
// DEMANGLED: CItem::getItemGroupName(void)const
int CItem::getItemGroupName() const {
    return item_group_name_;
}

// IDA_EA: 0x8110C54
// IDA_NAME: _ZNK5CItem9get_gradeEv
// DEMANGLED: CItem::get_grade(void)const
int CItem::get_grade() const {
    return grade;
}

// IDA_EA: 0x811ED82
// IDA_NAME: _ZNK5CItem11GetItemNameEv
// DEMANGLED: CItem::GetItemName(void)const
const char* CItem::GetItemName() const {
    return item_name_.c_str();
}

// IDA_EA: 0x85143EC
// IDA_NAME: _ZNK5CItem15GetItemIconNameEv
// DEMANGLED: CItem::GetItemIconName(void)const
const char* CItem::GetItemIconName() const {
    return item_icon_name_.c_str();
}

// IDA_EA: 0x80F1306
// IDA_NAME: _ZNK5CItem17getExpirationDateEv
// DEMANGLED: CItem::getExpirationDate(void)const
int CItem::getExpirationDate() const {
    return expiration_date_;
}

// IDA_EA: 0x81347CA
// IDA_NAME: _ZNK5CItem23getAdvanceAltarNeedGageEv
// DEMANGLED: CItem::getAdvanceAltarNeedGage(void)const
int CItem::getAdvanceAltarNeedGage() const {
    return advance_altar_need_gage_;
}

// IDA_EA: 0x8473612
// IDA_NAME: _ZNK5CItem12GetSellPriceEv
// DEMANGLED: CItem::GetSellPrice(void)const
int CItem::GetSellPrice() const {
    return sell_price_;
}

// IDA_EA: 0x8545FCC
// IDA_NAME: _ZNK5CItem14GetUpSkillTypeEv
// DEMANGLED: CItem::GetUpSkillType(void)const
int CItem::GetUpSkillType() const {
    return up_skill_type_;
}

// IDA_EA: 0x8545FDA
// IDA_NAME: _ZNK5CItem12GetNeedLevelEv
// DEMANGLED: CItem::GetNeedLevel(void)const
int CItem::GetNeedLevel() const {
    return need_level_;
}

// IDA_EA: 0x84E95CE
// IDA_NAME: _ZNK5CItem17isExpertJobUsableE20ENUM_EXPERT_JOB_TYPEi
// DEMANGLED: CItem::isExpertJobUsable(ENUM_EXPERT_JOB_TYPE,int)const
bool CItem::isExpertJobUsable(ENUM_EXPERT_JOB_TYPE expert_job_type,
                              int expert_job_level) const {
    const std::vector<STItemScript::SEXPERTJOB::SUSABLE>& usable =
        expert_job_usable_;

    if (usable.empty()) {
        return true;
    }

    for (std::vector<STItemScript::SEXPERTJOB::SUSABLE>::const_iterator it =
             usable.begin();
         it != usable.end();
         ++it) {
        if (it->type == static_cast<int>(expert_job_type)) {
            return it->value <= expert_job_level;
        }
        if (it->type == 5 && expert_job_type != 0) {
            return it->value <= expert_job_level;
        }
    }
    return false;
}

// IDA_EA: 0x850D27A
// IDA_NAME: _ZNK5CItem14get_need_skillEv
// DEMANGLED: CItem::get_need_skill(void)const
int CItem::get_need_skill() const {
    return need_skill_;
}

// IDA_EA: 0x850D286
// IDA_NAME: _ZNK5CItem23getFootControlRateLimitEv
// DEMANGLED: CItem::getFootControlRateLimit(void)const
int CItem::getFootControlRateLimit() const {
    return foot_control_rate_limit_;
}
