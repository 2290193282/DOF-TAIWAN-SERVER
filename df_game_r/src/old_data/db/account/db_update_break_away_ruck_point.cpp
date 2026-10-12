#include "old_data/db/account/db_update_break_away_ruck_point.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>



// IDA_EA: 0x844ED92
// IDA_NAME: _ZN27DB_UpdateBreakAwayRuckPointC2Ev
// DEMANGLED: DB_UpdateBreakAwayRuckPoint::DB_UpdateBreakAwayRuckPoint(void)
DB_UpdateBreakAwayRuckPoint::DB_UpdateBreakAwayRuckPoint() {}

// IDA_EA: 0x845CF58
// IDA_NAME: _ZN27DB_UpdateBreakAwayRuckPointD2Ev
// DEMANGLED: DB_UpdateBreakAwayRuckPoint::~DB_UpdateBreakAwayRuckPoint()
// IDA_EA: 0x845CF88
// IDA_NAME: _ZN27DB_UpdateBreakAwayRuckPointD0Ev
// DEMANGLED: DB_UpdateBreakAwayRuckPoint::~DB_UpdateBreakAwayRuckPoint() deleting destructor
DB_UpdateBreakAwayRuckPoint::~DB_UpdateBreakAwayRuckPoint() {}

// IDA_EA: 0x8438300
// IDA_NAME: _ZN27DB_UpdateBreakAwayRuckPoint8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBreakAwayRuckPoint::dispatch(int,int,Stream *)
bool DB_UpdateBreakAwayRuckPoint::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    *stream >> account_id;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate churn_member_info set luck_point=0 where m_id=%s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }
    return false;
}

// IDA_EA: 0x844ED5A
// IDA_NAME: _ZN29DB_UpdateBreakAwayAccruedCeraC2Ev
// DEMANGLED: DB_UpdateBreakAwayAccruedCera::DB_UpdateBreakAwayAccruedCera(void)
DB_UpdateBreakAwayAccruedCera::DB_UpdateBreakAwayAccruedCera() {}

// IDA_EA: 0x845CE6E
// IDA_NAME: _ZN29DB_UpdateBreakAwayAccruedCeraD2Ev
// DEMANGLED: DB_UpdateBreakAwayAccruedCera::~DB_UpdateBreakAwayAccruedCera()
// IDA_EA: 0x845CE9E
// IDA_NAME: _ZN29DB_UpdateBreakAwayAccruedCeraD0Ev
// DEMANGLED: DB_UpdateBreakAwayAccruedCera::~DB_UpdateBreakAwayAccruedCera() deleting destructor
DB_UpdateBreakAwayAccruedCera::~DB_UpdateBreakAwayAccruedCera() {}

// IDA_EA: 0x843856A
// IDA_NAME: _ZN29DB_UpdateBreakAwayAccruedCera11makeRequestEjj
// DEMANGLED: DB_UpdateBreakAwayAccruedCera::makeRequest(unsigned int,unsigned int)
void DB_UpdateBreakAwayAccruedCera::makeRequest(unsigned int account_id,
                                                unsigned int used_cera) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34302);
    CStreamGuard guard(stream, true);
    *guard << 326;
    *guard << -1;
    *guard << account_id;
    *guard << used_cera;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843865C
// IDA_NAME: _ZN29DB_UpdateBreakAwayAccruedCera8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBreakAwayAccruedCera::dispatch(int,int,Stream *)
bool DB_UpdateBreakAwayAccruedCera::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    unsigned int used_cera = 0;
    *stream >> account_id;
    *stream >> used_cera;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate churn_member_info set accrue_cera= accrue_cera+%u where m_id=%s",
        used_cera,
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true) {
        return true;
    }
    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "virtual bool DB_UpdateBreakAwayAccruedCera::dispatch(int, int, Stream*)",
        0x8619,
        "BREAK_AWAY : UPDATE CERA ERROR!!");
    return false;
}
