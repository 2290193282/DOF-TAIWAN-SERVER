#include "old_data/db/event/db_update_add_item_count_value.h"

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


namespace EventClassify {
class CEventEntity;
}  // namespace EventClassify


namespace {

typedef const std::string& (*EventEntityGetDbTableName)(
    EventClassify::CEventEntity*);

}  // namespace

// IDA_EA: 0x844F7DA
// IDA_NAME: _ZN26DB_UpdateAddItemCountValueC2Ev
// DEMANGLED: DB_UpdateAddItemCountValue::DB_UpdateAddItemCountValue(void)
DB_UpdateAddItemCountValue::DB_UpdateAddItemCountValue() {}

// IDA_EA: 0x845BF80
// IDA_NAME: _ZN26DB_UpdateAddItemCountValueD2Ev
// DEMANGLED: DB_UpdateAddItemCountValue::~DB_UpdateAddItemCountValue()
// IDA_EA: 0x845BFB0
// IDA_NAME: _ZN26DB_UpdateAddItemCountValueD0Ev
// DEMANGLED: DB_UpdateAddItemCountValue::~DB_UpdateAddItemCountValue()
// deleting destructor
DB_UpdateAddItemCountValue::~DB_UpdateAddItemCountValue() {}

// IDA_EA: 0x843ED96
// IDA_NAME: _ZN26DB_UpdateAddItemCountValue8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateAddItemCountValue::dispatch(int,int,Stream *)
bool DB_UpdateAddItemCountValue::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    unsigned int count = 0;
    *stream >> account_id;
    *stream >> count;

    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager->get_event_script_mng();
    EventClassify::CEventEntity* event_entity =
        event_script_mng->get_event_entity(0x193);
    if (event_entity == 0) {
        return false;
    }

    void** vtable = *reinterpret_cast<void***>(event_entity);
    EventEntityGetDbTableName get_db_table_name =
        reinterpret_cast<EventEntityGetDbTableName>(vtable[8]);
    const std::string& table_name = get_db_table_name(event_entity);

    db_handle->set_query(
        "upDate %s set item_cnt=item_cnt+%d where m_id=%s",
        ::toTString(table_name),
        count,
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into %s(m_id, item_cnt) values(%s, %d)",
            ::toTString(table_name),
            NumberToString(account_id, 0),
            count);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x843EF30
// IDA_NAME: _ZN26DB_UpdateAddItemCountValue11makeRequestEjj
// DEMANGLED: DB_UpdateAddItemCountValue::makeRequest(unsigned int,unsigned int)
void DB_UpdateAddItemCountValue::makeRequest(unsigned int account_id,
                                             unsigned int count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 38532);
    CStreamGuard guard(stream, true);
    *guard << 0x280;
    *guard << -1;
    *guard << account_id;
    *guard << count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
