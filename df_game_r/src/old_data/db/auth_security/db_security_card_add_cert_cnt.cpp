#include "old_data/db/auth_security/db_security_card_add_cert_cnt.h"

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
    kDbSignalSecurityCardAddCertCnt = 380,
    kSecurityDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844EF8A
// IDA_NAME: _ZN25DB_SecurityCardAddCertCntC2Ev
// DEMANGLED: DB_SecurityCardAddCertCnt::DB_SecurityCardAddCertCnt(void)
DB_SecurityCardAddCertCnt::DB_SecurityCardAddCertCnt() {}

// IDA_EA: 0x845D3EA
// IDA_NAME: _ZN25DB_SecurityCardAddCertCntD2Ev
// DEMANGLED: DB_SecurityCardAddCertCnt::~DB_SecurityCardAddCertCnt()
// IDA_EA: 0x845D41A
// IDA_NAME: _ZN25DB_SecurityCardAddCertCntD0Ev
// DEMANGLED: DB_SecurityCardAddCertCnt::~DB_SecurityCardAddCertCnt()
DB_SecurityCardAddCertCnt::~DB_SecurityCardAddCertCnt() {}

// IDA_EA: 0x843630E
// IDA_NAME: _ZN25DB_SecurityCardAddCertCnt8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardAddCertCnt::dispatch(int,int,Stream *)
bool DB_SecurityCardAddCertCnt::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kSecurityDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const SIG_SECURITY_CARD_ADD_CERT_CNT* request =
        stream->GetOutBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>();
    db_handle->set_query(
        "upDate member_security_card set cert_cnt = cert_cnt+1  where m_id=%s and apply_flag=1",
        NumberToString(request->account_id, 0));
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x8436388
// IDA_NAME: _ZN25DB_SecurityCardAddCertCnt11makeRequestEj
// DEMANGLED: DB_SecurityCardAddCertCnt::makeRequest(unsigned int)
void DB_SecurityCardAddCertCnt::makeRequest(unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 33177);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSecurityCardAddCertCnt;
    *guard << -1;
    SIG_SECURITY_CARD_ADD_CERT_CNT* request =
        guard->GetInBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>();
    request->account_id = account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
