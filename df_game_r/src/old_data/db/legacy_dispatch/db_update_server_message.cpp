#include "old_data/db/legacy_dispatch/db_update_server_message.h"

#include "common/manager/global_data.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


// IDA_EA: 0x8435A02
// IDA_NAME: _ZN22DB_UpdateServerMessage8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateServerMessage::dispatch(int,int,Stream *)
bool DB_UpdateServerMessage::dispatch(int signal, int user_idx,
                                      Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT);
    SIG_UPDATE_SERVER_MESSAGE* request =
        stream->GetOutBuffer<SIG_UPDATE_SERVER_MESSAGE>();
    char escaped_message[128];
    char escaped_charac_name[64];
    db_handle->escape_string(escaped_message, request->message);
    db_handle->escape_string(escaped_charac_name, request->charac_name);
    db_handle->set_query(
        "upDate event_server_message set message='%s', charac_name='%s', "
        "update_time=from_unixtime(%d) where server_info=%d and "
        "channel_no=%d and message_index=%d",
        escaped_message,
        escaped_charac_name,
        request->update_time,
        request->server_group,
        request->channel_no,
        request->message_index);

    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        db_handle->set_query(
            "inSert into event_server_message(server_info, channel_no, kind, "
            "message_index, charac_name, message, update_time) values(%d, %d, "
            "%d, %d, '%s', '%s', from_unixtime(%d))",
            request->server_group,
            request->channel_no,
            request->kind,
            request->message_index,
            escaped_charac_name,
            escaped_message,
            request->update_time);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 32995);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_UPDATE_SERVER_MESSAGE* reply =
        guard->GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>();
    std::memset(reply, 0, sizeof(*reply));
    reply->server_group = request->server_group;
    reply->channel_no = request->channel_no;
    reply->message_index = request->message_index;
    reply->kind = request->kind;
    reply->charac_name_len = request->charac_name_len;
    std::strncpy(reply->charac_name,
                 request->charac_name,
                 request->charac_name_len + 1);
    reply->message_len = request->message_len;
    std::strncpy(reply->message, request->message, request->message_len + 1);
    reply->update_time = request->update_time;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844EF1A
// IDA_NAME: _ZN22DB_UpdateServerMessageC2Ev
// DEMANGLED: DB_UpdateServerMessage::DB_UpdateServerMessage(void)
DB_UpdateServerMessage::DB_UpdateServerMessage() : DBDispatcher() {}

// IDA_EA: 0x845D522
// IDA_NAME: _ZN22DB_UpdateServerMessageD2Ev
// DEMANGLED: DB_UpdateServerMessage::~DB_UpdateServerMessage()
// IDA_EA: 0x845D552
// IDA_NAME: _ZN22DB_UpdateServerMessageD0Ev
// DEMANGLED: DB_UpdateServerMessage::~DB_UpdateServerMessage()
// The compiler-emitted deleting destructor invokes this D2 body then delete.
DB_UpdateServerMessage::~DB_UpdateServerMessage() {}
