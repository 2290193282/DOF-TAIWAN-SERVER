#include "old_data/db/statistics_log/db_insert_pvp_map.h"

#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <cstdio>
#include <string>

// IDA_EA: 0x844E9BE
// IDA_NAME: _ZN15DB_InsertPVPMapC2Ev
// DEMANGLED: DB_InsertPVPMap::DB_InsertPVPMap(void)
DB_InsertPVPMap::DB_InsertPVPMap() {}

// IDA_EA: 0x845E1A0
// IDA_NAME: _ZN15DB_InsertPVPMapD2Ev
// DEMANGLED: DB_InsertPVPMap::~DB_InsertPVPMap()
// IDA_EA: 0x845E1D0
// IDA_NAME: _ZN15DB_InsertPVPMapD0Ev
// DEMANGLED: DB_InsertPVPMap::~DB_InsertPVPMap() deleting destructor
DB_InsertPVPMap::~DB_InsertPVPMap() {}

// IDA_EA: 0x842CB12
// IDA_NAME: _ZN15DB_InsertPVPMap8dispatchEiiP6Stream
// DEMANGLED: DB_InsertPVPMap::dispatch(int,int,Stream *)
bool DB_InsertPVPMap::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(DB_HANDLE_LOG),
        static_cast<ENUM_SERVER_GROUP>(SERVER_GROUP_DEFAULT));

    int count = 0;
    int map_id = 0;
    int play_count = 0;
    std::string values;
    *stream >> count;
    for (int index = 0; index < count; ++index) {
        *stream >> map_id;
        *stream >> play_count;

        char row[1024];
        std::sprintf(row,
                     "(%d,now(),%d,%d)",
                     G_CEnvironment()->get_channel_no(),
                     map_id,
                     play_count);
        values += row;
        if (count - 1 > index) {
            values += ",";
        }
    }

    db_handle->set_query(
        "inSert into log_pvp_map(channel_no,occ_date,map_id,play_count) values%s",
        values.c_str());
    return db_handle->exec(true) == true;
}
