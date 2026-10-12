#include "old_data/db/event/db_event_growth_weapon_reset.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalEventGrowthWeaponReset = 0x316,
};

}  // namespace

// IDA_EA: 0x844FDDE
// IDA_NAME: _ZN25DB_EventGrowthWeaponResetC2Ev
// DEMANGLED: DB_EventGrowthWeaponReset::DB_EventGrowthWeaponReset(void)
DB_EventGrowthWeaponReset::DB_EventGrowthWeaponReset() {}

// IDA_EA: 0x845AD86
// IDA_NAME: _ZN25DB_EventGrowthWeaponResetD2Ev
// DEMANGLED: DB_EventGrowthWeaponReset::~DB_EventGrowthWeaponReset()
// IDA_EA: 0x845ADB6
// IDA_NAME: _ZN25DB_EventGrowthWeaponResetD0Ev
// DEMANGLED: DB_EventGrowthWeaponReset::~DB_EventGrowthWeaponReset()
// deleting destructor
DB_EventGrowthWeaponReset::~DB_EventGrowthWeaponReset() {}

// IDA_EA: 0x84498FE
// IDA_NAME: _ZN25DB_EventGrowthWeaponReset8dispatchEiiP6Stream
// DEMANGLED: DB_EventGrowthWeaponReset::dispatch(int,int,Stream *)
bool DB_EventGrowthWeaponReset::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    *stream >> account_id;
    db_handle->set_query(
        "deLete from event_1207_growthweapon where m_id = %u",
        account_id);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8449980
// IDA_NAME: _ZN25DB_EventGrowthWeaponReset11makeRequestEj
// DEMANGLED: DB_EventGrowthWeaponReset::makeRequest(unsigned int)
void DB_EventGrowthWeaponReset::makeRequest(unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 47454);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalEventGrowthWeaponReset;
    *guard << -1;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
