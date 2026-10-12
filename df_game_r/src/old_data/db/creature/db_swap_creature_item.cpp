#include "old_data/db/creature/db_swap_creature_item.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

enum {
    kDbSignalSwapCreatureItem = 140,
};

}  // namespace

// IDA_EA: 0x844E39E
// IDA_NAME: _ZN19DB_SwapCreatureItemC2Ev
// DEMANGLED: DB_SwapCreatureItem::DB_SwapCreatureItem(void)
DB_SwapCreatureItem::DB_SwapCreatureItem() {}

// IDA_EA: 0x845F39A
// IDA_NAME: _ZN19DB_SwapCreatureItemD2Ev
// DEMANGLED: DB_SwapCreatureItem::~DB_SwapCreatureItem()
// IDA_EA: 0x845F3CA
// IDA_NAME: _ZN19DB_SwapCreatureItemD0Ev
// DEMANGLED: DB_SwapCreatureItem::~DB_SwapCreatureItem() deleting destructor
DB_SwapCreatureItem::~DB_SwapCreatureItem() {}

// IDA_EA: 0x841FE0C
// IDA_NAME: _ZN19DB_SwapCreatureItem8dispatchEiiP6Stream
// DEMANGLED: DB_SwapCreatureItem::dispatch(int,int,Stream *)
bool DB_SwapCreatureItem::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SWAP_CREATURE_ITEM* payload =
        stream->GetOutBuffer<SIG_SWAP_CREATURE_ITEM>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    if (payload->first_empty != 1) {
        db_handle->set_query(
            "upDate creature_items set slot=%d where ui_id=%d",
            payload->first_slot,
            payload->first_ui_id);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    if (payload->second_empty == 1) {
        return true;
    }
    db_handle->set_query(
        "upDate creature_items set slot=%d where ui_id=%d",
        payload->second_slot,
        payload->second_ui_id);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x841FEF2
// IDA_NAME: _ZN19DB_SwapCreatureItem11makeRequestEii
// DEMANGLED: DB_SwapCreatureItem::makeRequest(int,int)
void DB_SwapCreatureItem::makeRequest(int ui_id, int slot) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 22114);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSwapCreatureItem;
    *guard << -1;
    SIG_SWAP_CREATURE_ITEM* payload =
        guard->GetInBuffer<SIG_SWAP_CREATURE_ITEM>();
    if (!payload) {
        return;
    }
    payload->first_empty = 0;
    payload->first_slot = slot;
    payload->first_ui_id = ui_id;
    payload->second_empty = 1;
    payload->second_slot = 0;
    payload->second_ui_id = 0;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
