#include "old_data/db/account/db_update_break_away_uv_list.h"

#include "old_data/db/account/break_away_payload.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <string.h>


// IDA_EA: 0x844ED76
// IDA_NAME: _ZN24DB_UpdateBreakAwayUvListC2Ev
// DEMANGLED: DB_UpdateBreakAwayUvList::DB_UpdateBreakAwayUvList(void)
DB_UpdateBreakAwayUvList::DB_UpdateBreakAwayUvList() {}

// IDA_EA: 0x845CE20
// IDA_NAME: _ZN24DB_UpdateBreakAwayUvListD2Ev
// DEMANGLED: DB_UpdateBreakAwayUvList::~DB_UpdateBreakAwayUvList()
// IDA_EA: 0x845CE50
// IDA_NAME: _ZN24DB_UpdateBreakAwayUvListD0Ev
// DEMANGLED: DB_UpdateBreakAwayUvList::~DB_UpdateBreakAwayUvList() deleting destructor
DB_UpdateBreakAwayUvList::~DB_UpdateBreakAwayUvList() {}

// IDA_EA: 0x843873A
// IDA_NAME: _ZN24DB_UpdateBreakAwayUvList11makeRequestEjPc
// DEMANGLED: DB_UpdateBreakAwayUvList::makeRequest(unsigned int,char *)
void DB_UpdateBreakAwayUvList::makeRequest(unsigned int account_id,
                                           char* play_info) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34338);
    CStreamGuard guard(stream, true);
    *guard << 327;
    *guard << -1;
    *guard << account_id;
    SIG_BREAK_AWAY_UV_LIST* payload =
        guard->GetInBuffer<SIG_BREAK_AWAY_UV_LIST>();
    strncpy(payload->play_info, play_info, sizeof(payload->play_info));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8438842
// IDA_NAME: _ZN24DB_UpdateBreakAwayUvList8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBreakAwayUvList::dispatch(int,int,Stream *)
bool DB_UpdateBreakAwayUvList::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    const int cur_sec = GlobalData::s_systemTime_.getCurSec();
    unsigned int account_id = 0;
    *stream >> account_id;
    const SIG_BREAK_AWAY_UV_LIST* payload =
        stream->GetOutBuffer<SIG_BREAK_AWAY_UV_LIST>();
    db_handle->set_query(
        "upDate churn_member_info set play_info ='%s', last_update_time=%u where m_id=%s",
        payload->play_info,
        cur_sec,
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }
    return false;
}
