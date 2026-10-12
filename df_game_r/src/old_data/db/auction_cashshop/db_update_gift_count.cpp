#include "old_data/db/auction_cashshop/db_update_gift_count.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>


// IDA_EA: 0x844E756
// IDA_NAME: _ZN18DB_UpdateGiftCountC2Ev
// DEMANGLED: DB_UpdateGiftCount::DB_UpdateGiftCount(void)
DB_UpdateGiftCount::DB_UpdateGiftCount() {}

// IDA_EA: 0x845EAC4
// IDA_NAME: _ZN18DB_UpdateGiftCountD2Ev
// DEMANGLED: DB_UpdateGiftCount::~DB_UpdateGiftCount()
// IDA_EA: 0x845EAF4
// IDA_NAME: _ZN18DB_UpdateGiftCountD0Ev
// DEMANGLED: DB_UpdateGiftCount::~DB_UpdateGiftCount() deleting destructor
DB_UpdateGiftCount::~DB_UpdateGiftCount() {}

// IDA_EA: 0x8427112
// IDA_NAME: _ZN18DB_UpdateGiftCount8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateGiftCount::dispatch(int,int,Stream *)
bool DB_UpdateGiftCount::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    int gift_count = 0;
    *stream >> account_id;
    *stream >> gift_count;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    char table_name[20];
    memset(table_name, 0, sizeof(table_name));
    memcpy(table_name, "member_login", 13);

    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "upDate member_login set last_gift_time=unix_timestamp(now()),gift_cnt=gift_cnt+%d where m_id=%s",
        gift_count,
        account_id_text);
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x84271EC
// IDA_NAME: _ZN18DB_UpdateGiftCount11makeRequestEji
// DEMANGLED: DB_UpdateGiftCount::makeRequest(unsigned int,int)
void DB_UpdateGiftCount::makeRequest(unsigned int account_id, int gift_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 26689);
    CStreamGuard guard(stream, true);
    *guard << 209;
    *guard << -1;
    *guard << account_id;
    *guard << gift_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
