#include "old_data/db/account/db_update_return_user_first_login.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844F5C6
// IDA_NAME: _ZN29DB_UpdateReturnUserFirstLoginC2Ev
// DEMANGLED: DB_UpdateReturnUserFirstLogin::DB_UpdateReturnUserFirstLogin(void)
DB_UpdateReturnUserFirstLogin::DB_UpdateReturnUserFirstLogin() {}

// IDA_EA: 0x845C106
// IDA_NAME: _ZN29DB_UpdateReturnUserFirstLoginD2Ev
// DEMANGLED: DB_UpdateReturnUserFirstLogin::~DB_UpdateReturnUserFirstLogin()
// IDA_EA: 0x845C136
// IDA_NAME: _ZN29DB_UpdateReturnUserFirstLoginD0Ev
// DEMANGLED: DB_UpdateReturnUserFirstLogin::~DB_UpdateReturnUserFirstLogin() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateReturnUserFirstLogin::~DB_UpdateReturnUserFirstLogin() {}

// IDA_EA: 0x843DFFE
// IDA_NAME: _ZN29DB_UpdateReturnUserFirstLogin8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateReturnUserFirstLogin::dispatch(int,int,Stream *)
bool DB_UpdateReturnUserFirstLogin::dispatch(int signal, int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int m_id = 0;
    int first_login = 0;
    int expire_time = 0;
    stream->get_binary(&m_id, sizeof(m_id));
    stream->get_binary(&first_login, sizeof(first_login));
    stream->get_binary(&expire_time, sizeof(expire_time));

    const char* account_id = NumberToString(m_id, 0);
    db_handle->set_query(
        "inSert into return_user(m_id, expire_time, first_login) values(%s, from_unixtime(%d), %d)",
        account_id,
        expire_time,
        first_login);
    if (db_handle->exec(true) == true) {
        return true;
    }
    if (db_handle->getDBError() != 1062) {
        return false;
    }

    account_id = NumberToString(m_id, 0);
    db_handle->set_query(
        "upDate return_user set expire_time = from_unixtime(%d), first_login=%d where m_id=%s",
        expire_time,
        first_login,
        account_id);
    if (db_handle->exec(true) == true) {
        return false;
    }
    return false;
}

// IDA_EA: 0x843E146
// IDA_NAME: _ZN29DB_UpdateReturnUserFirstLogin11makeRequestEjii
// DEMANGLED: DB_UpdateReturnUserFirstLogin::makeRequest(unsigned int,int,int)
void DB_UpdateReturnUserFirstLogin::makeRequest(unsigned int m_id,
                                                int first_login,
                                                int expire_time) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 37601);
    CStreamGuard guard(stream, true);
    *guard << 0x21F;
    *guard << -1;
    *guard << m_id;
    *guard << first_login;
    *guard << expire_time;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
