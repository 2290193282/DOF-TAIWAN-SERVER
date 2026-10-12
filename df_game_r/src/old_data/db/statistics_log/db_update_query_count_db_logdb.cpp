#include "old_data/db/statistics_log/db_update_query_count_db_logdb.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E286
// IDA_NAME: _ZN26DB_UpdateQueryCountDbLogdbC2Ev
// DEMANGLED: DB_UpdateQueryCountDbLogdb::DB_UpdateQueryCountDbLogdb(void)
DB_UpdateQueryCountDbLogdb::DB_UpdateQueryCountDbLogdb() {}

// IDA_EA: 0x845F658
// IDA_NAME: _ZN26DB_UpdateQueryCountDbLogdbD2Ev
// DEMANGLED: DB_UpdateQueryCountDbLogdb::~DB_UpdateQueryCountDbLogdb()
// IDA_EA: 0x845F688
// IDA_NAME: _ZN26DB_UpdateQueryCountDbLogdbD0Ev
// DEMANGLED: DB_UpdateQueryCountDbLogdb::~DB_UpdateQueryCountDbLogdb() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateQueryCountDbLogdb::~DB_UpdateQueryCountDbLogdb() {}

// IDA_EA: 0x841C61A
// IDA_NAME: _ZN26DB_UpdateQueryCountDbLogdb8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateQueryCountDbLogdb::dispatch(int,int,Stream *)
bool DB_UpdateQueryCountDbLogdb::dispatch(int signal, int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        DB_HANDLE_LOG),static_cast<ENUM_SERVER_GROUP>( SERVER_GROUP_DEFAULT));
    int occ_time = 0;
    int query_count = 0;
    *stream >> occ_time;
    *stream >> query_count;

    for (int index = 0; index < query_count; ++index) {
        int query_id = 0;
        int total = 0;
        int response_time = 0;
        *stream >> query_id;
        *stream >> total;
        *stream >> response_time;

        CEnvironment* environment = G_CEnvironment();
        const int channel_no = environment->get_channel_no();
        db_handle->set_query(
            "inSert into log_query_stat(occ_time,q_id,total,response_time,gc_no) values(from_unixtime(%d),%d,%d,%d,%d)",
            occ_time,
            query_id,
            total,
            response_time,
            channel_no);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
