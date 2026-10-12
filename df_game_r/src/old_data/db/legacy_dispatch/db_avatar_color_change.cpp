#include "old_data/db/avatar_aura/db_avatar_color_change.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844FE4E
// IDA_NAME: _ZN20DB_AvatarColorChangeC2Ev
// DEMANGLED: DB_AvatarColorChange::DB_AvatarColorChange(void)
DB_AvatarColorChange::DB_AvatarColorChange() {}

// IDA_EA: 0x845B2B4
// IDA_NAME: _ZN20DB_AvatarColorChangeD2Ev
// DEMANGLED: DB_AvatarColorChange::~DB_AvatarColorChange()
// IDA_EA: 0x845B2E4
// IDA_NAME: _ZN20DB_AvatarColorChangeD0Ev
// DEMANGLED: DB_AvatarColorChange::~DB_AvatarColorChange() deleting destructor
DB_AvatarColorChange::~DB_AvatarColorChange() {}

// IDA_EA: 0x8446944
// IDA_NAME: _ZN20DB_AvatarColorChange8dispatchEiiP6Stream
// DEMANGLED: DB_AvatarColorChange::dispatch(int,int,Stream *)
bool DB_AvatarColorChange::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_AVATAR_COLOR_CHANGE* payload =
        stream->GetOutBuffer<SIG_AVATAR_COLOR_CHANGE>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate user_items set color1=%d, color2=%d where ui_id=%d and charac_no=%d",
        payload->color_1,
        payload->color_2,
        payload->ui_id,
        payload->charac_no);
    if (db_handle->exec(true) == true) {
        return true;
    }
    cMyTrace trace(
        "virtual bool DB_AvatarColorChange::dispatch(int, int, Stream*)",
        44656,
        5);
    trace("AVATAR_COLOR_CHANGE::DB_UPDATE_FAIL");
    return false;
}
