#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_USER_PLAY_TIME_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_USER_PLAY_TIME_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_InsertUserPlayTime.
class DB_InsertUserPlayTime : public DBDispatcher {
public:
    DB_InsertUserPlayTime();
    virtual ~DB_InsertUserPlayTime();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_USER_PLAY_TIME_H_
