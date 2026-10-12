#include "old_data/db/channel/db_delete_channel.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"



// IDA_EA: 0x8402892
// IDA_NAME: _ZN16DB_DeleteChannelC2Ev
// DEMANGLED: DB_DeleteChannel::DB_DeleteChannel(void)
DB_DeleteChannel::DB_DeleteChannel() {}

// IDA_EA: 0x84028AE
// IDA_NAME: _ZN16DB_DeleteChannelD2Ev
// DEMANGLED: DB_DeleteChannel::~DB_DeleteChannel()
// IDA_EA: 0x84028DE
// IDA_NAME: _ZN16DB_DeleteChannelD0Ev
// DEMANGLED: DB_DeleteChannel::~DB_DeleteChannel() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_DeleteChannel::~DB_DeleteChannel() {}

// IDA_EA: 0x84028FC
// IDA_NAME: _ZN16DB_DeleteChannel8dispatchEiiP6Stream
// DEMANGLED: DB_DeleteChannel::dispatch(int,int,Stream *)
bool DB_DeleteChannel::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_DELETE_CHANNEL* channel = stream->GetOutBuffer<SIG_DELETE_CHANNEL>();
    if (!channel) {
        return false;
    }
    if (DeleteChannel(channel) == true) {
        return SendResult(signal, user_idx, channel) == true;
    }
    return false;
}

// IDA_EA: 0x84029AE
// IDA_NAME: _ZN16DB_DeleteChannel13DeleteChannelEP18SIG_DELETE_CHANNEL
// DEMANGLED: DB_DeleteChannel::DeleteChannel(SIG_DELETE_CHANNEL *)
bool DB_DeleteChannel::DeleteChannel(SIG_DELETE_CHANNEL* channel) {
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( channel->server_group));
    db_handle->set_query("deLete from game_channel where gc_no=%d",
                         channel->gc_no);
    char ok = db_handle->exec(true);
    if (ok != 1) {
        return false;
    }

    cMyTrace trace_channel(
        "bool DB_DeleteChannel::DeleteChannel(SIG_DELETE_CHANNEL*)",
        4706,
        0);
    trace_channel("deLete from game_channel where gc_no=%d", channel->gc_no);

    db_handle->set_query("deLete from channel_occ_info where gc_no=%d",
                         channel->gc_no);
    ok = db_handle->exec(true);
    if (ok != 1) {
        return false;
    }

    cMyTrace trace_occ(
        "bool DB_DeleteChannel::DeleteChannel(SIG_DELETE_CHANNEL*)",
        4714,
        0);
    trace_occ("deLete from channel_occ_info where gc_no=%d", channel->gc_no);
    return true;
}

// IDA_EA: 0x8402AEC
// IDA_NAME: _ZN16DB_DeleteChannel10SendResultEiiP18SIG_DELETE_CHANNEL
// DEMANGLED: DB_DeleteChannel::SendResult(int,int,SIG_DELETE_CHANNEL *)
bool DB_DeleteChannel::SendResult(int src, int src_1,
                                  SIG_DELETE_CHANNEL* channel) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 4721);
    CStreamGuard guard(stream, true);
    *guard << static_cast<char>(src);
    *guard << static_cast<char>(src_1);
    guard->put_binary(channel, sizeof(*channel));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
