#include "old_data/db/event/db_update_bingo_event.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"


namespace {

enum {
    kDbSignalUpdateBingoEvent = 0x31F,
};

}  // namespace

// IDA_EA: 0x844FD6E
// IDA_NAME: _ZN19DB_UpdateBingoEventC2Ev
// DEMANGLED: DB_UpdateBingoEvent::DB_UpdateBingoEvent(void)
DB_UpdateBingoEvent::DB_UpdateBingoEvent() {}

// IDA_EA: 0x845AF0C
// IDA_NAME: _ZN19DB_UpdateBingoEventD2Ev
// DEMANGLED: DB_UpdateBingoEvent::~DB_UpdateBingoEvent()
// IDA_EA: 0x845AF3C
// IDA_NAME: _ZN19DB_UpdateBingoEventD0Ev
// DEMANGLED: DB_UpdateBingoEvent::~DB_UpdateBingoEvent()
// deleting destructor
DB_UpdateBingoEvent::~DB_UpdateBingoEvent() {}

// IDA_EA: 0x8448EB8
// IDA_NAME: _ZN19DB_UpdateBingoEvent8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBingoEvent::dispatch(int,int,Stream *)
bool DB_UpdateBingoEvent::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    int bingo_count = 0;
    char selected_count = 0;
    *stream >> account_id;
    *stream >> bingo_count;
    *stream >> selected_count;
    CEnvironment* environment = G_CEnvironment();
    CSystemTime* system_time = GlobalData::system_time();
    const unsigned int reset_time = static_cast<unsigned int>(
        GetCurrentResetBaseTime(system_time->getCurSec(),
                                environment->get_fatigue_time()));

    db_handle->set_query(
        "upDate event_1208_bingo set board = %d,reward=%d where m_id = %s and occ_date > from_unixtime(%u)",
        bingo_count,
        selected_count,
        NumberToString(account_id, 0),
        reset_time);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into event_1208_bingo(m_id,occ_date,board,reward) values(%s,now(),%d,%d)",
        NumberToString(account_id, 0),
        bingo_count,
        selected_count);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8449032
// IDA_NAME: _ZN19DB_UpdateBingoEvent11makeRequestEijib
// DEMANGLED: DB_UpdateBingoEvent::makeRequest(int,unsigned int,int,bool)
void DB_UpdateBingoEvent::makeRequest(int user_idx,
                                      unsigned int account_id,
                                      int bingo_count,
                                      bool selected_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 46942);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateBingoEvent;
    *guard << user_idx;
    *guard << account_id;
    *guard << bingo_count;
    *guard << static_cast<char>(selected_count);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
