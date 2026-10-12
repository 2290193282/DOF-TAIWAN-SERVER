#include "old_data/db/creature/db_update_creature_item.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <string.h>


extern char* AddFieldString(bool& first,
                            char* dest,
                            int flags,
                            int mask,
                            const char* format,
                            ...)
    __asm__("_Z14AddFieldStringRbPciiPKcz");


namespace {

enum {
    kDbSignalUpdateCreatureItemResult = 139,
};

}  // namespace

// IDA_EA: 0x844E382
// IDA_NAME: _ZN21DB_UpdateCreatureItemC2Ev
// DEMANGLED: DB_UpdateCreatureItem::DB_UpdateCreatureItem(void)
DB_UpdateCreatureItem::DB_UpdateCreatureItem() {}

// IDA_EA: 0x845F34C
// IDA_NAME: _ZN21DB_UpdateCreatureItemD2Ev
// DEMANGLED: DB_UpdateCreatureItem::~DB_UpdateCreatureItem()
// IDA_EA: 0x845F37C
// IDA_NAME: _ZN21DB_UpdateCreatureItemD0Ev
// DEMANGLED: DB_UpdateCreatureItem::~DB_UpdateCreatureItem() deleting destructor
DB_UpdateCreatureItem::~DB_UpdateCreatureItem() {}

// IDA_EA: 0x84200A8
// IDA_NAME: _ZN21DB_UpdateCreatureItem8GetFieldEP5MySQLPcP17SIG_CREATURE_ITEM
// DEMANGLED: DB_UpdateCreatureItem::GetField(MySQL *,char *,SIG_CREATURE_ITEM *)
char* DB_UpdateCreatureItem::GetField(MySQL* db_handle,
                                      char* dest,
                                      SIG_CREATURE_ITEM* payload) {
    char escaped_name[25];
    memset(escaped_name, 0, sizeof(escaped_name));
    bool first = true;
    db_handle->escape_string(escaped_name, payload->detail.name);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   1,
                   "name='%s'",
                   escaped_name);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   2,
                   "stomach=%d",
                   payload->detail.stomach);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   4,
                   "creature_type=%d",
                   payload->detail.creature_type);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   8,
                   "exp=%d",
                   payload->detail.experience);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   16,
                   "charac_no=%d",
                   payload->detail.charac_no);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   32,
                   "slot=%d",
                   payload->detail.slot);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   64,
                   "it_id=%d",
                   payload->inven_item_copy.item_id);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   128,
                   "no_charge=%d",
                   payload->detail.no_charge);
    AddFieldString(first,
                   dest,
                   payload->update_mask,
                   256,
                   "stat=%d",
                   payload->detail.state);
    if (!payload->detail.expiration_value && !payload->detail.delete_tick) {
        return AddFieldString(first,
                              dest,
                              payload->update_mask,
                              512,
                              "expire_date='9999-12-31 23:59:59'");
    }
    if (!payload->detail.expiration_value && payload->detail.delete_tick) {
        return AddFieldString(first,
                              dest,
                              payload->update_mask,
                              512,
                              "expire_date=from_unixtime('%d')",
                              payload->detail.delete_tick);
    }
    CSystemTime* system_time = GlobalData::system_time();
    const int now = system_time ? system_time->getCurSec() : 0;
    return AddFieldString(first,
                          dest,
                          payload->update_mask,
                          512,
                          "expire_date=from_unixtime('%d')",
                          86400 * payload->detail.expiration_value + now);
}

// IDA_EA: 0x84203E8
// IDA_NAME: _ZN21DB_UpdateCreatureItem8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCreatureItem::dispatch(int,int,Stream *)
bool DB_UpdateCreatureItem::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;

    SIG_CREATURE_ITEM* payload = stream->GetOutBuffer<SIG_CREATURE_ITEM>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    const int update_kind = payload->update_kind;
    if (update_kind == 3) {
        if (payload->current_creature_flag == 1) {
            db_handle->set_query(
                "upDate inventory set creature_flag=%d where charac_no=%d",
                0,
                payload->previous_charac_no);
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
    } else if (update_kind > 3) {
        if (update_kind == 4) {
            if (!payload->current_creature_flag) {
                db_handle->set_query(
                    "upDate inventory set creature_flag=%d where charac_no=%d",
                    1,
                    payload->detail.charac_no);
                if (db_handle->exec(true) != true) {
                    return false;
                }
            }
        } else if (update_kind == 5 &&
                   payload->current_creature_flag == 1) {
            db_handle->set_query(
                "upDate inventory set creature_flag=%d where charac_no=%d",
                0,
                payload->previous_charac_no);
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
    } else if (update_kind == 1) {
        if (!payload->current_creature_flag) {
            db_handle->set_query(
                "upDate inventory set creature_flag=%d where charac_no=%d",
                1,
                payload->detail.charac_no);
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
        if (payload->previous_creature_flag == 1) {
            db_handle->set_query(
                "upDate inventory set creature_flag=%d where charac_no=%d",
                0,
                payload->previous_charac_no);
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
    }

    char fields[1024];
    memset(fields, 0, sizeof(fields));
    GetField(db_handle, fields, payload);
    db_handle->set_query("upDate creature_items set %s where ui_id=%d",
                         fields,
                         payload->detail.ui_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (payload->update_kind != 4 && payload->update_kind != 1) {
        return true;
    }

    Stream* result_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                               22288);
    CStreamGuard guard(result_stream, true);
    *guard << kDbSignalUpdateCreatureItemResult;
    *guard << user_idx;
    SIG_CREATURE_ITEM* result = guard.GetInBuffer<SIG_CREATURE_ITEM>();
    if (!result) {
        return false;
    }
    memcpy(result, payload, sizeof(*result));

    db_handle->set_query(
        "seLect slot,it_id,name,stomach,exp,endurance,creature_type,no_charge,unix_timestamp(expire_date) from creature_items where ui_id=%d",
        payload->detail.ui_id);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22313,
            5);
        trace("DB_UpdateCreatureItem::dispatch, exec() ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22321,
            5);
        trace("DB_UpdateCreatureItem::dispatch, fetch() ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    if (db_handle->get_int(0, result->detail.slot) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22329,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(0) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }

    unsigned long item_id = 0;
    if (db_handle->get_ulong(1, item_id) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22337,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_ushort(1) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    result->inven_item_copy.item_id = static_cast<uint32_t>(item_id);
    if (db_handle->get_str(2, result->detail.name, 13) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22358,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(2) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    if (db_handle->get_int(3, result->detail.stomach) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22366,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(3) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    if (db_handle->get_int(4, result->detail.experience) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22374,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(4) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }

    unsigned short endurance = 0;
    if (db_handle->get_ushort(5, endurance) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22381,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_ushort(5) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    result->inven_item_copy.endurance = endurance;

    int creature_type = 0;
    int no_charge = 0;
    if (db_handle->get_int(6, creature_type) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22389,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(6) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    result->detail.creature_type = static_cast<uint8_t>(creature_type);
    if (db_handle->get_int(7, no_charge) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22398,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(7) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }
    result->detail.no_charge = static_cast<uint8_t>(no_charge);
    if (db_handle->get_int(8, result->detail.expiration_value) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateCreatureItem::dispatch(int, int, Stream*)",
            22420,
            5);
        trace("DB_UpdateCreatureItem::dispatch, get_int(9) ERROR charac_no=%u",
              payload->detail.charac_no);
        return false;
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
