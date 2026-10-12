#include "old_data/db/legacy_dispatch/db_update_nat_type_user.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

#pragma pack(push, 1)
struct SIG_NAT_TYPE_USER_LOCAL {
    uint32_t nat_type;   // +0
    uint32_t nat_count;  // +4
    uint32_t occ_time;   // +8
};
#pragma pack(pop)

typedef char SIG_NAT_TYPE_USER_LOCAL_Size_Must_Be_12[
    (sizeof(SIG_NAT_TYPE_USER_LOCAL) == 12) ? 1 : -1];

// IDA_EA: 0x844E2BE
// IDA_NAME: _ZN20DB_UpdateNatTypeUserC2Ev
// DEMANGLED: DB_UpdateNatTypeUser::DB_UpdateNatTypeUser(void)
DB_UpdateNatTypeUser::DB_UpdateNatTypeUser() {}

// IDA_EA: 0x845F5BC
// IDA_NAME: _ZN20DB_UpdateNatTypeUserD2Ev
// DEMANGLED: DB_UpdateNatTypeUser::~DB_UpdateNatTypeUser()
// IDA_EA: 0x845F5EC
// IDA_NAME: _ZN20DB_UpdateNatTypeUserD0Ev
// DEMANGLED: DB_UpdateNatTypeUser::~DB_UpdateNatTypeUser() deleting destructor
DB_UpdateNatTypeUser::~DB_UpdateNatTypeUser() {}

// IDA_EA: 0x841C7F6
// IDA_NAME: _ZN20DB_UpdateNatTypeUser8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateNatTypeUser::dispatch(int,int,Stream *)
bool DB_UpdateNatTypeUser::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_NAT_TYPE_USER_LOCAL payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "inSert into log_nat_stat(occ_time,nat_type,nat_cnt) values(from_unixtime(%d),%d,%d)",
        payload.occ_time,
        payload.nat_type,
        payload.nat_count);
    return db_handle->exec(true) == true;
}
