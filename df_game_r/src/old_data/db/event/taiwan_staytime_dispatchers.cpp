#include "old_data/db/event/taiwan_staytime_dispatchers.h"

#include "old_data/db/core/mysql.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x815E14E
// IDA_NAME: _ZN18CheckStayTimeEventC2Ev
// DEMANGLED: CheckStayTimeEvent::CheckStayTimeEvent(void)
CheckStayTimeEvent::CheckStayTimeEvent()
    : TaiwanInterfaceDBDispatcher(TaiwanInternalPack::CHECK_STAY_TIME_EVENT) {
}

// IDA_EA: 0x815E172 (D2), 0x815E1A2 (D0)
// IDA_NAME: _ZN18CheckStayTimeEventD2Ev / D0Ev
CheckStayTimeEvent::~CheckStayTimeEvent() {
}

// IDA_EA: 0x815B362
// IDA_NAME: _ZN18CheckStayTimeEvent8dispatchEiiP6Stream
// DEMANGLED: CheckStayTimeEvent::dispatch(int,int,Stream *)
int CheckStayTimeEvent::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    Taiwan::SigStayTimeEvent* context =
        stream->GetOutBuffer<Taiwan::SigStayTimeEvent>();
    if (!context) {
        throw "context null";
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    db_handle->set_query(
        "seLect reward_flag from event_stay_time_charac where m_id=%s",
        NumberToString(context->account_id, 0));
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }

    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            throw "fetch";
        }
        int reward_flag = 0;
        if (db_handle->get_int(0, reward_flag) != true) {
            throw "get_int";
        }
        if (!reward_flag) {
            db_handle->set_query(
                "upDate event_stay_time_charac set reward_flag=1, "
                "reward_count=reward_count+1, mod_date=now() where m_id = %s",
                NumberToString(context->account_id, 0));
            if (db_handle->exec(true) != true) {
                throw "update_exec";
            }
            sendStayTimeData(user_idx);
        }
    } else {
        db_handle->set_query(
            "inSert into event_stay_time_charac values('%s', 1, 1, now())",
            NumberToString(context->account_id, 0));
        if (db_handle->exec(true) != true) {
            throw "insert_exec";
        }
        sendStayTimeData(user_idx);
    }
    return 1;
}

// IDA_EA: 0x815B6AC
// IDA_NAME: _ZN18CheckStayTimeEvent16sendStayTimeDataEi
// DEMANGLED: CheckStayTimeEvent::sendStayTimeData(int)
void CheckStayTimeEvent::sendStayTimeData(int user_idx) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        402);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard,
                            TaiwanInternalPack::CHECK_STAY_TIME_EVENT,
                            user_idx);

    // Old code only reserves the typed four-byte frame.  Inter code 5 is the
    // notification itself, so no field is synthesized or copied here.
    if (guard.GetInBuffer<Taiwan::SigStayTimeEvent>()) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
}

// IDA_EA: 0x815E1C0
// IDA_NAME: _ZN18ResetStayTimeEventC2Ev
// DEMANGLED: ResetStayTimeEvent::ResetStayTimeEvent(void)
ResetStayTimeEvent::ResetStayTimeEvent()
    : TaiwanInterfaceDBDispatcher(TaiwanInternalPack::RESET_STAY_TIME_EVENT) {
}

// IDA_EA: 0x815E1E4 (D2), 0x815E214 (D0)
// IDA_NAME: _ZN18ResetStayTimeEventD2Ev / D0Ev
ResetStayTimeEvent::~ResetStayTimeEvent() {
}

// IDA_EA: 0x815B76A
// IDA_NAME: _ZN18ResetStayTimeEvent8dispatchEiiP6Stream
// DEMANGLED: ResetStayTimeEvent::dispatch(int,int,Stream *)
int ResetStayTimeEvent::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }
    db_handle->set_query("upDate event_stay_time_charac set reward_flag = 0");
    if (db_handle->exec(true) != true) {
        throw "exec";
    }
    return 1;
}
