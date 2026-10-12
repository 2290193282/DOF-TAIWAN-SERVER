#include "old_data/db/event/db_update_chuseok_package_purchase.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>


namespace {

enum {
    kDbSignalUpdateChuseokPackagePurchase = 0x213,
    kChuseokPackageDbHandle = 9,
};

}  // namespace

// IDA_EA: 0x844F502
// IDA_NAME: _ZN31DB_UpdateChuseokPackagePurchaseC2Ev
// DEMANGLED: DB_UpdateChuseokPackagePurchase::DB_UpdateChuseokPackagePurchase(void)
DB_UpdateChuseokPackagePurchase::DB_UpdateChuseokPackagePurchase() {}

// IDA_EA: 0x845C328
// IDA_NAME: _ZN31DB_UpdateChuseokPackagePurchaseD2Ev
// DEMANGLED: DB_UpdateChuseokPackagePurchase::~DB_UpdateChuseokPackagePurchase()
// IDA_EA: 0x845C358
// IDA_NAME: _ZN31DB_UpdateChuseokPackagePurchaseD0Ev
// DEMANGLED: DB_UpdateChuseokPackagePurchase::~DB_UpdateChuseokPackagePurchase() deleting destructor
DB_UpdateChuseokPackagePurchase::~DB_UpdateChuseokPackagePurchase() {}

// IDA_EA: 0x843C7BC
// IDA_NAME: _ZN31DB_UpdateChuseokPackagePurchase8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateChuseokPackagePurchase::dispatch(int,int,Stream *)
bool DB_UpdateChuseokPackagePurchase::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kChuseokPackageDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    uint32_t member_id = 0;
    *stream >> member_id;
    db_handle->set_query(
        "upDate event_1009_package_count set purchase_cnt = purchase_cnt+1 where m_id=%s",
        NumberToString(member_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into event_1009_package_count(m_id, purchase_cnt) values(%s, %u)",
            NumberToString(member_id, 0),
            1);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x843C8B6
// IDA_NAME: _ZN31DB_UpdateChuseokPackagePurchase11makeRequestEj
// DEMANGLED: DB_UpdateChuseokPackagePurchase::makeRequest(unsigned int)
void DB_UpdateChuseokPackagePurchase::makeRequest(unsigned int member_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 37053);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateChuseokPackagePurchase;
    *guard << -1;
    *guard << member_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
