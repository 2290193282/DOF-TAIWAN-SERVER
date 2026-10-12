#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_UPDATE_SERVER_MESSAGE_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_UPDATE_SERVER_MESSAGE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateServerMessage.
// The provider is intentionally not registered until the complete DBTask /
// InterDispatcher / DispatchThread route is restored atomically.
class DB_UpdateServerMessage : public DBDispatcher {
public:
    DB_UpdateServerMessage();
    virtual ~DB_UpdateServerMessage();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};


#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_UPDATE_SERVER_MESSAGE_H_
