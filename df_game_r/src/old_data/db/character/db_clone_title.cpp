#include "old_data/db/character/db_clone_title.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"


namespace clone_title_db {
namespace {

const char kStateTable[] = "charac_clone_title_0828";

bool IsValidServerGroup(int server_group) {
    // CEnvironment maps the ordinary groups to 0..17 and retains the two
    // legacy first-server groups as 99/100.
    return (server_group >= 0 && server_group < 0x12) ||
           server_group == 99 || server_group == 100;
}

bool IsValidMessage(const Message& message) {
    if (!message.account_id || !message.charac_no ||
        !IsValidServerGroup(message.server_group) ||
        (message.operation != kOperationLoad &&
         message.operation != kOperationSave)) {
        return false;
    }
    if (message.operation == kOperationLoad) {
        return message.item_id == 0u &&
               message.reply_kind == kReplyBackground;
    }
    return message.reply_kind == kReplyBackground ||
           message.reply_kind == kReplyCommand;
}

bool SendResult(int user_idx, const Message& source) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return false;
    }

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const result = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!result) {
        return false;
    }
    *result = source;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool LoadState(MySQL* db, Message& message) {
    if (!db ||
        !db->set_query(
            "seLect clone_title_item_id from %s "
            "where charac_no=%u limit 1",
            kStateTable,
            static_cast<unsigned int>(message.charac_no)) ||
        !db->exec(true)) {
        return false;
    }

    const int row_count = db->get_n_rows();
    if (row_count == 0) {
        message.item_id = 0u;
        return true;
    }
    if (row_count != 1 || !db->fetch()) {
        return false;
    }

    unsigned int item_id = 0;
    if (!db->get_uint(0, item_id)) {
        return false;
    }
    message.item_id = static_cast<uint32_t>(item_id);
    return true;
}

bool SaveState(MySQL* db, const Message& message) {
    if (!db) {
        return false;
    }
    if (message.item_id == 0u) {
        return db->set_query(
                   "deLete from %s where charac_no=%u",
                   kStateTable,
                   static_cast<unsigned int>(message.charac_no)) &&
               db->exec(true);
    }
    return db->set_query(
               "inSert into %s(charac_no,clone_title_item_id) "
               "values(%u,%u) on duplicate key update "
               "clone_title_item_id=values(clone_title_item_id)",
               kStateTable,
               static_cast<unsigned int>(message.charac_no),
               static_cast<unsigned int>(message.item_id)) &&
           db->exec(true);
}

}  // namespace


DB_CloneTitle::DB_CloneTitle() : DBDispatcher() {}

DB_CloneTitle::~DB_CloneTitle() {}

bool DB_CloneTitle::Queue(int user_idx, const Message& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || !IsValidMessage(request) || !pool || !queues) {
        return false;
    }

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const queued = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!queued) {
        return false;
    }
    *queued = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_CloneTitle::dispatch(int signal,
                                 int user_idx,
                                 Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }

    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || !IsValidMessage(*request)) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultFailed);
    response.reserved = 0;

    // This is the sole SQL owner.  Handle 17 is a distinct MySQL object and
    // socket even though its db_connect row points at the character schema.
    MySQL* const db = GlobalData::s_db_mgr
                          ? GlobalData::s_db_mgr->GetDBHandleExact(
                                DB_HANDLE_FEATURE_DB3,
                                static_cast<ENUM_SERVER_GROUP>(
                                    response.server_group))
                          : 0;
    bool success = false;
    if (response.operation == kOperationLoad) {
        success = LoadState(db, response);
    } else if (response.operation == kOperationSave) {
        success = SaveState(db, response);
    }
    if (success) {
        response.result = static_cast<uint8_t>(kResultOk);
    }
    return SendResult(user_idx, response);
}

}  // namespace clone_title_db_0828
