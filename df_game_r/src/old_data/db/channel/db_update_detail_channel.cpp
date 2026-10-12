#include "old_data/db/channel/db_update_detail_channel.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "common/manager/system_time.h"



// IDA_EA: 0x844E1C2
// IDA_NAME: _ZN22DB_UpdateDetailChannelC2Ev
// DEMANGLED: DB_UpdateDetailChannel::DB_UpdateDetailChannel(void)
DB_UpdateDetailChannel::DB_UpdateDetailChannel() {}

// IDA_EA: 0x845F87A
// IDA_NAME: _ZN22DB_UpdateDetailChannelD2Ev
// DEMANGLED: DB_UpdateDetailChannel::~DB_UpdateDetailChannel()
// IDA_EA: 0x845F8AA
// IDA_NAME: _ZN22DB_UpdateDetailChannelD0Ev
// DEMANGLED: DB_UpdateDetailChannel::~DB_UpdateDetailChannel() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateDetailChannel::~DB_UpdateDetailChannel() {}

// IDA_EA: 0x841BCEE
// IDA_NAME: _ZN22DB_UpdateDetailChannel8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateDetailChannel::dispatch(int,int,Stream *)
bool DB_UpdateDetailChannel::dispatch(int signal, int user_idx,
                                      Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_DETAIL_CHANNEL* detail =
        stream->GetOutBuffer<SIG_UPDATE_DETAIL_CHANNEL>();

    int cur_sec = GlobalData::system_time()->getCurSec();
    const int remain = cur_sec % 1800;
    if (remain > 899) {
        cur_sec += 1800 - remain;
    } else {
        cur_sec -= remain;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into concurrent_user_status(occ_time,player_status,dungeon_index,channel_no,occ_count) values(from_unixtime(%d),%d,%d,%d,%d)",
        cur_sec,
        detail->player_status,
        detail->dungeon_index,
        detail->channel_no,
        detail->occ_count);
    if (db_handle->exec(true) != true) {
        return false;
    }

    const int player_status = detail->player_status;
    cMyTrace trace(
        "virtual bool DB_UpdateDetailChannel::dispatch(int, int, Stream*)",
        20264,
        0);
    trace("content concurrent %d,%d,%d,%d\n", player_status);
    return true;
}
