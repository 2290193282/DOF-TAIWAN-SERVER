#include "old_data/db/statistics_log/db_insert_query_counter_logdb.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/timer/query_counter.h"
#include "infra/network/packet/stream.h"

#include <cstring>

// IDA_EA: 0x844E5EA
// IDA_NAME: _ZN26DB_InsertQueryCounterLogdbC2Ev
// DEMANGLED: DB_InsertQueryCounterLogdb::DB_InsertQueryCounterLogdb(void)
DB_InsertQueryCounterLogdb::DB_InsertQueryCounterLogdb() {}

// IDA_EA: 0x845EC4A
// IDA_NAME: _ZN26DB_InsertQueryCounterLogdbD2Ev
// DEMANGLED: DB_InsertQueryCounterLogdb::~DB_InsertQueryCounterLogdb()
// IDA_EA: 0x845EC7A
// IDA_NAME: _ZN26DB_InsertQueryCounterLogdbD0Ev
// DEMANGLED: DB_InsertQueryCounterLogdb::~DB_InsertQueryCounterLogdb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_InsertQueryCounterLogdb::~DB_InsertQueryCounterLogdb() {}

// IDA_EA: 0x8425CDE
// IDA_NAME: _ZN26DB_InsertQueryCounterLogdb8dispatchEiiP6Stream
// DEMANGLED: DB_InsertQueryCounterLogdb::dispatch(int,int,Stream *)
bool DB_InsertQueryCounterLogdb::dispatch(int signal, int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    int query_id = 0;
    int query_size = 0;
    char query[24577];
    std::memset(query, 0, sizeof(query));

    *stream >> query_id;
    *stream >> query_size;
    stream->get_binary(query, query_size);

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        DB_HANDLE_LOG),static_cast<ENUM_SERVER_GROUP>( SERVER_GROUP_DEFAULT));
    CQueryCounter* query_counter = db_handle->GetQueryCounter();
    return query_counter->RegisterQuery(query, query_id, 0) == true;
}
