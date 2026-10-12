#include "old_data/db/dungeon/db_load_blood_best_record.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalBloodBestRecord = 319,
    kBloodBestRecordDbHandle = 2,
};

}  // namespace

// IDA_EA: 0x844ECEA
// IDA_NAME: _ZN22DB_LoadBloodBestRecordC2Ev
// DEMANGLED: DB_LoadBloodBestRecord::DB_LoadBloodBestRecord(void)
DB_LoadBloodBestRecord::DB_LoadBloodBestRecord() {}

// IDA_EA: 0x845DA02
// IDA_NAME: _ZN22DB_LoadBloodBestRecordD2Ev
// DEMANGLED: DB_LoadBloodBestRecord::~DB_LoadBloodBestRecord()
// IDA_EA: 0x845DA32
// IDA_NAME: _ZN22DB_LoadBloodBestRecordD0Ev
// DEMANGLED: DB_LoadBloodBestRecord::~DB_LoadBloodBestRecord() deleting destructor
DB_LoadBloodBestRecord::~DB_LoadBloodBestRecord() {}

// IDA_EA: 0x84318C4
// IDA_NAME: _ZN22DB_LoadBloodBestRecord11makeRequestEij
// DEMANGLED: DB_LoadBloodBestRecord::makeRequest(int,unsigned int)
void DB_LoadBloodBestRecord::makeRequest(int user_idx,
                                         unsigned int charac_no) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 30918);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalBloodBestRecord;
    *guard << user_idx;
    *guard << charac_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843199A
// IDA_NAME: _ZN22DB_LoadBloodBestRecord8dispatchEiiP6Stream
// DEMANGLED: DB_LoadBloodBestRecord::dispatch(int,int,Stream *)
bool DB_LoadBloodBestRecord::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    unsigned int charac_no = 0;
    *stream >> charac_no;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kBloodBestRecordDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect best_round, best_time, dungeon_index from charac_blood_best_record where charac_no=%u",
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->fetch() != true || db_handle->get_n_rows() == 0) {
        cMyTrace trace(
            "virtual bool DB_LoadBloodBestRecord::dispatch(int, int, Stream*)",
            30948,
            0);
        trace("Not exist data charac_blood_best_record. [charac_no:%d]",
              charac_no);
        return true;
    }

    const unsigned int row_count = db_handle->get_n_rows();
    for (unsigned int index = 0; index < row_count; ++index) {
        SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD result;
        result.charac_no = charac_no;
        if (db_handle->get_int(0, result.best_round) != true ||
            db_handle->get_uint(1, result.best_time) != true ||
            db_handle->get_int(2, result.dungeon_index) != true) {
            return false;
        }

        Stream* result_stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 30962);
        CStreamGuard guard(result_stream, true);
        *guard << kDbSignalBloodBestRecord;
        *guard << user_idx;
        SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD* result_payload =
            guard->GetInBuffer<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD>();
        *result_payload = result;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return true;
}
