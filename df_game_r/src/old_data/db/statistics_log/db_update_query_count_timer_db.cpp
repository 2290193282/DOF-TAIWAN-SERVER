#include "old_data/db/statistics_log/db_update_query_count_timer_db.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/timer/query_counter.h"

// IDA_EA: 0x844E26A
// IDA_NAME: _ZN26DB_UpdateQueryCountTimerDbC2Ev
// DEMANGLED: DB_UpdateQueryCountTimerDb::DB_UpdateQueryCountTimerDb(void)
DB_UpdateQueryCountTimerDb::DB_UpdateQueryCountTimerDb() {}

// IDA_EA: 0x845F6A6
// IDA_NAME: _ZN26DB_UpdateQueryCountTimerDbD2Ev
// DEMANGLED: DB_UpdateQueryCountTimerDb::~DB_UpdateQueryCountTimerDb()
// IDA_EA: 0x845F6D6
// IDA_NAME: _ZN26DB_UpdateQueryCountTimerDbD0Ev
// DEMANGLED: DB_UpdateQueryCountTimerDb::~DB_UpdateQueryCountTimerDb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateQueryCountTimerDb::~DB_UpdateQueryCountTimerDb() {}

// IDA_EA: 0x841C5D4
// IDA_NAME: _ZN26DB_UpdateQueryCountTimerDb8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateQueryCountTimerDb::dispatch(int,int,Stream *)
bool DB_UpdateQueryCountTimerDb::dispatch(int signal, int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        DB_HANDLE_2),static_cast<ENUM_SERVER_GROUP>( SERVER_GROUP_DEFAULT));
    CQueryCounter* query_counter = db_handle->GetQueryCounter();
    query_counter->UpdateQueryCounter();
    return true;
}
