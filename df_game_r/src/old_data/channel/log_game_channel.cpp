#include "old_data/channel/log_game_channel.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <cstring>


namespace WongWork {

typedef char SIG_SAVE_LOGGAMECHANNEL_Size_Must_Be_59[
    (sizeof(SIG_SAVE_LOGGAMECHANNEL) == 59) ? 1 : -1];
typedef char CLogGameChannel_Size_Must_Be_16[
    (sizeof(CLogGameChannel) == 16) ? 1 : -1];

// IDA_EA: 0x82A71C4
// IDA_NAME: _ZN8WongWork15CLogGameChannelC2Ev
// DEMANGLED: WongWork::CLogGameChannel::CLogGameChannel(void)
CLogGameChannel::CLogGameChannel()
    : in_user_(0), out_user_(0), now_user_(0), period_minutes_(10) {}

// IDA_EA: 0x85503C4
// IDA_NAME: _ZN8WongWork15CLogGameChannel14GetLogTargetDBEv
// DEMANGLED: WongWork::CLogGameChannel::GetLogTargetDB(void)
MySQL* CLogGameChannel::GetLogTargetDB() {
    return GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
}

// IDA_EA: 0x85503EA
// IDA_NAME: _ZN8WongWork15CLogGameChannel13StartLogTimerEv
// DEMANGLED: WongWork::CLogGameChannel::StartLogTimer(void)
int CLogGameChannel::StartLogTimer() {
    return SetNextLogTimer();
}

// IDA_EA: 0x85503FE
// IDA_NAME: _ZN8WongWork15CLogGameChannel15SetNextLogTimerEv
// DEMANGLED: WongWork::CLogGameChannel::SetNextLogTimer(void)
int CLogGameChannel::SetNextLogTimer() {
    TimerQueue* timer_queue = G_TimerQueue();
    return timer_queue->InsertTimer(2, 0, 88, 60 * period_minutes_, 0, 0);
}

// IDA_EA: 0x8550456
// IDA_NAME: _ZN8WongWork15CLogGameChannel10ReqSaveLogEv
// DEMANGLED: WongWork::CLogGameChannel::ReqSaveLog(void)
void CLogGameChannel::ReqSaveLog() {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("LogGameChannel.cpp", 32);
    CStreamGuard guard(stream, true);
    *guard << 121;
    *guard << -1;

    SIG_SAVE_LOGGAMECHANNEL* payload =
        guard->GetInBuffer<SIG_SAVE_LOGGAMECHANNEL>();
    std::memset(payload, 0, sizeof(*payload));

    CEnvironment* environment = G_CEnvironment();
    payload->gc_game =
        static_cast<char>(environment->get_accept_handshake_flag_888());
    payload->gc_no = environment->get_gc_no();
    payload->gc_ch_group = static_cast<short>(environment->get_channel_no());

    const char* channel_name = environment->get_channel_name();
    const unsigned int channel_name_len =
        static_cast<unsigned int>(std::strlen(channel_name));
    std::memcpy(payload->gc_channel, channel_name, channel_name_len);

    const char* server_ip = environment->get_server_ip();
    const unsigned int server_ip_len =
        static_cast<unsigned int>(std::strlen(server_ip));
    std::memcpy(payload->gc_ip, server_ip, server_ip_len);

    payload->gc_now = static_cast<short>(now_user_);
    payload->gc_new = static_cast<short>(in_user_);
    payload->gc_out = static_cast<short>(out_user_);

    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x8550612
// IDA_NAME: _ZN8WongWork15CLogGameChannel9DoSaveLogEP23SIG_SAVE_LOGGAMECHANNEL
// DEMANGLED: WongWork::CLogGameChannel::DoSaveLog(SIG_SAVE_LOGGAMECHANNEL *)
int CLogGameChannel::DoSaveLog(SIG_SAVE_LOGGAMECHANNEL* payload) {
    MySQL* db_handle = GetLogTargetDB();
    db_handle->set_query(
        "inSert into log_game_channel(gc_up_time,gc_game,gc_no,gc_channel,gc_ch_group,gc_ip,gc_now,gc_new,gc_out) values(now(),%d,%d,'%s',%d,'%s',%d,%d,%d)",
        payload->gc_game,
        payload->gc_no,
        payload->gc_channel,
        payload->gc_ch_group,
        payload->gc_ip,
        payload->gc_now,
        payload->gc_new,
        payload->gc_out);
    return db_handle->exec(true);
}

// IDA_EA: 0x85506C6
// IDA_NAME: _ZN8WongWork15CLogGameChannel14ResetInOutUserEv
// DEMANGLED: WongWork::CLogGameChannel::ResetInOutUser(void)
CLogGameChannel* CLogGameChannel::ResetInOutUser() {
    in_user_ = 0;
    out_user_ = 0;
    return this;
}

// IDA_EA: 0x85506DE
// IDA_NAME: _ZN8WongWork15CLogGameChannel9IncInUserEv
// DEMANGLED: WongWork::CLogGameChannel::IncInUser(void)
CLogGameChannel* CLogGameChannel::IncInUser() {
    ++in_user_;
    ++now_user_;
    return this;
}

// IDA_EA: 0x8550700
// IDA_NAME: _ZN8WongWork15CLogGameChannel10IncOutUserEv
// DEMANGLED: WongWork::CLogGameChannel::IncOutUser(void)
CLogGameChannel* CLogGameChannel::IncOutUser() {
    ++out_user_;
    --now_user_;
    return this;
}

}  // namespace WongWork
