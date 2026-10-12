#include "old_data/db/auth_security/db_security_card_update_cancel_cnt.h"

#include "old_data/db/auth_security/security_card_payloads.h"
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
    kDbSignalSecurityCardUpdateCancelCnt = 258,
    kSecurityDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844E94E
// IDA_NAME: _ZN30DB_SecurityCardUpdateCancelCntC2Ev
// DEMANGLED: DB_SecurityCardUpdateCancelCnt::DB_SecurityCardUpdateCancelCnt(void)
DB_SecurityCardUpdateCancelCnt::DB_SecurityCardUpdateCancelCnt() {}

// IDA_EA: 0x845E2D8
// IDA_NAME: _ZN30DB_SecurityCardUpdateCancelCntD2Ev
// DEMANGLED: DB_SecurityCardUpdateCancelCnt::~DB_SecurityCardUpdateCancelCnt()
// IDA_EA: 0x845E308
// IDA_NAME: _ZN30DB_SecurityCardUpdateCancelCntD0Ev
// DEMANGLED: DB_SecurityCardUpdateCancelCnt::~DB_SecurityCardUpdateCancelCnt()
DB_SecurityCardUpdateCancelCnt::~DB_SecurityCardUpdateCancelCnt() {}

// IDA_EA: 0x842C08E
// IDA_NAME: _ZN30DB_SecurityCardUpdateCancelCnt8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardUpdateCancelCnt::dispatch(int,int,Stream *)
bool DB_SecurityCardUpdateCancelCnt::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kSecurityDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const SIG_SECURITY_CARD_UPDATE_FAIL_CNT* request =
        stream->GetOutBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>();
    db_handle->set_query(
        "upDate member_security_card set cancel_cnt=%d where m_id=%s and apply_flag=1",
        request->count,
        NumberToString(request->account_id, 0));
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x842C112
// IDA_NAME: _ZN30DB_SecurityCardUpdateCancelCnt11makeRequestEjj
// DEMANGLED: DB_SecurityCardUpdateCancelCnt::makeRequest(unsigned int, unsigned int)
void DB_SecurityCardUpdateCancelCnt::makeRequest(
    unsigned int account_id,
    unsigned int cancel_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28453);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSecurityCardUpdateCancelCnt;
    *guard << -1;
    SIG_SECURITY_CARD_UPDATE_FAIL_CNT* request =
        guard->GetInBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>();
    request->account_id = account_id;
    request->count = cancel_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
