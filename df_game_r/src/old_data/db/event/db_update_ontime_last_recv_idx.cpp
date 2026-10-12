#include "old_data/db/event/db_update_ontime_last_recv_idx.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalUpdateOntimeLastRecvIdx = 0x297,
};

}  // namespace

// IDA_EA: 0x844F946
// IDA_NAME: _ZN26DB_UpdateOntimeLastRecvIdxC2Ev
// DEMANGLED: DB_UpdateOntimeLastRecvIdx::DB_UpdateOntimeLastRecvIdx(void)
DB_UpdateOntimeLastRecvIdx::DB_UpdateOntimeLastRecvIdx() {}

// IDA_EA: 0x845B9B6
// IDA_NAME: _ZN26DB_UpdateOntimeLastRecvIdxD2Ev
// DEMANGLED: DB_UpdateOntimeLastRecvIdx::~DB_UpdateOntimeLastRecvIdx()
// IDA_EA: 0x845B9E6
// IDA_NAME: _ZN26DB_UpdateOntimeLastRecvIdxD0Ev
// DEMANGLED: DB_UpdateOntimeLastRecvIdx::~DB_UpdateOntimeLastRecvIdx() deleting destructor
DB_UpdateOntimeLastRecvIdx::~DB_UpdateOntimeLastRecvIdx() {}

// IDA_EA: 0x844280A
// IDA_NAME: _ZN26DB_UpdateOntimeLastRecvIdx8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateOntimeLastRecvIdx::dispatch(int,int,Stream *)
bool DB_UpdateOntimeLastRecvIdx::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(13),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    int last_recv_idx = 0;
    *stream >> account_id;
    *stream >> last_recv_idx;

    const unsigned int table_idx = account_id % 10;
    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "upDate event_1112_ontime_user_%u set last_no=%u where m_id=%s",
        table_idx,
        last_recv_idx,
        account_id_text);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    account_id_text = NumberToString(account_id, 0);
    cMyTrace trace(
        "virtual bool DB_UpdateOntimeLastRecvIdx::dispatch(int, int, Stream*)",
        41991,
        5);
    trace("upDate event_1112_ontime_user_%u set last_no=%u failed at m_id(%s)",
          table_idx,
          last_recv_idx,
          account_id_text);
    return false;
}

// IDA_EA: 0x8442966
// IDA_NAME: _ZN26DB_UpdateOntimeLastRecvIdx11makeRequestEiji
// DEMANGLED: DB_UpdateOntimeLastRecvIdx::makeRequest(int,unsigned int,int)
void DB_UpdateOntimeLastRecvIdx::makeRequest(int user_idx,
                                             unsigned int account_id,
                                             int last_recv_idx) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 41999);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateOntimeLastRecvIdx;
    *guard << user_idx;
    *guard << account_id;
    *guard << last_recv_idx;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
