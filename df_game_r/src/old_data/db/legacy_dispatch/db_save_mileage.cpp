#include "old_data/db/auction_cashshop/db_save_mileage.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/auction_cashshop/db_mileage.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844F45A
// IDA_NAME: _ZN14DB_SaveMileageC2Ev
// DEMANGLED: DB_SaveMileage::DB_SaveMileage(void)
DB_SaveMileage::DB_SaveMileage() {}

// IDA_EA: 0x845B7E2
// IDA_NAME: _ZN14DB_SaveMileageD2Ev
// DEMANGLED: DB_SaveMileage::~DB_SaveMileage()
// IDA_EA: 0x845B812
// IDA_NAME: _ZN14DB_SaveMileageD0Ev
// DEMANGLED: DB_SaveMileage::~DB_SaveMileage() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveMileage::~DB_SaveMileage() {}

// IDA_EA: 0x8443C62
// IDA_NAME: _ZN14DB_SaveMileage8dispatchEiiP6Stream
// DEMANGLED: DB_SaveMileage::dispatch(int,int,Stream *)
bool DB_SaveMileage::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_MILEAGE_STRUCT mileage;
    if (stream->get_binary(&mileage, sizeof(mileage)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        cMyTrace trace("virtual bool DB_SaveMileage::dispatch(int, int, Stream*)",
                       42906,
                       5);
        trace("[ERROR] Database Handle is NULL [index:%d]", 1);
        return false;
    }

    db_handle->set_query("upDate member_miles set miles = %d where m_id = %u",
                         mileage.miles,
                         mileage.m_id);
    return db_handle->exec(true) == true;
}
