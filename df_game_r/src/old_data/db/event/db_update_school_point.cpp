#include "old_data/db/event/db_update_school_point.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>


namespace {

enum {
    kDbSignalUpdateSchoolPoint = 0x129,
};

}  // namespace

// IDA_EA: 0x844EB7E
// IDA_NAME: _ZN20DB_UpdateSchoolPointC2Ev
// DEMANGLED: DB_UpdateSchoolPoint::DB_UpdateSchoolPoint(void)
DB_UpdateSchoolPoint::DB_UpdateSchoolPoint() {}

// IDA_EA: 0x845DE46
// IDA_NAME: _ZN20DB_UpdateSchoolPointD2Ev
// DEMANGLED: DB_UpdateSchoolPoint::~DB_UpdateSchoolPoint()
// IDA_EA: 0x845DE76
// IDA_NAME: _ZN20DB_UpdateSchoolPointD0Ev
// DEMANGLED: DB_UpdateSchoolPoint::~DB_UpdateSchoolPoint() deleting destructor
DB_UpdateSchoolPoint::~DB_UpdateSchoolPoint() {}

// IDA_EA: 0x842F05E
// IDA_NAME: _ZN20DB_UpdateSchoolPoint8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateSchoolPoint::dispatch(int,int,Stream *)
bool DB_UpdateSchoolPoint::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_UPDATE_SCHOOL_POINT request = {0, 0};
    *stream >> request.account_id;
    *stream >> request.point;
    db_handle->set_query(
        "upDate event_school_point set point=%d+point where m_id=%s",
        request.point,
        NumberToString(request.account_id, 0));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x842F0F0
// IDA_NAME: _ZN20DB_UpdateSchoolPoint11makeRequestEjii
// DEMANGLED: DB_UpdateSchoolPoint::makeRequest(unsigned int,int,int)
void DB_UpdateSchoolPoint::makeRequest(unsigned int account_id,
                                       int cur_charac_school_point,
                                       int uid) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 29733);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateSchoolPoint;
    *guard << uid;
    if (cur_charac_school_point > 100) {
        cur_charac_school_point = 100;
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "static void DB_UpdateSchoolPoint::makeRequest(memberIdentificationNumber_t, int, int)",
            0x742D,
            "School event point error (m_id: %s)(point: %d)",
            NumberToString(account_id, 0),
            100);
    }
    SIG_UPDATE_SCHOOL_POINT request = {
        account_id,
        cur_charac_school_point,
    };
    *guard << request.account_id;
    *guard << request.point;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
