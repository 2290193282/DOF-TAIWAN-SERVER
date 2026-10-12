#include "old_data/db/dungeon/db_update_revenge_data.h"

#include <cstdio>
#include <ctime>

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"


namespace {

enum {
    kDbSignalUpdateRevengeData = 580,
    kDbHandleVillageAttackDungeon = 3,
};

}  // namespace

// IDA_EA: 0x844F68A
// IDA_NAME: _ZN20DB_UpdateRevengeDataC2Ev
// DEMANGLED: DB_UpdateRevengeData::DB_UpdateRevengeData(void)
DB_UpdateRevengeData::DB_UpdateRevengeData() {}

// IDA_EA: 0x845BEE4
// IDA_NAME: _ZN20DB_UpdateRevengeDataD2Ev
// DEMANGLED: DB_UpdateRevengeData::~DB_UpdateRevengeData()
// IDA_EA: 0x845BF14
// IDA_NAME: _ZN20DB_UpdateRevengeDataD0Ev
// DEMANGLED: DB_UpdateRevengeData::~DB_UpdateRevengeData() deleting destructor
DB_UpdateRevengeData::~DB_UpdateRevengeData() {}

// IDA_EA: 0x843F232
// IDA_NAME: _ZN20DB_UpdateRevengeData11makeRequestEjhh
// DEMANGLED: DB_UpdateRevengeData::makeRequest(unsigned int,unsigned char,unsigned char)
void DB_UpdateRevengeData::makeRequest(unsigned int charac_no,
                                       unsigned char attack_count,
                                       unsigned char revenge_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 39101);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateRevengeData;
    *guard << -1;
    *guard << charac_no;
    *guard << attack_count;
    *guard << revenge_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843F34C
// IDA_NAME: _ZN20DB_UpdateRevengeData8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateRevengeData::dispatch(int,int,Stream *)
bool DB_UpdateRevengeData::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int charac_no = 0;
    unsigned char attack_count = 0;
    unsigned char revenge_count = 0;
    *stream >> charac_no;
    *stream >> attack_count;
    *stream >> revenge_count;
    if (charac_no == 0) {
        return false;
    }

    time_t timer =
        static_cast<time_t>(GlobalData::system_time()->getCurSec());
    tm* local_time = std::localtime(&timer);
    const int daily_reset_hour =
        G_CEnvironment() ? G_CEnvironment()->get_fatigue_time() : 0;
    if (local_time != 0 && local_time->tm_hour < daily_reset_hour) {
        timer -= 86400;
        local_time = std::localtime(&timer);
    }
    if (local_time == 0) {
        return false;
    }

    char occ_date[12] = {};
    std::snprintf(occ_date,
                  sizeof(occ_date),
                  "%04d-%02d-%02d",
                  local_time->tm_year + 1900,
                  local_time->tm_mon + 1,
                  local_time->tm_mday);

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbHandleVillageAttackDungeon),static_cast<ENUM_SERVER_GROUP>( 0));
    if (db_handle == 0) {
        return false;
    }

    db_handle->set_query(
        "upDate village_attack_dungeon set attack_count = %u, revenge_dungeon = %u where occ_date = '%s' and charac_no = %u",
        attack_count,
        revenge_count,
        occ_date,
        charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "virtual bool DB_UpdateRevengeData::dispatch(int, int, Stream*)",
            39164,
            5);
        trace("village_attack_dungeon upDate exec fail");
        return false;
    }

    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into village_attack_dungeon(occ_date, charac_no, attack_count, revenge_dungeon) values('%s', %u, %u, %u)",
        occ_date,
        charac_no,
        attack_count,
        revenge_count);
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace(
        "virtual bool DB_UpdateRevengeData::dispatch(int, int, Stream*)",
        39175,
        5);
    trace("event_integration_charac inSert exec fail");
    return false;
}
