#include "old_data/db/statistics_log/db_insert_query_counter_db.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/timer/query_counter.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

// IDA_EA: 0x844E5CE
// IDA_NAME: _ZN23DB_InsertQueryCounterDbC2Ev
// DEMANGLED: DB_InsertQueryCounterDb::DB_InsertQueryCounterDb(void)
DB_InsertQueryCounterDb::DB_InsertQueryCounterDb() {}

// IDA_EA: 0x845EC98
// IDA_NAME: _ZN23DB_InsertQueryCounterDbD2Ev
// DEMANGLED: DB_InsertQueryCounterDb::~DB_InsertQueryCounterDb()
// IDA_EA: 0x845ECC8
// IDA_NAME: _ZN23DB_InsertQueryCounterDbD0Ev
// DEMANGLED: DB_InsertQueryCounterDb::~DB_InsertQueryCounterDb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_InsertQueryCounterDb::~DB_InsertQueryCounterDb() {}

// IDA_EA: 0x8425B0C
// IDA_NAME: _ZN23DB_InsertQueryCounterDb8dispatchEiiP6Stream
// DEMANGLED: DB_InsertQueryCounterDb::dispatch(int,int,Stream *)
bool DB_InsertQueryCounterDb::dispatch(int signal, int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    int query_id = 0;
    int query_size = 0;
    char query[24577];
    std::memset(query, 0, sizeof(query));

    *stream >> query_size;
    stream->get_binary(query, query_size);

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        DB_HANDLE_2),static_cast<ENUM_SERVER_GROUP>( SERVER_GROUP_DEFAULT));
    CQueryCounter* query_counter = db_handle->GetQueryCounter();
    if (query_counter->SaveQuery(query, query_id) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 25537);
    CStreamGuard guard(out_stream, true);
    guard << 191;
    guard << -1;
    guard << query_id;
    guard << query_size;
    guard->put_binary(query, query_size);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
    return true;
}
