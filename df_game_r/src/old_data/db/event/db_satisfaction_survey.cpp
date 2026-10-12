#include "old_data/db/event/db_satisfaction_survey.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstdio>
#include <cstring>

namespace satisfaction_survey {
namespace {

const char kSurveyTable[] = "satisfaction_survey_response_0828";

bool BuildAnswerCsv(const Request& request,
                    char* output,
                    unsigned int capacity) {
    if (!output || capacity == 0u || request.question_count == 0u ||
        request.question_count > kMaximumQuestionCount) {
        return false;
    }
    output[0] = 0;
    unsigned int used = 0u;
    for (unsigned int index = 0u; index < request.question_count; ++index) {
        const int written = std::snprintf(
            output + used,
            static_cast<size_t>(capacity - used),
            index == 0u ? "%u" : ",%u",
            static_cast<unsigned int>(request.answers[index]));
        if (written <= 0 ||
            static_cast<unsigned int>(written) >= capacity - used) {
            return false;
        }
        used += static_cast<unsigned int>(written);
    }
    return true;
}

}  // namespace

DB_SaveSatisfactionSurvey::DB_SaveSatisfactionSurvey()
    : DBDispatcher() {}

DB_SaveSatisfactionSurvey::~DB_SaveSatisfactionSurvey() {}

bool DB_SaveSatisfactionSurvey::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)user_idx;
    if (signal != kDbSignal || !stream ||
        stream->length() != static_cast<int>(sizeof(Request))) {
        return false;
    }

    Request request;
    std::memset(&request, 0, sizeof(request));
    if (stream->get_binary(&request, sizeof(request)) != 1 ||
        request.account_id == 0u || request.question_count == 0u ||
        request.question_count > kMaximumQuestionCount ||
        request.comment_size > kMaximumCommentBytes ||
        request.comment[request.comment_size] != 0) {
        return false;
    }

    char answer_csv[192] = {};
    if (!BuildAnswerCsv(request, answer_csv, sizeof(answer_csv))) {
        return false;
    }

    MySQL* const db = GlobalData::s_db_mgr
                          ? GlobalData::s_db_mgr->GetDBHandle(
                                DB_HANDLE_LOG, SERVER_GROUP_DEFAULT)
                          : 0;
    if (!db) {
        return false;
    }

    char escaped_comment[kMaximumCommentBytes * 2 + 1] = {};
    if (db->escape_string(escaped_comment, request.comment) < 0 ||
        !db->set_query(
            "inSert into %s(account_id,charac_no,server_group,survey_id,"
            "question_count,answers,comment,created_at) values("
            "%u,%u,%d,%u,%u,'%s','%s',now())",
            kSurveyTable,
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<int>(request.server_group),
            static_cast<unsigned int>(request.survey_id),
            static_cast<unsigned int>(request.question_count),
            answer_csv,
            escaped_comment)) {
        return false;
    }
    return db->exec(true);
}

bool Queue(int user_idx, const Request& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || request.account_id == 0u || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Request* const payload = reinterpret_cast<Request*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Request))));
    if (!payload) {
        return false;
    }
    *payload = request;
    // In the native worker TLS context DB_HANDLE_LOG resolves only to the
    // dedicated type-19 socket.  The legacy LOGDB_Q/thread never consumes
    // this reconstructed append.
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

}  // namespace satisfaction_survey_0828
