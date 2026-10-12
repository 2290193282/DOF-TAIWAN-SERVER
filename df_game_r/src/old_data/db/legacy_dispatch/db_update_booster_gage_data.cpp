#include "old_data/db/character/db_update_booster_gage_data.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/character/load_etc_payload.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844FB76
// IDA_NAME: _ZN24DB_UpdateBoosterGageDataC2Ev
// DEMANGLED: DB_UpdateBoosterGageData::DB_UpdateBoosterGageData(void)
DB_UpdateBoosterGageData::DB_UpdateBoosterGageData() {}

// IDA_EA: 0x845B218
// IDA_NAME: _ZN24DB_UpdateBoosterGageDataD2Ev
// DEMANGLED: DB_UpdateBoosterGageData::~DB_UpdateBoosterGageData()
// IDA_EA: 0x845B248
// IDA_NAME: _ZN24DB_UpdateBoosterGageDataD0Ev
// DEMANGLED: DB_UpdateBoosterGageData::~DB_UpdateBoosterGageData() deleting destructor
DB_UpdateBoosterGageData::~DB_UpdateBoosterGageData() {}

// IDA_EA: 0x8446EEC
// IDA_NAME: _ZN24DB_UpdateBoosterGageData8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBoosterGageData::dispatch(int,int,Stream *)
bool DB_UpdateBoosterGageData::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_BOOSTER_GAGE_DATA* payload =
        stream->GetOutBuffer<SIG_BOOSTER_GAGE_DATA>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate member_booster_gage set gage=%u where m_id=%u",
        payload->gage,
        payload->account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }

    if (db_handle->getAffectedRowCount() == 0) {
        db_handle->set_query(
            "inSert into member_booster_gage(m_id, gage) values(%u, %u)",
            payload->account_id,
            payload->gage);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    return true;
}
