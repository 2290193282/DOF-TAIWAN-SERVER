#include "old_data/db/character/db_update_charac_play_time.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalUpdateCharacPlayTime = 590,
};

}  // namespace

// IDA_EA: 0x844F6C2
// IDA_NAME: _ZN23DB_UpdateCharacPlayTimeC2Ev
// DEMANGLED: DB_UpdateCharacPlayTime::DB_UpdateCharacPlayTime(void)
DB_UpdateCharacPlayTime::DB_UpdateCharacPlayTime() {}

// IDA_EA: 0x845BE48
// IDA_NAME: _ZN23DB_UpdateCharacPlayTimeD2Ev
// DEMANGLED: DB_UpdateCharacPlayTime::~DB_UpdateCharacPlayTime()
// IDA_EA: 0x845BE78
// IDA_NAME: _ZN23DB_UpdateCharacPlayTimeD0Ev
// DEMANGLED: DB_UpdateCharacPlayTime::~DB_UpdateCharacPlayTime() deleting destructor
DB_UpdateCharacPlayTime::~DB_UpdateCharacPlayTime() {}

// IDA_EA: 0x843F75A
// IDA_NAME: _ZN23DB_UpdateCharacPlayTime8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCharacPlayTime::dispatch(int,int,Stream *)
bool DB_UpdateCharacPlayTime::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    int charac_no = 0;
    int play_time = 0;
    stream->get_binary(&charac_no, sizeof(charac_no));
    stream->get_binary(&play_time, sizeof(play_time));
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate charac_stat set total_play_time=total_play_time + %d where charac_no = %d",
        play_time,
        charac_no);
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x843F7EE
// IDA_NAME: _ZN23DB_UpdateCharacPlayTime11makeRequestEil
// DEMANGLED: DB_UpdateCharacPlayTime::makeRequest(int,long)
void DB_UpdateCharacPlayTime::makeRequest(int charac_no, long play_time) {
    if (charac_no) {
        if (play_time) {
            Stream* stream =
                GlobalData::stream_pool()->Acquire("DBThread.cpp", 39334);
            CStreamGuard guard(stream, true);
            *guard << kDbSignalUpdateCharacPlayTime;
            *guard << -1;
            *guard << charac_no;
            *guard << play_time;
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        }
    }
}
