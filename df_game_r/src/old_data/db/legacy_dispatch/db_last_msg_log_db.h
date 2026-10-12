#ifndef OLDSRV_DB_DB_LAST_MSG_LOG_DB_H_
#define OLDSRV_DB_DB_LAST_MSG_LOG_DB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_LastMsgLogDb.
class DB_LastMsgLogDb : public DBDispatcher {
public:
    DB_LastMsgLogDb();
    virtual ~DB_LastMsgLogDb();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_LAST_MSG_LOG_DB_H_
