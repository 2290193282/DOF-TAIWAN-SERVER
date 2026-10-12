#include "old_data/db/statistics_log/db_update_query_count_timer_logdb.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/timer/query_counter.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x844E24E
// IDA_NAME: _ZN29DB_UpdateQueryCountTimerLogdbC2Ev
// DEMANGLED: DB_UpdateQueryCountTimerLogdb::DB_UpdateQueryCountTimerLogdb(void)
DB_UpdateQueryCountTimerLogdb::DB_UpdateQueryCountTimerLogdb() {}

// IDA_EA: 0x845F6F4
// IDA_NAME: _ZN29DB_UpdateQueryCountTimerLogdbD2Ev
// DEMANGLED: DB_UpdateQueryCountTimerLogdb::~DB_UpdateQueryCountTimerLogdb()
// IDA_EA: 0x845F724
// IDA_NAME: _ZN29DB_UpdateQueryCountTimerLogdbD0Ev
// DEMANGLED: DB_UpdateQueryCountTimerLogdb::~DB_UpdateQueryCountTimerLogdb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateQueryCountTimerLogdb::~DB_UpdateQueryCountTimerLogdb() {}

// IDA_EA: 0x841C458
// IDA_NAME: _ZN29DB_UpdateQueryCountTimerLogdb8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateQueryCountTimerLogdb::dispatch(int,int,Stream *)
bool DB_UpdateQueryCountTimerLogdb::dispatch(int signal, int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        DB_HANDLE_LOG),static_cast<ENUM_SERVER_GROUP>( SERVER_GROUP_DEFAULT));
    CQueryCounter* query_counter = db_handle->GetQueryCounter();
    CQueryCounter::QueryCountTable* query_count_table =
        query_counter->GetQueryCountTable();
    const int cur_sec = GlobalData::system_time()->getCurSec();

    for (CQueryCounter::QueryCountIterator it = query_count_table->begin();
         it != query_count_table->end(); ++it) {
        const int query_id = it->first;
        const int total = it->second.count;
        if (total > 0) {
            const int response_time =
                static_cast<int>(it->second.response_time * 1000.0);
            CEnvironment* environment = G_CEnvironment();
            const int channel_no = environment->get_channel_no();
            db_handle->set_query(
                "inSert into log_query_stat(occ_time,q_id,total,response_time,gc_no) values(from_unixtime(%d),%d,%d,%d,%d)",
                cur_sec,
                query_id,
                total,
                response_time,
                channel_no);
            db_handle->exec(true);
        }
    }

    query_counter->Reset();
    return true;
}
