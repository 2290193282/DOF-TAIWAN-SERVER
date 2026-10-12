#include "old_data/db/anti_cheat/db_req_black_count.h"

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

const char kDispatchFunctionName[] =
    "virtual bool DB_ReqBlackCount::dispatch(int, int, Stream*)";

const int kBlackCountDbHandle = 3;
const int kBlackCountResponseSignal = 179;
const int kSecondsPerDay = 86400;

}  // namespace

// IDA_EA: 0x844E542
// IDA_NAME: _ZN16DB_ReqBlackCountC2Ev
// DEMANGLED: DB_ReqBlackCount::DB_ReqBlackCount(void)
DB_ReqBlackCount::DB_ReqBlackCount() : DBDispatcher() {}

// IDA_EA: 0x845EE1E
// IDA_NAME: _ZN16DB_ReqBlackCountD2Ev
// DEMANGLED: DB_ReqBlackCount::~DB_ReqBlackCount()
// IDA_EA: 0x845EE4E
// IDA_NAME: _ZN16DB_ReqBlackCountD0Ev
// DEMANGLED: DB_ReqBlackCount::~DB_ReqBlackCount() deleting destructor
DB_ReqBlackCount::~DB_ReqBlackCount() {}

// IDA_EA: 0x8424CE6
// IDA_NAME: _ZN16DB_ReqBlackCount8dispatchEiiP6Stream
// DEMANGLED: DB_ReqBlackCount::dispatch(int,int,Stream *)
bool DB_ReqBlackCount::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    // IDA @0x8424CF8 calls the typed out-buffer accessor.  It consumes the
    // request at Stream::out_ptr(), not Stream::in_ptr().
    SIG_REQ_BLACK_COUNT* request =
        stream->GetOutBuffer<SIG_REQ_BLACK_COUNT>();
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kBlackCountDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));

    int result = 0;
    db_handle->set_query(
        "seLect black_point,offset_point,unix_timestamp(problem_child_time) "
        "from charac_black_info where charac_no=%d",
        request->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    if (row_count <= 1) {
        result = (row_count == 0);
    } else {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            kDispatchFunctionName,
            0x622C,
            "DB_ReqBlackCount::dispatch error! data(%d)",
            row_count);
    }

    if (db_handle->fetch() != true) {
        result = 2;
    }

    int32_t black_point = 0;
    int32_t offset_point = 0;
    uint32_t problem_child_time = 0;
    if (db_handle->get_int(0, black_point) != true) {
        result = 3;
    }
    if (db_handle->get_int(1, offset_point) != true) {
        result = 3;
    }
    if (db_handle->get_uint(2, problem_child_time) != true) {
        result = 3;
    }

    if (problem_child_time != 0) {
        // IDA @0x8424E73/@0x8424F06 reads CDataManager+0x3B4/+0x3B8.
        // CDataManager::C2 proves those are the two named values in its
        // embedded ServerParameterScript (+0x68 + +0x34C/+0x350).
        const ServerParameterScript* parameters =
            G_CDataManager()->get_server_parameter_script();
        const int32_t period_seconds =
            kSecondsPerDay * parameters->getBlackListHelpPeriodDays();

        if (period_seconds != 0 &&
            GlobalData::s_systemTime_.getCurSec() >
                static_cast<int32_t>(problem_child_time +
                                     static_cast<uint32_t>(period_seconds))) {
            const uint32_t elapsed_periods =
                (static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec()) -
                 problem_child_time) /
                static_cast<uint32_t>(period_seconds);
            const int32_t black_count_before_adjustment =
                black_point - offset_point;

            if (black_count_before_adjustment > 0) {
                const int32_t points_to_offset = static_cast<int32_t>(
                    elapsed_periods * static_cast<uint32_t>(
                                          parameters
                                              ->getBlackListHelpOffsetPointsPerPeriod()));
                const int32_t computed_black_count =
                    black_count_before_adjustment - points_to_offset;
                cMyTrace trace(kDispatchFunctionName, 0x6259, 0);
                trace(
                    "BlackList Modify : c(%d) iPassCnt(%d) iOffsetPoint(%d) "
                    "iBlackPoint(%d) uProblemChildTime(%d) "
                    "iComputedBlackPoint(%d)",
                    request->charac_no,
                    elapsed_periods,
                    offset_point,
                    black_point,
                    problem_child_time,
                    computed_black_count);

                if (computed_black_count <= 0) {
                    db_handle->set_query(
                        "upDate charac_black_info set offset_point = "
                        "black_point , problem_child_time = 0 where "
                        "charac_no=%d",
                        request->charac_no);
                    offset_point = black_point;
                    problem_child_time = 0;
                    cMyTrace reset_trace(kDispatchFunctionName, 0x6269, 0);
                    reset_trace(
                        "BlackList Modify : c(%d) iPassCnt(%d) "
                        "iOffsetPoint(%d) iBlackPoint(%d) "
                        "uProblemChildTime(%d)",
                        request->charac_no,
                        elapsed_periods,
                        offset_point,
                        black_point,
                        problem_child_time);
                } else {
                    const int32_t update_points_to_offset =
                        static_cast<int32_t>(
                            elapsed_periods * static_cast<uint32_t>(
                                                   parameters
                                                       ->getBlackListHelpOffsetPointsPerPeriod()));
                    const uint32_t advanced_problem_child_time =
                        elapsed_periods *
                            static_cast<uint32_t>(period_seconds) +
                        problem_child_time;
                    db_handle->set_query(
                        "upDate charac_black_info set offset_point = "
                        "offset_point  + %d, problem_child_time = "
                        "from_unixtime(%d) where charac_no=%d",
                        update_points_to_offset,
                        advanced_problem_child_time,
                        request->charac_no);
                    offset_point += static_cast<int32_t>(
                        elapsed_periods * static_cast<uint32_t>(
                                              parameters
                                                  ->getBlackListHelpOffsetPointsPerPeriod()));
                    cMyTrace offset_trace(kDispatchFunctionName, 0x6262, 0);
                    offset_trace(
                        "BlackList Modify : c(%d) iPassCnt(%d) "
                        "iOffsetPoint(%d) iBlackPoint(%d) "
                        "uProblemChildTime(%d)",
                        request->charac_no,
                        elapsed_periods,
                        offset_point,
                        black_point,
                        problem_child_time);
                }

                if (db_handle->exec(true) != true) {
                    LogManager::logFormat(
                        1,
                        "DBThread.cpp",
                        kDispatchFunctionName,
                        0x626C,
                        "BlackList Modify db exec error: iPassCnt(%d) "
                        "iOffsetPoint(%d) iBlackPoint(%d) "
                        "uProblemChildTime(%d)",
                        elapsed_periods,
                        offset_point,
                        black_point,
                        problem_child_time);
                }
            }
        }
    }

    Stream* reply_stream = GlobalData::s_stream_pool->Acquire(
        "DBThread.cpp", 0x6271);
    CStreamGuard reply(reply_stream, true);
    *reply << kBlackCountResponseSignal;
    *reply << user_idx;
    *reply << static_cast<int32_t>(request->charac_no);
    *reply << result;

    const int32_t active_black_points = black_point - offset_point;
    *reply << (active_black_points < 0 ? 0 : active_black_points);
    *reply << problem_child_time;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}
