#include "old_data/db/dungeon/db_update_break_away_clear_add_info.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/dungeon/db_update_break_away_dungeon_clear_cnt.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

// IDA_EA: 0x844EDAE
// IDA_NAME: _ZN30DB_UpdateBreakAwayClearAddInfoC2Ev
// DEMANGLED: DB_UpdateBreakAwayClearAddInfo::DB_UpdateBreakAwayClearAddInfo(void)
DB_UpdateBreakAwayClearAddInfo::DB_UpdateBreakAwayClearAddInfo() {}

// IDA_EA: 0x845CEBC
// IDA_NAME: _ZN30DB_UpdateBreakAwayClearAddInfoD2Ev
// DEMANGLED: DB_UpdateBreakAwayClearAddInfo::~DB_UpdateBreakAwayClearAddInfo()
// IDA_EA: 0x845CEEC
// IDA_NAME: _ZN30DB_UpdateBreakAwayClearAddInfoD0Ev
// DEMANGLED: DB_UpdateBreakAwayClearAddInfo::~DB_UpdateBreakAwayClearAddInfo() deleting destructor
DB_UpdateBreakAwayClearAddInfo::~DB_UpdateBreakAwayClearAddInfo() {}

// IDA_EA: 0x843848C
// IDA_NAME: _ZN30DB_UpdateBreakAwayClearAddInfo8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBreakAwayClearAddInfo::dispatch(int,int,Stream *)
bool DB_UpdateBreakAwayClearAddInfo::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    *stream >> account_id;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate churn_member_info set add_info=-1 where m_id=%s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }
    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "virtual bool DB_UpdateBreakAwayClearAddInfo::dispatch(int, int, Stream*)",
        0x85F4,
        "BREAK_AWAY : ADD INFO UPDATE ERROR!!");
    return false;
}

// IDA_EA: 0x844EDCA
// IDA_NAME: _ZN33DB_UpdateBreakAwayDungeonClearCntC2Ev
// DEMANGLED: DB_UpdateBreakAwayDungeonClearCnt::DB_UpdateBreakAwayDungeonClearCnt(void)
DB_UpdateBreakAwayDungeonClearCnt::DB_UpdateBreakAwayDungeonClearCnt() {}

// IDA_EA: 0x845CF0A
// IDA_NAME: _ZN33DB_UpdateBreakAwayDungeonClearCntD2Ev
// DEMANGLED: DB_UpdateBreakAwayDungeonClearCnt::~DB_UpdateBreakAwayDungeonClearCnt()
// IDA_EA: 0x845CF3A
// IDA_NAME: _ZN33DB_UpdateBreakAwayDungeonClearCntD0Ev
// DEMANGLED: DB_UpdateBreakAwayDungeonClearCnt::~DB_UpdateBreakAwayDungeonClearCnt() deleting destructor
DB_UpdateBreakAwayDungeonClearCnt::~DB_UpdateBreakAwayDungeonClearCnt() {}

// IDA_EA: 0x84383BA
// IDA_NAME: _ZN33DB_UpdateBreakAwayDungeonClearCnt8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBreakAwayDungeonClearCnt::dispatch(int,int,Stream *)
bool DB_UpdateBreakAwayDungeonClearCnt::dispatch(int signal,
                                                 int user_idx,
                                                 Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    int add_info = 0;
    *stream >> account_id;
    *stream >> add_info;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate churn_member_info set add_info=%d where m_id=%s",
        add_info,
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }
    return false;
}
