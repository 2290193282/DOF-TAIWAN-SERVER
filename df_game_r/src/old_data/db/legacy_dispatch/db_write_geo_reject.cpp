#include "old_data/db/auth_security/db_write_geo_reject.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/legacy_dispatch/geo_reject_payload.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844EB2A
// IDA_NAME: _ZN17DB_WriteGeoRejectC2Ev
// DEMANGLED: DB_WriteGeoReject::DB_WriteGeoReject(void)
DB_WriteGeoReject::DB_WriteGeoReject() {}

// IDA_EA: 0x845AB16
// IDA_NAME: _ZN17DB_WriteGeoRejectD2Ev
// DEMANGLED: DB_WriteGeoReject::~DB_WriteGeoReject()
// IDA_EA: 0x845AB46
// IDA_NAME: _ZN17DB_WriteGeoRejectD0Ev
// DEMANGLED: DB_WriteGeoReject::~DB_WriteGeoReject() deleting destructor
DB_WriteGeoReject::~DB_WriteGeoReject() {}

// IDA_EA: 0x844BB70
// IDA_NAME: _ZN17DB_WriteGeoReject8dispatchEiiP6Stream
// DEMANGLED: DB_WriteGeoReject::dispatch(int,int,Stream *)
bool DB_WriteGeoReject::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1), static_cast<ENUM_SERVER_GROUP>(0));
    const SIG_SAVE_GEO_REJECT* const payload =
        stream->GetOutBuffer<SIG_SAVE_GEO_REJECT>();
    db_handle->set_query(
        "upDate geo_reject set rej_ip_count = rej_ip_count + 1, rej_src = 'g' where rej_ip = '%s'",
        payload->reject_ip);
    if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into geo_reject(rej_ip, rej_c_code, rej_ip_count, rej_src) values('%s', '%s', 1, 'g')",
            payload->reject_ip,
            payload->country_code);
        db_handle->exec(true);
    }
    return true;
}
