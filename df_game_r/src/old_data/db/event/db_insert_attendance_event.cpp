#include "old_data/db/event/db_insert_attendance_event.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalInsertAttendanceEvent = 0x30E,
};

}  // namespace

// IDA_EA: 0x844FD52
// IDA_NAME: _ZN24DB_InsertAttendanceEventC2Ev
// DEMANGLED: DB_InsertAttendanceEvent::DB_InsertAttendanceEvent(void)
DB_InsertAttendanceEvent::DB_InsertAttendanceEvent() {}

// IDA_EA: 0x845AF5A
// IDA_NAME: _ZN24DB_InsertAttendanceEventD2Ev
// DEMANGLED: DB_InsertAttendanceEvent::~DB_InsertAttendanceEvent()
// IDA_EA: 0x845AF8A
// IDA_NAME: _ZN24DB_InsertAttendanceEventD0Ev
// DEMANGLED: DB_InsertAttendanceEvent::~DB_InsertAttendanceEvent()
// deleting destructor
DB_InsertAttendanceEvent::~DB_InsertAttendanceEvent() {}

// IDA_EA: 0x8448A48
// IDA_NAME: _ZN24DB_InsertAttendanceEvent8dispatchEiiP6Stream
// DEMANGLED: DB_InsertAttendanceEvent::dispatch(int,int,Stream *)
bool DB_InsertAttendanceEvent::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    *stream >> account_id;
    *stream >> charac_no;
    db_handle->set_query(
        "inSert into event_1207_attendance(m_id, charac_no, occ_date) \t\t\t\t  values(%s, %u, now())",
        NumberToString(account_id, 0),
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8448AFE
// IDA_NAME: _ZN24DB_InsertAttendanceEvent11makeRequestEijj
// DEMANGLED: DB_InsertAttendanceEvent::makeRequest(int,unsigned int,unsigned int)
void DB_InsertAttendanceEvent::makeRequest(int user_idx,
                                           unsigned int account_id,
                                           unsigned int charac_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 46673);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalInsertAttendanceEvent;
    *guard << user_idx;
    *guard << account_id;
    *guard << charac_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
