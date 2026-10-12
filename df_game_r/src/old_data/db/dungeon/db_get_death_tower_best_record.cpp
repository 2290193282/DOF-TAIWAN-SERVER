#include "old_data/db/dungeon/db_get_death_tower_best_record.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>



namespace {

enum {
    kDbSignalGetDeathTowerBestRecord = 226,
    kDeathTowerMaxRecordCount = 5,
};

}  // namespace

// IDA_EA: 0x844E81A
// IDA_NAME: _ZN26DB_GetDeathTowerBestRecordC2Ev
// DEMANGLED: DB_GetDeathTowerBestRecord::DB_GetDeathTowerBestRecord(void)
DB_GetDeathTowerBestRecord::DB_GetDeathTowerBestRecord() {}

// IDA_EA: 0x845E632
// IDA_NAME: _ZN26DB_GetDeathTowerBestRecordD2Ev
// DEMANGLED: DB_GetDeathTowerBestRecord::~DB_GetDeathTowerBestRecord()
// IDA_EA: 0x845E662
// IDA_NAME: _ZN26DB_GetDeathTowerBestRecordD0Ev
// DEMANGLED: DB_GetDeathTowerBestRecord::~DB_GetDeathTowerBestRecord() deleting destructor
DB_GetDeathTowerBestRecord::~DB_GetDeathTowerBestRecord() {}

// IDA_EA: 0x8429AD0
// IDA_NAME: _ZN26DB_GetDeathTowerBestRecord8dispatchEiiP6Stream
// DEMANGLED: DB_GetDeathTowerBestRecord::dispatch(int,int,Stream *)
bool DB_GetDeathTowerBestRecord::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    if (!DBDispatcher::dispatch(signal, user_idx, stream)) {
        return false;
    }

    unsigned int charac_no = 0;
    if (stream->get_binary(&charac_no, sizeof(charac_no)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect tower_index, \t\tmember_info_1,stage_1,play_time_1, \t\tmember_info_2,stage_2,play_time_2, \t\tmember_info_3,"
        "stage_3,play_time_3, \t\tmember_info_4,stage_4,play_time_4 \t\tfrom charac_tower_record where charac_no=%d limit %d",
        charac_no,
        kDeathTowerMaxRecordCount);
    if (db_handle->exec(true) != true || !db_handle->get_n_rows()) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 27570);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalGetDeathTowerBestRecord;
    *guard << user_idx;
    SIG_DEATH_TOWER_GET_BEST_RECORD* payload =
        guard->GetInBuffer<SIG_DEATH_TOWER_GET_BEST_RECORD>();
    if (!payload) {
        return false;
    }

    payload->record_count = db_handle->get_n_rows();
    payload->charac_no = charac_no;

    for (unsigned int row = 0;
         row < static_cast<unsigned int>(db_handle->get_n_rows());
         ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }

        if (db_handle->get_ushort(
                0,
                payload->records[row][0].tower_index) != true) {
            return false;
        }
        if (db_handle->get_binary(
                1,
                payload->records[row][0].member_info,
                sizeof(stDeathTowerRecordMemberInfo_t)) != true) {
            return false;
        }
        if (db_handle->get_uint(
                2,
                payload->records[row][0].stage) != true) {
            return false;
        }
        if (db_handle->get_uint(
                3,
                payload->records[row][0].play_time) != true) {
            return false;
        }
        payload->records[row][0].member_count = 1;

        if (db_handle->get_binary(
                4,
                payload->records[row][1].member_info,
                2 * sizeof(stDeathTowerRecordMemberInfo_t)) != true) {
            return false;
        }
        if (db_handle->get_uint(
                5,
                payload->records[row][1].stage) != true) {
            return false;
        }
        if (db_handle->get_uint(
                6,
                payload->records[row][1].play_time) != true) {
            return false;
        }
        payload->records[row][1].member_count = 2;

        if (db_handle->get_binary(
                7,
                payload->records[row][2].member_info,
                3 * sizeof(stDeathTowerRecordMemberInfo_t)) != true) {
            return false;
        }
        if (db_handle->get_uint(
                8,
                payload->records[row][2].stage) != true) {
            return false;
        }
        if (db_handle->get_uint(
                9,
                payload->records[row][2].play_time) != true) {
            return false;
        }
        payload->records[row][2].member_count = 3;

        if (db_handle->get_binary(
                10,
                payload->records[row][3].member_info,
                4 * sizeof(stDeathTowerRecordMemberInfo_t)) != true) {
            return false;
        }
        if (db_handle->get_uint(
                11,
                payload->records[row][3].stage) != true) {
            return false;
        }
        if (db_handle->get_uint(
                12,
                payload->records[row][3].play_time) != true) {
            return false;
        }
        payload->records[row][3].member_count = 4;
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842A048
// IDA_NAME: _ZN26DB_GetDeathTowerBestRecord11makeRequestEij
// DEMANGLED: DB_GetDeathTowerBestRecord::makeRequest(int,unsigned int)
void DB_GetDeathTowerBestRecord::makeRequest(int user_idx,
                                             unsigned int charac_no) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27616);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalGetDeathTowerBestRecord;
    *guard << user_idx;
    *guard << charac_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
