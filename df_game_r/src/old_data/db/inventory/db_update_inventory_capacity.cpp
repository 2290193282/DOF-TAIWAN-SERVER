#include "old_data/db/inventory/db_update_inventory_capacity.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844FA0A
// IDA_NAME: _ZN26DB_UpdateInventoryCapacityC2Ev
// DEMANGLED: DB_UpdateInventoryCapacity::DB_UpdateInventoryCapacity(void)
DB_UpdateInventoryCapacity::DB_UpdateInventoryCapacity() {}

// IDA_EA: 0x845B60E
// IDA_NAME: _ZN26DB_UpdateInventoryCapacityD2Ev
// DEMANGLED: DB_UpdateInventoryCapacity::~DB_UpdateInventoryCapacity()
// IDA_EA: 0x845B63E
// IDA_NAME: _ZN26DB_UpdateInventoryCapacityD0Ev
// DEMANGLED: DB_UpdateInventoryCapacity::~DB_UpdateInventoryCapacity() deleting destructor
DB_UpdateInventoryCapacity::~DB_UpdateInventoryCapacity() {}

// IDA_EA: 0x8444BEA
// IDA_NAME: _ZN26DB_UpdateInventoryCapacity8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateInventoryCapacity::dispatch(int,int,Stream *)
bool DB_UpdateInventoryCapacity::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    int server_group = 0;
    stream->get_binary(&server_group, sizeof(server_group));
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( server_group));
    int charac_no = 0;
    int inventory_capacity = 0;
    stream->get_binary(&charac_no, sizeof(charac_no));
    stream->get_binary(&inventory_capacity, sizeof(inventory_capacity));
    db_handle->set_query(
        "upDate inventory set inventory_capacity=%d where charac_no=%u",
        inventory_capacity,
        charac_no);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    cMyTrace trace(
        "virtual bool DB_UpdateInventoryCapacity::dispatch(int, int, Stream*)",
        43976,
        5);
    trace("upDate inventory set inventory_capacity=%d where charac_no=%u",
          inventory_capacity,
          charac_no);
    return false;
}

// IDA_EA: 0x8444D12
// IDA_NAME: _ZN26DB_UpdateInventoryCapacity11makeRequestE17ENUM_SERVER_GROUPiji
// DEMANGLED: DB_UpdateInventoryCapacity::makeRequest(ENUM_SERVER_GROUP,int,unsigned int,int)
void DB_UpdateInventoryCapacity::makeRequest(ENUM_SERVER_GROUP server_group,
                                             int user_idx,
                                             unsigned int charac_no,
                                             int inventory_capacity) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 43984);
    CStreamGuard guard(stream, true);
    *guard << 686;
    *guard << -1;
    *guard << server_group;
    *guard << charac_no;
    *guard << inventory_capacity;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    (void)user_idx;
}
