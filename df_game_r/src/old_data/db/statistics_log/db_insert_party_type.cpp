#include "old_data/db/statistics_log/db_insert_party_type.h"

#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E986
// IDA_NAME: _ZN18DB_InsertPartyTypeC2Ev
// DEMANGLED: DB_InsertPartyType::DB_InsertPartyType(void)
DB_InsertPartyType::DB_InsertPartyType() {}

// IDA_EA: 0x845E23C
// IDA_NAME: _ZN18DB_InsertPartyTypeD2Ev
// DEMANGLED: DB_InsertPartyType::~DB_InsertPartyType()
// IDA_EA: 0x845E26C
// IDA_NAME: _ZN18DB_InsertPartyTypeD0Ev
// DEMANGLED: DB_InsertPartyType::~DB_InsertPartyType() deleting destructor
DB_InsertPartyType::~DB_InsertPartyType() {}

// IDA_EA: 0x842C906
// IDA_NAME: _ZN18DB_InsertPartyType8dispatchEiiP6Stream
// DEMANGLED: DB_InsertPartyType::dispatch(int,int,Stream *)
bool DB_InsertPartyType::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(DB_HANDLE_LOG),
        static_cast<ENUM_SERVER_GROUP>(SERVER_GROUP_DEFAULT));

    int guild_count = 0;
    int member_count = 0;
    int helped_count = 0;
    int normal_count = 0;
    int member_helped_count = 0;
    *stream >> guild_count;
    *stream >> member_count;
    *stream >> helped_count;
    *stream >> normal_count;
    *stream >> member_helped_count;

    const int channel_no = G_CEnvironment()->get_channel_no();
    db_handle->set_query(
        "inSert into log_party_type(channel_no,occ_date,guild_count,member_count,helped_count,normal_count,member_helped_count) values(%d,now(),%d,%d,%d,%d,%d)",
        channel_no,
        guild_count,
        member_count,
        helped_count,
        normal_count,
        member_helped_count);
    return db_handle->exec(true) == true;
}
