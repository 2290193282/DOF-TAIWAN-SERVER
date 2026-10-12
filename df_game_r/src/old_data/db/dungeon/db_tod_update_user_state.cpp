#include "old_data/db/dungeon/db_tod_update_user_state.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844F7A2
// IDA_NAME: _ZN23DB_TOD_Update_UserStateC2Ev
// DEMANGLED: DB_TOD_Update_UserState::DB_TOD_Update_UserState(void)
DB_TOD_Update_UserState::DB_TOD_Update_UserState() {}

// IDA_EA: 0x845BC74
// IDA_NAME: _ZN23DB_TOD_Update_UserStateD2Ev
// DEMANGLED: DB_TOD_Update_UserState::~DB_TOD_Update_UserState()
// IDA_EA: 0x845BCA4
// IDA_NAME: _ZN23DB_TOD_Update_UserStateD0Ev
// DEMANGLED: DB_TOD_Update_UserState::~DB_TOD_Update_UserState()
DB_TOD_Update_UserState::~DB_TOD_Update_UserState() {}

// IDA_EA: 0x84403AA
// IDA_NAME: _ZN23DB_TOD_Update_UserState8dispatchEiiP6Stream
// DEMANGLED: DB_TOD_Update_UserState::dispatch(int,int,Stream *)
bool DB_TOD_Update_UserState::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    SIG_REQUEST_TOD_USER_STATE* payload =
        stream->GetOutBuffer<SIG_REQUEST_TOD_USER_STATE>();
    if (!payload->first_layer_start_time) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_tower_despair(charac_no,"
        "first_layer_start_date,today_enter_count,last_clear_layer,"
        "enter_count_by_week,m_date,last_clear_date) values"
        "(%u,from_unixtime(%d),%d,%d,%d,from_unixtime(%d),"
        "from_unixtime(%d)) on duplicate key upDate "
        "first_layer_start_date=values(first_layer_start_date),"
        "today_enter_count=values(today_enter_count),"
        "last_clear_layer=values(last_clear_layer),"
        "enter_count_by_week=values(enter_count_by_week),"
        "m_date=values(m_date),last_clear_date=values(last_clear_date)",
        payload->charac_no,
        payload->first_layer_start_time,
        payload->today_enter_count,
        payload->last_clear_layer.GetLayer(),
        payload->enter_count_by_week,
        payload->modified_time,
        payload->last_clear_time);
    return db_handle->exec(true) == true;
}
