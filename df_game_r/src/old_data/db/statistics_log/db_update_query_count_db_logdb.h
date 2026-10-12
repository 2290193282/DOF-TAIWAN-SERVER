#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_DB_LOGDB_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_DB_LOGDB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateQueryCountDbLogdb.
class DB_UpdateQueryCountDbLogdb : public DBDispatcher {
public:
    DB_UpdateQueryCountDbLogdb();
    virtual ~DB_UpdateQueryCountDbLogdb();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_DB_LOGDB_H_
