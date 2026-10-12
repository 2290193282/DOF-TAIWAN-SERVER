#include "old_data/db/inventory/db_update_item_lock.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_game/economy/inventory/item_lock_payload.h"
#include "infra/network/packet/stream_item_lock.h"

#include <string.h>


// IDA_EA: 0x844F12E
// IDA_NAME: _ZN17DB_UpdateItemLockC2Ev
// DEMANGLED: DB_UpdateItemLock::DB_UpdateItemLock(void)
DB_UpdateItemLock::DB_UpdateItemLock() {}

// IDA_EA: 0x845CCE8
// IDA_NAME: _ZN17DB_UpdateItemLockD2Ev
// DEMANGLED: DB_UpdateItemLock::~DB_UpdateItemLock()
// IDA_EA: 0x845CD18
// IDA_NAME: _ZN17DB_UpdateItemLockD0Ev
// DEMANGLED: DB_UpdateItemLock::~DB_UpdateItemLock()
DB_UpdateItemLock::~DB_UpdateItemLock() {}

// IDA_EA: 0x843993A
// IDA_NAME: _ZN17DB_UpdateItemLock8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateItemLock::dispatch(int,int,Stream *)
bool DB_UpdateItemLock::dispatch(int signal,
                                 int user_idx,
                                 Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    SIG_ITEM_LOCK_UPDATE* item_lock_update =
        stream->GetOutBuffer<SIG_ITEM_LOCK_UPDATE>();

    char compressed[972];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(
            compressed,
            compressed_len,
            reinterpret_cast<const char*>(item_lock_update->item_locks),
            sizeof(item_lock_update->item_locks)) != 1) {
        return false;
    }

    char* blob =
        db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "upDate charac_item_lock_info set item_lock_info='%s' where charac_no=%u",
        blob,
        item_lock_update->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (!db_handle->getAffectedRowCount()) {
        blob = db_handle->blob_to_str(0, compressed, compressed_len);
        db_handle->set_query(
            "inSert into charac_item_lock_info(charac_no, item_lock_info) values(%u, '%s')",
            item_lock_update->charac_no,
            blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
