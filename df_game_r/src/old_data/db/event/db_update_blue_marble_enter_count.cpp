#include "old_data/db/event/db_update_blue_marble_enter_count.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalUpdateBlueMarbleEnterCount = 0x30C,
};

}  // namespace

// IDA_EA: 0x844FD36
// IDA_NAME: _ZN29DB_UpdateBlueMarbleEnterCountC2Ev
// DEMANGLED: DB_UpdateBlueMarbleEnterCount::DB_UpdateBlueMarbleEnterCount(void)
DB_UpdateBlueMarbleEnterCount::DB_UpdateBlueMarbleEnterCount() {}

// IDA_EA: 0x845AFA8
// IDA_NAME: _ZN29DB_UpdateBlueMarbleEnterCountD2Ev
// DEMANGLED: DB_UpdateBlueMarbleEnterCount::~DB_UpdateBlueMarbleEnterCount()
// IDA_EA: 0x845AFD8
// IDA_NAME: _ZN29DB_UpdateBlueMarbleEnterCountD0Ev
// DEMANGLED: DB_UpdateBlueMarbleEnterCount::~DB_UpdateBlueMarbleEnterCount()
// deleting destructor
DB_UpdateBlueMarbleEnterCount::~DB_UpdateBlueMarbleEnterCount() {}

// IDA_EA: 0x844874E
// IDA_NAME: _ZN29DB_UpdateBlueMarbleEnterCount8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBlueMarbleEnterCount::dispatch(int,int,Stream *)
bool DB_UpdateBlueMarbleEnterCount::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int charac_no = 0;
    unsigned char enter_count = 0;
    *stream >> charac_no;
    *stream >> enter_count;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate charac_option set blue_marble_enter_count = %u where charac_no = %u",
        enter_count,
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_option(charac_no, blue_marble_enter_count) values(%u, %u)",
        charac_no,
        enter_count);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844884C
// IDA_NAME: _ZN29DB_UpdateBlueMarbleEnterCount11makeRequestEjh
// DEMANGLED: DB_UpdateBlueMarbleEnterCount::makeRequest(unsigned int,unsigned char)
void DB_UpdateBlueMarbleEnterCount::makeRequest(unsigned int charac_no,
                                                unsigned char enter_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 46469);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateBlueMarbleEnterCount;
    *guard << -1;
    *guard << charac_no;
    *guard << enter_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
