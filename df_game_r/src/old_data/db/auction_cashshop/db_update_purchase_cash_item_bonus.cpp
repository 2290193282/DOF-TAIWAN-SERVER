#include "old_data/db/auction_cashshop/db_update_purchase_cash_item_bonus.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/data_manager.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string>


namespace {

enum {
    kDbSignalUpdatePurchaseCashItemBonus = 572,
    kPurchaseCashItemBonusDbHandle = 9,
    kPurchaseCashItemBonusEventType = 154,
};

}  // namespace

// IDA_EA: 0x844F61A
// IDA_NAME: _ZN30DB_UpdatePurchaseCashItemBonusC2Ev
// DEMANGLED: DB_UpdatePurchaseCashItemBonus::DB_UpdatePurchaseCashItemBonus(void)
DB_UpdatePurchaseCashItemBonus::DB_UpdatePurchaseCashItemBonus() {}

// IDA_EA: 0x845C0B8
// IDA_NAME: _ZN30DB_UpdatePurchaseCashItemBonusD2Ev
// DEMANGLED: DB_UpdatePurchaseCashItemBonus::~DB_UpdatePurchaseCashItemBonus()
// IDA_EA: 0x845C0E8
// IDA_NAME: _ZN30DB_UpdatePurchaseCashItemBonusD0Ev
// DEMANGLED: DB_UpdatePurchaseCashItemBonus::~DB_UpdatePurchaseCashItemBonus() deleting destructor
DB_UpdatePurchaseCashItemBonus::~DB_UpdatePurchaseCashItemBonus() {}

// IDA_EA: 0x843E252
// IDA_NAME: _ZN30DB_UpdatePurchaseCashItemBonus8dispatchEiiP6Stream
// DEMANGLED: DB_UpdatePurchaseCashItemBonus::dispatch(int,int,Stream *)
bool DB_UpdatePurchaseCashItemBonus::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    (void)signal;

    unsigned int account_id = 0;
    *stream >> account_id;

    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    const std::string* table_name = event_script_mng
        ? event_script_mng->get_db_table_name(
              kPurchaseCashItemBonusEventType)
        : 0;
    if (!table_name) {
        return false;
    }

    std::string table_name_copy(*table_name);
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kPurchaseCashItemBonusDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));

    db_handle->set_query(
        "upDate %s set purchase_cnt = purchase_cnt + 1 where m_id=%s",
        ::toTString(table_name_copy),
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into %s(m_id, purchase_cnt) values(%s, %u)",
            ::toTString(table_name_copy),
            NumberToString(account_id, 0),
            1);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    db_handle->set_query("seLect purchase_cnt from %s where m_id=%s",
                         ::toTString(table_name_copy),
                         NumberToString(account_id, 0));
    if (db_handle->exec(true) != true ||
        db_handle->get_n_rows() == 0 ||
        db_handle->fetch() != true) {
        return false;
    }

    unsigned int purchase_count = 0;
    if (db_handle->get_uint(0, purchase_count) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 37895);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalUpdatePurchaseCashItemBonus;
    *guard << user_idx;
    SIG_PURCHASE_CASHITEM_BONUS* payload =
        guard->GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS>();
    payload->purchase_count = purchase_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x843E560
// IDA_NAME: _ZN30DB_UpdatePurchaseCashItemBonus11makeRequestEij
// DEMANGLED: DB_UpdatePurchaseCashItemBonus::makeRequest(int,unsigned int)
void DB_UpdatePurchaseCashItemBonus::makeRequest(int uid,
                                                 unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 37909);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdatePurchaseCashItemBonus;
    *guard << uid;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
