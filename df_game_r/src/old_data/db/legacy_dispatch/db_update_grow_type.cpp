#include "old_data/db/character/db_update_grow_type.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

typedef char SIG_UPDATE_GROWTYPE_Size_Must_Be_5[
    (sizeof(SIG_UPDATE_GROWTYPE) == 5) ? 1 : -1];

// IDA_EA: 0x8418130
// IDA_NAME: _ZN17DB_UpdateGrowTypeC2Ev
// DEMANGLED: DB_UpdateGrowType::DB_UpdateGrowType(void)
DB_UpdateGrowType::DB_UpdateGrowType() {}

// IDA_EA: 0x841814C
// IDA_NAME: _ZN17DB_UpdateGrowTypeD2Ev
// DEMANGLED: DB_UpdateGrowType::~DB_UpdateGrowType()
// IDA_EA: 0x841817C
// IDA_NAME: _ZN17DB_UpdateGrowTypeD0Ev
// DEMANGLED: DB_UpdateGrowType::~DB_UpdateGrowType() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateGrowType::~DB_UpdateGrowType() {}

// IDA_EA: 0x841819A
// IDA_NAME: _ZN17DB_UpdateGrowType8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateGrowType::dispatch(int,int,Stream *)
bool DB_UpdateGrowType::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    const int bytes = sizeof(SIG_UPDATE_GROWTYPE);
    if (!stream->enable_out(bytes)) {
        return false;
    }
    SIG_UPDATE_GROWTYPE* payload =
        reinterpret_cast<SIG_UPDATE_GROWTYPE*>(stream->get_buffer());
    stream->out_ptr(bytes);
    return UpdateGrowType(payload) == true;
}

// IDA_EA: 0x84181EA
// IDA_NAME: _ZN17DB_UpdateGrowType14UpdateGrowTypeEP19SIG_UPDATE_GROWTYPE
// DEMANGLED: DB_UpdateGrowType::UpdateGrowType(SIG_UPDATE_GROWTYPE *)
bool DB_UpdateGrowType::UpdateGrowType(SIG_UPDATE_GROWTYPE* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate charac_info set grow_type=%d where charac_no=%u",
        payload->grow_type,
        payload->charac_no);
    return db_handle->exec(true) == true;
}
