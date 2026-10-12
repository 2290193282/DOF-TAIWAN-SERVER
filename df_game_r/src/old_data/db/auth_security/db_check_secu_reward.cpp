#include "old_data/db/auth_security/db_check_secu_reward.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <time.h>



namespace {

enum {
    kDbSignalCheckSecuReward = 683,
    kInterSignalSendSecuReward = 171,
    kSecuRewardBoundarySecond = 75599,
    kSecuRewardDailyShift = 10800,
};

}  // namespace

// IDA_EA: 0x844F9B6
// IDA_NAME: _ZN18DB_CheckSecuRewardC2Ev
// DEMANGLED: DB_CheckSecuReward::DB_CheckSecuReward(void)
DB_CheckSecuReward::DB_CheckSecuReward() {}

// IDA_EA: 0x845B6F8
// IDA_NAME: _ZN18DB_CheckSecuRewardD2Ev
// DEMANGLED: DB_CheckSecuReward::~DB_CheckSecuReward()
// IDA_EA: 0x845B728
// IDA_NAME: _ZN18DB_CheckSecuRewardD0Ev
// DEMANGLED: DB_CheckSecuReward::~DB_CheckSecuReward() deleting destructor
DB_CheckSecuReward::~DB_CheckSecuReward() {}

// IDA_EA: 0x84441E6
// IDA_NAME: _ZN18DB_CheckSecuReward8dispatchEiiP6Stream
// DEMANGLED: DB_CheckSecuReward::dispatch(int,int,Stream *)
bool DB_CheckSecuReward::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    SIG_CHECK_USER_SECU_REWARD* payload =
        stream->GetOutBuffer<SIG_CHECK_USER_SECU_REWARD>();

    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }

    unsigned int last_check_time = 0;
    db_handle->set_query(
        "seLect unix_timestamp(last_check_time) from member_security_grade where m_id=%s",
        NumberToString(payload->account_id, 0));
    if (db_handle->exec(true) != true || !db_handle->get_n_rows() ||
        db_handle->fetch() != true) {
        return true;
    }
    db_handle->get_uint(0, last_check_time);

    time_t timer = 0;
    time(&timer);
    const int day_second = static_cast<int>(timer % 86400);
    timer = timer - day_second - kSecuRewardDailyShift;
    if (day_second > kSecuRewardBoundarySecond) {
        timer += 86400;
    }

    if (static_cast<int>(last_check_time) < static_cast<int>(timer)) {
        Stream* out_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 43813);
        CStreamGuard guard(out_stream, true);
        *guard << kInterSignalSendSecuReward;
        *guard << user_idx;
        SIG_SEND_USER_REWARD* reward_payload =
            guard->GetInBuffer<SIG_SEND_USER_REWARD>();
        reward_payload->service_type = payload->service_type;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return true;
}

// IDA_EA: 0x8444408
// IDA_NAME: _ZN18DB_CheckSecuReward11makeRequestEiji
// DEMANGLED: DB_CheckSecuReward::makeRequest(int,unsigned int,int)
void DB_CheckSecuReward::makeRequest(int user_idx,
                                     unsigned int account_id,
                                     int service_type) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 43828);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalCheckSecuReward;
    *guard << user_idx;
    SIG_CHECK_USER_SECU_REWARD* payload =
        guard->GetInBuffer<SIG_CHECK_USER_SECU_REWARD>();
    payload->account_id = account_id;
    payload->service_type = static_cast<uint32_t>(service_type);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
