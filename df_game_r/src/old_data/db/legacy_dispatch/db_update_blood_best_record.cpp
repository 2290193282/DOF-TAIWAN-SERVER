#include "old_data/db/dungeon/db_update_blood_best_record.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

namespace {

enum {
    kBloodBestRecordDbHandle = 2,
};

}  // namespace

// IDA_EA: 0x844ED06
// IDA_NAME: _ZN24DB_UpdateBloodBestRecordC2Ev
// DEMANGLED: DB_UpdateBloodBestRecord::DB_UpdateBloodBestRecord(void)
DB_UpdateBloodBestRecord::DB_UpdateBloodBestRecord() {}

// IDA_EA: 0x845D9B4
// IDA_NAME: _ZN24DB_UpdateBloodBestRecordD2Ev
// DEMANGLED: DB_UpdateBloodBestRecord::~DB_UpdateBloodBestRecord()
// IDA_EA: 0x845D9E4
// IDA_NAME: _ZN24DB_UpdateBloodBestRecordD0Ev
// DEMANGLED: DB_UpdateBloodBestRecord::~DB_UpdateBloodBestRecord() deleting destructor
DB_UpdateBloodBestRecord::~DB_UpdateBloodBestRecord() {}

// IDA_EA: 0x8431C74
// IDA_NAME: _ZN24DB_UpdateBloodBestRecord8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateBloodBestRecord::dispatch(int,int,Stream *)
bool DB_UpdateBloodBestRecord::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* payload =
        stream->GetOutBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>();
    if (payload == 0) {
        return false;
    }

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(
            static_cast<ENUM_DB_HANDLE_IDX>(kBloodBestRecordDbHandle),
            static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate charac_blood_best_record set best_round=%d, best_time=%u where charac_no=%u and dungeon_index=%d",
        payload->best_round,
        payload->best_time,
        payload->charac_no,
        payload->dungeon_index);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_blood_best_record(charac_no,dungeon_index,best_round,best_time) values(%u,%d,%d,%u)",
        payload->charac_no,
        payload->dungeon_index,
        payload->best_round,
        payload->best_time);
    return db_handle->exec(true) == true;
}
