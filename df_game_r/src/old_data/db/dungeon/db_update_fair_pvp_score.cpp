#include "old_data/db/dungeon/db_update_fair_pvp_score.h"

#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/mysql.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "infra/network/packet/fair_pvp_score_stream.h"

// IDA_EA: 0x844F6FA
// IDA_NAME: _ZN21DB_UpdateFairPvPScoreC2Ev
// DEMANGLED: DB_UpdateFairPvPScore::DB_UpdateFairPvPScore(void)
DB_UpdateFairPvPScore::DB_UpdateFairPvPScore() {}

// IDA_EA: 0x845BDAC
// IDA_NAME: _ZN21DB_UpdateFairPvPScoreD2Ev
// DEMANGLED: DB_UpdateFairPvPScore::~DB_UpdateFairPvPScore()
// IDA_EA: 0x845BDDC
// IDA_NAME: _ZN21DB_UpdateFairPvPScoreD0Ev
// DEMANGLED: DB_UpdateFairPvPScore::~DB_UpdateFairPvPScore()
DB_UpdateFairPvPScore::~DB_UpdateFairPvPScore() {}

// IDA_EA: 0x843FA78
// IDA_NAME: _ZN21DB_UpdateFairPvPScore8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateFairPvPScore::dispatch(int,int,Stream *)
bool DB_UpdateFairPvPScore::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_FAIR_PVP_SCORE* const payload =
        stream->GetOutBuffer<SIG_FAIR_PVP_SCORE>();
    if (!payload) {
        return false;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* compressed_job_score = 0;
    if (put_compressed_blob_data(
            db_handle,
            0,
            reinterpret_cast<char*>(&payload->job_score),
            sizeof(payload->job_score),
            &compressed_job_score) != 1) {
        return false;
    }

    db_handle->set_query(
        "upDate fair_pvp_score set private_win=%u,private_lose=%u,"
        "private_draw=%u,relay_battle_win=%u,relay_battle_lose=%u,"
        "relay_battle_draw=%u,relay_battle_2kill=%u,relay_battle_3kill=%u,"
        "successive_win=%u,max_successive_win=%u,job_score='%s',"
        "daily_play_count =%u, last_play_time=from_unixtime(%d),"
        "give_item=%d where charac_no=%u",
        payload->mode_totals[0].win,
        payload->mode_totals[0].lose,
        payload->mode_totals[0].draw,
        payload->mode_totals[1].win,
        payload->mode_totals[1].lose,
        payload->mode_totals[1].draw,
        payload->relay_two_kill,
        payload->relay_three_kill,
        payload->successive_win,
        payload->maximum_successive_win,
        compressed_job_score,
        payload->daily_play_count,
        payload->last_play_time,
        payload->give_item,
        payload->charac_no);
    return db_handle->exec(true) == true;
}
