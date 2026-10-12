#ifndef OLDSRV_OLD_DATA_DB_PRIVATE_STORE_DB_LOAD_PRIVATE_STORE_H_
#define OLDSRV_OLD_DATA_DB_PRIVATE_STORE_DB_LOAD_PRIVATE_STORE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_LoadPrivateStore.
class DB_LoadPrivateStore : public DBDispatcher {
public:
    DB_LoadPrivateStore();
    virtual ~DB_LoadPrivateStore();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_PRIVATE_STORE_DB_LOAD_PRIVATE_STORE_H_
