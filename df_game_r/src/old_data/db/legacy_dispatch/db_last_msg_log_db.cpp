#include "old_data/db/legacy_dispatch/db_last_msg_log_db.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844E40E
// IDA_NAME: _ZN15DB_LastMsgLogDbC2Ev
// DEMANGLED: DB_LastMsgLogDb::DB_LastMsgLogDb(void)
DB_LastMsgLogDb::DB_LastMsgLogDb() {}

// IDA_EA: 0x845F214
// IDA_NAME: _ZN15DB_LastMsgLogDbD2Ev
// DEMANGLED: DB_LastMsgLogDb::~DB_LastMsgLogDb()
// IDA_EA: 0x845F244
// IDA_NAME: _ZN15DB_LastMsgLogDbD0Ev
// DEMANGLED: DB_LastMsgLogDb::~DB_LastMsgLogDb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LastMsgLogDb::~DB_LastMsgLogDb() {}

// IDA_EA: 0x8421C8E
// IDA_NAME: _ZN15DB_LastMsgLogDb8dispatchEiiP6Stream
// DEMANGLED: DB_LastMsgLogDb::dispatch(int,int,Stream *)
bool DB_LastMsgLogDb::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)stream;

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 23031);
    CStreamGuard guard(out_stream, true);
    guard << 151;
    guard << user_idx;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
