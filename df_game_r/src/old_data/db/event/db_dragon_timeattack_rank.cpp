#include "old_data/db/event/db_dragon_timeattack_rank.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"

#include <cstring>

namespace dragon_timeattack_rank {
namespace {

const char kRankingTable[] = "dragon_timeattack_rank_0828";
const int kMaximumServerGroupCount = 18;

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

MySQL* GetDatabase(int server_group) {
    if (!GlobalData::s_db_mgr || !IsValidServerGroup(server_group)) {
        return 0;
    }
    return GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kDbHandle),
        static_cast<ENUM_SERVER_GROUP>(server_group));
}

bool IsTerminatedName(const char* name) {
    return name && name[0] != 0 &&
           name[kMaximumCharacterNameBytes] == 0;
}

bool UpdateBestTime(MySQL* db, const Payload& payload) {
    if (!db || payload.score_charac_no == 0u ||
        payload.score_charac_no != payload.request_charac_no ||
        payload.score_clear_time_ms == 0u ||
        payload.score_job_type > kMaximumJobType ||
        !IsTerminatedName(payload.score_character_name)) {
        return false;
    }

    char escaped_name[kMaximumCharacterNameBytes * 2 + 1] = {};
    if (db->escape_string(escaped_name,
                          payload.score_character_name) < 0 ||
        !db->set_query(
            "inSert into %s(charac_no,job_type,grow_type,"
            "second_grow_type,charac_level,charac_name,best_clear_time,"
            "updated_at) values(%u,%u,%u,%u,%u,'%s',%u,now()) "
            "on duplicate key update "
            "job_type=if(values(best_clear_time)<best_clear_time,"
            "values(job_type),job_type),"
            "grow_type=if(values(best_clear_time)<best_clear_time,"
            "values(grow_type),grow_type),"
            "second_grow_type=if(values(best_clear_time)<best_clear_time,"
            "values(second_grow_type),second_grow_type),"
            "charac_level=if(values(best_clear_time)<best_clear_time,"
            "values(charac_level),charac_level),"
            "charac_name=if(values(best_clear_time)<best_clear_time,"
            "values(charac_name),charac_name),"
            "updated_at=if(values(best_clear_time)<best_clear_time,"
            "now(),updated_at),"
            "best_clear_time=least(best_clear_time,"
            "values(best_clear_time))",
            kRankingTable,
            static_cast<unsigned int>(payload.score_charac_no),
            static_cast<unsigned int>(payload.score_job_type),
            static_cast<unsigned int>(payload.score_grow_type),
            static_cast<unsigned int>(payload.score_second_grow_type),
            static_cast<unsigned int>(payload.score_charac_level),
            escaped_name,
            static_cast<unsigned int>(payload.score_clear_time_ms))) {
        return false;
    }
    return db->exec(true);
}

bool LoadPersonalRecord(MySQL* db, Payload& payload) {
    payload.personal_best_clear_time_ms = 0u;
    payload.personal_rank = 0u;
    if (!db || payload.request_charac_no == 0u ||
        !db->set_query(
            "seLect r.best_clear_time,(seLect count(*)+1 from %s x "
            "where x.job_type=r.job_type and x.best_clear_time>0 and "
            "(x.best_clear_time<r.best_clear_time or "
            "(x.best_clear_time=r.best_clear_time and "
            "(x.updated_at<r.updated_at or "
            "(x.updated_at=r.updated_at and x.charac_no<r.charac_no))))) "
            "from %s r where r.charac_no=%u and r.job_type=%u and "
            "r.best_clear_time>0",
            kRankingTable,
            kRankingTable,
            static_cast<unsigned int>(payload.request_charac_no),
            static_cast<unsigned int>(payload.requested_job_type)) ||
        !db->exec(true)) {
        return false;
    }
    const int row_count = db->get_n_rows();
    if (row_count == 0) {
        return true;
    }
    if (row_count != 1 || !db->fetch()) {
        return false;
    }
    unsigned int best = 0u;
    unsigned int rank = 0u;
    if (!db->get_uint(0, best) || !db->get_uint(1, rank) ||
        best == 0u || rank == 0u) {
        return false;
    }
    payload.personal_best_clear_time_ms = static_cast<uint32_t>(best);
    payload.personal_rank = static_cast<uint32_t>(rank);
    return true;
}

