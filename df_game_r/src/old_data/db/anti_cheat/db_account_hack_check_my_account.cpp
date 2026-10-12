#include "old_data/db/anti_cheat/db_account_hack_check_my_account.h"

#include "old_data/db/anti_cheat/auto_punish_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>


namespace {

enum {
    kDbSignalCheckMyAccount = 395,
    kDispatchSignalCheckMyAccountOk = 396,
    kCharacterDbHandle = 2,
};

}  // namespace

// IDA_EA: 0x844F1BA
// IDA_NAME: _ZN29DB_AccountHack_CheckMyAccountC2Ev
// DEMANGLED: DB_AccountHack_CheckMyAccount::DB_AccountHack_CheckMyAccount(void)
DB_AccountHack_CheckMyAccount::DB_AccountHack_CheckMyAccount() {}

// IDA_EA: 0x845CB62
// IDA_NAME: _ZN29DB_AccountHack_CheckMyAccountD2Ev
// DEMANGLED: DB_AccountHack_CheckMyAccount::~DB_AccountHack_CheckMyAccount()
// IDA_EA: 0x845CB92
// IDA_NAME: _ZN29DB_AccountHack_CheckMyAccountD0Ev
// DEMANGLED: DB_AccountHack_CheckMyAccount::~DB_AccountHack_CheckMyAccount()
DB_AccountHack_CheckMyAccount::~DB_AccountHack_CheckMyAccount() {}

// IDA_EA: 0x8439F06
// IDA_NAME: _ZN29DB_AccountHack_CheckMyAccount8dispatchEiiP6Stream
// DEMANGLED: DB_AccountHack_CheckMyAccount::dispatch(int,int,Stream *)
bool DB_AccountHack_CheckMyAccount::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;

    const SIG_CHECK_MY_ACCOUNT_IN_DB* request =
        stream->GetOutBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>();
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kCharacterDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));

    char escaped_name[59];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, request->charac_name);
    db_handle->set_query(
        "seLect m_id from charac_info where charac_name='%s' limit 1",
        escaped_name);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }

    unsigned int owner_account_id = 0;
    if (db_handle->get_int(0, owner_account_id) != true) {
        return false;
    }
    if (request->account_id == owner_account_id) {
        return true;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 34867);
    CStreamGuard guard(out_stream, true);
    *guard << kDispatchSignalCheckMyAccountOk;
    *guard << user_idx;
    SIG_CHECK_MY_ACCOUNT_OK* response =
        guard->GetInBuffer<SIG_CHECK_MY_ACCOUNT_OK>();
    response->uid = request->uid;
    response->package_money = request->package_money;
    response->owner_account_id = owner_account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x843A13E
// IDA_NAME: _ZN29DB_AccountHack_CheckMyAccount11makeRequestEjjjjPci
// DEMANGLED: DB_AccountHack_CheckMyAccount::makeRequest(unsigned int, unsigned int, unsigned int, unsigned int, char *, int)
void DB_AccountHack_CheckMyAccount::makeRequest(
    unsigned int user_idx,
    unsigned int account_id,
    unsigned int reply_uid,
    unsigned int package_money,
    char* charac_name,
    int name_length) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34886);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalCheckMyAccount;
    *guard << static_cast<int>(user_idx);

    SIG_CHECK_MY_ACCOUNT_IN_DB* request =
        guard->GetInBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>();
    request->uid = reply_uid;
    request->account_id = account_id;
    request->package_money = package_money;
    memset(request->charac_name, 0, sizeof(request->charac_name));
    strncpy(request->charac_name, charac_name, name_length);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
