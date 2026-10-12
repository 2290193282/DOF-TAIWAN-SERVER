#include "old_data/db/quest/db_load_expert_job.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/quest/expert_job_payload.h"
#include "infra/network/packet/stream_expert_job.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


// IDA_EA: 0x8436464
// IDA_NAME: _ZN16DB_LoadExpertJob8dispatchEiiP6Stream
// DEMANGLED: DB_LoadExpertJob::dispatch(int,int,Stream *)
bool DB_LoadExpertJob::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    int charac_no = 0;
    *stream >> charac_no;
    db_handle->set_query(
        "seLect expert_job_giveup_cnt, expert_job_info, expert_job_info_ex, recipe  from charac_expert_job where charac_no=%d",
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (db_handle->get_n_rows() != 0) {
        if (db_handle->fetch() != true) {
            return false;
        }
        Stream* out_stream =
            GlobalData::s_stream_pool->Acquire("DBThread.cpp", 33218);
        CStreamGuard guard(out_stream, true);
        *guard << signal;
        *guard << user_idx;
        SIG_EXPERT_JOB* payload = guard->GetInBuffer<SIG_EXPERT_JOB>();
        std::memset(payload, 0, sizeof(*payload));
        if (db_handle->get_ushort(0, payload->expert_job_giveup_cnt) == true &&
            db_handle->get_int(1, payload->expert_job_info) == true &&
            db_handle->get_int(2, payload->expert_job_info_ex) == true &&
            db_handle->get_binary(3, payload->recipe, 30) == true) {
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
            return true;
        }
        GlobalData::s_stream_pool->Free(guard);
        return false;
    }

    Stream* out_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 33202);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_EXPERT_JOB* payload = guard->GetInBuffer<SIG_EXPERT_JOB>();
    std::memset(payload, 0, sizeof(*payload));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844EFA6
// IDA_NAME: _ZN16DB_LoadExpertJobC2Ev
// DEMANGLED: DB_LoadExpertJob::DB_LoadExpertJob(void)
DB_LoadExpertJob::DB_LoadExpertJob() {}

// IDA_EA: 0x845D39C
// IDA_NAME: _ZN16DB_LoadExpertJobD2Ev
// DEMANGLED: DB_LoadExpertJob::~DB_LoadExpertJob()
// IDA_EA: 0x845D3CC
// IDA_NAME: _ZN16DB_LoadExpertJobD0Ev
// DEMANGLED: DB_LoadExpertJob::~DB_LoadExpertJob()
DB_LoadExpertJob::~DB_LoadExpertJob() {}
