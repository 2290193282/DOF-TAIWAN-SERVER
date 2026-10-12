#include "old_data/db/pvp_party/db_update_pvp_grade.h"

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <ctime>
#include <string.h>
#include <vector>

namespace {

enum {
    kDbSignalUpdatePvPGrade = 212,
    kPvPRankBucketCount = 5,
    kPvPRankRateScale = 1000,
    kPvPRankRateRound = 500,
    kPvPGradeUpdateHour = 1,
    kSecondsPerDay = 86400,
    kInQueryBufferSize = 24576,
};

}  // namespace

// IDA_EA: 0x844E7AA
// IDA_NAME: _ZN17DB_UpdatePvPGradeC2Ev
DB_UpdatePvPGrade::DB_UpdatePvPGrade() {}

// IDA_EA: 0x845E76A
// IDA_NAME: _ZN17DB_UpdatePvPGradeD2Ev
// IDA_EA: 0x845E79A
// IDA_NAME: _ZN17DB_UpdatePvPGradeD0Ev
DB_UpdatePvPGrade::~DB_UpdatePvPGrade() {}

// IDA_EA: 0x8428B26
// IDA_NAME: _ZN17DB_UpdatePvPGrade8dispatchEiiP6Stream
// DEMANGLED: DB_UpdatePvPGrade::dispatch(int, int, Stream*)
bool DB_UpdatePvPGrade::dispatch(int signal,
                                 int user_idx,
                                 Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    RefPvpGrade* ref_pvp_grade = GlobalData::s_ref_pvp_grade;
    db_handle->set_query(
        "seLect charac_no,pvp_grade,pvp_point,unix_timestamp(last_play_time) from pvp_grade_expand where pvp_grade>=%d order by pvp_point desc limit %d",
        ref_pvp_grade->getPvPRankRateGrade(),
        ref_pvp_grade->getPvPRankTotalCount());
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }

    time_t timer = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    tm update_time;
    localtime_r(&timer, &update_time);
    update_time.tm_hour = kPvPGradeUpdateHour;
    update_time.tm_min = 0;
    update_time.tm_sec = 0;
    timer = mktime(&update_time);
    const int inactive_cutoff = static_cast<int>(timer) - kSecondsPerDay;

    std::vector<stPvPTempTable_t> rank_rows;
    std::vector<int> loss_above_floor;
    std::vector<int> loss_to_floor;
    const int loss_grade = ref_pvp_grade->getPvPExpLossGrade();
    const int floor_point = ref_pvp_grade->GetPvpPoint(loss_grade);

    const unsigned int row_count = db_handle->get_n_rows();
    for (unsigned int row_index = 0; row_index < row_count; ++row_index) {
        stPvPTempTable_t row;
        unsigned int last_play_time = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_uint(0, row.charac_no) != true ||
            db_handle->get_int(1, row.pvp_grade) != true ||
            db_handle->get_int(2, row.pvp_point) != true ||
            db_handle->get_uint(3, last_play_time) != true) {
            return false;
        }
        row.last_play_time = static_cast<int>(last_play_time);

        if (ref_pvp_grade->getPvPExpLossGrade() <= row.pvp_grade &&
            row.last_play_time <= inactive_cutoff &&
            row.pvp_point > floor_point) {
            const int loss_exp = ref_pvp_grade->getPvPExpLossExp();
            if (row.pvp_point <= floor_point + loss_exp) {
                loss_to_floor.push_back(static_cast<int>(row.charac_no));
            } else {
                loss_above_floor.push_back(static_cast<int>(row.charac_no));
            }
            row.pvp_point = std::max(
                floor_point,
                row.pvp_point - ref_pvp_grade->getPvPExpLossExp());
        }
        rank_rows.push_back(row);
    }

    std::vector<int>* rank_rates = ref_pvp_grade->getPvPRankRate();
    int bucket_sizes[kPvPRankBucketCount] = {0};
    for (int bucket = 0; bucket < kPvPRankBucketCount; ++bucket) {
        bucket_sizes[bucket] =
            (static_cast<int>(rank_rows.size()) * (*rank_rates)[bucket] +
             kPvPRankRateRound) /
            kPvPRankRateScale;
    }
    for (int bucket = 1; bucket < kPvPRankBucketCount; ++bucket) {
        if (bucket_sizes[bucket - 1] > 0) {
            bucket_sizes[bucket] = std::max(
                1,
                bucket_sizes[bucket] - bucket_sizes[bucket - 1]);
        }
    }
    if (!rank_rows.empty() && !bucket_sizes[3]) {
        bucket_sizes[4] = 1;
    }

    char in_query[kInQueryBufferSize];
    memset(in_query, 0, sizeof(in_query));
    if (!loss_above_floor.empty()) {
        db_handle->set_query(
            "upDate pvp_result set pvp_point=pvp_point-%d where charac_no %s",
            ref_pvp_grade->getPvPExpLossExp(),
            getList2inQuery(loss_above_floor, in_query));
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    if (!loss_to_floor.empty()) {
        db_handle->set_query(
            "upDate pvp_result set pvp_point=%d where charac_no %s",
            floor_point,
            getList2inQuery(loss_to_floor, in_query));
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    std::vector<int> grade_charac_nos[kPvPRankBucketCount];
    int bucket = 0;
    int bucket_remaining = bucket_sizes[0];
    for (unsigned int row_index = 0;
         row_index < rank_rows.size();
         ++row_index) {
        if (!bucket_remaining) {
            while (++bucket != kPvPRankBucketCount) {
                bucket_remaining = bucket_sizes[bucket];
                if (bucket_remaining) {
                    break;
                }
            }
            if (bucket == kPvPRankBucketCount) {
                break;
            }
        }
        grade_charac_nos[bucket].push_back(
            static_cast<int>(rank_rows[row_index].charac_no));
        --bucket_remaining;
    }

    const int current_sec = GlobalData::s_systemTime_.getCurSec();
    for (int grade_bucket = 0;
         grade_bucket < kPvPRankBucketCount;
         ++grade_bucket) {
        if (grade_charac_nos[grade_bucket].empty()) {
            continue;
        }
        db_handle->set_query(
            "upDate pvp_result set pvp_grade_ext=%d, pvp_grade_ext_update_time=from_unixtime(%d) where charac_no %s",
            5 - grade_bucket,
            current_sec,
            getList2inQuery(grade_charac_nos[grade_bucket], in_query));
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x84293A0
// IDA_NAME: _ZN17DB_UpdatePvPGrade11makeRequestEv
// DEMANGLED: DB_UpdatePvPGrade::makeRequest()
void DB_UpdatePvPGrade::makeRequest() {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27336);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdatePvPGrade;
    *guard << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
