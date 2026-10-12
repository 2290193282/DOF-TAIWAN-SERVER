#include "old_data/db/statistics_log/db_insert_queue_size_log.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844E446
// IDA_NAME: _ZN21DB_InsertQueueSizeLogC2Ev
// DEMANGLED: DB_InsertQueueSizeLog::DB_InsertQueueSizeLog(void)
DB_InsertQueueSizeLog::DB_InsertQueueSizeLog() {}

// IDA_EA: 0x845F12A
// IDA_NAME: _ZN21DB_InsertQueueSizeLogD2Ev
// DEMANGLED: DB_InsertQueueSizeLog::~DB_InsertQueueSizeLog()
// IDA_EA: 0x845F15A
// IDA_NAME: _ZN21DB_InsertQueueSizeLogD0Ev
// DEMANGLED: DB_InsertQueueSizeLog::~DB_InsertQueueSizeLog() deleting destructor
DB_InsertQueueSizeLog::~DB_InsertQueueSizeLog() {}

// IDA_EA: 0x84225A8
// IDA_NAME: _ZN21DB_InsertQueueSizeLog8dispatchEiiP6Stream
// DEMANGLED: DB_InsertQueueSizeLog::dispatch(int,int,Stream *)
bool DB_InsertQueueSizeLog::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    int dbq = 0;
    int channel_no = 0;
    stream->operator>>(dbq);
    stream->operator>>(channel_no);

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into dnf_dbq_log(occ_time,channel_no,dbq) values(NOW(),%d,%d)",
        channel_no,
        dbq);
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace("virtual bool DB_InsertQueueSizeLog::dispatch(int, int, Stream*)",
                   23256,
                   5);
    trace("DB_InsertQueueSizeLog, exec ERROR");
    return false;
}
