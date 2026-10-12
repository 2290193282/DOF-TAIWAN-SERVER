#include "old_data/db/statistics_log/db_insert_unlimit_support_log.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
namespace {

enum {
    kDbSignalInsertUnlimitSupportLog = 290,
    kUnlimitSupportDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844EB46
// IDA_NAME: _ZN26DB_InsertUnlimitSupportLogC2Ev
// DEMANGLED: DB_InsertUnlimitSupportLog::DB_InsertUnlimitSupportLog(void)
DB_InsertUnlimitSupportLog::DB_InsertUnlimitSupportLog() {}

// IDA_EA: 0x845DEE2
// IDA_NAME: _ZN26DB_InsertUnlimitSupportLogD2Ev
// DEMANGLED: DB_InsertUnlimitSupportLog::~DB_InsertUnlimitSupportLog()
// IDA_EA: 0x845DF12
// IDA_NAME: _ZN26DB_InsertUnlimitSupportLogD0Ev
// DEMANGLED: DB_InsertUnlimitSupportLog::~DB_InsertUnlimitSupportLog() deleting destructor
DB_InsertUnlimitSupportLog::~DB_InsertUnlimitSupportLog() {}

// IDA_EA: 0x842EB2E
// IDA_NAME: _ZN26DB_InsertUnlimitSupportLog8dispatchEiiP6Stream
// DEMANGLED: DB_InsertUnlimitSupportLog::dispatch(int,int,Stream *)
bool DB_InsertUnlimitSupportLog::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_INSERT_UNLIMIT_SUPPORT_LOG* payload =
        stream->GetOutBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>();

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kUnlimitSupportDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into event_unlimitsupport_lev(m_id,occ_time,server_id,charac_no,charac_name) values(%s,now(),%d,%d,'%s')",
        NumberToString(payload->account_id, 0),
        payload->server_id,
        payload->charac_no,
        payload->charac_name);
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x842EBD0
// IDA_NAME: _ZN26DB_InsertUnlimitSupportLog11makeRequestEjjPKc
// DEMANGLED: DB_InsertUnlimitSupportLog::makeRequest(unsigned int,unsigned int,char const*)
void DB_InsertUnlimitSupportLog::makeRequest(unsigned int account_id,
                                             unsigned int charac_no,
                                             const char* charac_name) {
    if (account_id <= 0x946501u) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 29586);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalInsertUnlimitSupportLog;
    *guard << -1;

    SIG_INSERT_UNLIMIT_SUPPORT_LOG* payload =
        guard->GetInBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>();
    if (!payload) {
        return;
    }
    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = account_id;
    payload->charac_no = charac_no;
    CEnvironment* environment = G_CEnvironment();
    // makeRequest @ 0x842EBD0 sources server_id from CEnvironment+888.
    payload->server_id = environment ? environment->get_server_group() : 0;
    if (charac_name) {
        std::strncpy(payload->charac_name, charac_name, 0x1D);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
