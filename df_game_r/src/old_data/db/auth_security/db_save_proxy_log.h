#ifndef OLDSRV_DB_DB_SAVE_PROXY_LOG_H_
#define OLDSRV_DB_DB_SAVE_PROXY_LOG_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SaveProxyLog.
class DB_SaveProxyLog : public DBDispatcher {
public:
    DB_SaveProxyLog();
    virtual ~DB_SaveProxyLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_PROXY_LOG_H_
