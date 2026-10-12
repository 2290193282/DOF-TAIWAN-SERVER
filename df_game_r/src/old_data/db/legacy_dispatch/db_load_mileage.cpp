#include "old_data/db/auction_cashshop/db_load_mileage.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/auction_cashshop/db_mileage.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x844F476
// IDA_NAME: _ZN14DB_LoadMileageC2Ev
// DEMANGLED: DB_LoadMileage::DB_LoadMileage(void)
DB_LoadMileage::DB_LoadMileage() {}

// IDA_EA: 0x845B794
// IDA_NAME: _ZN14DB_LoadMileageD2Ev
// DEMANGLED: DB_LoadMileage::~DB_LoadMileage()
// IDA_EA: 0x845B7C4
// IDA_NAME: _ZN14DB_LoadMileageD0Ev
// DEMANGLED: DB_LoadMileage::~DB_LoadMileage() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadMileage::~DB_LoadMileage() {}

// IDA_EA: 0x8443D2C
// IDA_NAME: _ZN14DB_LoadMileage8dispatchEiiP6Stream
// DEMANGLED: DB_LoadMileage::dispatch(int,int,Stream *)
bool DB_LoadMileage::dispatch(int signal, int user_idx, Stream* stream) {
    SIG_MILEAGE_STRUCT mileage;
    if (stream->get_binary(&mileage, sizeof(mileage)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        cMyTrace trace("virtual bool DB_LoadMileage::dispatch(int, int, Stream*)",
                       42926,
                       5);
        trace("[ERROR] Database Handle is NULL [index:%d]", 1);
        return false;
    }

    db_handle->set_query("seLect miles from member_miles where m_id = %u",
                         mileage.m_id);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("virtual bool DB_LoadMileage::dispatch(int, int, Stream*)",
                       42934,
                       5);
        trace("[ERROR] Exec [m_id:%u][charac_no:%u]",
              mileage.m_id,
              mileage.charac_no);
        return false;
    }

    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            cMyTrace trace(
                "virtual bool DB_LoadMileage::dispatch(int, int, Stream*)",
                42955,
                5);
            trace("[ERROR] Fetch [m_id:%u][charac_no:%u]",
                  mileage.m_id,
                  mileage.charac_no);
            return false;
        }
        if (db_handle->get_int(0, mileage.miles) != true) {
            cMyTrace trace(
                "virtual bool DB_LoadMileage::dispatch(int, int, Stream*)",
                42962,
                5);
            trace("[ERROR] Mileage [m_id:%u][charac_no:%u]",
                  mileage.m_id,
                  mileage.charac_no);
            return false;
        }
    } else {
        db_handle->set_query(
            "inSert into member_miles (m_id, miles, daily_miles) values (%u, 0, 0)",
            mileage.m_id);
        db_handle->exec(true);
        mileage.miles = 0;
    }

    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            42968);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(&mileage, sizeof(mileage));
    return GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard) != 0;
}
