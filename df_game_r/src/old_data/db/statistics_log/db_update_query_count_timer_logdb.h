#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_TIMER_LOGDB_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_TIMER_LOGDB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateQueryCountTimerLogdb.
class DB_UpdateQueryCountTimerLogdb : public DBDispatcher {
public:
    DB_UpdateQueryCountTimerLogdb();
    virtual ~DB_UpdateQueryCountTimerLogdb();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_TIMER_LOGDB_H_
