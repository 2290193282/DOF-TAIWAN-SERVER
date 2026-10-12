#ifndef OLDSRV_OLD_DATA_DB_PRIVATE_STORE_DB_SAVE_PRIVATE_STORE_H_
#define OLDSRV_OLD_DATA_DB_PRIVATE_STORE_DB_SAVE_PRIVATE_STORE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_SavePrivateStore.
class DB_SavePrivateStore : public DBDispatcher {
public:
    DB_SavePrivateStore();
    virtual ~DB_SavePrivateStore();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_PRIVATE_STORE_DB_SAVE_PRIVATE_STORE_H_
