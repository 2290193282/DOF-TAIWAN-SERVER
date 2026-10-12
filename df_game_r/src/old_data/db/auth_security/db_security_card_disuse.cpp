#include "old_data/db/auth_security/db_security_card_disuse.h"

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
    kDbSignalSecurityCardDisuse = 254,
    kSecurityDbHandle = 6,
    kSecurityCardDisuseHistoryType = 2,
};

}  // namespace

// IDA_EA: 0x844E8DE
// IDA_NAME: _ZN21DB_SecurityCardDisuseC2Ev
// DEMANGLED: DB_SecurityCardDisuse::DB_SecurityCardDisuse(void)
DB_SecurityCardDisuse::DB_SecurityCardDisuse() {}

// IDA_EA: 0x845E410
// IDA_NAME: _ZN21DB_SecurityCardDisuseD2Ev
// DEMANGLED: DB_SecurityCardDisuse::~DB_SecurityCardDisuse()
// IDA_EA: 0x845E440
// IDA_NAME: _ZN21DB_SecurityCardDisuseD0Ev
// DEMANGLED: DB_SecurityCardDisuse::~DB_SecurityCardDisuse()
DB_SecurityCardDisuse::~DB_SecurityCardDisuse() {}

// IDA_EA: 0x842B6E6
// IDA_NAME: _ZN21DB_SecurityCardDisuse8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardDisuse::dispatch(int,int,Stream *)
bool DB_SecurityCardDisuse::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kSecurityDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const SIG_SECURITY_CARD_DISUSE* request =
        stream->GetOutBuffer<SIG_SECURITY_CARD_DISUSE>();
    turnOffSecurityFlag(db_handle, request->account_id);
    db_handle->set_query(
        "upDate member_security_card set apply_flag=0 where m_id=%s",
        NumberToString(request->account_id, 0));
    db_handle->exec(true);
    saveSecurityCardHistory(db_handle,
                            request->account_id,
                            kSecurityCardDisuseHistoryType);
    return true;
}

// IDA_EA: 0x842B790
// IDA_NAME: _ZN21DB_SecurityCardDisuse11makeRequestEj
// DEMANGLED: DB_SecurityCardDisuse::makeRequest(unsigned int)
void DB_SecurityCardDisuse::makeRequest(unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28205);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSecurityCardDisuse;
    *guard << -1;
    SIG_SECURITY_CARD_DISUSE* request =
        guard->GetInBuffer<SIG_SECURITY_CARD_DISUSE>();
    request->account_id = account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
