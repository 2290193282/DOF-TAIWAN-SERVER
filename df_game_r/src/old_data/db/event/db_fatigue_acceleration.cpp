#include "old_data/db/event/db_fatigue_acceleration.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"


namespace fatigue_acceleration_db {
namespace {

const char kStateTable[] = "charac_fatigue_acceleration_0828";

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
            "seLect requested_enabled from %s where charac_no=%u limit 1",
            kStateTable,
            static_cast<unsigned int>(message.charac_no)) ||
        !db->exec(true)) {
        return false;
    }

    const int row_count = db->get_n_rows();
    if (row_count == 0) {
        message.requested_enabled = 0;
        return true;
    }
    if (row_count != 1 || !db->fetch()) {
        return false;
    }

    unsigned int enabled = 0;
    if (!db->get_uint(0, enabled) || enabled > 1u) {
        return false;
    }
    message.requested_enabled = static_cast<uint8_t>(enabled);
    return true;
}

bool SaveState(MySQL* db, const Message& message) {
    return db && message.requested_enabled <= 1u &&
           db->set_query(
               "inSert into %s(charac_no,requested_enabled) values(%u,%u) "
               "on duplicate key update "
               "requested_enabled=values(requested_enabled)",
               kStateTable,
               static_cast<unsigned int>(message.charac_no),
               static_cast<unsigned int>(message.requested_enabled)) &&
           db->exec(true);
}

}  // namespace


DB_FatigueAcceleration::DB_FatigueAcceleration()
    : DBDispatcher() {}

DB_FatigueAcceleration::~DB_FatigueAcceleration() {}

bool DB_FatigueAcceleration::Queue(int user_idx,
                                       const Message& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || request.charac_no == 0 || request.token == 0 ||
        (request.operation != kOperationLoad &&
         request.operation != kOperationSave) ||
        !pool || !queues) {
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

bool DB_FatigueAcceleration::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }

    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || request->charac_no == 0 || request->token == 0) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultFailed);
    response.reserved = 0;

    MySQL* const db = GlobalData::s_db_mgr
                          ? GlobalData::s_db_mgr->GetDBHandle(
                                DB_HANDLE_3,
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

}  // namespace fatigue_acceleration_db
