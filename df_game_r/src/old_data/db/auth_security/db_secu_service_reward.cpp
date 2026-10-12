#include "old_data/db/auth_security/db_secu_service_reward.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>


// IDA_EA: 0x844F0A2
// IDA_NAME: _ZN20DB_SecuServiceRewardC2Ev
// DEMANGLED: DB_SecuServiceReward::DB_SecuServiceReward(void)
DB_SecuServiceReward::DB_SecuServiceReward() {}

// IDA_EA: 0x845D0DE
// IDA_NAME: _ZN20DB_SecuServiceRewardD2Ev
// DEMANGLED: DB_SecuServiceReward::~DB_SecuServiceReward()
// IDA_EA: 0x845D10E
// IDA_NAME: _ZN20DB_SecuServiceRewardD0Ev
// DEMANGLED: DB_SecuServiceReward::~DB_SecuServiceReward() deleting destructor
DB_SecuServiceReward::~DB_SecuServiceReward() {}

// IDA_EA: 0x8437764
// IDA_NAME: _ZN20DB_SecuServiceReward8dispatchEiiP6Stream
// DEMANGLED: DB_SecuServiceReward::dispatch(int,int,Stream *)
bool DB_SecuServiceReward::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SECU_SERVICE_REWARD payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate member_security_grade set last_check_time=now() where m_id=%s",
        NumberToString(payload.account_id, 0));
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    cMyTrace trace(
        "virtual bool DB_SecuServiceReward::dispatch(int, int, Stream*)",
        33889,
        0);
    trace("DB_SecuServiceReward Update Error");
    return false;
}

// IDA_EA: 0x8437844
// IDA_NAME: _ZN20DB_SecuServiceReward11makeRequestEjc
// DEMANGLED: DB_SecuServiceReward::makeRequest(unsigned int,char)
void DB_SecuServiceReward::makeRequest(unsigned int account_id,
                                       char service_type) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 33916);
    CStreamGuard guard(stream, true);
    *guard << 413;
    *guard << -1;

    SIG_SECU_SERVICE_REWARD* payload =
        static_cast<SIG_SECU_SERVICE_REWARD*>(
            guard->GetInBufferUntyped(sizeof(SIG_SECU_SERVICE_REWARD)));
    memset(payload, 0, sizeof(SIG_SECU_SERVICE_REWARD));
    payload->account_id = account_id;
    payload->service_type = service_type;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
