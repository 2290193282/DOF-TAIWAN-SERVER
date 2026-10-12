#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_AVATAR_ITEM_LOCK_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_AVATAR_ITEM_LOCK_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 06_userinfo_stat_inventory/DB_UpdateAvatarItemLock.
// DBTask slot 450.
class DB_UpdateAvatarItemLock : public DBDispatcher {
public:
    DB_UpdateAvatarItemLock();
    virtual ~DB_UpdateAvatarItemLock();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_DB_UPDATE_AVATAR_ITEM_LOCK_H_
