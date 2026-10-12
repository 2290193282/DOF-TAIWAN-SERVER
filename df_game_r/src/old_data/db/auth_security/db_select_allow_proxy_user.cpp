#include "old_data/db/auth_security/db_select_allow_proxy_user.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844FFF2
// IDA_NAME: _ZN23DB_SelectAllowProxyUserC2Ev
// DEMANGLED: DB_SelectAllowProxyUser::DB_SelectAllowProxyUser(void)
DB_SelectAllowProxyUser::DB_SelectAllowProxyUser() {}

// IDA_EA: 0x845A70A
// IDA_NAME: _ZN23DB_SelectAllowProxyUserD2Ev
// DEMANGLED: DB_SelectAllowProxyUser::~DB_SelectAllowProxyUser()
// IDA_EA: 0x845A73A
// IDA_NAME: _ZN23DB_SelectAllowProxyUserD0Ev
// DEMANGLED: DB_SelectAllowProxyUser::~DB_SelectAllowProxyUser() deleting destructor
DB_SelectAllowProxyUser::~DB_SelectAllowProxyUser() {}

// IDA_EA: 0x844D17C
// IDA_NAME: _ZN23DB_SelectAllowProxyUser8dispatchEiiP6Stream
// DEMANGLED: DB_SelectAllowProxyUser::dispatch(int,int,Stream *)
bool DB_SelectAllowProxyUser::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;

    unsigned int account_id = 0;
    *stream >> account_id;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect m_id from allow_proxy_user where m_id=%s",
                         NumberToString(account_id, 0));
    if (db_handle->exec(true) == true && !db_handle->get_n_rows()) {
        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 49847);
        CStreamGuard guard(out_stream, true);
        *guard << 850;
        *guard << user_idx;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return true;
}

// IDA_EA: 0x844D2D4
// IDA_NAME: _ZN23DB_SelectAllowProxyUser11makeRequestEij
// DEMANGLED: DB_SelectAllowProxyUser::makeRequest(int,unsigned int)
void DB_SelectAllowProxyUser::makeRequest(int user_idx,
                                          unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 49862);
    CStreamGuard guard(stream, true);
    *guard << 850;
    *guard << user_idx;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
