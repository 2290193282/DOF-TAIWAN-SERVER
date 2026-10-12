#include "old_data/db/inventory/db_upgrade_account_cargo.h"
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

// IDA_EA: 0x844F3EA
// IDA_NAME: _ZN22DB_UpgradeAccountCargoC2Ev
// DEMANGLED: DB_UpgradeAccountCargo::DB_UpgradeAccountCargo(void)
DB_UpgradeAccountCargo::DB_UpgradeAccountCargo() {}

// IDA_EA: 0x845C7BA
// IDA_NAME: _ZN22DB_UpgradeAccountCargoD2Ev
// DEMANGLED: DB_UpgradeAccountCargo::~DB_UpgradeAccountCargo()
// IDA_EA: 0x845C7EA
// IDA_NAME: _ZN22DB_UpgradeAccountCargoD0Ev
// DEMANGLED: DB_UpgradeAccountCargo::~DB_UpgradeAccountCargo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpgradeAccountCargo::~DB_UpgradeAccountCargo() {}

// IDA_EA: 0x843AF76
// IDA_NAME: _ZN22DB_UpgradeAccountCargo8dispatchEiiP6Stream
// DEMANGLED: DB_UpgradeAccountCargo::dispatch(int,int,Stream *)
bool DB_UpgradeAccountCargo::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            35835);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_UPGRADE_ACCOUNT_CARGO_RESULT* result =
        guard.GetInBuffer<SIG_UPGRADE_ACCOUNT_CARGO_RESULT>();
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
    db_handle->set_query(
        "upDate account_cargo set capacity = %d where m_id = %s",
        capacity,
        account_id_text);
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (db_handle->getAffectedRowCount()) {
        result->result = 0;
        result->capacity = static_cast<unsigned int>(capacity);
    } else {
        result->result = 21;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x843B15E
// IDA_NAME: _ZN22DB_UpgradeAccountCargo11makeRequestEijj
// DEMANGLED: DB_UpgradeAccountCargo::makeRequest(int,unsigned int,unsigned int)
void DB_UpgradeAccountCargo::makeRequest(int user_idx,
                                         unsigned int account_id,
                                         unsigned int capacity) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35869);
    CStreamGuard guard(stream, true);
    *guard << 495;
    *guard << user_idx;
    *guard << account_id;
    *guard << capacity;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
