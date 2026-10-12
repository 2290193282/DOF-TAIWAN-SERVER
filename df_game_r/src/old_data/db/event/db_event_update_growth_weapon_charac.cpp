#include "old_data/db/event/db_event_update_growth_weapon_charac.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalEventUpdateGrowthWeaponCharac = 0x314,
};

}  // namespace

// IDA_EA: 0x844FDA6
// IDA_NAME: _ZN32DB_EventUpdateGrowthWeaponCharacC2Ev
// DEMANGLED: DB_EventUpdateGrowthWeaponCharac::DB_EventUpdateGrowthWeaponCharac(void)
DB_EventUpdateGrowthWeaponCharac::DB_EventUpdateGrowthWeaponCharac() {}

// IDA_EA: 0x845AE22
// IDA_NAME: _ZN32DB_EventUpdateGrowthWeaponCharacD2Ev
// DEMANGLED: DB_EventUpdateGrowthWeaponCharac::~DB_EventUpdateGrowthWeaponCharac()
// IDA_EA: 0x845AE52
// IDA_NAME: _ZN32DB_EventUpdateGrowthWeaponCharacD0Ev
// DEMANGLED: DB_EventUpdateGrowthWeaponCharac::~DB_EventUpdateGrowthWeaponCharac()
// deleting destructor
DB_EventUpdateGrowthWeaponCharac::~DB_EventUpdateGrowthWeaponCharac() {}

// IDA_EA: 0x84494E2
// IDA_NAME: _ZN32DB_EventUpdateGrowthWeaponCharac8dispatchEiiP6Stream
// DEMANGLED: DB_EventUpdateGrowthWeaponCharac::dispatch(int,int,Stream *)
bool DB_EventUpdateGrowthWeaponCharac::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    int growth_weapon_id = 0;
    short server_id = 0;
    *stream >> account_id;
    *stream >> charac_no;
    *stream >> growth_weapon_id;
    *stream >> server_id;
    db_handle->set_query(
        "upDate event_1207_growthweapon set occ_date = now(), charac_no = %d, growthweapon_id = %d, server_id = %d where m_id = %u",
        charac_no,
        growth_weapon_id,
        server_id,
        account_id);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84495C6
// IDA_NAME: _ZN32DB_EventUpdateGrowthWeaponCharac11makeRequestEjjis
// DEMANGLED: DB_EventUpdateGrowthWeaponCharac::makeRequest(unsigned int,unsigned int,int,short)
void DB_EventUpdateGrowthWeaponCharac::makeRequest(unsigned int account_id,
                                                    unsigned int charac_no,
                                                    int growth_weapon_id,
                                                    short server_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 47380);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalEventUpdateGrowthWeaponCharac;
    *guard << -1;
    *guard << account_id;
    *guard << charac_no;
    *guard << growth_weapon_id;
    *guard << server_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
