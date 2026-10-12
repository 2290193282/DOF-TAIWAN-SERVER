#include "old_data/db/channel/db_update_channel.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x8417F4A
// IDA_NAME: _ZN16DB_UpdateChannelC2Ev
// DEMANGLED: DB_UpdateChannel::DB_UpdateChannel(void)
DB_UpdateChannel::DB_UpdateChannel() {}

// IDA_EA: 0x8417F66
// IDA_NAME: _ZN16DB_UpdateChannelD2Ev
// DEMANGLED: DB_UpdateChannel::~DB_UpdateChannel()
// IDA_EA: 0x8417F96
// IDA_NAME: _ZN16DB_UpdateChannelD0Ev
// DEMANGLED: DB_UpdateChannel::~DB_UpdateChannel() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateChannel::~DB_UpdateChannel() {}

// IDA_EA: 0x8417FB4
// IDA_NAME: _ZN16DB_UpdateChannel8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateChannel::dispatch(int,int,Stream *)
bool DB_UpdateChannel::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_UPDATE_CHANNEL* channel = stream->GetOutBuffer<SIG_UPDATE_CHANNEL>();
    return channel && UpdateChannel(channel) == true;
}

// IDA_EA: 0x8418032
// IDA_NAME: _ZN16DB_UpdateChannel13UpdateChannelEP18SIG_UPDATE_CHANNEL
// DEMANGLED: DB_UpdateChannel::UpdateChannel(SIG_UPDATE_CHANNEL *)
bool DB_UpdateChannel::UpdateChannel(SIG_UPDATE_CHANNEL* channel) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate game_channel set gc_now=%d,gc_up_time=now(),gc_swordman_cnt=%d,gc_fighter_cnt=%d,gc_gunner_cnt=%d,gc_mage_cnt"
        "=%d,gc_priest_cnt=%d,gc_at_gunner_cnt=%d,gc_thief_cnt=%d,gc_hangame=%d,gc_nexon=%d where gc_no=%d",
        channel->gc_now,
        channel->swordman_count,
        channel->fighter_count,
        channel->gunner_count,
        channel->mage_count,
        channel->priest_count,
        channel->at_gunner_count,
        channel->thief_count,
        channel->hangame_count,
        channel->nexon_count,
        channel->gc_no);
    return db_handle->exec(true) == true;
}
