#include "old_data/db/creature/db_insert_creature_item.h"

#include "old_data/db/creature/compound_creature_marker.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


extern int GetIdentityFromDB(MySQL* mysql)
    __asm__("_Z17GetIdentityFromDBP5MySQL");

namespace {

enum {
    kDbSignalInsertCreatureItemResult = 136,
};

bool QueueInsertResult(int user_idx,
                       const SIG_CREATURE_ITEM* payload,
                       int ui_id) {
    Stream* result_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                               21960);
    if (!result_stream) return false;
    CStreamGuard guard(result_stream, true);
    *guard << kDbSignalInsertCreatureItemResult;
    *guard << user_idx;
    SIG_CREATURE_ITEM* result = guard.GetInBuffer<SIG_CREATURE_ITEM>();
    if (!result) return false;
    memcpy(result, payload, sizeof(*result));
    result->detail.ui_id = static_cast<uint32_t>(ui_id);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

void CompensateUnpublishedInsert(MySQL* db_handle,
                                 int ui_id,
                                 unsigned int charac_no) {
    if (!db_handle || ui_id <= 0) return;
    db_handle->set_query(
        "upDate creature_items set stat=1 where ui_id=%d and charac_no=%u",
        ui_id,
        charac_no);
    (void)db_handle->exec(true);
}

}  // namespace

// IDA_EA: 0x844E34A
// IDA_NAME: _ZN21DB_InsertCreatureItemC2Ev
// DEMANGLED: DB_InsertCreatureItem::DB_InsertCreatureItem(void)
DB_InsertCreatureItem::DB_InsertCreatureItem() {}

// IDA_EA: 0x845F436
// IDA_NAME: _ZN21DB_InsertCreatureItemD2Ev
// DEMANGLED: DB_InsertCreatureItem::~DB_InsertCreatureItem()
// IDA_EA: 0x845F466
// IDA_NAME: _ZN21DB_InsertCreatureItemD0Ev
// DEMANGLED: DB_InsertCreatureItem::~DB_InsertCreatureItem() deleting destructor
DB_InsertCreatureItem::~DB_InsertCreatureItem() {}

// IDA_EA: 0x841F8D4
// IDA_NAME: _ZN21DB_InsertCreatureItem8dispatchEiiP6Stream
// DEMANGLED: DB_InsertCreatureItem::dispatch(int,int,Stream *)
bool DB_InsertCreatureItem::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;

    SIG_CREATURE_ITEM* payload =
        stream->GetOutBuffer<SIG_CREATURE_ITEM>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    if (insertCreatureItem(db_handle, payload) != true) {
        // Native CMD226 does not mutate inventory until it sees a durable
        // ui_id.  Return an explicit zero-id failure for its marker; legacy
        // signal135 callers keep the original no-callback failure behavior.
        if (payload->update_kind == compound_creature::kInsertUpdateKind) {
            return QueueInsertResult(user_idx, payload, 0);
        }
        return false;
    }

    const int ui_id = GetIdentityFromDB(db_handle);
    if (QueueInsertResult(user_idx, payload, ui_id)) return true;

    // The row exists but no owner can learn its ui_id.  Only the marker path
    // is guaranteed not to have exposed the item yet, so it is safe to
    // soft-delete here; preserve legacy behavior for all other signal135s.
    if (payload->update_kind == compound_creature::kInsertUpdateKind) {
        CompensateUnpublishedInsert(db_handle,
                                    ui_id,
                                    payload->detail.charac_no);
    }
    return false;
}

// IDA_EA: 0x841FA28
// IDA_NAME: _ZN21DB_InsertCreatureItem18insertCreatureItemEP5MySQLPK17SIG_CREATURE_ITEM
// DEMANGLED: DB_InsertCreatureItem::insertCreatureItem(MySQL *,SIG_CREATURE_ITEM const*)
bool DB_InsertCreatureItem::insertCreatureItem(MySQL* db_handle,
                                               const SIG_CREATURE_ITEM* payload) {
    if (!payload->current_creature_flag) {
        db_handle->set_query(
            "upDate inventory set creature_flag=%d where charac_no=%d",
            1,
            payload->detail.charac_no);
        if (db_handle->exec(true) != true) {
            cMyTrace trace(
                "static bool DB_InsertCreatureItem::insertCreatureItem(MySQL*, const SIG_CREATURE_ITEM*)",
                21981,
                5);
            trace("DB_InsertCreatureItem::dispatch, exec() ERROR charac_no=%u",
                  payload->detail.charac_no);
            return false;
        }
    }

    char expire_date[64];
    char delete_date[64];
    memset(expire_date, 0, sizeof(expire_date));
    memset(delete_date, 0, sizeof(delete_date));
    strcpy(expire_date, "'9999-12-31 23:59:59'");
    strcpy(delete_date, "'9999-12-31 23:59:59'");

    const int duration_days = payload->detail.expiration_value;
    const int delete_tick = payload->detail.delete_tick;
    if (duration_days) {
        CSystemTime* system_time = GlobalData::system_time();
        const int now = system_time ? system_time->getCurSec() : 0;
        snprintf(expire_date,
                 sizeof(expire_date),
                 "from_unixtime('%d')",
                 now + 86400 * duration_days);
    }
    if (delete_tick) {
        snprintf(delete_date,
                 sizeof(delete_date),
                 "from_unixtime('%d')",
                 delete_tick);
    }
    if (!duration_days && delete_tick) {
        snprintf(expire_date,
                 sizeof(expire_date),
                 "from_unixtime('%d')",
                 delete_tick);
    }

    db_handle->set_query(
        "inSert into creature_items(charac_no,slot,it_id,expire_date,delete_date,reg_date,name,stomach,exp,endurance,no_charge,stat,creature_type)values(%d,%d,%d,%s,%s,NOW(),'%s',%d,%d,%d,0,%d,%d)",
        payload->detail.charac_no,
        payload->detail.slot,
        payload->inven_item_copy.item_id,
        expire_date,
        delete_date,
        payload->detail.name,
        payload->detail.stomach,
        payload->detail.experience,
        payload->inven_item_copy.endurance,
        payload->detail.state,
        payload->detail.creature_type);
    return db_handle->exec(true) == true;
}
