#include "old_data/db/event/db_create_dnf_user_info.h"
#include "old_game/player/core/userinfo_dispatcher_reads.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x815E316
// IDA_NAME: _ZN25DBSelectCreateDnfUserInfoC2Ev
// DEMANGLED: DBSelectCreateDnfUserInfo::DBSelectCreateDnfUserInfo(void)
DBSelectCreateDnfUserInfo::DBSelectCreateDnfUserInfo()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::SELECT_CREATE_DNF_USER_INFO) {
}

// IDA_EA: 0x815E33A (D2), 0x815E36A (D0)
// IDA_NAME: _ZN25DBSelectCreateDnfUserInfoD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBSelectCreateDnfUserInfo::~DBSelectCreateDnfUserInfo() {
}

// IDA_EA: 0x815E388
// IDA_NAME: _ZN25DBInsertCreateDnfUserInfoC2Ev
// DEMANGLED: DBInsertCreateDnfUserInfo::DBInsertCreateDnfUserInfo(void)
DBInsertCreateDnfUserInfo::DBInsertCreateDnfUserInfo()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::INSERT_CREATE_DNF_USER_INFO) {
}

// IDA_EA: 0x815E3AC (D2), 0x815E3DC (D0)
// IDA_NAME: _ZN25DBInsertCreateDnfUserInfoD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBInsertCreateDnfUserInfo::~DBInsertCreateDnfUserInfo() {
}

// IDA_EA: 0x815BEE0
// IDA_NAME: _ZN25DBSelectCreateDnfUserInfo11makeRequestEij
// DEMANGLED: DBSelectCreateDnfUserInfo::makeRequest(int,unsigned int)
void DBSelectCreateDnfUserInfo::makeRequest(int uid,
                                            unsigned int account_id) {
    CEventBase* repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(160);
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        551);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 10, uid);
    MSG* msg = guard.GetInBuffer<MSG>();
    if (msg) {
        msg->account_id = account_id;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        cMyTrace trace(
            "static void DBSelectCreateDnfUserInfo::makeRequest(int, memberIdentificationNumber_t)",
            556,
            0);
        trace("[Taiwan, Event] msg is null");
    }
}

// IDA_EA: 0x815C034
// IDA_NAME: _ZN25DBSelectCreateDnfUserInfo8dispatchEiiP6Stream
// DEMANGLED: DBSelectCreateDnfUserInfo::dispatch(int,int,Stream *)
int DBSelectCreateDnfUserInfo::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    MSG* msg = stream->GetOutBuffer<MSG>();
    if (!msg) {
        throw "msg is null";
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    const char* account_id_text = NumberToString(msg->account_id, 0);
    db_handle->set_query(
        "seLect m_id from event_create_dnf_user where occ_date=CURDATE() and m_id=%s",
        account_id_text);
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }

    const bool received = db_handle->get_n_rows() != 0;
    InterSelectCreateDnfUserInfo::makeRequest(
        user_idx,
        msg->account_id,
        received);

    account_id_text = NumberToString(msg->account_id, 0);
    cMyTrace trace(
        "virtual bool DBSelectCreateDnfUserInfo::dispatch(int, int, Stream*)",
        589,
        0);
    trace("[Taiwan, Event] select event_create_dnf_user. m_id:%s isReceived:%d",
          account_id_text,
          received ? 1 : 0);
    return 1;
}

// IDA_EA: 0x815C25C
// IDA_NAME: _ZN25DBInsertCreateDnfUserInfo11makeRequestEij
// DEMANGLED: DBInsertCreateDnfUserInfo::makeRequest(int,unsigned int)
void DBInsertCreateDnfUserInfo::makeRequest(int uid,
                                            unsigned int account_id) {
    CEventBase* repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(160);
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        609);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 11, uid);
    MSG* msg = guard.GetInBuffer<MSG>();
    if (msg) {
        msg->account_id = account_id;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    } else {
        cMyTrace trace(
            "static void DBInsertCreateDnfUserInfo::makeRequest(int, memberIdentificationNumber_t)",
            615,
            0);
        trace("[Taiwan, Event] msg is null");
    }
}

// IDA_EA: 0x815C3B0
// IDA_NAME: _ZN25DBInsertCreateDnfUserInfo8dispatchEiiP6Stream
// DEMANGLED: DBInsertCreateDnfUserInfo::dispatch(int,int,Stream *)
int DBInsertCreateDnfUserInfo::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MSG* msg = stream->GetOutBuffer<MSG>();
    if (!msg) {
        throw "msg is null";
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }

    const int server_id = G_CEnvironment()->get_server_group();
    const char* account_id_text = NumberToString(msg->account_id, 0);
    db_handle->set_query(
        "inSert into event_create_dnf_user(occ_date,m_id,server_id,occ_time) VALUES(CURDATE(),%s,%d,CURTIME())",
        account_id_text,
        server_id);
    if (db_handle->exec(true) != true) {
        throw "insert_exec";
    }

    cMyTrace trace(
        "virtual bool DBInsertCreateDnfUserInfo::dispatch(int, int, Stream*)",
        641,
        0);
    trace("[Taiwan, Event] insert event_create_dnf_user. m_id:%d",
          msg->account_id);
    return 1;
}
