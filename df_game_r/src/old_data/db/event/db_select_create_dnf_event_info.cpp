#include "old_data/db/event/db_select_create_dnf_event_info.h"
#include "old_game/activity/schedule/event_value_types.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x815E2A4
// IDA_NAME: _ZN26DBSelectCreateDnfEventInfoC2Ev
// DEMANGLED: DBSelectCreateDnfEventInfo::DBSelectCreateDnfEventInfo(void)
DBSelectCreateDnfEventInfo::DBSelectCreateDnfEventInfo()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::SELECT_CREATE_DNF_EVENT_INFO) {
}

// IDA_EA: 0x815E2C8 (D2), 0x815E2F8 (D0)
// IDA_NAME: _ZN26DBSelectCreateDnfEventInfoD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBSelectCreateDnfEventInfo::~DBSelectCreateDnfEventInfo() {
}

// IDA_EA: 0x815BA94
// IDA_NAME: _ZN26DBSelectCreateDnfEventInfo11makeRequestEv
// DEMANGLED: DBSelectCreateDnfEventInfo::makeRequest(void)
void DBSelectCreateDnfEventInfo::makeRequest() {
    CEventBase* repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(160);
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        484);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 9, -1);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x815BB8C
// IDA_NAME: _ZN26DBSelectCreateDnfEventInfo8dispatchEiiP6Stream
// DEMANGLED: DBSelectCreateDnfEventInfo::dispatch(int,int,Stream *)
int DBSelectCreateDnfEventInfo::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    int rate = 0;
    int hour = 0;
    int event_day = 0;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle null";
    }
    db_handle->set_query(
        "seLect rate, HOUR(CURTIME()), event_day FROM event_create_dnf_info WHERE occ_time=DATE_ADD(CURDATE(), INTERVAL HOUR(CURTIME()) HOUR)");
    if (db_handle->exec(true) != true) {
        throw "select_exec";
    }
    if (!db_handle->get_n_rows()) {
        throw "get_n_rows=0";
    }
    if (db_handle->fetch() != true) {
        throw "fetch";
    }
    if (db_handle->get_int(0, rate) != true) {
        throw "get_int 0";
    }
    if (db_handle->get_int(1, hour) != true) {
        throw "get_int 1";
    }
    if (db_handle->get_int(2, event_day) != true) {
        throw "get_int 2";
    }

    InterSelectCreateDnfEventInfo::makeRequest(rate, hour, event_day);
    cMyTrace trace("virtual bool DBSelectCreateDnfEventInfo::dispatch(int, int, Stream*)",
                   525,
                   0);
    trace("[Taiwan, Event] select event_create_dnf_info. hour:%d rate:%d event_day:%d",
          hour,
          rate,
          event_day);
    return 1;
}
