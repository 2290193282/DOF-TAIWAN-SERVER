#include "old_data/db/skill/db_update_expert_job_statistic.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844EFDE
// IDA_NAME: _ZN27DB_UpdateExpertJobStatisticC2Ev
// DEMANGLED: DB_UpdateExpertJobStatistic::DB_UpdateExpertJobStatistic(void)
DB_UpdateExpertJobStatistic::DB_UpdateExpertJobStatistic() {}

// IDA_EA: 0x845D300
// IDA_NAME: _ZN27DB_UpdateExpertJobStatisticD2Ev
// DEMANGLED: DB_UpdateExpertJobStatistic::~DB_UpdateExpertJobStatistic()
// IDA_EA: 0x845D330
// IDA_NAME: _ZN27DB_UpdateExpertJobStatisticD0Ev
// DEMANGLED: DB_UpdateExpertJobStatistic::~DB_UpdateExpertJobStatistic() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateExpertJobStatistic::~DB_UpdateExpertJobStatistic() {}

// IDA_EA: 0x8436964
// IDA_NAME: _ZN27DB_UpdateExpertJobStatistic8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateExpertJobStatistic::dispatch(int,int,Stream *)
bool DB_UpdateExpertJobStatistic::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    int count = 0;
    stream->get_binary(&count, sizeof(count));
    if (!count) {
        return true;
    }

    for (unsigned int index = 0; index < static_cast<unsigned int>(count);
         ++index) {
        int expert_job_type = 0;
        int item_id = 0;
        int item_count = 0;
        stream->get_binary(&expert_job_type, sizeof(expert_job_type));
        stream->get_binary(&item_id, sizeof(item_id));
        stream->get_binary(&item_count, sizeof(item_count));
        if (!item_id) {
            break;
        }

        db_handle->set_query(
            "upDate expert_job_product_cnt set item_cnt=item_cnt+%d where expert_job_type=%d and occ_date=curdate() and item_id=%d",
            item_count,
            expert_job_type,
            item_id);
        if (db_handle->exec(true) == true) {
            if (db_handle->getAffectedRowCount()) {
                continue;
            }
        }
        db_handle->set_query(
            "inSert into expert_job_product_cnt(expert_job_type,occ_date,item_id,item_cnt) values(%d,curdate(),%d,%d)",
            expert_job_type,
            item_id,
            item_count);
        db_handle->exec(true);
    }
    return true;
}
