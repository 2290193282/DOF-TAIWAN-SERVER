#include "old_data/db/pvp_party/db_save_pvp.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x841720E
// IDA_NAME: _ZN10DB_SavePvpC2Ev
// DEMANGLED: DB_SavePvp::DB_SavePvp(void)
DB_SavePvp::DB_SavePvp() {}

// IDA_EA: 0x841722A
// IDA_NAME: _ZN10DB_SavePvpD2Ev
// DEMANGLED: DB_SavePvp::~DB_SavePvp()
// IDA_EA: 0x841725A
// IDA_NAME: _ZN10DB_SavePvpD0Ev
// DEMANGLED: DB_SavePvp::~DB_SavePvp() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SavePvp::~DB_SavePvp() {}

// IDA_EA: 0x8417278
// IDA_NAME: _ZN10DB_SavePvp8dispatchEiiP6Stream
// DEMANGLED: DB_SavePvp::dispatch(int,int,Stream *)
bool DB_SavePvp::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_PVP* payload = stream->GetOutBuffer<SIG_SAVE_PVP>();
    return payload && SavePvp(payload) == true;
}

// IDA_EA: 0x84172C8
// IDA_NAME: _ZN10DB_SavePvp7SavePvpEP12SIG_SAVE_PVP
// DEMANGLED: DB_SavePvp::SavePvp(SIG_SAVE_PVP *)
bool DB_SavePvp::SavePvp(SIG_SAVE_PVP* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const int32_t* values = payload_data->pvp_result.values;
    const uint32_t play_time = static_cast<uint32_t>(values[24]);
    const uint32_t play_count = static_cast<uint32_t>(values[25]);
    const int cur_sec = GlobalData::system_time()->getCurSec();
    db_handle->set_query(
        "upDate pvp_result set win=%d,lose=%d,pvp_point=%d,pvp_grade=%d,avg_kill_count=%d,avg_buf_count=%d,avg_debuf_count=%d"
        ",avg_heal_count=%d,avg_counter_count=%d,avg_back_atk_count=%d,avg_union_hit_count=%d,avg_overkill_count=%d,avg_combo"
        "_count=%d,avg_aerial_count=%d,avg_attacked_count=%d,avg_deal_damage=%d,avg_technic=%d,avg_style=%d,avg_hit_penalty=%"
        "d,pvp_count=%d,win_point=%d,last_play_time=from_unixtime(%d), play_count=%u,play_time=%u where charac_no=%u",
        values[0],
        values[1],
        values[2],
        values[5],
        values[6],
        values[7],
        values[8],
        values[9],
        values[10],
        values[11],
        values[12],
        values[13],
        values[14],
        values[15],
        values[16],
        values[17],
        values[18],
        values[19],
        values[20],
        values[21],
        values[22],
        cur_sec,
        play_count,
        play_time,
        payload_data->charac_no);
    return db_handle->exec(true) == true;
}
