#include "old_data/db/event/db_update_item_count_value.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalUpdateItemCountValue = 0x240,
};

}  // namespace

// IDA_EA: 0x844F636
// IDA_NAME: _ZN23DB_UpdateItemCountValueC2Ev
// DEMANGLED: DB_UpdateItemCountValue::DB_UpdateItemCountValue(void)
DB_UpdateItemCountValue::DB_UpdateItemCountValue() {}

// IDA_EA: 0x845BFCE
// IDA_NAME: _ZN23DB_UpdateItemCountValueD2Ev
// DEMANGLED: DB_UpdateItemCountValue::~DB_UpdateItemCountValue()
// IDA_EA: 0x845BFFE
// IDA_NAME: _ZN23DB_UpdateItemCountValueD0Ev
// DEMANGLED: DB_UpdateItemCountValue::~DB_UpdateItemCountValue() deleting destructor
DB_UpdateItemCountValue::~DB_UpdateItemCountValue() {}

// IDA_EA: 0x843EB6E
// IDA_NAME: _ZN23DB_UpdateItemCountValue8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateItemCountValue::dispatch(int,int,Stream *)
bool DB_UpdateItemCountValue::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    unsigned int count = 0;
    *stream >> account_id;
    *stream >> count;

    db_handle->set_query(
        "upDate %s set item_cnt=item_cnt+%d where m_id=%s",
        get_item_count_table_name(),
        count,
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into %s(m_id, item_cnt) values(%s, %d)",
            get_item_count_table_name(),
            NumberToString(account_id, 0),
            count);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x843ECA4
// IDA_NAME: _ZN23DB_UpdateItemCountValue11makeRequestEjj
// DEMANGLED: DB_UpdateItemCountValue::makeRequest(unsigned int,unsigned int)
void DB_UpdateItemCountValue::makeRequest(unsigned int account_id,
                                          unsigned int count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 38414);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateItemCountValue;
    *guard << -1;
    *guard << account_id;
    *guard << count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
