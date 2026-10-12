#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_CREATURE_ITEM_LOCK_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_CREATURE_ITEM_LOCK_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 06_userinfo_stat_inventory/DB_UpdateCreatureItemLock.
// DBTask slot 449.
class DB_UpdateCreatureItemLock : public DBDispatcher {
public:
    DB_UpdateCreatureItemLock();
    virtual ~DB_UpdateCreatureItemLock();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_CREATURE_ITEM_LOCK_H_
