#include "old_data/db/account/db_update_user_gift_fatigue_quantity.h"

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
#include "infra/log/format/trace.h"

#include <string>


namespace {

typedef const std::string& (*UsedFatigueGetDbTableName)(
    EventClassify::CUsedFatigueGiveItem*, int);

}  // namespace

// IDA_EA: 0x844F882
// IDA_NAME: _ZN32DB_UpdateUserGiftFatigueQuantityC2Ev
// DEMANGLED: DB_UpdateUserGiftFatigueQuantity::DB_UpdateUserGiftFatigueQuantity(void)
DB_UpdateUserGiftFatigueQuantity::DB_UpdateUserGiftFatigueQuantity() {}

// IDA_EA: 0x845BAEE
// IDA_NAME: _ZN32DB_UpdateUserGiftFatigueQuantityD2Ev
// DEMANGLED: DB_UpdateUserGiftFatigueQuantity::~DB_UpdateUserGiftFatigueQuantity()
// IDA_EA: 0x845BB1E
// IDA_NAME: _ZN32DB_UpdateUserGiftFatigueQuantityD0Ev
// DEMANGLED: DB_UpdateUserGiftFatigueQuantity::~DB_UpdateUserGiftFatigueQuantity() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateUserGiftFatigueQuantity::~DB_UpdateUserGiftFatigueQuantity() {}

// IDA_EA: 0x8441D1C
// IDA_NAME: _ZN32DB_UpdateUserGiftFatigueQuantity8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateUserGiftFatigueQuantity::dispatch(int,int,Stream *)
bool DB_UpdateUserGiftFatigueQuantity::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream || !GlobalData::s_db_mgr) {
        return false;
    }
    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    if (!event_script_mng) {
        return false;
    }
    EventClassify::CUsedFatigueGiveItem* event_entity =
        reinterpret_cast<EventClassify::CUsedFatigueGiveItem*>(
            event_script_mng->get_event_entity(101));
    bool is_charac_event = false;
    if (!event_entity ||
        event_entity->getIsCharacEvent(is_charac_event) != 1) {
        return false;
    }

    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    unsigned int fatigue_quantity = 0;
    if (stream->get_binary(&account_id, sizeof(account_id)) != 1 ||
        stream->get_binary(&charac_no, sizeof(charac_no)) != 1 ||
        stream->get_binary(&fatigue_quantity, sizeof(fatigue_quantity)) != 1) {
        return false;
    }

    void** vtable = *reinterpret_cast<void***>(event_entity);
    UsedFatigueGetDbTableName get_db_table_name =
        reinterpret_cast<UsedFatigueGetDbTableName>(vtable[8]);
    const std::string& table_name = get_db_table_name(event_entity, 101);
    if (table_name.find_first_not_of(" \t\r\n") == std::string::npos) {
        // Do not acknowledge an event-state save that has no persistence
        // owner, and never construct `update  set ...` / `insert into (...)`.
        LogManager::logFormat(
            1,
            "db_update_user_gift_fatigue_quantity.cpp",
            "bool DB_UpdateUserGiftFatigueQuantity::dispatch(int,int,Stream*)",
            __LINE__,
            "[USED_FATIGUE_EVENT][EMPTY_DB_TABLE] phase=save event_id=101 account_id=%u charac_no=%u fatigue_quantity=%u is_charac_event=%d",
            account_id,
            charac_no,
            fatigue_quantity,
            is_charac_event ? 1 : 0);
        return false;
    }
    MySQL* db_handle = 0;
    if (is_charac_event) {
        db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
        if (!db_handle) {
            return false;
        }
        db_handle->set_query(
            "inSert into %s(charac_no, fatigue_quantity) values(%u, %u) on duplicate key upDate fatigue_quantity=values(fatigue_quantity)",
            ::toTString(table_name),
            charac_no,
            fatigue_quantity);
    } else {
        db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
        if (!db_handle) {
            return false;
        }
        const char* account_id_text = NumberToString(account_id, 0);
        if (!account_id_text) {
            return false;
        }
        db_handle->set_query(
            "inSert into %s(m_id, fatigue_quantity) values(%s, %u) on duplicate key upDate fatigue_quantity=values(fatigue_quantity)",
            ::toTString(table_name),
            account_id_text,
            fatigue_quantity);
    }
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8441FDE
// IDA_NAME: _ZN32DB_UpdateUserGiftFatigueQuantity11makeRequestEjjj
// DEMANGLED: DB_UpdateUserGiftFatigueQuantity::makeRequest(unsigned int,unsigned int,unsigned int)
void DB_UpdateUserGiftFatigueQuantity::makeRequest(
    unsigned int account_id,
    unsigned int charac_no,
    unsigned int fatigue_quantity) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return;
    }
    Stream* stream = pool->Acquire("DBThread.cpp", 41393);
    if (!stream) {
        return;
    }
    CStreamGuard guard(stream, true);
    *guard << 0x288;
    *guard << -1;
    *guard << account_id;
    *guard << charac_no;
    *guard << fatigue_quantity;
    (void)queues->put(MsgQueueMgr::DB_Q, guard);
}
