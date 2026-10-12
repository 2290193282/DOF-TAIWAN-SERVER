#include "old_data/db/account/db_update_user_fatigue_quantity.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalUpdateUserFatigueQuantity = 647,
    kDbHandleAccountEvent = 9,
};

// 140828 deployment evidence (SHOW CREATE TABLE) proves the account-wide
// owner as taiwan_game_event.event_account_fatigue(m_id PK, fatigue).  The
// 0627 implementation resolved event-table key 153 and wrote a column named
// fatigue_quantity; that mapping is absent from the target PVF and must not
// be retained as a runtime compatibility dependency.
const char kAccountFatigueTable[] = "event_account_fatigue";

}  // namespace

// IDA_EA: 0x844F866
// IDA_NAME: _ZN28DB_UpdateUserFatigueQuantityC2Ev
// DEMANGLED: DB_UpdateUserFatigueQuantity::DB_UpdateUserFatigueQuantity(void)
DB_UpdateUserFatigueQuantity::DB_UpdateUserFatigueQuantity() {}

// IDA_EA: 0x845BB3C
// IDA_NAME: _ZN28DB_UpdateUserFatigueQuantityD2Ev
// DEMANGLED: DB_UpdateUserFatigueQuantity::~DB_UpdateUserFatigueQuantity()
// IDA_EA: 0x845BB6C
// IDA_NAME: _ZN28DB_UpdateUserFatigueQuantityD0Ev
// DEMANGLED: DB_UpdateUserFatigueQuantity::~DB_UpdateUserFatigueQuantity() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateUserFatigueQuantity::~DB_UpdateUserFatigueQuantity() {}

// IDA_EA: 0x8441A8E
// IDA_NAME: _ZN28DB_UpdateUserFatigueQuantity8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateUserFatigueQuantity::dispatch(int,int,Stream *)
bool DB_UpdateUserFatigueQuantity::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)user_idx;

    if (signal != kDbSignalUpdateUserFatigueQuantity || !stream ||
        stream->length() !=
            static_cast<int>(sizeof(unsigned int) * 2u)) {
        return false;
    }

    unsigned int account_id = 0;
    unsigned int fatigue_quantity = 0;
    if (stream->get_binary(&account_id, sizeof(account_id)) != 1 ||
        stream->get_binary(&fatigue_quantity,
                           sizeof(fatigue_quantity)) != 1 ||
        !account_id || !fatigue_quantity || !GlobalData::s_db_mgr) {
        return false;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kDbHandleAccountEvent),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }

    // One statement closes the old update-then-insert race on the MyISAM
    // account table.  Each DB_Q request is one authoritative fatigue delta;
    // a duplicate primary key therefore accumulates exactly that delta.
    return db_handle->set_query(
               "inSert into %s(m_id,fatigue) values(%u,%u) "
               "on duplicate key update fatigue=fatigue+values(fatigue)",
               kAccountFatigueTable,
               account_id,
               fatigue_quantity) &&
           db_handle->exec(true) == true;
}

// IDA_EA: 0x8441C2A
// IDA_NAME: _ZN28DB_UpdateUserFatigueQuantity11makeRequestEjj
// DEMANGLED: DB_UpdateUserFatigueQuantity::makeRequest(unsigned int,unsigned int)
bool DB_UpdateUserFatigueQuantity::makeRequest(
    unsigned int account_id,
    unsigned int fatigue_quantity) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!account_id || !fatigue_quantity || !pool || !queues) {
        return false;
    }

    Stream* stream = pool->Acquire("DBThread.cpp", 41292);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateUserFatigueQuantity;
    *guard << -1;
    *guard << account_id;
    *guard << fatigue_quantity;
    return queues->put(MsgQueueMgr::DB_Q, guard) == 1;
}
