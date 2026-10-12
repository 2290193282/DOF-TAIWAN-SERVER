#include "old_data/db/legacy_dispatch/db_last_msg_db.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844E3F2
// IDA_NAME: _ZN12DB_LastMsgDbC2Ev
// DEMANGLED: DB_LastMsgDb::DB_LastMsgDb(void)
DB_LastMsgDb::DB_LastMsgDb() {}

// IDA_EA: 0x845F262
// IDA_NAME: _ZN12DB_LastMsgDbD2Ev
// DEMANGLED: DB_LastMsgDb::~DB_LastMsgDb()
// IDA_EA: 0x845F292
// IDA_NAME: _ZN12DB_LastMsgDbD0Ev
// DEMANGLED: DB_LastMsgDb::~DB_LastMsgDb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LastMsgDb::~DB_LastMsgDb() {}

// IDA_EA: 0x8421BCC
// IDA_NAME: _ZN12DB_LastMsgDb8dispatchEiiP6Stream
// DEMANGLED: DB_LastMsgDb::dispatch(int,int,Stream *)
bool DB_LastMsgDb::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)stream;

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 23021);
    CStreamGuard guard(out_stream, true);
    guard << 151;
    guard << user_idx;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
