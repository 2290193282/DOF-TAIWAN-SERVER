#include "old_data/db/inventory/db_update_creature_item_lock.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"


// IDA_EA: 0x844F14A
// IDA_NAME: _ZN25DB_UpdateCreatureItemLockC2Ev
// DEMANGLED: DB_UpdateCreatureItemLock::DB_UpdateCreatureItemLock(void)
DB_UpdateCreatureItemLock::DB_UpdateCreatureItemLock() {}

// IDA_EA: 0x845CC9A
// IDA_NAME: _ZN25DB_UpdateCreatureItemLockD2Ev
// DEMANGLED: DB_UpdateCreatureItemLock::~DB_UpdateCreatureItemLock()
// IDA_EA: 0x845CCCA
// IDA_NAME: _ZN25DB_UpdateCreatureItemLockD0Ev
// DEMANGLED: DB_UpdateCreatureItemLock::~DB_UpdateCreatureItemLock()
DB_UpdateCreatureItemLock::~DB_UpdateCreatureItemLock() {}

// IDA_EA: 0x8439ABA
// IDA_NAME: _ZN25DB_UpdateCreatureItemLock8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCreatureItemLock::dispatch(int,int,Stream *)
bool DB_UpdateCreatureItemLock::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    int ui_id = 0;
    int item_lock_key = 0;
    *stream >> ui_id;
    *stream >> item_lock_key;
    db_handle->set_query(
        "upDate creature_items set item_lock_key=%d where ui_id=%d",
        item_lock_key,
        ui_id);
    return db_handle->exec(true) == true;
}
