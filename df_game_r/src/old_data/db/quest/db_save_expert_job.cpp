#include "old_data/db/quest/db_save_expert_job.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/quest/expert_job_payload.h"
#include "infra/network/packet/stream_expert_job.h"


// IDA_EA: 0x84367F4
// IDA_NAME: _ZN16DB_SaveExpertJob8dispatchEiiP6Stream
// DEMANGLED: DB_SaveExpertJob::dispatch(int,int,Stream *)
bool DB_SaveExpertJob::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    SIG_EXPERT_JOB* payload = stream->GetOutBuffer<SIG_EXPERT_JOB>();
    char* recipe_blob = db_handle->blob_to_str(0, payload->recipe, 30);
    db_handle->set_query(
        "upDate charac_expert_job set expert_job_giveup_cnt=%d, expert_job_info=%d, expert_job_info_ex=%d, recipe='%s' where charac_no=%u",
        payload->expert_job_giveup_cnt,
        payload->expert_job_info,
        payload->expert_job_info_ex,
        recipe_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount() != 0) {
        return true;
    }

    recipe_blob = db_handle->blob_to_str(0, payload->recipe, 30);
    db_handle->set_query(
        "inSert into charac_expert_job(charac_no, expert_job_giveup_cnt, expert_job_info, expert_job_info_ex, recipe) values(%u, %d, %d, %d, '%s')",
        payload->charac_no,
        payload->expert_job_giveup_cnt,
        payload->expert_job_info,
        payload->expert_job_info_ex,
        recipe_blob);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844EFC2
// IDA_NAME: _ZN16DB_SaveExpertJobC2Ev
// DEMANGLED: DB_SaveExpertJob::DB_SaveExpertJob(void)
DB_SaveExpertJob::DB_SaveExpertJob() {}

// IDA_EA: 0x845D34E
// IDA_NAME: _ZN16DB_SaveExpertJobD2Ev
// DEMANGLED: DB_SaveExpertJob::~DB_SaveExpertJob()
// IDA_EA: 0x845D37E
// IDA_NAME: _ZN16DB_SaveExpertJobD0Ev
// DEMANGLED: DB_SaveExpertJob::~DB_SaveExpertJob()
DB_SaveExpertJob::~DB_SaveExpertJob() {}
