#include "old_data/db/legacy_dispatch/db_update_nat_type.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/nat_type_payload.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E2A2
// IDA_NAME: _ZN16DB_UpdateNatTypeC2Ev
// DEMANGLED: DB_UpdateNatType::DB_UpdateNatType(void)
DB_UpdateNatType::DB_UpdateNatType() {}

// IDA_EA: 0x845F60A
// IDA_NAME: _ZN16DB_UpdateNatTypeD2Ev
// DEMANGLED: DB_UpdateNatType::~DB_UpdateNatType()
// IDA_EA: 0x845F63A
// IDA_NAME: _ZN16DB_UpdateNatTypeD0Ev
// DEMANGLED: DB_UpdateNatType::~DB_UpdateNatType() deleting destructor
DB_UpdateNatType::~DB_UpdateNatType() {}

// IDA_EA: 0x841C74E
// IDA_NAME: _ZN16DB_UpdateNatType8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateNatType::dispatch(int,int,Stream *)
bool DB_UpdateNatType::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_NAT_TYPE payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "inSert into log_nat_daily(occ_time,nat_type,nat_type_other,success_cnt,fail_cnt) values(from_unixtime(%d),%d,%d,%d,%d)",
        payload.occ_time,
        payload.nat_type,
        payload.nat_type_other,
        payload.success_count,
        payload.fail_count);
    return db_handle->exec(true) == true;
}
