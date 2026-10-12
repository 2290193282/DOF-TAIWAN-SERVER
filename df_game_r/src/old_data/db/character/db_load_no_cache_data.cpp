#include "old_data/db/character/db_load_no_cache_data.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>
#include <time.h>


extern long getDailyInitializeTime(int hour, int minute, int second)
    __asm__("_Z22getDailyInitializeTimeiii");

// IDA_EA: 0x844F112
// IDA_NAME: _ZN18DB_LoadNoCacheDataC2Ev
// DEMANGLED: DB_LoadNoCacheData::DB_LoadNoCacheData(void)
DB_LoadNoCacheData::DB_LoadNoCacheData() {}

// IDA_EA: 0x845CFA6
// IDA_NAME: _ZN18DB_LoadNoCacheDataD2Ev
// DEMANGLED: DB_LoadNoCacheData::~DB_LoadNoCacheData()
// IDA_EA: 0x845CFD6
// IDA_NAME: _ZN18DB_LoadNoCacheDataD0Ev
// DEMANGLED: DB_LoadNoCacheData::~DB_LoadNoCacheData() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadNoCacheData::~DB_LoadNoCacheData() {}

// IDA_EA: 0x8437DF4
// IDA_NAME: _ZN18DB_LoadNoCacheData8dispatchEiiP6Stream
// DEMANGLED: DB_LoadNoCacheData::dispatch(int,int,Stream *)
bool DB_LoadNoCacheData::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    int charac_no = 0;
    *stream >> charac_no;

    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            34121);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_LOAD_NO_CACHE_DATA* payload =
        guard->GetInBuffer<SIG_LOAD_NO_CACHE_DATA>();
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(charac_no);
    LoadBloodDungeonReward(payload);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x8437F58
// IDA_NAME: _ZN18DB_LoadNoCacheData22LoadBloodDungeonRewardEP22SIG_LOAD_NO_CACHE_DATA
// DEMANGLED: DB_LoadNoCacheData::LoadBloodDungeonReward(SIG_LOAD_NO_CACHE_DATA *)
bool DB_LoadNoCacheData::LoadBloodDungeonReward(SIG_LOAD_NO_CACHE_DATA* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    time_t timer = static_cast<time_t>(getDailyInitializeTime(6, 0, 0));
    tm current_tm;
    tm* week_tm = localtime_r(&timer, &current_tm);
    int week_offset = 0;
    if (week_tm->tm_wday) {
        week_offset = 86400 * (week_tm->tm_wday - 1);
    } else {
        week_offset = 518400;
    }

    db_handle->set_query(
        "seLect week_point,week_enter_count,week_use_gold,unix_timestamp(last_play_date),enter_count,rank,reward,reward_item_id,reward_gold from charac_blood_dungeon_reward where charac_no=%d and week_occ_date = cast(from_unixtime(%d) as date)",
        payload_data->charac_no,
        static_cast<int>(timer) - week_offset);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() == 1) {
        if (db_handle->fetch() != true) {
            return false;
        }
        int column = 0;
        if (db_handle->get_uint(column++, payload_data->week_point) != true) return false;
        if (db_handle->get_uint(column++, payload_data->week_enter_count) != true) return false;
        if (db_handle->get_uint(column++, payload_data->week_use_gold) != true) return false;
        if (db_handle->get_int(column++, payload_data->last_play_date) != true) return false;
        if (db_handle->get_uint(column++, payload_data->enter_count) != true) return false;
        week_tm->tm_hour = 0;
        week_tm->tm_min = 0;
        week_tm->tm_sec = 0;
        const time_t reset_day = mktime(week_tm);
        if (payload_data->last_play_date != reset_day) {
            payload_data->enter_count = 0;
            payload_data->last_play_date =
                static_cast<int32_t>(reset_day);
        }
    }

    db_handle->set_query(
        "seLect rank,reward,reward_item_id,reward_gold from charac_blood_dungeon_reward where charac_no=%d and week_occ_date = cast(from_unixtime(%d) as date)",
        payload_data->charac_no,
        static_cast<int>(timer) - week_offset - 604800);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() != 1) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }

    int column = 0;
    if (db_handle->get_uint(column++, payload_data->previous_rank) != true) return false;
    char reward = 0;
    if (db_handle->get_byte(column++, reward) != true) return false;
    payload_data->previous_reward_available = reward == 0;
    unsigned long reward_item_id = 0;
    if (db_handle->get_ulong(column++, reward_item_id) != true) return false;
    payload_data->previous_reward_item_id =
        static_cast<uint32_t>(reward_item_id);
    return db_handle->get_uint(column++, payload_data->previous_reward_gold) ==
           true;
}
