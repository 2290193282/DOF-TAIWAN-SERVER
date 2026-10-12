#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUEUE_SIZE_LOG_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUEUE_SIZE_LOG_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_InsertQueueSizeLog.
class DB_InsertQueueSizeLog : public DBDispatcher {
public:
    DB_InsertQueueSizeLog();
    virtual ~DB_InsertQueueSizeLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_QUEUE_SIZE_LOG_H_
