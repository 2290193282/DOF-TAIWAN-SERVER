#include "old_data/db/avatar_aura/db_insert_cera_item_history.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalInsertCeraItemHistory = 689,
    kCeraItemHistoryDbHandle = 3,
};

}  // namespace

// IDA_EA: 0x844FA42
// IDA_NAME: _ZN24DB_InsertCeraItemHistoryC2Ev
// DEMANGLED: DB_InsertCeraItemHistory::DB_InsertCeraItemHistory(void)
DB_InsertCeraItemHistory::DB_InsertCeraItemHistory() {}

// IDA_EA: 0x845B572
// IDA_NAME: _ZN24DB_InsertCeraItemHistoryD2Ev
// DEMANGLED: DB_InsertCeraItemHistory::~DB_InsertCeraItemHistory()
// IDA_EA: 0x845B5A2
// IDA_NAME: _ZN24DB_InsertCeraItemHistoryD0Ev
// DEMANGLED: DB_InsertCeraItemHistory::~DB_InsertCeraItemHistory()
DB_InsertCeraItemHistory::~DB_InsertCeraItemHistory() {}

// IDA_EA: 0x8445032
// IDA_NAME: _ZN24DB_InsertCeraItemHistory8dispatchEiiP6Stream
// DEMANGLED: DB_InsertCeraItemHistory::dispatch(int,int,Stream *)
bool DB_InsertCeraItemHistory::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    int avatar_ui_id = 0;
    unsigned int old_charac_no = 0;
    unsigned int new_charac_no = 0;
    char history_kind = 0;
    *stream >> avatar_ui_id;
    *stream >> old_charac_no;
    *stream >> new_charac_no;
    *stream >> history_kind;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kCeraItemHistoryDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into user_items_change_history(ui_id,from_charac_no,to_charac_no,occ_date,change_type) values(%u,%u,%u,now(),%u)",
        static_cast<unsigned int>(avatar_ui_id),
        old_charac_no,
        new_charac_no,
        static_cast<unsigned int>(static_cast<unsigned char>(history_kind)));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844511A
// IDA_NAME: _ZN24DB_InsertCeraItemHistory11makeRequestEijjc
// DEMANGLED: DB_InsertCeraItemHistory::makeRequest(int,unsigned int,unsigned int,char)
void DB_InsertCeraItemHistory::makeRequest(int avatar_ui_id,
                                           unsigned int old_charac_no,
                                           unsigned int new_charac_no,
                                           char history_kind) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 44059);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalInsertCeraItemHistory;
    *guard << -1;
    *guard << avatar_ui_id;
    *guard << old_charac_no;
    *guard << new_charac_no;
    *guard << history_kind;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
