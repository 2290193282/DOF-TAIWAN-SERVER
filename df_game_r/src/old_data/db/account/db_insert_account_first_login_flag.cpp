#include "old_data/db/account/db_insert_account_first_login_flag.h"

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

#include <string>


namespace {

typedef const std::string& (*EventEntityGetDbTableName)(
    EventClassify::CEventEntity*);

}  // namespace

// IDA_EA: 0x844F5E2
// IDA_NAME: _ZN30DB_InsertAccountFirstLoginFlagC2Ev
// DEMANGLED: DB_InsertAccountFirstLoginFlag::DB_InsertAccountFirstLoginFlag(void)
DB_InsertAccountFirstLoginFlag::DB_InsertAccountFirstLoginFlag() {}

// IDA_EA: 0x845C06A
// IDA_NAME: _ZN30DB_InsertAccountFirstLoginFlagD2Ev
// DEMANGLED: DB_InsertAccountFirstLoginFlag::~DB_InsertAccountFirstLoginFlag()
// IDA_EA: 0x845C09A
// IDA_NAME: _ZN30DB_InsertAccountFirstLoginFlagD0Ev
// DEMANGLED: DB_InsertAccountFirstLoginFlag::~DB_InsertAccountFirstLoginFlag() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_InsertAccountFirstLoginFlag::~DB_InsertAccountFirstLoginFlag() {}

// IDA_EA: 0x843E636
// IDA_NAME: _ZN30DB_InsertAccountFirstLoginFlag8dispatchEiiP6Stream
// DEMANGLED: DB_InsertAccountFirstLoginFlag::dispatch(int,int,Stream *)
bool DB_InsertAccountFirstLoginFlag::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    int server_id = 0;
    unsigned int charac_no = 0;
    int event_type = 0;
    stream->get_binary(&account_id, sizeof(account_id));
    stream->get_binary(&server_id, sizeof(server_id));
    stream->get_binary(&charac_no, sizeof(charac_no));
    stream->get_binary(&event_type, sizeof(event_type));

    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager->get_event_script_mng();
    if (event_type != 401 && event_type != 402) {
        return false;
    }

    EventClassify::CEventEntity* event_entity =
        event_script_mng->get_event_entity(event_type);
    if (event_entity == 0) {
        return false;
    }

    void** vtable = *reinterpret_cast<void***>(event_entity);
    EventEntityGetDbTableName get_db_table_name =
        reinterpret_cast<EventEntityGetDbTableName>(vtable[8]);
    const std::string& table_name = get_db_table_name(event_entity);
    const char* table_name_text = ::toTString(table_name);
    const char* account_id_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "inSert into %s(m_id,server_id, charac_no, occ_date) values(%s,%d, %u, now())",
        table_name_text,
        account_id_text,
        server_id,
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x843E79A
// IDA_NAME: _ZN30DB_InsertAccountFirstLoginFlag11makeRequestEijiji
// DEMANGLED: DB_InsertAccountFirstLoginFlag::makeRequest(int,unsigned int,int,unsigned int,int)
void DB_InsertAccountFirstLoginFlag::makeRequest(int user_idx,
                                                 unsigned int account_id,
                                                 int server_id,
                                                 unsigned int charac_no,
                                                 int event_type) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 38147);
    CStreamGuard guard(stream, true);
    *guard << 0x25E;
    *guard << user_idx;
    *guard << account_id;
    *guard << server_id;
    *guard << charac_no;
    *guard << event_type;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
