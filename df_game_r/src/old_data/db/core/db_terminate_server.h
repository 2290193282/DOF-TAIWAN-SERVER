#ifndef OLDSRV_DB_DB_TERMINATE_SERVER_H_
#define OLDSRV_DB_DB_TERMINATE_SERVER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_TerminateServer.
class DB_TerminateServer : public DBDispatcher {
public:
    DB_TerminateServer();
    virtual ~DB_TerminateServer();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_TERMINATE_SERVER_H_
