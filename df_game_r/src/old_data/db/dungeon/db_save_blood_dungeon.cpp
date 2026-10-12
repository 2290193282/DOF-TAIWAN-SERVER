#include "old_data/db/dungeon/db_save_blood_dungeon.h"

#include <ctime>

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/dungeon/blood_dungeon_payload.h"
#include "infra/network/packet/blood_dungeon_stream.h"

long getDailyInitializeTime(int hour, int minute, int second);

// IDA_EA: 0x844F06A
// IDA_NAME: _ZN19DB_SaveBloodDungeonC2Ev
// DEMANGLED: DB_SaveBloodDungeon::DB_SaveBloodDungeon(void)
DB_SaveBloodDungeon::DB_SaveBloodDungeon() {}

// IDA_EA: 0x845D17A
// IDA_NAME: _ZN19DB_SaveBloodDungeonD2Ev
// DEMANGLED: DB_SaveBloodDungeon::~DB_SaveBloodDungeon()
// IDA_EA: 0x845D1AA
// IDA_NAME: _ZN19DB_SaveBloodDungeonD0Ev
// DEMANGLED: DB_SaveBloodDungeon::~DB_SaveBloodDungeon()
DB_SaveBloodDungeon::~DB_SaveBloodDungeon() {}

// IDA_EA: 0x84374F8
// IDA_NAME: _ZN19DB_SaveBloodDungeon22SaveBloodDungeonRewardEP17SIG_BLOOD_DUNGEON
// DEMANGLED: DB_SaveBloodDungeon::SaveBloodDungeonReward(SIG_BLOOD_DUNGEON *)
bool DB_SaveBloodDungeon::SaveBloodDungeonReward(
    SIG_BLOOD_DUNGEON* payload) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    time_t timer =
        static_cast<time_t>(getDailyInitializeTime(6, 0, 0));
    tm local_tm;
    tm* const current = localtime_r(&timer, &local_tm);
    const int week_offset = current->tm_wday
                                ? 86400 * (current->tm_wday - 1)
                                : 518400;

    if (payload->previous_reward_claimed) {
        db_handle->set_query(
            "upDate charac_blood_dungeon_reward set reward=1 where "
            "charac_no=%u and week_occ_date = cast(from_unixtime(%d) as "
            "date)",
            payload->charac_no,
            static_cast<int>(timer) - week_offset - 604800);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    if (!payload->enter_count) {
        return true;
    }

    db_handle->set_query(
        "upDate charac_blood_dungeon_reward set week_point=%u,"
        "week_enter_count=%u,week_use_gold=%u,last_play_date=cast("
        "from_unixtime(%d) as date),enter_count=%u where charac_no=%u and "
        "week_occ_date = cast(from_unixtime(%d) as date)",
        payload->week_point,
        payload->week_enter_count,
        payload->week_use_gold,
        payload->last_play_date,
        payload->enter_count,
        payload->charac_no,
        static_cast<int>(timer) - week_offset);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_blood_dungeon_reward(charac_no,week_occ_date,"
        "week_point,week_enter_count,week_use_gold,last_play_date,"
        "enter_count) values(%u,cast(from_unixtime(%d) as date),%u,%u,%u,"
        "cast(from_unixtime(%d) as date),%u)",
        payload->charac_no,
        static_cast<int>(timer) - week_offset,
        payload->week_point,
        payload->week_enter_count,
        payload->week_use_gold,
        payload->last_play_date,
        payload->enter_count);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8437728
// IDA_NAME: _ZN19DB_SaveBloodDungeon8dispatchEiiP6Stream
// DEMANGLED: DB_SaveBloodDungeon::dispatch(int,int,Stream *)
bool DB_SaveBloodDungeon::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_BLOOD_DUNGEON* const payload =
        stream->GetOutBuffer<SIG_BLOOD_DUNGEON>();
    SaveBloodDungeonReward(payload);
    // The old function at 0x8437728 explicitly returns one after invoking
    // the real SQL worker; this fixed result is direct binary evidence.
    return true;
}
