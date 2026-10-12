#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_ITEM_LOCK_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_ITEM_LOCK_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateItemLock.
// DBTask slot 440.
class DB_UpdateItemLock : public DBDispatcher {
public:
    DB_UpdateItemLock();
    virtual ~DB_UpdateItemLock();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_ITEM_LOCK_H_
