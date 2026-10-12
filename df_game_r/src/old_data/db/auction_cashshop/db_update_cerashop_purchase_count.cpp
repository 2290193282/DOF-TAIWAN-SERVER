#include "old_data/db/auction_cashshop/db_update_cerashop_purchase_count.h"

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
    kDbSignalUpdateCeraShopPurchaseCount = 729,
    kCeraShopPurchaseCountDbHandle = 9,
};

}  // namespace

// IDA_EA: 0x844FBE6
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCountC2Ev
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::DB_UpdateCeraShopPurchaseCount(void)
DB_UpdateCeraShopPurchaseCount::DB_UpdateCeraShopPurchaseCount() {}

// IDA_EA: 0x845AB64
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCountD2Ev
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::~DB_UpdateCeraShopPurchaseCount()
// IDA_EA: 0x845AB94
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCountD0Ev
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::~DB_UpdateCeraShopPurchaseCount() deleting destructor
DB_UpdateCeraShopPurchaseCount::~DB_UpdateCeraShopPurchaseCount() {}

// IDA_EA: 0x844B0CC
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCount8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::dispatch(int,int,Stream *)
bool DB_UpdateCeraShopPurchaseCount::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    (void)signal;

    SIG_CERASHOP_PURCHASE_COUNT* request =
        stream->GetOutBuffer<SIG_CERASHOP_PURCHASE_COUNT>();
    if (!request) {
        cMyTrace trace(
            "virtual bool DB_UpdateCeraShopPurchaseCount::dispatch(int, int, Stream*)",
            48044,
            5);
        trace("[PURCAHSE BONUS] ptrData is null.");
        return false;
    }

    if (_selectPurchaseCount(request->member_id, request->purchase_count) != 1) {
        cMyTrace trace(
            "virtual bool DB_UpdateCeraShopPurchaseCount::dispatch(int, int, Stream*)",
            48050,
            5);
        trace("[PURCAHSE BONUS] select query fail.");
        return false;
    }

    if (request->purchase_count) {
        _updatePurchaseCount(request->member_id, ++request->purchase_count);
    } else {
        request->purchase_count = 1;
        _insertPurchaseCount(request->member_id);
    }

    Stream* result_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 48065);
    CStreamGuard guard(result_stream, true);
    *guard << kDbSignalUpdateCeraShopPurchaseCount;
    *guard << user_idx;
    SIG_CERASHOP_PURCHASE_COUNT* result =
        guard->GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>();
    if (!result) {
        cMyTrace trace(
            "virtual bool DB_UpdateCeraShopPurchaseCount::dispatch(int, int, Stream*)",
            48073,
            5);
        trace("[PURCAHSE BONUS] ptrData is null");
        return false;
    }

    *result = *request;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844B31C
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCount11makeRequestEjmii
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::makeRequest(unsigned int,unsigned long,int,int)
void DB_UpdateCeraShopPurchaseCount::makeRequest(unsigned int member_id,
                                                 unsigned long item_index,
                                                 int server_id,
                                                 int user_idx) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 48086);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateCeraShopPurchaseCount;
    *guard << user_idx;
    SIG_CERASHOP_PURCHASE_COUNT* request =
        guard->GetInBuffer<SIG_CERASHOP_PURCHASE_COUNT>();
    if (!request) {
        cMyTrace trace(
            "static void DB_UpdateCeraShopPurchaseCount::makeRequest(memberIdentificationNumber_t, itemIndexOnlyServer_t, int, int)",
            48094,
            5);
        trace("[PURCAHSE BONUS] ptrData is null");
        return;
    }

    request->member_id = member_id;
    request->item_index = static_cast<unsigned int>(item_index);
    request->purchase_count = 0;
    request->server_id = server_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844B450
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCount20_selectPurchaseCountEjRt
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::_selectPurchaseCount(unsigned int,unsigned short &)
int DB_UpdateCeraShopPurchaseCount::_selectPurchaseCount(
    unsigned int member_id,
    unsigned short& purchase_count) {
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kCeraShopPurchaseCountDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) {
        throw "handle_null";
    }
    if (db_handle->set_query(
            " seLect purchase_cnt from event_purchase_cnt where m_id = %s ",
            NumberToString(member_id, 0)) != true) {
        throw "set_query";
    }
    if (db_handle->exec(true) != true) {
        throw "exec";
    }
    if (db_handle->get_n_rows() == 1) {
        if (db_handle->fetch() != true) {
            throw "fetch";
        }
        if (db_handle->get_ushort(0, purchase_count) != true) {
            throw "get_ushort";
        }
    } else {
        purchase_count = 0;
    }
    return 1;
}

// IDA_EA: 0x844B680
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCount20_updatePurchaseCountEjt
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::_updatePurchaseCount(unsigned int,unsigned short)
int DB_UpdateCeraShopPurchaseCount::_updatePurchaseCount(
    unsigned int member_id,
    unsigned short purchase_count) {
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kCeraShopPurchaseCountDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) {
        throw "handle_null";
    }
    if (db_handle->set_query(
            " upDate event_purchase_cnt set purchase_cnt=%d where m_id = %s ",
            purchase_count,
            NumberToString(member_id, 0)) != true) {
        throw "set_query";
    }
    if (db_handle->exec(true) != true) {
        throw "exec";
    }
    return 0;
}

// IDA_EA: 0x844B808
// IDA_NAME: _ZN30DB_UpdateCeraShopPurchaseCount20_insertPurchaseCountEj
// DEMANGLED: DB_UpdateCeraShopPurchaseCount::_insertPurchaseCount(unsigned int)
int DB_UpdateCeraShopPurchaseCount::_insertPurchaseCount(unsigned int member_id) {
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kCeraShopPurchaseCountDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) {
        throw "handle_null";
    }
    if (db_handle->set_query(
            " inSert into event_purchase_cnt(m_id, purchase_cnt) values(%s, 1) ",
            NumberToString(member_id, 0)) != true) {
        throw "set_query";
    }
    if (db_handle->exec(true) != true) {
        throw "exec";
    }
    return 0;
}
