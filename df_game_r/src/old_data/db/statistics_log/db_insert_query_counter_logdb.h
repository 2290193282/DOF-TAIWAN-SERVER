#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUERY_COUNTER_LOGDB_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUERY_COUNTER_LOGDB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_InsertQueryCounterLogdb.
class DB_InsertQueryCounterLogdb : public DBDispatcher {
public:
    DB_InsertQueryCounterLogdb();
    virtual ~DB_InsertQueryCounterLogdb();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUERY_COUNTER_LOGDB_H_
