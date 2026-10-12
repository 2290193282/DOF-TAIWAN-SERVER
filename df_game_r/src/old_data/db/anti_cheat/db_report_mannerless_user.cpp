#include "old_data/db/anti_cheat/db_report_mannerless_user.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "common/manager/system_time.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

const uint32_t kMannerlessLogDbHandle = 4;
const uint32_t kMannerlessCacheQueryLimit = 0x6000;
const uint32_t kMannerlessCacheQueryReserve = 0x273;

}  // namespace

// IDA_EA: 0x844E6AE
// IDA_NAME: _ZN23DB_ReportMannerlessUserC2Ev
// DEMANGLED: DB_ReportMannerlessUser::DB_ReportMannerlessUser(void)
DB_ReportMannerlessUser::DB_ReportMannerlessUser() : DBDispatcher() {}

// IDA_EA: 0x845E9DA
// IDA_NAME: _ZN23DB_ReportMannerlessUserD2Ev
// DEMANGLED: DB_ReportMannerlessUser::~DB_ReportMannerlessUser()
// IDA_EA: 0x845EA0A
// IDA_NAME: _ZN23DB_ReportMannerlessUserD0Ev
// DEMANGLED: DB_ReportMannerlessUser::~DB_ReportMannerlessUser()
DB_ReportMannerlessUser::~DB_ReportMannerlessUser() {}

// IDA_EA: 0x842796A
// IDA_NAME: _ZN23DB_ReportMannerlessUser8dispatchEiiP6Stream
// DEMANGLED: DB_ReportMannerlessUser::dispatch(int,int,Stream *)
bool DB_ReportMannerlessUser::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    // IDA 0x842798E obtains (4,0) before consuming the stream, even though
    // this accumulator-only leaf does not issue the resulting handle here.
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kMannerlessLogDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));
    (void)db_handle;

    // No null branch exists after the old GetOutBuffer call.  The exact typed
    // specialization fails internally only before it returns the record.
    SIG_MANNERLESS_USER* const record =
        stream->GetOutBuffer<SIG_MANNERLESS_USER>();

    static std::string cache_query;
    if (!cache_query.empty()) {
        cache_query += ",";
    }
    cache_query += "(";
    cache_query += record->sql_fragment_00;
    cache_query += ",";
    cache_query += record->sql_fragment_1d;
    cache_query += ",";
    cache_query += record->sql_fragment_3a;
    cache_query += ",";

    char occurrence_time[256];
    std::memset(occurrence_time, 0, sizeof(occurrence_time));
    std::sprintf(occurrence_time,
                 "from_unixtime(%d)",
                 GlobalData::s_systemTime_.getCurSec());
    cache_query += occurrence_time;
    cache_query += ")";

    // IDA 0x8427B94..0x8427BB9 tests unsigned (0x6000 - size) <= 0x273.
    if (static_cast<uint32_t>(kMannerlessCacheQueryLimit -
                               cache_query.size()) <=
        kMannerlessCacheQueryReserve) {
        cache_query.clear();
    }
    return true;
}
