#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_TIMER_STATISTICS_LOG_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_TIMER_STATISTICS_LOG_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_TimerStatisticsLog, DBTask slot 205.
class DB_TimerStatisticsLog : public DBDispatcher {
public:
    DB_TimerStatisticsLog();
    virtual ~DB_TimerStatisticsLog();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_TIMER_STATISTICS_LOG_H_
