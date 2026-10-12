#include "old_data/db/private_store/db_save_private_store.h"

#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844E1FA
// IDA_NAME: _ZN19DB_SavePrivateStoreC2Ev
// DEMANGLED: DB_SavePrivateStore::DB_SavePrivateStore(void)
DB_SavePrivateStore::DB_SavePrivateStore() {}

// IDA_EA: 0x845F7DE
// IDA_NAME: _ZN19DB_SavePrivateStoreD2Ev
// DEMANGLED: DB_SavePrivateStore::~DB_SavePrivateStore()
// IDA_EA: 0x845F80E
// IDA_NAME: _ZN19DB_SavePrivateStoreD0Ev
// DEMANGLED: DB_SavePrivateStore::~DB_SavePrivateStore() deleting destructor
DB_SavePrivateStore::~DB_SavePrivateStore() {}

// IDA_EA: 0x841BF02
// IDA_NAME: _ZN19DB_SavePrivateStore8dispatchEiiP6Stream
// DEMANGLED: DB_SavePrivateStore::dispatch(int,int,Stream *)
bool DB_SavePrivateStore::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    private_store::SIG_PRIVATE_STORE* payload =
        stream->GetOutBuffer<private_store::SIG_PRIVATE_STORE>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char* store_blob = db_handle->blob_to_str(
        0,
        payload->item,
        sizeof(payload->item));
    db_handle->set_query(
        "upDate store set store = '%s', use_doll = '%d' where charac_no =%d",
        store_blob,
        payload->use_doll,
        payload->charac_no);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    store_blob = db_handle->blob_to_str(
        0,
        payload->item,
        sizeof(payload->item));
    db_handle->set_query(
        "inSert into store(charac_no, store, use_doll) values(%d, '%s', '%d')",
        payload->charac_no,
        store_blob,
        payload->use_doll);
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace("virtual bool DB_SavePrivateStore::dispatch(int, int, Stream*)",
                   20333,
                   5);
    trace("inSert store failed at %d", payload->charac_no);
    return false;
}
