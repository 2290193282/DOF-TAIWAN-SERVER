#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_SERVER_LOAD_STATISTIC_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_SERVER_LOAD_STATISTIC_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/statistics_log/statistics_log_payloads.h"

// IDA unit: 13_database_dispatch/DB_InsertServerLoadStatistic.
class DB_InsertServerLoadStatistic : public DBDispatcher {
public:
    DB_InsertServerLoadStatistic();
    virtual ~DB_InsertServerLoadStatistic();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_SERVER_LOAD_STATISTIC_H_
