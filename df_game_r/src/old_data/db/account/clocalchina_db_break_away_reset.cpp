#include "old_data/db/account/clocalchina_db_break_away_reset.h"

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
    kDbSignalLocalChinaBreakAwayReset = 733,
    kBreakAwayDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844FB92
// IDA_NAME: _ZN29CLocalChina_DB_BreakAwayResetC2Ev
// DEMANGLED: CLocalChina_DB_BreakAwayReset::CLocalChina_DB_BreakAwayReset(void)
CLocalChina_DB_BreakAwayReset::CLocalChina_DB_BreakAwayReset() {}

// IDA_EA: 0x8128BD0
// IDA_NAME: _ZN29CLocalChina_DB_BreakAwayResetD2Ev
// DEMANGLED: CLocalChina_DB_BreakAwayReset::~CLocalChina_DB_BreakAwayReset()
// IDA_EA: 0x8128C00
// IDA_NAME: _ZN29CLocalChina_DB_BreakAwayResetD0Ev
// DEMANGLED: CLocalChina_DB_BreakAwayReset::~CLocalChina_DB_BreakAwayReset() deleting destructor
CLocalChina_DB_BreakAwayReset::~CLocalChina_DB_BreakAwayReset() {}

// IDA_EA: 0x8127F42
// IDA_NAME: _ZN29CLocalChina_DB_BreakAwayReset11makeRequestEj
// DEMANGLED: CLocalChina_DB_BreakAwayReset::makeRequest(unsigned int)
void CLocalChina_DB_BreakAwayReset::makeRequest(unsigned int account_id) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("localchina/localchina_DBThread.cpp",
                                           1032);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLocalChinaBreakAwayReset;
    *guard << -1;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8127E84
// IDA_NAME: _ZN29CLocalChina_DB_BreakAwayReset8dispatchEiiP6Stream
// DEMANGLED: CLocalChina_DB_BreakAwayReset::dispatch(int,int,Stream *)
bool CLocalChina_DB_BreakAwayReset::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    *stream >> account_id;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kBreakAwayDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "UPDATE churn_member_info "
        "   SET accrue_cera = 0, first_reward_time = 0, last_reward_time = 0, "
        "       add_info = 0, luck_point = 0, last_update_time = 0, "
        "       second_reward_time = 0, quest_time = 0 "
        " WHERE m_id=%s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true) {
        return true;
    }

    LogManager::logFormat(
        1,
        "localchina/localchina_DBThread.cpp",
        "virtual bool CLocalChina_DB_BreakAwayReset::dispatch(int, int, Stream*)",
        0x3FF,
        "[GM]CLocalChina_DB_BreakAwayReset Fail!!");
    return false;
}
