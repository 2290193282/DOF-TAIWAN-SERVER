#include "old_data/db/legacy_dispatch/db_load_server_message.h"

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


// IDA_EA: 0x8435ECC
// IDA_NAME: _ZN20DB_LoadServerMessage8dispatchEiiP6Stream
// DEMANGLED: DB_LoadServerMessage::dispatch(int,int,Stream *)
bool DB_LoadServerMessage::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT);
    SIG_SELECT_SERVER_MESSAGE* selector =
        stream->GetOutBuffer<SIG_SELECT_SERVER_MESSAGE>();
    db_handle->set_query(
        "seLect kind, message_index, charac_name, message, "
        "unix_timestamp(update_time) from event_server_message where "
        "server_info=%d and (channel_no=%d or channel_no=0)",
        selector->server_group,
        selector->channel_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 33073);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    SIG_EVENT_SERVER_MESSAGE* reply =
        guard->GetInBuffer<SIG_EVENT_SERVER_MESSAGE>();
    std::memset(reply, 0, sizeof(*reply));

    const int row_count = db_handle->get_n_rows();
    char kind = 0;
    char message_index = 0;
    char charac_name[20] = {};
    char message[50] = {};
    int update_time = 0;
    int result_index = 0;

    // The old body has no 50-row clamp.  Preserve that DB/result contract;
    // the table is only an L1 provider until the full original route is live.
    for (; result_index < row_count; ++result_index) {
        if (db_handle->fetch() != true ||
            db_handle->get_byte(0, kind) != true ||
            db_handle->get_byte(1, message_index) != true ||
            db_handle->get_str(2, charac_name, sizeof(charac_name)) != true ||
            db_handle->get_str(3, message, sizeof(message)) != true ||
            db_handle->get_int(4, update_time) != true) {
            return false;
        }

        EventServerMessagePayload& target = reply->messages[result_index];
        target.kind = static_cast<int8_t>(kind);
        target.message_index = static_cast<int8_t>(message_index);
        target.message_length =
            static_cast<uint32_t>(std::strlen(message));
        std::strncpy(target.message, message, std::strlen(message));
        target.charac_name_length =
            static_cast<uint32_t>(std::strlen(charac_name));
        std::strncpy(target.charac_name, charac_name,
                     std::strlen(charac_name));
        target.update_time = update_time;
    }

    reply->count = static_cast<uint8_t>(row_count);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844EF36
// IDA_NAME: _ZN20DB_LoadServerMessageC2Ev
// DEMANGLED: DB_LoadServerMessage::DB_LoadServerMessage(void)
DB_LoadServerMessage::DB_LoadServerMessage() : DBDispatcher() {}

// IDA_EA: 0x845D438
// IDA_NAME: _ZN20DB_LoadServerMessageD2Ev
// DEMANGLED: DB_LoadServerMessage::~DB_LoadServerMessage()
// IDA_EA: 0x845D468
// IDA_NAME: _ZN20DB_LoadServerMessageD0Ev
// DEMANGLED: DB_LoadServerMessage::~DB_LoadServerMessage()
// The compiler-emitted deleting destructor invokes this D2 body then delete.
DB_LoadServerMessage::~DB_LoadServerMessage() {}
