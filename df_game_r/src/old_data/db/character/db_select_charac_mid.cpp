#include "old_data/db/character/db_select_charac_mid.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>



namespace {

enum {
    kDbSignalSelectCharacMid = 697,
};

}  // namespace

// IDA_EA: 0x844FB06
// IDA_NAME: _ZN18DB_SelectCharacMidC2Ev
// DEMANGLED: DB_SelectCharacMid::DB_SelectCharacMid(void)
DB_SelectCharacMid::DB_SelectCharacMid() {}

// IDA_EA: 0x845B350
// IDA_NAME: _ZN18DB_SelectCharacMidD2Ev
// DEMANGLED: DB_SelectCharacMid::~DB_SelectCharacMid()
// IDA_EA: 0x845B380
// IDA_NAME: _ZN18DB_SelectCharacMidD0Ev
// DEMANGLED: DB_SelectCharacMid::~DB_SelectCharacMid() deleting destructor
DB_SelectCharacMid::~DB_SelectCharacMid() {}

// IDA_EA: 0x8446516
// IDA_NAME: _ZN18DB_SelectCharacMid8dispatchEiiP6Stream
// DEMANGLED: DB_SelectCharacMid::dispatch(int,int,Stream *)
bool DB_SelectCharacMid::dispatch(int signal, int user_idx, Stream* stream) {
    SIG_CHECK_CHAR_MID* payload =
        stream->GetOutBuffer<SIG_CHECK_CHAR_MID>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    char escaped_name[59];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, payload->charac_name);
    db_handle->set_query(
        "seLect m_id from charac_info where charac_name='%s' limit 1",
        escaped_name);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            return false;
        }
        if (db_handle->get_uint(0, account_id) != true) {
            return false;
        }

        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 44592);
        CStreamGuard guard(out_stream, true);
        *guard << signal;
        *guard << user_idx;
        SIG_CHECK_CHAR_MID_RESULT* result =
            guard->GetInBuffer<SIG_CHECK_CHAR_MID_RESULT>();
        if (!result) {
            return false;
        }
        result->request_id = payload->request_id;
        result->value1 = payload->value1;
        result->value2 = payload->value2;
        result->value3 = payload->value3;
        result->account_id = account_id;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return true;
}

// IDA_EA: 0x844676C
// IDA_NAME: _ZN18DB_SelectCharacMid11makeRequestEijPKcjjj
// DEMANGLED: DB_SelectCharacMid::makeRequest(int,unsigned int,char const*,unsigned int,unsigned int,unsigned int)
void DB_SelectCharacMid::makeRequest(int user_idx,
                                     unsigned int request_id,
                                     const char* charac_name,
                                     unsigned int value1,
                                     unsigned int value2,
                                     unsigned int value3) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 44610);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSelectCharacMid;
    *guard << user_idx;
    SIG_CHECK_CHAR_MID* payload =
        guard->GetInBuffer<SIG_CHECK_CHAR_MID>();
    if (!payload) {
        return;
    }
    memset(payload, 0, sizeof(*payload));
    payload->request_id = request_id;
    payload->value1 = value1;
    payload->value2 = value2;
    payload->value3 = value3;
    if (charac_name) {
        strncpy(payload->charac_name, charac_name, 0x1Eu);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
