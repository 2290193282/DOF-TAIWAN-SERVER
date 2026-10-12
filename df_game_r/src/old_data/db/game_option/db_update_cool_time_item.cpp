#include "old_data/db/game_option/db_update_cool_time_item.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



namespace {

enum {
    kSigSaveAndLoadContinuousItemInfoSize = 408,
    kContinuousItemInfoBytes = 404,
};

#define DB_CONTINUOUS_ITEM_U32(payload, offset) \
    (*reinterpret_cast<uint32_t*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844EC5E
// IDA_NAME: _ZN21DB_UpdateCoolTimeItemC2Ev
// DEMANGLED: DB_UpdateCoolTimeItem::DB_UpdateCoolTimeItem(void)
DB_UpdateCoolTimeItem::DB_UpdateCoolTimeItem() {}

// IDA_EA: 0x845DB88
// IDA_NAME: _ZN21DB_UpdateCoolTimeItemD2Ev
// DEMANGLED: DB_UpdateCoolTimeItem::~DB_UpdateCoolTimeItem()
// IDA_EA: 0x845DBB8
// IDA_NAME: _ZN21DB_UpdateCoolTimeItemD0Ev
// DEMANGLED: DB_UpdateCoolTimeItem::~DB_UpdateCoolTimeItem() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateCoolTimeItem::~DB_UpdateCoolTimeItem() {}

// IDA_EA: 0x8430570
// IDA_NAME: _ZN21DB_UpdateCoolTimeItem8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCoolTimeItem::dispatch(int,int,Stream *)
bool DB_UpdateCoolTimeItem::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!stream->enable_out(kSigSaveAndLoadContinuousItemInfoSize)) {
        return false;
    }
    unsigned char* payload =
        reinterpret_cast<unsigned char*>(stream->get_buffer());
    stream->out_ptr(kSigSaveAndLoadContinuousItemInfoSize);

    char* item_blob =
        db_handle->blob_to_str(0, payload + 4, kContinuousItemInfoBytes);
    db_handle->set_query(
        "upDate charac_item_stat set cooltime_item='%s' where charac_no=%u",
        item_blob,
        DB_CONTINUOUS_ITEM_U32(payload, 0));

    bool need_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount()) {
            need_insert = false;
        }
    }
    if (need_insert) {
        db_handle->set_query(
            "inSert into charac_item_stat(charac_no, cooltime_item) values(%u, '%s')",
            DB_CONTINUOUS_ITEM_U32(payload, 0),
            item_blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

#undef DB_CONTINUOUS_ITEM_U32
