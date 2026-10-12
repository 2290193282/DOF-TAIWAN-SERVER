#include "old_data/db/anti_cheat/db_auto_punished_second_user_data.h"

#include "old_data/db/anti_cheat/auto_punish_payloads.h"
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
    kDbSignalSecondUserData = 458,
    kAutoPunishDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844F1F2
// IDA_NAME: _ZN29DB_AutoPunishedSecondUserDataC2Ev
// DEMANGLED: DB_AutoPunishedSecondUserData::DB_AutoPunishedSecondUserData(void)
DB_AutoPunishedSecondUserData::DB_AutoPunishedSecondUserData() {}

// IDA_EA: 0x845CAC6
// IDA_NAME: _ZN29DB_AutoPunishedSecondUserDataD2Ev
// DEMANGLED: DB_AutoPunishedSecondUserData::~DB_AutoPunishedSecondUserData()
// IDA_EA: 0x845CAF6
// IDA_NAME: _ZN29DB_AutoPunishedSecondUserDataD0Ev
// DEMANGLED: DB_AutoPunishedSecondUserData::~DB_AutoPunishedSecondUserData()
DB_AutoPunishedSecondUserData::~DB_AutoPunishedSecondUserData() {}

// IDA_EA: 0x843A462
// IDA_NAME: _ZN29DB_AutoPunishedSecondUserData8dispatchEiiP6Stream
// DEMANGLED: DB_AutoPunishedSecondUserData::dispatch(int,int,Stream *)
bool DB_AutoPunishedSecondUserData::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoPunishDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    const SIG_SECOND_USER_DATA* request =
        stream->GetOutBuffer<SIG_SECOND_USER_DATA>();
    db_handle->set_query(
        "seLect trade_cnt,trade_gold from auto_punish_second_user where m_id=%s",
        NumberToString(request->account_id, 0));

    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 35057);
        CStreamGuard guard(out_stream, true);
        *guard << kDbSignalSecondUserData;
        *guard << request->user_idx;
        SIG_SECOND_USER_DATA_SET* response =
            guard->GetInBuffer<SIG_SECOND_USER_DATA_SET>();
        memset(response, 0, sizeof(*response));
        response->related_account_id = request->related_account_id;
        response->request_gold_low = request->request_gold_low;
        response->request_gold_high = request->request_gold_high;
        response->database_gold_low = 0;
        response->database_gold_high = 0;
        response->trade_count = 1;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
        return false;
    }

    unsigned int trade_count;
    unsigned long long database_gold;
    db_handle->fetch();
    db_handle->get_uint(0, trade_count);
    db_handle->get_ulonglong(1, database_gold);

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 35073);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalSecondUserData;
    *guard << request->user_idx;
    SIG_SECOND_USER_DATA_SET* response =
        guard->GetInBuffer<SIG_SECOND_USER_DATA_SET>();
    memset(response, 0, sizeof(*response));
    response->related_account_id = request->related_account_id;
    response->request_gold_low = request->request_gold_low;
    response->request_gold_high = request->request_gold_high;
    response->database_gold_low = static_cast<uint32_t>(database_gold);
    response->database_gold_high =
        static_cast<uint32_t>(database_gold >> 32);
    response->trade_count = trade_count + 1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x843A790
// IDA_NAME: _ZN29DB_AutoPunishedSecondUserData11makeRequestEjyjj
// DEMANGLED: DB_AutoPunishedSecondUserData::makeRequest(unsigned int, unsigned long long, unsigned int, unsigned int)
void DB_AutoPunishedSecondUserData::makeRequest(
    unsigned int account_id,
    unsigned long long trade_gold,
    unsigned int user_idx,
    unsigned int related_account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 35090);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSecondUserData;
    *guard << user_idx;
    SIG_SECOND_USER_DATA* request =
        guard->GetInBuffer<SIG_SECOND_USER_DATA>();
    memset(request, 0, sizeof(*request));
    request->account_id = account_id;
    request->request_gold_low = static_cast<uint32_t>(trade_gold);
    request->request_gold_high = static_cast<uint32_t>(trade_gold >> 32);
    request->user_idx = user_idx;
    request->related_account_id = related_account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
