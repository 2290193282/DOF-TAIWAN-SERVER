#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUERY_COUNTER_DB_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUERY_COUNTER_DB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_InsertQueryCounterDb.
class DB_InsertQueryCounterDb : public DBDispatcher {
public:
    DB_InsertQueryCounterDb();
    virtual ~DB_InsertQueryCounterDb();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUERY_COUNTER_DB_H_
