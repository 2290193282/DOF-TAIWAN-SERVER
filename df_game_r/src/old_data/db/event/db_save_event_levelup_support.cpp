#include "old_data/db/event/db_save_event_levelup_support.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

enum {
    kDbSignalSaveEventLevelupSupport = 847,
};

}  // namespace

// IDA_EA: 0x844FFBA
// IDA_NAME: _ZN26DB_SaveEventLevelupSupportC2Ev
DB_SaveEventLevelupSupport::DB_SaveEventLevelupSupport() {}

// IDA_EA: 0x845A7A6
// IDA_NAME: _ZN26DB_SaveEventLevelupSupportD2Ev
// IDA_EA: 0x845A7D6
// IDA_NAME: _ZN26DB_SaveEventLevelupSupportD0Ev
DB_SaveEventLevelupSupport::~DB_SaveEventLevelupSupport() {}

// IDA_EA: 0x844CAFE
// IDA_NAME: _ZN26DB_SaveEventLevelupSupport8dispatchEiiP6Stream
// DEMANGLED: DB_SaveEventLevelupSupport::dispatch(int, int, Stream*)
bool DB_SaveEventLevelupSupport::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    int charac_no = 0;
    int level = 0;
    int state = 0;
    *stream >> charac_no;
    *stream >> level;
    *stream >> state;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "upDate event_levelup_support set state = %d where charac_no = %d and level = %d",
        state,
        charac_no,
        level);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into event_levelup_support(charac_no, level, state) values(%d, %d, %d)",
        charac_no,
        level,
        state);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844CC2E
// IDA_NAME: _ZN26DB_SaveEventLevelupSupport11makeRequestEiiii
// DEMANGLED: DB_SaveEventLevelupSupport::makeRequest(int, int, int, int)
void DB_SaveEventLevelupSupport::makeRequest(int user_idx,
                                              int charac_no,
                                              int level,
                                              int state) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 49754);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSaveEventLevelupSupport;
    *guard << user_idx;
    *guard << charac_no;
    *guard << level;
    *guard << state;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
