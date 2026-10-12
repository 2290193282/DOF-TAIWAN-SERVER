#include "old_data/db/private_store/db_load_private_store.h"

#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>



// IDA_EA: 0x844E216
// IDA_NAME: _ZN19DB_LoadPrivateStoreC2Ev
// DEMANGLED: DB_LoadPrivateStore::DB_LoadPrivateStore(void)
DB_LoadPrivateStore::DB_LoadPrivateStore() {}

// IDA_EA: 0x845F790
// IDA_NAME: _ZN19DB_LoadPrivateStoreD2Ev
// DEMANGLED: DB_LoadPrivateStore::~DB_LoadPrivateStore()
// IDA_EA: 0x845F7C0
// IDA_NAME: _ZN19DB_LoadPrivateStoreD0Ev
// DEMANGLED: DB_LoadPrivateStore::~DB_LoadPrivateStore() deleting destructor
DB_LoadPrivateStore::~DB_LoadPrivateStore() {}

// IDA_EA: 0x841C08C
// IDA_NAME: _ZN19DB_LoadPrivateStore8dispatchEiiP6Stream
// DEMANGLED: DB_LoadPrivateStore::dispatch(int,int,Stream *)
bool DB_LoadPrivateStore::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    int charac_no = 0;
    if (stream->get_binary(&charac_no, sizeof(charac_no)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect store, use_doll from store where charac_no=%d",
                         charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("virtual bool DB_LoadPrivateStore::dispatch(int, int, Stream*)",
                       20363,
                       5);
        trace("DB_LoadPrivateStore::dispatch, exec ERROR charac_no=%d",
              static_cast<char>(charac_no));
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 20368);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    private_store::SIG_PRIVATE_STORE* payload =
        guard.GetInBuffer<private_store::SIG_PRIVATE_STORE>();
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(charac_no);

    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            cMyTrace trace("virtual bool DB_LoadPrivateStore::dispatch(int, int, Stream*)",
                           20384,
                           5);
            trace("DB_LoadPrivateStore::dispatch, fetch ERROR charac_no=%d",
                  static_cast<char>(charac_no));
            return false;
        }
        if (db_handle->get_binary(0, payload->item,
                                  sizeof(payload->item)) != true) {
            cMyTrace trace("virtual bool DB_LoadPrivateStore::dispatch(int, int, Stream*)",
                           20392,
                           5);
            trace("DB_LoadPrivateStore::GetUserInven, get_binary(2) ERROR charac_no=%d",
                  static_cast<char>(charac_no));
            return false;
        }
        payload->loaded = 1;
        char use_doll = 0;
        if (db_handle->get_byte(1, use_doll) != true) {
            cMyTrace trace("virtual bool DB_LoadPrivateStore::dispatch(int, int, Stream*)",
                           20404,
                           5);
            trace("DB_LoadPrivateStore::Get_Use_State, get_byte ERROR charac_no=%d",
                  static_cast<char>(charac_no));
            return false;
        }
        payload->use_doll = use_doll;
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
