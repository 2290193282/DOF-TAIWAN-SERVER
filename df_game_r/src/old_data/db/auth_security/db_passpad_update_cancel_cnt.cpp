#include "old_data/db/auth_security/db_passpad_update_cancel_cnt.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



#pragma pack(push, 1)
struct SIG_PASSPAD_UPDATE_CANCEL_CNT {
    uint32_t account_id;     // +0
    uint32_t cancel_count;   // +4
    char ip[16];             // +8
};
#pragma pack(pop)

typedef char SIG_PASSPAD_UPDATE_CANCEL_CNT_Size_Must_Be_24[
    (sizeof(SIG_PASSPAD_UPDATE_CANCEL_CNT) == 24) ? 1 : -1];

// IDA_EA: 0x844E4EE
// IDA_NAME: _ZN25DB_PassPadUpdateCancelCntC2Ev
// DEMANGLED: DB_PassPadUpdateCancelCnt::DB_PassPadUpdateCancelCnt(void)
DB_PassPadUpdateCancelCnt::DB_PassPadUpdateCancelCnt() {}

// IDA_EA: 0x845EF56
// IDA_NAME: _ZN25DB_PassPadUpdateCancelCntD2Ev
// DEMANGLED: DB_PassPadUpdateCancelCnt::~DB_PassPadUpdateCancelCnt()
// IDA_EA: 0x845EF86
// IDA_NAME: _ZN25DB_PassPadUpdateCancelCntD0Ev
// DEMANGLED: DB_PassPadUpdateCancelCnt::~DB_PassPadUpdateCancelCnt() deleting destructor
DB_PassPadUpdateCancelCnt::~DB_PassPadUpdateCancelCnt() {}

// IDA_EA: 0x8423FB6
// IDA_NAME: _ZN25DB_PassPadUpdateCancelCnt8dispatchEiiP6Stream
// DEMANGLED: DB_PassPadUpdateCancelCnt::dispatch(int,int,Stream *)
bool DB_PassPadUpdateCancelCnt::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_PASSPAD_UPDATE_CANCEL_CNT payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    db_handle->set_query(
        "upDate member_mousepass set cancel_cnt=%d where m_id=%s and enable_flag = '1'",
        payload.cancel_count,
        NumberToString(payload.account_id, 0));
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x842403A
// IDA_NAME: _ZN25DB_PassPadUpdateCancelCnt11makeRequestEjjPc
// DEMANGLED: DB_PassPadUpdateCancelCnt::makeRequest(unsigned int,unsigned int,char *)
void DB_PassPadUpdateCancelCnt::makeRequest(unsigned int account_id,
                                            unsigned int cancel_count,
                                            char* ip) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 24414);
    CStreamGuard guard(stream, true);
    *guard << 165;
    *guard << -1;

    SIG_PASSPAD_UPDATE_CANCEL_CNT* payload =
        static_cast<SIG_PASSPAD_UPDATE_CANCEL_CNT*>(
            guard->GetInBufferUntyped(sizeof(SIG_PASSPAD_UPDATE_CANCEL_CNT)));
    payload->account_id = account_id;
    payload->cancel_count = cancel_count;
    strncpy(payload->ip, ip, 0x10u);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
