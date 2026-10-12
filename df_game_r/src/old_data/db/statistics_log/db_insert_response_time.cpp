#include "old_data/db/statistics_log/db_insert_response_time.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <cstdio>
#include <ctime>
#include <cstring>
#include <string>



// IDA_EA: 0x844F732
// IDA_NAME: _ZN21DB_InsertResponseTimeC2Ev
// DEMANGLED: DB_InsertResponseTime::DB_InsertResponseTime(void)
DB_InsertResponseTime::DB_InsertResponseTime() {}

// IDA_EA: 0x845BD5E
// IDA_NAME: _ZN21DB_InsertResponseTimeD2Ev
// DEMANGLED: DB_InsertResponseTime::~DB_InsertResponseTime()
// IDA_EA: 0x845BD8E
// IDA_NAME: _ZN21DB_InsertResponseTimeD0Ev
// DEMANGLED: DB_InsertResponseTime::~DB_InsertResponseTime() deleting destructor
DB_InsertResponseTime::~DB_InsertResponseTime() {}

// IDA_EA: 0x843FC26
// IDA_NAME: _ZN21DB_InsertResponseTime8dispatchEiiP6Stream
// DEMANGLED: DB_InsertResponseTime::dispatch(int,int,Stream *)
bool DB_InsertResponseTime::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_INSERT_RESPONSE_TIME* payload =
        stream->GetOutBuffer<SIG_INSERT_RESPONSE_TIME>();
    std::string values;
    char row[256];
    std::memset(row, 0, sizeof(row));

    for (int index = 0; index <= 99 && payload->entry_count > index; ++index) {
        if (!values.empty()) {
            values += ",";
        }

        const SIG_RESPONSE_TIME_ENTRY& entry = payload->entries[index];
        const unsigned long long total_response_time =
            (static_cast<unsigned long long>(entry.total_response_time_high) <<
             32) |
            static_cast<unsigned long long>(entry.total_response_time_low);
        CEnvironment* environment = G_CEnvironment();
        const int channel_no = environment->get_channel_no();

        std::sprintf(row,
                     "(%d,from_unixtime(%d),%u,%u,%llu,%u)",
                     channel_no,
                     payload->occ_time,
                     entry.packet_id,
                     entry.packet_count,
                     total_response_time,
                     entry.avg_response_time);
        values += row;
    }

    char month[10];
    std::memset(month, 0, sizeof(month));
    time_t timer = static_cast<time_t>(payload->occ_time);
    tm local_time;
    localtime_r(&timer, &local_time);
    std::strftime(month, sizeof(month), "%Y%m", &local_time);

    db_handle->set_query(
        "inSert into log_response_time_%s(channel_no,occ_time,packet_id,packet_count,total_response_time,avg_response_time) "
        "values%s",
        month,
        values.c_str());
    return db_handle->exec(true) == true;
}
