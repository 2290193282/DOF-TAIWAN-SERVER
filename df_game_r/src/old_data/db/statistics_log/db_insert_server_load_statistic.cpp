#include "old_data/db/statistics_log/db_insert_server_load_statistic.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844EFFA
// IDA_NAME: _ZN28DB_InsertServerLoadStatisticC2Ev
// DEMANGLED: DB_InsertServerLoadStatistic::DB_InsertServerLoadStatistic(void)
DB_InsertServerLoadStatistic::DB_InsertServerLoadStatistic() {}

// IDA_EA: 0x845D2B2
// IDA_NAME: _ZN28DB_InsertServerLoadStatisticD2Ev
// DEMANGLED: DB_InsertServerLoadStatistic::~DB_InsertServerLoadStatistic()
// IDA_EA: 0x845D2E2
// IDA_NAME: _ZN28DB_InsertServerLoadStatisticD0Ev
// DEMANGLED: DB_InsertServerLoadStatistic::~DB_InsertServerLoadStatistic() deleting destructor
DB_InsertServerLoadStatistic::~DB_InsertServerLoadStatistic() {}

// IDA_EA: 0x8436AD4
// IDA_NAME: _ZN28DB_InsertServerLoadStatistic8dispatchEiiP6Stream
// DEMANGLED: DB_InsertServerLoadStatistic::dispatch(int,int,Stream *)
bool DB_InsertServerLoadStatistic::dispatch(int signal, int user_idx,
                                            Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_INSERT_SERVER_LOAD_STATISTIC* payload =
        stream->GetOutBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>();
    db_handle->set_query(
        "inSert into log_gameserver_load_stat set occ_time=now(), server_id=%d, channel_no=%d, dbq_cnt=%d, logdbq_cnt=%d, net"
        "q_cnt=%d, packetq_cnt=%d",
        payload->server_id,
        payload->channel_no,
        payload->dbq_cnt,
        payload->logdbq_cnt,
        payload->netq_cnt,
        payload->packetq_cnt);
    return db_handle->exec(true) == true;
}
