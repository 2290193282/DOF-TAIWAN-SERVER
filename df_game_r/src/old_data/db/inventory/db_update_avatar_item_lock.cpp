#include "old_data/db/inventory/db_update_avatar_item_lock.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"


// IDA_EA: 0x844F166
// IDA_NAME: _ZN23DB_UpdateAvatarItemLockC2Ev
// DEMANGLED: DB_UpdateAvatarItemLock::DB_UpdateAvatarItemLock(void)
DB_UpdateAvatarItemLock::DB_UpdateAvatarItemLock() {}

// IDA_EA: 0x845CC4C
// IDA_NAME: _ZN23DB_UpdateAvatarItemLockD2Ev
// DEMANGLED: DB_UpdateAvatarItemLock::~DB_UpdateAvatarItemLock()
// IDA_EA: 0x845CC7C
// IDA_NAME: _ZN23DB_UpdateAvatarItemLockD0Ev
// DEMANGLED: DB_UpdateAvatarItemLock::~DB_UpdateAvatarItemLock()
DB_UpdateAvatarItemLock::~DB_UpdateAvatarItemLock() {}

// IDA_EA: 0x8439B4E
// IDA_NAME: _ZN23DB_UpdateAvatarItemLock8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateAvatarItemLock::dispatch(int,int,Stream *)
bool DB_UpdateAvatarItemLock::dispatch(int signal,
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
        "upDate user_items set item_lock_key=%d where ui_id=%d",
        item_lock_key,
        ui_id);
    return db_handle->exec(true) == true;
}
