#include "old_data/db/avatar_aura/db_avatar_option_change.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844FB22
// IDA_NAME: _ZN21DB_AvatarOptionChangeC2Ev
// DEMANGLED: DB_AvatarOptionChange::DB_AvatarOptionChange(void)
DB_AvatarOptionChange::DB_AvatarOptionChange() {}

// IDA_EA: 0x845B302
// IDA_NAME: _ZN21DB_AvatarOptionChangeD2Ev
// DEMANGLED: DB_AvatarOptionChange::~DB_AvatarOptionChange()
// IDA_EA: 0x845B332
// IDA_NAME: _ZN21DB_AvatarOptionChangeD0Ev
// DEMANGLED: DB_AvatarOptionChange::~DB_AvatarOptionChange() deleting destructor
DB_AvatarOptionChange::~DB_AvatarOptionChange() {}

// IDA_EA: 0x844687E
// IDA_NAME: _ZN21DB_AvatarOptionChange8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarOptionChange::dispatch(int,int,Stream *)
bool DB_AvatarOptionChange::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_AVATAR_OPTION_CHANGE* payload =
        stream->GetOutBuffer<SIG_AVATAR_OPTION_CHANGE>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate user_items set ability_no=%d where ui_id=%d and charac_no=%d",
        payload->ability_no,
        payload->ui_id,
        payload->charac_no);
    if (db_handle->exec(true) == true) {
        return true;
    }
    cMyTrace trace(
        "virtual bool DB_AvatarOptionChange::dispatch(int, int, Stream*)",
        44636,
        5);
    trace("AVATAR_OPTION_CHAGE::DB_UPDATE_FAIL");
    return false;
}
