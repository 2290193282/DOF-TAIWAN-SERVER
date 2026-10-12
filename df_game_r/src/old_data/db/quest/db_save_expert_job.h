#ifndef OLDSRV_OLD_DATA_DB_QUEST_DB_SAVE_EXPERT_JOB_H_
#define OLDSRV_OLD_DATA_DB_QUEST_DB_SAVE_EXPERT_JOB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 08_quest_skill_growth/DB_SaveExpertJob.
class DB_SaveExpertJob : public DBDispatcher {
public:
    DB_SaveExpertJob();
    virtual ~DB_SaveExpertJob();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_QUEST_DB_SAVE_EXPERT_JOB_H_