bool LoadRanking(MySQL* db, Payload& payload) {
    payload.record_count = 0u;
    std::memset(payload.records, 0, sizeof(payload.records));
    if (!db ||
        !db->set_query(
            "seLect charac_name,job_type,grow_type,second_grow_type,"
            "charac_level,best_clear_time from %s where job_type=%u and "
            "best_clear_time>0 order by best_clear_time asc,updated_at asc,"
            "charac_no asc limit %u",
            kRankingTable,
            static_cast<unsigned int>(payload.requested_job_type),
            static_cast<unsigned int>(kMaximumRecordCount)) ||
        !db->exec(true)) {
        return false;
    }
    const int row_count = db->get_n_rows();
    if (row_count < 0 || row_count > kMaximumRecordCount) {
        return false;
    }
    for (int row = 0; row < row_count; ++row) {
        if (!db->fetch()) {
            return false;
        }
        RankRecord& record = payload.records[row];
        unsigned char job = 0u;
        unsigned char grow = 0u;
        unsigned char second = 0u;
        unsigned char level = 0u;
        unsigned int clear_time = 0u;
        if (!db->get_str(0,
                         record.character_name,
                         sizeof(record.character_name)) ||
            !db->get_ubyte(1, job) || !db->get_ubyte(2, grow) ||
            !db->get_ubyte(3, second) || !db->get_ubyte(4, level) ||
            !db->get_uint(5, clear_time) ||
            !IsTerminatedName(record.character_name) || clear_time == 0u) {
            return false;
        }
        record.job_type = job;
        record.grow_type = grow;
        record.second_grow_type = second;
        record.charac_level = level;
        record.clear_time_ms = static_cast<uint32_t>(clear_time);
        ++payload.record_count;
    }
    return true;
}

bool SendQueryResult(int user_idx, const Payload& payload) {
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
    Payload* const result = reinterpret_cast<Payload*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Payload))));
    if (!result) {
        return false;
    }
    *result = payload;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

void ClearPayload(Payload& payload) {
    std::memset(&payload, 0, sizeof(payload));
    payload.result = 1u;
}

bool Queue(int user_idx, const Payload& payload) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || payload.request_charac_no == 0u ||
        !IsValidServerGroup(payload.server_group) || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Payload* const request = reinterpret_cast<Payload*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Payload))));
    if (!request) {
        return false;
    }
    *request = payload;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

DB_DragonTimeAttackRank::DB_DragonTimeAttackRank()
    : DBDispatcher() {}

DB_DragonTimeAttackRank::~DB_DragonTimeAttackRank() {}

bool DB_DragonTimeAttackRank::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Payload))) {
        return false;
    }
    const Payload* const request =
        reinterpret_cast<const Payload*>(stream->get_buffer());
    if (!request || request->request_charac_no == 0u ||
        !IsValidServerGroup(request->server_group)) {
        return false;
    }

    MySQL* const db = GetDatabase(request->server_group);
    if (request->operation == OPERATION_UPDATE_BEST_TIME) {
        return UpdateBestTime(db, *request);
    }
    if (request->operation != OPERATION_QUERY_RANKING ||
        request->requested_job_type > kMaximumJobType) {
        return false;
    }

    Payload response = *request;
    response.result = 1u;
    if (LoadPersonalRecord(db, response) && LoadRanking(db, response)) {
        response.result = 0u;
    }
    return SendQueryResult(user_idx, response);
}

}  // namespace dragon_timeattack_rank_0828
