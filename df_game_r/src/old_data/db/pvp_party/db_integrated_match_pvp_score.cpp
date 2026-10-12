#include "old_data/db/pvp_party/db_integrated_match_pvp_score.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <cstring>


namespace integrated_match_pvp_score_db {
namespace {

const uint32_t kMaximumCounter = 0xffffffffu;

bool IsValidRequest(const Message& message, int user_idx) {
    if (user_idx < 0 || message.user_idx != user_idx ||
        message.target_charac_no == 0u || message.server_group < 0 ||
        message.result != kResultRequest) {
        return false;
    }
    if (message.operation == kOperationLoad) {
        return message.reply_required == 1u &&
               message.requester_account_id != 0u &&
               message.requester_charac_no != 0u &&
               message.target_unique_id != 0u;
    }
    return message.operation == kOperationRecord &&
           message.reply_required == 0u && message.match_mode <= 3u &&
           message.won <= 1u && message.lost <= 1u &&
           !(message.won && message.lost);
}

MySQL* GetDatabase(int server_group) {
    return GlobalData::s_db_mgr
               ? GlobalData::s_db_mgr->GetDBHandleExact(
                     DB_HANDLE_FEATURE_DB3,
                     static_cast<ENUM_SERVER_GROUP>(server_group))
               : 0;
}

bool EnsureRow(MySQL* database,
               uint32_t charac_no,
               uint32_t legacy_seed_wins) {
    if (!database ||
        !database->set_query(
            "seLect charac_no from integrated_match_pvp_score_0828 "
            "where charac_no=%u limit 1",
            charac_no) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows == 1) {
        return true;
    }
    if (rows != 0) {
        return false;
    }
    return database->set_query(
               "inSert ignore into integrated_match_pvp_score_0828"
               "(charac_no,legacy_accumulated_wins) values(%u,%u)",
               charac_no,
               legacy_seed_wins) &&
           database->exec(true);
}

bool LoadScore(MySQL* database, uint32_t charac_no, Message& response) {
    if (!database ||
        !database->set_query(
            "seLect one_on_one_wins,one_on_one_current_streak,"
            "one_on_one_max_streak,one_on_one_perfect_wins,"
            "leader_battle_wins,leader_battle_kills,"
            "leader_battle_double_kills,team_battle_wins,"
            "team_battle_kills,team_battle_double_kills,"
            "individual_battle_wins,individual_battle_kills,"
            "individual_battle_multi_kills,legacy_accumulated_wins "
            "from integrated_match_pvp_score_0828 "
            "where charac_no=%u limit 1",
            charac_no) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    for (int column = 0; column < kScoreFieldCount; ++column) {
        if (!database->get_uint(column, response.score[column])) {
            return false;
        }
    }
    return true;
}

bool UpdateOneOnOne(MySQL* database,
                    uint32_t charac_no,
                    bool won,
                    bool lost) {
    if (!won && !lost) {
        return true;
    }
    if (won) {
        return database->set_query(
                   "upDate integrated_match_pvp_score_0828 set "
                   "one_on_one_wins=least(one_on_one_wins+1,%u),"
                   "one_on_one_max_streak=greatest("
                   "one_on_one_max_streak,"
                   "least(one_on_one_current_streak+1,%u)),"
                   "one_on_one_current_streak=least("
                   "one_on_one_current_streak+1,%u) "
                   "where charac_no=%u",
                   kMaximumCounter,
                   kMaximumCounter,
                   kMaximumCounter,
                   charac_no) &&
               database->exec(true);
    }
    return database->set_query(
               "upDate integrated_match_pvp_score_0828 set "
               "one_on_one_current_streak=0 where charac_no=%u",
               charac_no) &&
           database->exec(true);
}

bool UpdateKillMode(MySQL* database,
                    uint32_t charac_no,
                    uint8_t match_mode,
                    bool won,
                    uint32_t kill_count) {
    const char* wins_column = 0;
    const char* kills_column = 0;
    const char* multi_column = 0;
    if (match_mode == 1u) {
        wins_column = "leader_battle_wins";
        kills_column = "leader_battle_kills";
        multi_column = "leader_battle_double_kills";
    } else if (match_mode == 2u) {
        wins_column = "team_battle_wins";
        kills_column = "team_battle_kills";
        multi_column = "team_battle_double_kills";
    } else if (match_mode == 3u) {
        wins_column = "individual_battle_wins";
        kills_column = "individual_battle_kills";
        multi_column = "individual_battle_multi_kills";
    } else {
        return false;
    }
    const uint32_t win_increment = won ? 1u : 0u;
    const uint32_t multi_increment = kill_count >= 2u ? 1u : 0u;
    return database->set_query(
               "upDate integrated_match_pvp_score_0828 set "
               "%s=least(%s+%u,%u),%s=least(%s+%u,%u),"
               "%s=least(%s+%u,%u) where charac_no=%u",
               wins_column,
               wins_column,
               win_increment,
               kMaximumCounter,
               kills_column,
               kills_column,
               kill_count,
               kMaximumCounter,
               multi_column,
               multi_column,
               multi_increment,
               kMaximumCounter,
               charac_no) &&
           database->exec(true);
}

bool SendResult(int user_idx, const Message& response) {
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
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace


DB_IntegratedMatchPvpScore::DB_IntegratedMatchPvpScore()
    : DBDispatcher() {}

DB_IntegratedMatchPvpScore::~DB_IntegratedMatchPvpScore() {}

bool DB_IntegratedMatchPvpScore::Queue(int user_idx,
                                           const Message& request) {
    if (!IsValidRequest(request, user_idx)) {
        return false;
    }
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
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_IntegratedMatchPvpScore::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    if (signal != kDbSignal || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request, user_idx)) {
        return false;
    }
    Message response = request;
    std::memset(response.score, 0, sizeof(response.score));
    response.result = kResultDatabaseFailure;

    MySQL* const database = GetDatabase(request.server_group);
    bool success = EnsureRow(database,
                             request.target_charac_no,
                             request.legacy_seed_wins);
    if (success && request.operation == kOperationLoad) {
        success = LoadScore(database, request.target_charac_no, response);
    } else if (success && request.operation == kOperationRecord) {
        success = request.match_mode == 0u
                      ? UpdateOneOnOne(database,
                                       request.target_charac_no,
                                       request.won != 0u,
                                       request.lost != 0u)
                      : UpdateKillMode(database,
                                       request.target_charac_no,
                                       request.match_mode,
                                       request.won != 0u,
                                       request.kill_count);
    }
    if (success) {
        response.result = kResultSuccess;
    }
    if (!request.reply_required) {
        return success;
    }
    return SendResult(user_idx, response);
}

}  // namespace integrated_match_pvp_score_db_0828
