#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x843D19A
// IDA_NAME: _ZN19DB_RequestJoinGuild11makeRequestEiP22SIG_REQUEST_JOIN_GUILD
// DEMANGLED: DB_RequestJoinGuild::makeRequest(int,SIG_REQUEST_JOIN_GUILD *)
void DB_RequestJoinGuild::makeRequest(int user_idx,
                                      SIG_REQUEST_JOIN_GUILD* request) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37288);
    CStreamGuard guard(stream, true);
    *guard << 0x217;
    *guard << user_idx;
    SIG_REQUEST_JOIN_GUILD* const destination =
        guard->GetInBuffer<SIG_REQUEST_JOIN_GUILD>();
    std::memcpy(destination, request, sizeof(*destination));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}
