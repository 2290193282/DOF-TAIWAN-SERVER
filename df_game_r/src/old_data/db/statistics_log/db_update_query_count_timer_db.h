#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_TIMER_DB_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_TIMER_DB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateQueryCountTimerDb.
class DB_UpdateQueryCountTimerDb : public DBDispatcher {
public:
    DB_UpdateQueryCountTimerDb();
    virtual ~DB_UpdateQueryCountTimerDb();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_UPDATE_QUERY_COUNT_TIMER_DB_H_
