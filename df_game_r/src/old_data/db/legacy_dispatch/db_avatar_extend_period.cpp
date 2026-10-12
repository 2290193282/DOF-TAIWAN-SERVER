#include "old_data/db/avatar_aura/db_avatar_extend_period.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x83FFAB8
// IDA_NAME: _ZN21DB_AvatarExtendPeriodC2Ev
// DEMANGLED: DB_AvatarExtendPeriod::DB_AvatarExtendPeriod(void)
DB_AvatarExtendPeriod::DB_AvatarExtendPeriod() {}

// IDA_EA: 0x83FFAD4
// IDA_NAME: _ZN21DB_AvatarExtendPeriodD2Ev
// DEMANGLED: DB_AvatarExtendPeriod::~DB_AvatarExtendPeriod()
// IDA_EA: 0x83FFB04
// IDA_NAME: _ZN21DB_AvatarExtendPeriodD0Ev
// DEMANGLED: DB_AvatarExtendPeriod::~DB_AvatarExtendPeriod() deleting destructor
DB_AvatarExtendPeriod::~DB_AvatarExtendPeriod() {}

// IDA_EA: 0x83FFB22
// IDA_NAME: _ZN21DB_AvatarExtendPeriod8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarExtendPeriod::dispatch(int,int,Stream *)
bool DB_AvatarExtendPeriod::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    SIG_AVATAR_RENEW payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }
    return ExtendPeriod(payload) == true;
}

// IDA_EA: 0x83FFBB4
// IDA_NAME: _ZN21DB_AvatarExtendPeriod12ExtendPeriodER16SIG_AVATAR_RENEW
// DEMANGLED: DB_AvatarExtendPeriod::ExtendPeriod(SIG_AVATAR_RENEW &)
bool DB_AvatarExtendPeriod::ExtendPeriod(SIG_AVATAR_RENEW& payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate user_items set expire_date = adddate(expire_date, interval %d day) where charac_no=%u and slot=%d",
        payload.days,
        payload.charac_no,
        payload.slot);
    return db_handle->exec(true) == true;
}
