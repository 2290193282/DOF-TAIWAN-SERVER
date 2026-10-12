#include "old_data/db/pvp_party/db_pvp_practice_combo_score.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"

namespace pvp_practice_combo_score {
namespace {

const char kScoreTable[] = "pvp_practice_combo_score_0828";
const char* const kScoreColumns[kScoreCount] = {
    "score_0", "score_1", "score_2"
};
const int kMaximumServerGroupCount = 18;

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsValidMessageIdentity(const Message& message) {
    return message.charac_no != 0u &&
           IsValidServerGroup(message.server_group) &&
           message.score_index < kScoreCount;
}

MySQL* GetDatabase(int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) {
        return 0;
    }
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    // DBMgr::GetDBHandle falls back to DB_HANDLE_LOG when active-global mode
    // has no DB_HANDLE_3 mapping for the requested partition.  A combo score
    // must never query or create its table in that unrelated schema.  Ask the
    // group map directly in active-global mode so an unconfigured partition
    // fails closed; ordinary primary-handle deployments retain their native
    // DB_HANDLE_3 path.
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(DB_HANDLE_3, group)
               : manager->GetDBHandle(DB_HANDLE_3, group);
}

bool SendResult(int user_idx, const Message& source) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || !pool || !queues) {
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

bool LoadScore(MySQL* db, Message& response) {
    response.score = 0u;
    const char* const column = kScoreColumns[response.score_index];
    if (!db ||
        !db->set_query("seLect %s from %s where charac_no=%u limit 1",
                       column,
                       kScoreTable,
                       static_cast<unsigned int>(response.charac_no)) ||
        !db->exec(true)) {
        return false;
    }
    const int row_count = db->get_n_rows();
    if (row_count == 0) {
        // Missing rows are the protocol-defined all-zero initial state.
        return true;
    }
    unsigned int score = 0u;
    if (row_count != 1 || !db->fetch() || !db->get_uint(0, score)) {
        return false;
    }
    response.score = static_cast<uint32_t>(score);
    return true;
}

bool UpdateScore(MySQL* db, const Message& request) {
    if (!db || request.score == 0u) {
        return false;
    }
    const char* const column = kScoreColumns[request.score_index];
    // The timestamp assignment deliberately precedes the score assignment:
    // the target MySQL evaluates same-row UPDATE assignments left-to-right.
    return db->set_query(
               "inSert into %s(charac_no,%s,updated_at) "
               "values(%u,%u,now()) on duplicate key update "
               "updated_at=if(values(%s)>%s,now(),updated_at),"
               "%s=greatest(%s,values(%s))",
               kScoreTable,
               column,
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.score),
               column,
               column,
               column,
               column,
               column) &&
           db->exec(true);
}

bool Queue(int user_idx, const Message& source) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || !IsValidMessageIdentity(source) || !pool ||
        !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const request = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!request) {
        return false;
    }
    *request = source;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

}  // namespace

DB_PvpPracticeComboScore::DB_PvpPracticeComboScore()
    : DBDispatcher() {}

DB_PvpPracticeComboScore::~DB_PvpPracticeComboScore() {}

bool DB_PvpPracticeComboScore::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || !IsValidMessageIdentity(*request)) {
        return false;
    }

    MySQL* const db = GetDatabase(request->server_group);
    if (request->operation == OPERATION_UPDATE_MONOTONIC) {
        return UpdateScore(db, *request);
    }
    if (request->operation != OPERATION_QUERY || request->score != 0u) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultFailed);
    if (LoadScore(db, response)) {
        response.result = static_cast<uint8_t>(kResultOk);
    }
    return SendResult(user_idx, response);
}

bool QueueQuery(int user_idx,
                int server_group,
                uint32_t charac_no,
                uint16_t score_index) {
    Message request = {};
    request.charac_no = charac_no;
    request.server_group = server_group;
    request.score_index = score_index;
    request.operation = static_cast<uint8_t>(OPERATION_QUERY);
    request.result = static_cast<uint8_t>(kResultFailed);
    return Queue(user_idx, request);
}

bool QueueMonotonicUpdate(int user_idx,
                          int server_group,
                          uint32_t charac_no,
                          uint16_t score_index,
                          uint32_t score) {
    if (score == 0u) {
        return false;
    }
    Message request = {};
    request.charac_no = charac_no;
    request.score = score;
    request.server_group = server_group;
    request.score_index = score_index;
    request.operation =
        static_cast<uint8_t>(OPERATION_UPDATE_MONOTONIC);
    request.result = static_cast<uint8_t>(kResultFailed);
    return Queue(user_idx, request);
}

}  // namespace pvp_practice_combo_score_0828
