#include "old_data/db/inventory/db_create_account_cargo.h"
#include "old_data/db/inventory/account_cargo_payload.h"

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

// IDA_EA: 0x844F3CE
// IDA_NAME: _ZN21DB_CreateAccountCargoC2Ev
// DEMANGLED: DB_CreateAccountCargo::DB_CreateAccountCargo(void)
DB_CreateAccountCargo::DB_CreateAccountCargo() {}

// IDA_EA: 0x845C808
// IDA_NAME: _ZN21DB_CreateAccountCargoD2Ev
// DEMANGLED: DB_CreateAccountCargo::~DB_CreateAccountCargo()
// IDA_EA: 0x845C838
// IDA_NAME: _ZN21DB_CreateAccountCargoD0Ev
// DEMANGLED: DB_CreateAccountCargo::~DB_CreateAccountCargo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_CreateAccountCargo::~DB_CreateAccountCargo() {}

// IDA_EA: 0x843AC52
// IDA_NAME: _ZN21DB_CreateAccountCargo8dispatchEiiP6Stream
// DEMANGLED: DB_CreateAccountCargo::dispatch(int,int,Stream *)
bool DB_CreateAccountCargo::dispatch(int signal, int user_idx, Stream* stream) {
    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            35783);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_CREATE_ACCOUNT_CARGO_RESULT* result =
        guard.GetInBuffer<SIG_CREATE_ACCOUNT_CARGO_RESULT>();
    if (!result) {
        return false;
    }
    memset(result, 0, sizeof(*result));

    unsigned int account_id = 0;
    int capacity = 0;
    if (stream->get_binary(&account_id, sizeof(account_id)) != 1 ||
        stream->get_binary(&capacity, sizeof(capacity)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query("seLect * from account_cargo where m_id = %s",
                         account_id_text);
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (db_handle->get_n_rows()) {
        result->result = 20;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
        return true;
    }

    account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "inSert into account_cargo(m_id, capacity, money, occ_time, cargo) values(%s, %d, 0, now(),'')",
        account_id_text,
        capacity);
    if (db_handle->exec(true) != true) {
        return false;
    }

    result->result = 0;
    result->capacity = static_cast<unsigned int>(capacity);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x843AE86
// IDA_NAME: _ZN21DB_CreateAccountCargo11makeRequestEijj
// DEMANGLED: DB_CreateAccountCargo::makeRequest(int,unsigned int,unsigned int)
void DB_CreateAccountCargo::makeRequest(int user_idx,
                                        unsigned int account_id,
                                        unsigned int capacity) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35823);
    CStreamGuard guard(stream, true);
    *guard << 494;
    *guard << user_idx;
    *guard << account_id;
    *guard << capacity;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
