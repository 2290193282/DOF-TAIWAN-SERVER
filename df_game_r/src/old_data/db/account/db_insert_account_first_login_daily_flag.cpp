#include "old_data/db/account/db_insert_account_first_login_daily_flag.h"

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

// IDA_EA: 0x844FF9E
// IDA_NAME: _ZN35DB_InsertAccountFirstLoginDailyFlagC2Ev
// DEMANGLED: DB_InsertAccountFirstLoginDailyFlag::DB_InsertAccountFirstLoginDailyFlag(void)
DB_InsertAccountFirstLoginDailyFlag::DB_InsertAccountFirstLoginDailyFlag() {}

// IDA_EA: 0x845C01C
// IDA_NAME: _ZN35DB_InsertAccountFirstLoginDailyFlagD2Ev
// DEMANGLED: DB_InsertAccountFirstLoginDailyFlag::~DB_InsertAccountFirstLoginDailyFlag()
// IDA_EA: 0x845C04C
// IDA_NAME: _ZN35DB_InsertAccountFirstLoginDailyFlagD0Ev
// DEMANGLED: DB_InsertAccountFirstLoginDailyFlag::~DB_InsertAccountFirstLoginDailyFlag() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_InsertAccountFirstLoginDailyFlag::~DB_InsertAccountFirstLoginDailyFlag() {}

// IDA_EA: 0x843E8BE
// IDA_NAME: _ZN35DB_InsertAccountFirstLoginDailyFlag8dispatchEiiP6Stream
// DEMANGLED: DB_InsertAccountFirstLoginDailyFlag::dispatch(int,int,Stream *)
bool DB_InsertAccountFirstLoginDailyFlag::dispatch(int signal,
                                                   int user_idx,
                                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    int event_type = 0;
    stream->get_binary(&account_id, sizeof(account_id));
    stream->get_binary(&charac_no, sizeof(charac_no));
    stream->get_binary(&event_type, sizeof(event_type));

    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager->get_event_script_mng();
    if (event_type != 412) {
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
    db_handle->set_query(
        "upDate %s set charac_no=%u, occ_date=now() where m_id=%u",
        table_name_text,
        charac_no,
        account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (db_handle->getAffectedRowCount() == 0) {
        table_name_text = ::toTString(table_name);
        db_handle->set_query(
            "inSert into %s(m_id, charac_no, occ_date) values(%u, %u, now())",
            table_name_text,
            account_id,
            charac_no);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x843EA64
// IDA_NAME: _ZN35DB_InsertAccountFirstLoginDailyFlag11makeRequestEijji
// DEMANGLED: DB_InsertAccountFirstLoginDailyFlag::makeRequest(int,unsigned int,unsigned int,int)
void DB_InsertAccountFirstLoginDailyFlag::makeRequest(int user_idx,
                                                      unsigned int account_id,
                                                      unsigned int charac_no,
                                                      int event_type) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 38205);
    CStreamGuard guard(stream, true);
    *guard << 0x342;
    *guard << user_idx;
    *guard << account_id;
    *guard << charac_no;
    *guard << event_type;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
