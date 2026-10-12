#include "old_data/db/character/db_check_charac_name.h"

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
    kDbSignalCheckCharacName = 674,
    kInterSignalGoldControlExceptionMid = 675,
};

}  // namespace

// IDA_EA: 0x844F97E
// IDA_NAME: _ZN18DB_CheckCharacNameC2Ev
// DEMANGLED: DB_CheckCharacName::DB_CheckCharacName(void)
DB_CheckCharacName::DB_CheckCharacName() {}

// IDA_EA: 0x845B87E
// IDA_NAME: _ZN18DB_CheckCharacNameD2Ev
// DEMANGLED: DB_CheckCharacName::~DB_CheckCharacName()
// IDA_EA: 0x845B8AE
// IDA_NAME: _ZN18DB_CheckCharacNameD0Ev
// DEMANGLED: DB_CheckCharacName::~DB_CheckCharacName() deleting destructor
DB_CheckCharacName::~DB_CheckCharacName() {}

// IDA_EA: 0x844317E
// IDA_NAME: _ZN18DB_CheckCharacName8dispatchEiiP6Stream
// DEMANGLED: DB_CheckCharacName::dispatch(int,int,Stream *)
bool DB_CheckCharacName::dispatch(int signal,
                                  int user_idx,
                                  Stream* stream) {
    (void)signal;

    SIG_CHECK_CHARAC_NAME* payload =
        stream->GetOutBuffer<SIG_CHECK_CHARAC_NAME>();

    unsigned int found_account_id = 0;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect m_id from charac_info where charac_name='%s' limit 1",
        payload->charac_name);

    unsigned char is_same_mid = 0;
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            return false;
        }
        if (db_handle->get_uint(0, found_account_id) != true) {
            return false;
        }
        if (payload->account_id == found_account_id) {
            is_same_mid = 1;
        }
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 42220);
    CStreamGuard guard(out_stream, true);
    *guard << kInterSignalGoldControlExceptionMid;
    *guard << user_idx;
    SIG_GOLD_CONTROL_EXCEPTION_MID* result =
        guard->GetInBuffer<SIG_GOLD_CONTROL_EXCEPTION_MID>();
    if (!result) {
        return false;
    }
    result->is_same_mid = is_same_mid;
    result->flag = payload->flag;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x8443352
// IDA_NAME: _ZN18DB_CheckCharacName11makeRequestEijPKcb
// DEMANGLED: DB_CheckCharacName::makeRequest(int,unsigned int,char const*,bool)
void DB_CheckCharacName::makeRequest(int user_idx,
                                     unsigned int account_id,
                                     const char* charac_name,
                                     bool request_flag) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 42233);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalCheckCharacName;
    *guard << user_idx;
    SIG_CHECK_CHARAC_NAME* payload =
        guard->GetInBuffer<SIG_CHECK_CHARAC_NAME>();
    if (!payload) {
        return;
    }
    memset(payload, 0, sizeof(*payload));
    payload->account_id = account_id;
    if (charac_name && *charac_name) {
        strncpy(payload->charac_name, charac_name, 0x1Eu);
    }
    payload->flag = static_cast<unsigned char>(request_flag);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
