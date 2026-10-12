#include "old_data/db/event/db_hero_mission_event_data.h"

#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x815E5C2
// IDA_NAME: _ZN26DBLoadHeroMissionEventDataC2Ev
// DEMANGLED: DBLoadHeroMissionEventData::DBLoadHeroMissionEventData(void)
DBLoadHeroMissionEventData::DBLoadHeroMissionEventData()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::LOAD_HERO_MISSION_EVENT_DATA) {
}

// IDA_EA: 0x815E5E6 (D2), 0x815E616 (D0)
// IDA_NAME: _ZN26DBLoadHeroMissionEventDataD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBLoadHeroMissionEventData::~DBLoadHeroMissionEventData() {
}

// IDA_EA: 0x815D7A8
// IDA_NAME: _ZN26DBLoadHeroMissionEventData8dispatchEiiP6Stream
// DEMANGLED: DBLoadHeroMissionEventData::dispatch(int,int,Stream *)
int DBLoadHeroMissionEventData::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;

    Taiwan::SigLoadHeroMissionData* request =
        stream->GetOutBuffer<Taiwan::SigLoadHeroMissionData>();
    if (!request) {
        throw "context_null";
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle_null";
    }

    const int server_group = G_CEnvironment()->get_server_group();
    db_handle->set_query(
        "seLect mission from event_hero_mission_data where server_group=%d and charac_no=%u",
        server_group,
        request->charac_no);
    if (db_handle->exec(true) != true) {
        throw "exec_select";
    }

    // The old C2 calls init, then dispatch explicitly calls init again before
    // assigning charac_no.  Preserve that zeroed 0x104-byte response ABI.
    Taiwan::SigLoadHeroMissionData response;
    response.init();
    response.charac_no = request->charac_no;

    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            throw "fetch";
        }
        if (db_handle->get_binary(0, response.mission,
                                  sizeof(response.mission)) != true) {
            throw "get_binary";
        }
    } else {
        db_handle->set_query(
            "inSert into event_hero_mission_data values(%d,%u,'',now())",
            G_CEnvironment()->get_server_group(),
            request->charac_no);
        if (db_handle->exec(true) != true) {
            throw "exec_insert";
        }
    }

    Stream* reply_stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_DbThread.cpp",
        1002);
    CStreamGuard reply(reply_stream, true);
    Taiwan::internal_stream(reply,
                            TaiwanInternalPack::LOAD_HERO_MISSION_EVENT_DATA,
                            user_idx);
    Taiwan::SigLoadHeroMissionData* reply_payload =
        reply.GetInBuffer<Taiwan::SigLoadHeroMissionData>();
    if (!reply_payload) {
        throw "stream_alloc";
    }
    *reply_payload = response;

    // The original calls MsgQueueMgr::put(..., 1, ...): its INTER_Q is the
    // current typed DISPATCH_Q, consumed by TaiwanInternalDispatcher.
    return GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, reply);
}

// IDA_EA: 0x815E634
// IDA_NAME: _ZN26DBSaveHeroMissionEventDataC2Ev
// DEMANGLED: DBSaveHeroMissionEventData::DBSaveHeroMissionEventData(void)
DBSaveHeroMissionEventData::DBSaveHeroMissionEventData()
    : TaiwanInterfaceDBDispatcher(
          TaiwanInternalPack::SAVE_HERO_MISSION_EVENT_DATA) {
}

// IDA_EA: 0x815E658 (D2), 0x815E688 (D0)
// IDA_NAME: _ZN26DBSaveHeroMissionEventDataD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
DBSaveHeroMissionEventData::~DBSaveHeroMissionEventData() {
}

// IDA_EA: 0x815DB98
// IDA_NAME: _ZN26DBSaveHeroMissionEventData8dispatchEiiP6Stream
// DEMANGLED: DBSaveHeroMissionEventData::dispatch(int,int,Stream *)
int DBSaveHeroMissionEventData::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    Taiwan::SigSaveHeroMissionData* request =
        stream->GetOutBuffer<Taiwan::SigSaveHeroMissionData>();
    if (!request) {
        throw "context_null";
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        throw "handle_null";
    }

    const int server_group = G_CEnvironment()->get_server_group();
    char* mission = db_handle->blob_to_str(
        0,
        request->mission,
        sizeof(request->mission));
    db_handle->set_query(
        "upDate event_hero_mission_data set mission='%s' where server_group=%d and charac_no=%u",
        mission,
        server_group,
        request->charac_no);
    if (db_handle->exec(true) != true) {
        throw "exec_update";
    }
    return 1;
}
