#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_JOB_STATISTIC_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_JOB_STATISTIC_H_

#include <map>

enum ENUM_EXPERT_JOB_TYPE : int;

class CExpertJobStatistic {
public:
    class ExpertJobKey {
    public:
        // IDA_EA: 0x8610720
        // IDA_NAME: _ZN19CExpertJobStatistic12ExpertJobKeyC2E20ENUM_EXPERT_JOB_TYPEm
        // DEMANGLED: CExpertJobStatistic::ExpertJobKey::ExpertJobKey(ENUM_EXPERT_JOB_TYPE,unsigned long)
        ExpertJobKey(ENUM_EXPERT_JOB_TYPE expert_job_type,
                     unsigned long item_index);

        // IDA_EA: 0x86106DA
        // IDA_NAME: _ZNK19CExpertJobStatistic12ExpertJobKeyltERKS0_
        // DEMANGLED: CExpertJobStatistic::ExpertJobKey::operator<(CExpertJobStatistic::ExpertJobKey const&)const
        bool operator<(const ExpertJobKey& other) const;

    private:
        friend class CExpertJobStatistic;

        ENUM_EXPERT_JOB_TYPE expert_job_type_;
        unsigned long item_index_;
    };

    // IDA_EA: 0x861074A
    // IDA_NAME: _ZN19CExpertJobStatisticC2Ev
    // DEMANGLED: CExpertJobStatistic::CExpertJobStatistic(void)
    CExpertJobStatistic();

    // IDA_EA: 0x86108AE
    // IDA_NAME: _ZN19CExpertJobStatisticD2Ev
    // DEMANGLED: CExpertJobStatistic::~CExpertJobStatistic()
    ~CExpertJobStatistic();

    // IDA_EA: 0x860F9D4
    // IDA_NAME: _ZN19CExpertJobStatistic17increaseItemCountE20ENUM_EXPERT_JOB_TYPEmi
    // DEMANGLED: CExpertJobStatistic::increaseItemCount(ENUM_EXPERT_JOB_TYPE,unsigned long,int)
    void increaseItemCount(ENUM_EXPERT_JOB_TYPE expert_job_type,
                           unsigned long item_index,
                           int item_count);

    // IDA_EA: 0x860FACA
    // IDA_NAME: _ZN19CExpertJobStatistic15UpdateExpertJobEv
    // DEMANGLED: CExpertJobStatistic::UpdateExpertJob(void)
    void UpdateExpertJob();

private:
    typedef std::map<ExpertJobKey, int> ItemCountMap;

    ItemCountMap item_counts_;
};

static_assert(sizeof(CExpertJobStatistic::ExpertJobKey) == 0x08u,
              "CExpertJobStatistic::ExpertJobKey i386 layout drift");

// IDA_EA: 0x860F940
// IDA_NAME: _Z29GetInstanceExpertJobStatisticv
// DEMANGLED: GetInstanceExpertJobStatistic(void)
CExpertJobStatistic* GetInstanceExpertJobStatistic();

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_JOB_STATISTIC_H_
