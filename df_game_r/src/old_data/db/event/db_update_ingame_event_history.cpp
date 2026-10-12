#include "old_data/db/event/db_update_ingame_event_history.h"

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
    kDbSignalUpdateInGameEventHistory = 685,
};

}  // namespace

// IDA_EA: 0x844F9EE
// IDA_NAME: _ZN27DB_UpdateInGameEventHistoryC2Ev
DB_UpdateInGameEventHistory::DB_UpdateInGameEventHistory() {}

// IDA_EA: 0x845B65C
// IDA_NAME: _ZN27DB_UpdateInGameEventHistoryD2Ev
// IDA_EA: 0x845B68C
// IDA_NAME: _ZN27DB_UpdateInGameEventHistoryD0Ev
DB_UpdateInGameEventHistory::~DB_UpdateInGameEventHistory() {}

// IDA_EA: 0x8444972
// IDA_NAME: _ZN27DB_UpdateInGameEventHistory8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateInGameEventHistory::dispatch(int, int, Stream*)
bool DB_UpdateInGameEventHistory::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    int event_type = 0;
    int history_1 = 0;
    int history_2 = 0;
    *stream >> account_id;
    *stream >> event_type;
    *stream >> history_1;
    *stream >> history_2;

    db_handle->set_query(
        "upDate event_ingame_history set history_1 = %u,history_2 = %u where m_id=%s and type = %d",
        history_1,
        history_2,
        NumberToString(account_id, 0),
        event_type);
    if (db_handle->exec(true) != true ||
        !db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into event_ingame_history(m_id,type,history_1,history_2) values(%s,%d,%d,%d)",
            NumberToString(account_id, 0),
            event_type,
            history_1,
            history_2);
        db_handle->exec(true);
    }
    return true;
}

// IDA_EA: 0x8444AC4
// IDA_NAME: _ZN27DB_UpdateInGameEventHistory11makeRequestEjiii
// DEMANGLED: DB_UpdateInGameEventHistory::makeRequest(unsigned int, int, int, int)
void DB_UpdateInGameEventHistory::makeRequest(unsigned int account_id,
                                               int event_type,
                                               int history_1,
                                               int history_2) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 43944);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateInGameEventHistory;
    *guard << -1;
    *guard << account_id;
    *guard << event_type;
    *guard << history_1;
    *guard << history_2;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
