#ifndef OLDSRV_OLD_DATA_DB_QUEST_DB_LOAD_EXPERT_JOB_H_
#define OLDSRV_OLD_DATA_DB_QUEST_DB_LOAD_EXPERT_JOB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 08_quest_skill_growth/DB_LoadExpertJob.
class DB_LoadExpertJob : public DBDispatcher {
public:
    DB_LoadExpertJob();
    virtual ~DB_LoadExpertJob();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_QUEST_DB_LOAD_EXPERT_JOB_H_
