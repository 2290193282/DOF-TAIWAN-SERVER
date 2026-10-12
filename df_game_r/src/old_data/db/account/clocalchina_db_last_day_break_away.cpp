#include "old_data/db/account/clocalchina_db_last_day_break_away.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalLocalChinaLastDayBreakAway = 734,
    kBreakAwayDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844FBAE
// IDA_NAME: _ZN32CLocalChina_DB_LastDay_BreakAwayC2Ev
// DEMANGLED: CLocalChina_DB_LastDay_BreakAway::CLocalChina_DB_LastDay_BreakAway(void)
CLocalChina_DB_LastDay_BreakAway::CLocalChina_DB_LastDay_BreakAway() {}

// IDA_EA: 0x8128B82
// IDA_NAME: _ZN32CLocalChina_DB_LastDay_BreakAwayD2Ev
// DEMANGLED: CLocalChina_DB_LastDay_BreakAway::~CLocalChina_DB_LastDay_BreakAway()
// IDA_EA: 0x8128BB2
// IDA_NAME: _ZN32CLocalChina_DB_LastDay_BreakAwayD0Ev
// DEMANGLED: CLocalChina_DB_LastDay_BreakAway::~CLocalChina_DB_LastDay_BreakAway() deleting destructor
CLocalChina_DB_LastDay_BreakAway::~CLocalChina_DB_LastDay_BreakAway() {}

// IDA_EA: 0x8128152
// IDA_NAME: _ZN32CLocalChina_DB_LastDay_BreakAway11makeRequestEjii
// DEMANGLED: CLocalChina_DB_LastDay_BreakAway::makeRequest(unsigned int,int,int)
void CLocalChina_DB_LastDay_BreakAway::makeRequest(unsigned int account_id,
                                                   int day_count,
                                                   int reward_kind) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("localchina/localchina_DBThread.cpp",
                                           1080);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLocalChinaLastDayBreakAway;
    *guard << -1;
    *guard << account_id;
    *guard << reward_kind;
    *guard << day_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x812801A
// IDA_NAME: _ZN32CLocalChina_DB_LastDay_BreakAway8dispatchEiiP6Stream
// DEMANGLED: CLocalChina_DB_LastDay_BreakAway::dispatch(int,int,Stream *)
bool CLocalChina_DB_LastDay_BreakAway::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    int reward_kind = 0;
    int day_count = 0;
    *stream >> account_id;
    *stream >> reward_kind;
    *stream >> day_count;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kBreakAwayDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    if (reward_kind == 0) {
        db_handle->set_query(
            "UPDATE churn_member_info "
            "   SET quest_time = UNIX_TIMESTAMP(SUBDATE(FROM_UNIXTIME(quest_time), INTERVAL %d DAY)) "
            " WHERE m_id=%s",
            day_count,
            NumberToString(account_id, 0));
    } else if (reward_kind == 1) {
        db_handle->set_query(
            "UPDATE churn_member_info "
            "   SET last_reward_time = UNIX_TIMESTAMP(SUBDATE(FROM_UNIXTIME(last_reward_time), INTERVAL %d DAY)) "
            " WHERE m_id=%s",
            day_count,
            NumberToString(account_id, 0));
    }

    if (db_handle->exec(true) == true) {
        return true;
    }

    LogManager::logFormat(
        1,
        "localchina/localchina_DBThread.cpp",
        "virtual bool CLocalChina_DB_LastDay_BreakAway::dispatch(int, int, Stream*)",
        0x42F,
        "[GM]CLocalChina_DB_LastDay_BreakAway Fail!!");
    return false;
}
