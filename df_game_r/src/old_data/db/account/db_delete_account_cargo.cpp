#include "old_data/db/account/db_delete_account_cargo.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844F406
// IDA_NAME: _ZN21DB_DeleteAccountCargoC2Ev
// DEMANGLED: DB_DeleteAccountCargo::DB_DeleteAccountCargo(void)
DB_DeleteAccountCargo::DB_DeleteAccountCargo() {}

// IDA_EA: 0x845C76C
// IDA_NAME: _ZN21DB_DeleteAccountCargoD2Ev
// DEMANGLED: DB_DeleteAccountCargo::~DB_DeleteAccountCargo()
// IDA_EA: 0x845C79C
// IDA_NAME: _ZN21DB_DeleteAccountCargoD0Ev
// DEMANGLED: DB_DeleteAccountCargo::~DB_DeleteAccountCargo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_DeleteAccountCargo::~DB_DeleteAccountCargo() {}

// IDA_EA: 0x843B24E
// IDA_NAME: _ZN21DB_DeleteAccountCargo8dispatchEiiP6Stream
// DEMANGLED: DB_DeleteAccountCargo::dispatch(int,int,Stream *)
bool DB_DeleteAccountCargo::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    if (stream->get_binary(&account_id, sizeof(account_id)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query("deLete from account_cargo where m_id = %s",
                         account_id_text);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x843B2E0
// IDA_NAME: _ZN21DB_DeleteAccountCargo11makeRequestEij
// DEMANGLED: DB_DeleteAccountCargo::makeRequest(int,unsigned int)
void DB_DeleteAccountCargo::makeRequest(int user_idx, unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35896);
    CStreamGuard guard(stream, true);
    *guard << 496;
    *guard << user_idx;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
