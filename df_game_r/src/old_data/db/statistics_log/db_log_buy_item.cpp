#include "old_data/db/statistics_log/db_log_buy_item.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <ctime>
#include <cstdio>
#include <cstring>


namespace {

enum {
    kDBLogBuyItemSignal = 593,
};

}  // namespace

// IDA_EA: 0x844F66E
// IDA_NAME: _ZN13DB_LogBuyItemC2Ev
// DEMANGLED: DB_LogBuyItem::DB_LogBuyItem(void)
DB_LogBuyItem::DB_LogBuyItem() {}

// IDA_EA: 0x8128AE6
// IDA_NAME: _ZN13DB_LogBuyItemD2Ev
// DEMANGLED: DB_LogBuyItem::~DB_LogBuyItem()
// IDA_EA: 0x8128B16
// IDA_NAME: _ZN13DB_LogBuyItemD0Ev
// DEMANGLED: DB_LogBuyItem::~DB_LogBuyItem() deleting destructor
DB_LogBuyItem::~DB_LogBuyItem() {}

// IDA_EA: 0x812859A
// IDA_NAME: _ZN13DB_LogBuyItem8dispatchEiiP6Stream
// DEMANGLED: DB_LogBuyItem::dispatch(int,int,Stream *)
bool DB_LogBuyItem::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    std::time_t timer = std::time(0);
    std::tm* time_info = std::localtime(&timer);
    if (!time_info) {
        return false;
    }

    CEnvironment* environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    if (time_info->tm_hour < reset_hour) {
        timer -= 86400;
        time_info = std::localtime(&timer);
        if (!time_info) {
            return false;
        }
    }
    time_info->tm_hour = reset_hour;
    time_info->tm_min = 0;
    time_info->tm_sec = 0;
    (void)std::mktime(time_info);

    char occ_date[12];
    std::memset(occ_date, 0, sizeof(occ_date));
    std::snprintf(occ_date,
                  sizeof(occ_date),
                  "%04d-%02d-%02d",
                  time_info->tm_year + 1900,
                  time_info->tm_mon + 1,
                  time_info->tm_mday);

    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    unsigned int item_id = 0;
    unsigned int count = 0;
    int item_type = 0;
    *stream >> account_id;
    *stream >> charac_no;
    *stream >> item_id;
    *stream >> count;
    *stream >> item_type;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "update log_acquire_item\t\t\t\t\t\tset item_count = item_count + %d\t\t\t\t\t\twhere occ_date = '%s' and charac_no = %u and item_id = %u",
        count,
        occ_date,
        charac_no,
        item_id);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "insert into log_acquire_item\t\t\t\t\t\t\t(occ_date, m_id, charac_no, item_id, item_count, item_type)\t\t\t\t\t\t\tvalues ('%s', %s, %u, %u, %u, %d)",
        occ_date,
        NumberToString(account_id, 0),
        charac_no,
        item_id,
        count,
        item_type);
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace("virtual bool DB_LogBuyItem::dispatch(int, int, Stream*)",
                   1382,
                   5);
    trace("DB_LogBuyItem, exec ERROR m_id=%s charac_no=%u ItemID=%u Count=%u Type=%d",
          NumberToString(account_id, 0),
          charac_no,
          item_id,
          count,
          item_type);
    return false;
}

// IDA_EA: 0x812887E
// IDA_NAME: _ZN13DB_LogBuyItem11makeRequestEjjjj20ENUM_DBLOG_ITEM_TYPE
// DEMANGLED: DB_LogBuyItem::makeRequest(unsigned int,unsigned int,unsigned int,unsigned int,ENUM_DBLOG_ITEM_TYPE)
void DB_LogBuyItem::makeRequest(unsigned int account_id,
                                unsigned int charac_no,
                                unsigned int item_id,
                                unsigned int count,
                                ENUM_DBLOG_ITEM_TYPE item_type) {
    if (!charac_no) {
        return;
    }

    Stream* stream =
        GlobalData::stream_pool()->Acquire("localchina/localchina_DBThread.cpp",
                                           1435);
    CStreamGuard guard(stream, true);
    *guard << kDBLogBuyItemSignal;
    *guard << -1;
    *guard << account_id;
    *guard << charac_no;
    *guard << item_id;
    *guard << count;
    *guard << item_type;

    cMyTrace trace(
        "static void DB_LogBuyItem::makeRequest(memberIdentificationNumber_t, unsigned int, unsigned int, unsigned int, ENUM_DBLOG_ITEM_TYPE)",
        1455,
        0);
    trace("DB_LogBuyItem::makeRequest m_id=%s nCharacNo=%d nItemID=%u nCount=%u nType=%d",
          NumberToString(account_id, 0),
          charac_no,
          item_id,
          count,
          item_type);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

extern "C" {
}
