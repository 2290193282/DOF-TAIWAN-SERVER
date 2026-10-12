#include "old_data/db/statistics_log/db_insert_ontime_reward_user_log.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

enum {
    kDbSignalInsertOnTimeRewardUserLog = 664,
};
const char kOnTimeEventSuffix[] = "1112";

}  // namespace

// IDA_EA: 0x844F962
// IDA_NAME: _ZN28DB_InsertOnTimeRewardUserLogC2Ev
DB_InsertOnTimeRewardUserLog::DB_InsertOnTimeRewardUserLog() {}

// IDA_EA: 0x845B968
// IDA_NAME: _ZN28DB_InsertOnTimeRewardUserLogD2Ev
// IDA_EA: 0x845B998
// IDA_NAME: _ZN28DB_InsertOnTimeRewardUserLogD0Ev
DB_InsertOnTimeRewardUserLog::~DB_InsertOnTimeRewardUserLog() {}

// IDA_EA: 0x8442A56
// IDA_NAME: _ZN28DB_InsertOnTimeRewardUserLog8dispatchEiiP6Stream
// DEMANGLED: DB_InsertOnTimeRewardUserLog::dispatch(int, int, Stream*)
bool DB_InsertOnTimeRewardUserLog::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(13),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    int receive_no = 0;
    *stream >> account_id;
    *stream >> receive_no;

    db_handle->set_query(
        "inSert into event_%s_ontime_reward_user(m_id,recv_no) values(%s,%u)",
        kOnTimeEventSuffix,
        NumberToString(account_id, 0),
        receive_no);
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace(
        "virtual bool DB_InsertOnTimeRewardUserLog::dispatch(int, int, Stream*)",
        42041,
        5);
    trace("inSert into event_%s_ontime_reward_user(m_id,recv_no) failed at m_id(%s,%u) ",
          kOnTimeEventSuffix,
          NumberToString(account_id, 0),
          receive_no);
    return false;
}

// IDA_EA: 0x8442B7A
// IDA_NAME: _ZN28DB_InsertOnTimeRewardUserLog11makeRequestEiji
// DEMANGLED: DB_InsertOnTimeRewardUserLog::makeRequest(int, unsigned int, int)
void DB_InsertOnTimeRewardUserLog::makeRequest(int user_idx,
                                               unsigned int account_id,
                                               int receive_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 42053);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalInsertOnTimeRewardUserLog;
    *guard << user_idx;
    *guard << account_id;
    *guard << receive_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
