#include "old_data/db/mercenary/db_save_mercenary_data.h"

#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844FA96
// IDA_NAME: _ZN20DB_SaveMercenaryDataC2Ev
// DEMANGLED: DB_SaveMercenaryData::DB_SaveMercenaryData(void)
DB_SaveMercenaryData::DB_SaveMercenaryData() {}

// IDA_EA: 0x845B488
// IDA_NAME: _ZN20DB_SaveMercenaryDataD2Ev
// DEMANGLED: DB_SaveMercenaryData::~DB_SaveMercenaryData()
// IDA_EA: 0x845B4B8
// IDA_NAME: _ZN20DB_SaveMercenaryDataD0Ev
// DEMANGLED: DB_SaveMercenaryData::~DB_SaveMercenaryData() deleting destructor
DB_SaveMercenaryData::~DB_SaveMercenaryData() {}

// IDA_EA: 0x84458C6
// IDA_NAME: _ZN20DB_SaveMercenaryData8dispatchEiiP6Stream
// DEMANGLED: DB_SaveMercenaryData::dispatch(int,int,Stream *)
bool DB_SaveMercenaryData::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_MERCENARY_DB_DATA* payload =
        stream->GetOutBuffer<SIG_MERCENARY_DB_DATA>();
    if (!payload) {
        return false;
    }

    if (!payload || payload->count == 0 || payload->count > 30) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    for (uint32_t index = 0; index < payload->count; ++index) {
        const CHARAC_LOAD_MERCENARY& mercenary = payload->mercenary[index];
        db_handle->set_query(
            "upDate charac_link_bonus set mercenary_start_time=%d,mercenary_finish_time=%d,mercenary_area=%d,mercenary_period=%d where charac_no=%u",
            mercenary.start_time,
            mercenary.end_time,
            static_cast<int>(static_cast<int8_t>(mercenary.competition_area)),
            static_cast<int>(static_cast<int8_t>(mercenary.competition_period)),
            mercenary.charac_no);
        if (db_handle->exec(true) == true) {
            continue;
        }

        // This fallback SQL, including its table and where-clause spelling,
        // is the original DB_SaveMercenaryData failure path.
        db_handle->set_query(
            "inSert into charac_info(mercenary_start_time,mercenary_finish_time,mercenary_area,mercenary_period) values(%d,%d,%d,%d) where charac_no=%u",
            mercenary.start_time,
            mercenary.end_time,
            static_cast<int>(static_cast<int8_t>(mercenary.competition_area)),
            static_cast<int>(static_cast<int8_t>(mercenary.competition_period)),
            mercenary.charac_no);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
