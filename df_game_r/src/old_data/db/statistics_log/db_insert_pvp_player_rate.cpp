#include "old_data/db/statistics_log/db_insert_pvp_player_rate.h"

#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E9A2
// IDA_NAME: _ZN22DB_InsertPVPPlayerRateC2Ev
// DEMANGLED: DB_InsertPVPPlayerRate::DB_InsertPVPPlayerRate(void)
DB_InsertPVPPlayerRate::DB_InsertPVPPlayerRate() {}

// IDA_EA: 0x845E1EE
// IDA_NAME: _ZN22DB_InsertPVPPlayerRateD2Ev
// DEMANGLED: DB_InsertPVPPlayerRate::~DB_InsertPVPPlayerRate()
// IDA_EA: 0x845E21E
// IDA_NAME: _ZN22DB_InsertPVPPlayerRateD0Ev
// DEMANGLED: DB_InsertPVPPlayerRate::~DB_InsertPVPPlayerRate() deleting destructor
DB_InsertPVPPlayerRate::~DB_InsertPVPPlayerRate() {}

// IDA_EA: 0x842CA06
// IDA_NAME: _ZN22DB_InsertPVPPlayerRate8dispatchEiiP6Stream
// DEMANGLED: DB_InsertPVPPlayerRate::dispatch(int,int,Stream *)
bool DB_InsertPVPPlayerRate::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(DB_HANDLE_LOG),
        static_cast<ENUM_SERVER_GROUP>(SERVER_GROUP_DEFAULT));

    int player_rate[6];
    for (unsigned int index = 0; index < 6; ++index) {
        player_rate[index] = 0;
    }
    for (int index = 0; index <= 5; ++index) {
        *stream >> player_rate[index];
    }

    const int channel_no = G_CEnvironment()->get_channel_no();
    db_handle->set_query(
        "inSert into log_pvp_player_rate(channel_no,occ_date,single_one_to_one,single_one_to_one_other,team_one_to_one,team_one_to_one_other,relay_one_to_one,relay_one_to_one_other) values(%d,now(),%d,%d,%d,%d,%d,%d)",
        channel_no,
        player_rate[0],
        player_rate[1],
        player_rate[2],
        player_rate[3],
        player_rate[4],
        player_rate[5]);
    return db_handle->exec(true) == true;
}
