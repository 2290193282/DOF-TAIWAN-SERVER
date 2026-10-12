#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_LOAD_SERVER_MESSAGE_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_LOAD_SERVER_MESSAGE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_LoadServerMessage.
// The class is a real DB provider but is deliberately not registered until the
// original DBTask/InterDispatcher/DispatchThread route is restored as one
// complete table closure.
class DB_LoadServerMessage : public DBDispatcher {
public:
    DB_LoadServerMessage();
    virtual ~DB_LoadServerMessage();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};


#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_LOAD_SERVER_MESSAGE_H_
