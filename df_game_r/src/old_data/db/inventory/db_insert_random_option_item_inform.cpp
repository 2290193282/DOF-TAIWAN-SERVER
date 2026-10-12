#include "old_data/db/inventory/db_insert_random_option_item_inform.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844F8F2
// IDA_NAME: _ZN31DB_InsertRandomOptionItemInformC2Ev
// DEMANGLED: DB_InsertRandomOptionItemInform::DB_InsertRandomOptionItemInform(void)
DB_InsertRandomOptionItemInform::DB_InsertRandomOptionItemInform() {}

// IDA_EA: 0x845B91A
// IDA_NAME: _ZN31DB_InsertRandomOptionItemInformD2Ev
// DEMANGLED: DB_InsertRandomOptionItemInform::~DB_InsertRandomOptionItemInform()
// IDA_EA: 0x845B94A
// IDA_NAME: _ZN31DB_InsertRandomOptionItemInformD0Ev
// DEMANGLED: DB_InsertRandomOptionItemInform::~DB_InsertRandomOptionItemInform() deleting destructor
DB_InsertRandomOptionItemInform::~DB_InsertRandomOptionItemInform() {}

// IDA_EA: 0x8442C6A
// IDA_NAME: _ZN31DB_InsertRandomOptionItemInform8dispatchEiiP6Stream
// DEMANGLED: DB_InsertRandomOptionItemInform::dispatch(int,int,Stream *)
bool DB_InsertRandomOptionItemInform::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    unsigned char check_count = 0;
    *stream >> account_id;
    *stream >> check_count;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "inSert into check_pick_up_random_option_item(m_id, check_count) values(%s, %u)",
        account_id_text,
        static_cast<unsigned int>(check_count));
    if (db_handle->exec(true) == true) {
        return true;
    }

    account_id_text = NumberToString(account_id, 0);
    cMyTrace trace(
        "virtual bool DB_InsertRandomOptionItemInform::dispatch(int, int, Stream*)",
        42110,
        5);
    trace("inSert into check_pickup_random_option_item failed at m_id(%s) ",
          account_id_text);
    return false;
}

// IDA_EA: 0x8442D70
// IDA_NAME: _ZN31DB_InsertRandomOptionItemInform11makeRequestEijh
// DEMANGLED: DB_InsertRandomOptionItemInform::makeRequest(int,unsigned int,unsigned char)
void DB_InsertRandomOptionItemInform::makeRequest(
    int user_idx,
    unsigned int account_id,
    unsigned char check_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 42120);
    CStreamGuard guard(stream, true);
    *guard << 665;
    *guard << user_idx;
    *guard << account_id;
    *guard << check_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
