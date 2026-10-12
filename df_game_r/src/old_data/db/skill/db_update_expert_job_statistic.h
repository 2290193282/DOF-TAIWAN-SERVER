#ifndef OLDSRV_DB_DB_UPDATE_EXPERT_JOB_STATISTIC_H_
#define OLDSRV_DB_DB_UPDATE_EXPERT_JOB_STATISTIC_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8436964/0x844EFDE/0x845D300/0x845D330.
// IDA unit: 08_quest_skill_growth/DB_UpdateExpertJobStatistic.
class DB_UpdateExpertJobStatistic : public DBDispatcher {
public:
    DB_UpdateExpertJobStatistic();
    virtual ~DB_UpdateExpertJobStatistic();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_EXPERT_JOB_STATISTIC_H_
