#include "old_data/db/anti_cheat/db_load_clean_pad_point.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



// IDA_EA: 0x844EB62
// IDA_NAME: _ZN20DB_LoadCleanPadPointC2Ev
// DEMANGLED: DB_LoadCleanPadPoint::DB_LoadCleanPadPoint(void)
DB_LoadCleanPadPoint::DB_LoadCleanPadPoint() {}

// IDA_EA: 0x845DE94
// IDA_NAME: _ZN20DB_LoadCleanPadPointD2Ev
// DEMANGLED: DB_LoadCleanPadPoint::~DB_LoadCleanPadPoint()
// IDA_EA: 0x845DEC4
// IDA_NAME: _ZN20DB_LoadCleanPadPointD0Ev
// DEMANGLED: DB_LoadCleanPadPoint::~DB_LoadCleanPadPoint() deleting destructor
DB_LoadCleanPadPoint::~DB_LoadCleanPadPoint() {}

// IDA_EA: 0x842ECF4
// IDA_NAME: _ZN20DB_LoadCleanPadPoint8dispatchEiiP6Stream
// DEMANGLED: DB_LoadCleanPadPoint::dispatch(int,int,Stream *)
bool DB_LoadCleanPadPoint::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect hack_type,value from hack_cleanpad_ratio_info");
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_LoadCleanPadPoint::dispatch(int, int, Stream*)",
            0x73B0,
            "[QUERY_ERROR] select hack_cleanpad_ratio_info");
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    if (!row_count) {
        cMyTrace trace(
            "virtual bool DB_LoadCleanPadPoint::dispatch(int, int, Stream*)",
            29622,
            0);
        trace("[QUERY_NOTICE] have no data hack_cleanpad_ratio_info");
        return true;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 29630);
    CStreamGuard guard(out_stream, true);
    *guard << 295;
    *guard << -1;
    SIG_LOAD_CLEANPAD_POINT* payload =
        guard->GetInBuffer<SIG_LOAD_CLEANPAD_POINT>();
    memset(payload, 0, sizeof(*payload));

    for (int row = 0; row < row_count; ++row) {
        int hack_type = 0;
        int value = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, hack_type) != true ||
            db_handle->get_int(1, value) != true) {
            return false;
        }
        if (hack_type >= CLEANPAD_HACK_TYPE_MIN &&
            hack_type <= CLEANPAD_HACK_TYPE_MAX) {
            payload->value_by_hack_type[
                hack_type - CLEANPAD_HACK_TYPE_MIN] = value;
        }
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842EFA0
// IDA_NAME: _ZN20DB_LoadCleanPadPoint11makeRequestEv
// DEMANGLED: DB_LoadCleanPadPoint::makeRequest(void)
void DB_LoadCleanPadPoint::makeRequest() {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 29656);
    CStreamGuard guard(stream, true);
    *guard << 295;
    *guard << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
