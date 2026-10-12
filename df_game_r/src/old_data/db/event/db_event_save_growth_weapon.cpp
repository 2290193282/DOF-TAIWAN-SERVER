#include "old_data/db/event/db_event_save_growth_weapon.h"

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
    kDbSignalEventSaveGrowthWeapon = 0x313,
};

}  // namespace

// IDA_EA: 0x844FD8A
// IDA_NAME: _ZN24DB_EventSaveGrowthWeaponC2Ev
// DEMANGLED: DB_EventSaveGrowthWeapon::DB_EventSaveGrowthWeapon(void)
DB_EventSaveGrowthWeapon::DB_EventSaveGrowthWeapon() {}

// IDA_EA: 0x845AE70
// IDA_NAME: _ZN24DB_EventSaveGrowthWeaponD2Ev
// DEMANGLED: DB_EventSaveGrowthWeapon::~DB_EventSaveGrowthWeapon()
// IDA_EA: 0x845AEA0
// IDA_NAME: _ZN24DB_EventSaveGrowthWeaponD0Ev
// DEMANGLED: DB_EventSaveGrowthWeapon::~DB_EventSaveGrowthWeapon()
// deleting destructor
DB_EventSaveGrowthWeapon::~DB_EventSaveGrowthWeapon() {}

// IDA_EA: 0x84492FC
// IDA_NAME: _ZN24DB_EventSaveGrowthWeapon8dispatchEiiP6Stream
// DEMANGLED: DB_EventSaveGrowthWeapon::dispatch(int,int,Stream *)
bool DB_EventSaveGrowthWeapon::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    short server_id = 0;
    *stream >> account_id;
    *stream >> server_id;
    db_handle->set_query(
        "inSert into event_1207_growthweapon(m_id, server_id, occ_date) values(%s, %d, now())",
        NumberToString(account_id, 0),
        server_id);
    if (db_handle->exec(true) == true) {
        return true;
    }

    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "virtual bool DB_EventSaveGrowthWeapon::dispatch(int, int, Stream*)",
        47334,
        "DB_EventSaveGrowthWeapon Query Error : m_id(%d)",
        account_id);
    return false;
}

// IDA_EA: 0x84493E8
// IDA_NAME: _ZN24DB_EventSaveGrowthWeapon11makeRequestEjs
// DEMANGLED: DB_EventSaveGrowthWeapon::makeRequest(unsigned int,short)
void DB_EventSaveGrowthWeapon::makeRequest(unsigned int account_id,
                                           short server_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 47343);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalEventSaveGrowthWeapon;
    *guard << -1;
    *guard << account_id;
    *guard << server_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
