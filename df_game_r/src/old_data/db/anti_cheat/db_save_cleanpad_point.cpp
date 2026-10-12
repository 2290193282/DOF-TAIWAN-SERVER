#include "old_data/db/anti_cheat/db_save_cleanpad_point.h"

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
    kDbSignalSaveCleanpadPoint = 340,
    kCleanpadDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844EA9E
// IDA_NAME: _ZN20DB_SaveCleanpadPointC2Ev
// DEMANGLED: DB_SaveCleanpadPoint::DB_SaveCleanpadPoint(void)
DB_SaveCleanpadPoint::DB_SaveCleanpadPoint() {}

// IDA_EA: 0x845D7E0
// IDA_NAME: _ZN20DB_SaveCleanpadPointD2Ev
// DEMANGLED: DB_SaveCleanpadPoint::~DB_SaveCleanpadPoint()
// IDA_EA: 0x845D810
// IDA_NAME: _ZN20DB_SaveCleanpadPointD0Ev
// DEMANGLED: DB_SaveCleanpadPoint::~DB_SaveCleanpadPoint() deleting destructor
DB_SaveCleanpadPoint::~DB_SaveCleanpadPoint() {}

// IDA_EA: 0x8432F3E
// IDA_NAME: _ZN20DB_SaveCleanpadPoint8dispatchEiiP6Stream
// DEMANGLED: DB_SaveCleanpadPoint::dispatch(int,int,Stream *)
bool DB_SaveCleanpadPoint::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    int cleanpad_point = 0;
    unsigned int account_id = 0;
    *stream >> cleanpad_point;
    *stream >> account_id;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kCleanpadDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate member_login set cleanpad_point=%d where m_id=%s",
        cleanpad_point,
        NumberToString(account_id, 0));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8432FE2
// IDA_NAME: _ZN20DB_SaveCleanpadPoint11makeRequestEji
// DEMANGLED: DB_SaveCleanpadPoint::makeRequest(unsigned int,int)
void DB_SaveCleanpadPoint::makeRequest(unsigned int cleanpad_point,
                                       int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 31402);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSaveCleanpadPoint;
    *guard << -1;
    *guard << cleanpad_point;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
