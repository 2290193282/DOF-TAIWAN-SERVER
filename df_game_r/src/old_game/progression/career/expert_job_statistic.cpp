#include "old_game/progression/career/expert_job_statistic.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <utility>

// IDA_EA: 0x86106DA
// IDA_NAME: _ZNK19CExpertJobStatistic12ExpertJobKeyltERKS0_
// DEMANGLED: CExpertJobStatistic::ExpertJobKey::operator<(CExpertJobStatistic::ExpertJobKey const&)const
bool CExpertJobStatistic::ExpertJobKey::operator<(
    const ExpertJobKey& other) const {
    if (expert_job_type_ < other.expert_job_type_) {
        return true;
    }
    return expert_job_type_ == other.expert_job_type_ &&
           item_index_ < other.item_index_;
}

// IDA_EA: 0x8610720
// IDA_NAME: _ZN19CExpertJobStatistic12ExpertJobKeyC2E20ENUM_EXPERT_JOB_TYPEm
// DEMANGLED: CExpertJobStatistic::ExpertJobKey::ExpertJobKey(ENUM_EXPERT_JOB_TYPE,unsigned long)
CExpertJobStatistic::ExpertJobKey::ExpertJobKey(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    unsigned long item_index)
    : expert_job_type_(expert_job_type),
      item_index_(item_index) {}

// IDA_EA: 0x861074A
// IDA_NAME: _ZN19CExpertJobStatisticC2Ev
// DEMANGLED: CExpertJobStatistic::CExpertJobStatistic(void)
CExpertJobStatistic::CExpertJobStatistic()
    : item_counts_() {
    item_counts_.clear();
}

// IDA_EA: 0x86108AE
// IDA_NAME: _ZN19CExpertJobStatisticD2Ev
// DEMANGLED: CExpertJobStatistic::~CExpertJobStatistic()
CExpertJobStatistic::~CExpertJobStatistic() {}

// IDA_EA: 0x860F9D4
// IDA_NAME: _ZN19CExpertJobStatistic17increaseItemCountE20ENUM_EXPERT_JOB_TYPEmi
// DEMANGLED: CExpertJobStatistic::increaseItemCount(ENUM_EXPERT_JOB_TYPE,unsigned long,int)
void CExpertJobStatistic::increaseItemCount(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    unsigned long item_index,
    int item_count) {
    ExpertJobKey key(expert_job_type, item_index);
    ItemCountMap::iterator found = item_counts_.find(key);
    if (found != item_counts_.end()) {
        found->second += item_count;
        return;
    }

    item_counts_.insert(std::make_pair(
        ExpertJobKey(expert_job_type, item_index), item_count));
}

// IDA_EA: 0x860FACA
// IDA_NAME: _ZN19CExpertJobStatistic15UpdateExpertJobEv
// DEMANGLED: CExpertJobStatistic::UpdateExpertJob(void)
void CExpertJobStatistic::UpdateExpertJob() {
    if (item_counts_.empty()) {
        return;
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1626), true);
    *guard << 386;  // IDA 0x860FB24: immediate 0x182.
    *guard << -1;
    *guard << static_cast<int>(item_counts_.size());

    for (ItemCountMap::const_iterator it = item_counts_.begin();
         it != item_counts_.end();
         ++it) {
        *guard << static_cast<int>(it->first.expert_job_type_);
        *guard << static_cast<int>(it->first.item_index_);
        *guard << it->second;
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    item_counts_.clear();
}

// IDA_EA: 0x860F940
// IDA_NAME: _Z29GetInstanceExpertJobStatisticv
// DEMANGLED: GetInstanceExpertJobStatistic(void)
CExpertJobStatistic* GetInstanceExpertJobStatistic() {
    static CExpertJobStatistic instance;
    return &instance;
}
