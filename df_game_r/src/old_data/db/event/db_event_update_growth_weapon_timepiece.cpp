#include "old_data/db/event/db_event_update_growth_weapon_timepiece.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalEventUpdateGrowthWeaponTimepiece = 0x315,
};

}  // namespace

// IDA_EA: 0x844FDC2
// IDA_NAME: _ZN35DB_EventUpdateGrowthWeaponTimepieceC2Ev
// DEMANGLED: DB_EventUpdateGrowthWeaponTimepiece::DB_EventUpdateGrowthWeaponTimepiece(void)
DB_EventUpdateGrowthWeaponTimepiece::DB_EventUpdateGrowthWeaponTimepiece() {}

// IDA_EA: 0x845ADD4
// IDA_NAME: _ZN35DB_EventUpdateGrowthWeaponTimepieceD2Ev
// DEMANGLED: DB_EventUpdateGrowthWeaponTimepiece::~DB_EventUpdateGrowthWeaponTimepiece()
// IDA_EA: 0x845AE04
// IDA_NAME: _ZN35DB_EventUpdateGrowthWeaponTimepieceD0Ev
// DEMANGLED: DB_EventUpdateGrowthWeaponTimepiece::~DB_EventUpdateGrowthWeaponTimepiece()
// deleting destructor
DB_EventUpdateGrowthWeaponTimepiece::~DB_EventUpdateGrowthWeaponTimepiece() {}

// IDA_EA: 0x84496F4
// IDA_NAME: _ZN35DB_EventUpdateGrowthWeaponTimepiece8dispatchEiiP6Stream
// DEMANGLED: DB_EventUpdateGrowthWeaponTimepiece::dispatch(int,int,Stream *)
bool DB_EventUpdateGrowthWeaponTimepiece::dispatch(int signal,
                                                   int user_idx,
                                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    unsigned int account_id = 0;
    short timepiece = 0;
    int infinity_weapon_id = 0;
    *stream >> account_id;
    *stream >> timepiece;
    *stream >> infinity_weapon_id;
    if (infinity_weapon_id) {
        db_handle->set_query(
            "upDate event_1207_growthweapon set  reward_occ_date = now(), timepiece = %d, infinityweapon_id = %d  where m_id = %u",
            timepiece,
            infinity_weapon_id,
            account_id);
    } else {
        db_handle->set_query(
            "upDate event_1207_growthweapon set  timepiece = %d, infinityweapon_id = %d  where m_id = %u",
            timepiece,
            0,
            account_id);
    }
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84497EA
// IDA_NAME: _ZN35DB_EventUpdateGrowthWeaponTimepiece11makeRequestEjsi
// DEMANGLED: DB_EventUpdateGrowthWeaponTimepiece::makeRequest(unsigned int,short,int)
void DB_EventUpdateGrowthWeaponTimepiece::makeRequest(unsigned int account_id,
                                                       short timepiece,
                                                       int infinity_weapon_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 47425);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalEventUpdateGrowthWeaponTimepiece;
    *guard << -1;
    *guard << account_id;
    *guard << timepiece;
    *guard << infinity_weapon_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
