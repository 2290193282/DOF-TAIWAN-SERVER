#include "old_data/db/event/db_skill_damage_rank_event.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstdio>
#include <cstring>

namespace skill_damage_rank_event {
namespace {

const char kRankingTable[] = "skill_damage_rank_event_0828";
const int kMaximumServerGroupCount = 18;

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsValidGrowTypeSet(const Payload& payload) {
    if (payload.grow_type_count == 0u ||
        payload.grow_type_count > kMaximumGrowTypeCount) {
        return false;
    }
    for (unsigned int index = 0u; index < payload.grow_type_count; ++index) {
        if (payload.grow_types[index] == 0u) {
            return false;
        }
        for (unsigned int previous = 0u; previous < index; ++previous) {
            if (payload.grow_types[previous] == payload.grow_types[index]) {
                return false;
            }
        }
    }
    return true;
}

MySQL* GetDatabase(int server_group) {
    if (!GlobalData::s_db_mgr || !IsValidServerGroup(server_group)) {
        return 0;
    }
    return GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kDbHandle),
        static_cast<ENUM_SERVER_GROUP>(server_group));
}

bool UpdateBestDamage(MySQL* db, const Payload& payload) {
    if (!db || payload.score_charac_no == 0u ||
        payload.best_damage == 0u || payload.job_type == 0u ||
        payload.grow_type == 0u || payload.second_grow_type == 0u ||
        payload.character_name[0] == 0 ||
        payload.character_name[kMaximumCharacterNameBytes] != 0) {
        return false;
    }

    char escaped_name[kMaximumCharacterNameBytes * 2 + 1] = {};
    if (db->escape_string(escaped_name, payload.character_name) < 0 ||
        !db->set_query(
            "inSert into %s(charac_no,job_type,grow_type,"
            "second_grow_type,charac_name,best_damage,updated_at) values("
            "%u,%u,%u,%u,'%s',%u,now()) on duplicate key update "
            "job_type=if(values(best_damage)>best_damage,"
            "values(job_type),job_type),"
            "grow_type=if(values(best_damage)>best_damage,"
            "values(grow_type),grow_type),"
            "second_grow_type=if(values(best_damage)>best_damage,"
            "values(second_grow_type),second_grow_type),"
            "charac_name=if(values(best_damage)>best_damage,"
            "values(charac_name),charac_name),"
            "updated_at=if(values(best_damage)>best_damage,now(),updated_at),"
            "best_damage=greatest(best_damage,values(best_damage))",
            kRankingTable,
            static_cast<unsigned int>(payload.score_charac_no),
            static_cast<unsigned int>(payload.job_type),
            static_cast<unsigned int>(payload.grow_type),
            static_cast<unsigned int>(payload.second_grow_type),
            escaped_name,
            static_cast<unsigned int>(payload.best_damage))) {
        return false;
    }
    return db->exec(true);
}

bool LoadPersonalBest(MySQL* db,
                      uint32_t charac_no,
                      uint32_t& best_damage) {
    best_damage = 0u;
    if (!db || charac_no == 0u ||
        !db->set_query("seLect best_damage from %s where charac_no=%u",
                       kRankingTable,
                       static_cast<unsigned int>(charac_no)) ||
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
    unsigned int value = 0u;
    if (!db->get_uint(0, value)) {
        return false;
    }
    best_damage = static_cast<uint32_t>(value);
    return true;
}

bool LoadRanking(MySQL* db, Payload& payload) {
    payload.record_count = 0u;
    for (unsigned int group = 0u; group < payload.grow_type_count; ++group) {
        const unsigned int grow_type = payload.grow_types[group];
        if (!db->set_query(
                "seLect charac_name,best_damage from %s where "
                "job_type=%u and second_grow_type=%u and grow_type=%u "
                "and best_damage>0 order by best_damage desc,updated_at asc,"
                "charac_no asc limit %u",
                kRankingTable,
                static_cast<unsigned int>(payload.job_type),
                static_cast<unsigned int>(payload.second_grow_type),
                grow_type,
                static_cast<unsigned int>(kMaximumRowsPerGrowType)) ||
            !db->exec(true)) {
            return false;
        }
        const int row_count = db->get_n_rows();
        if (row_count < 0 || row_count > kMaximumRowsPerGrowType) {
            return false;
        }
        for (int row = 0; row < row_count; ++row) {
            if (!db->fetch() ||
                payload.record_count >= kMaximumRecordCount) {
                return false;
            }
            RankRecord& record = payload.records[payload.record_count];
            std::memset(&record, 0, sizeof(record));
            unsigned int damage = 0u;
            if (!db->get_str(0,
                             record.character_name,
                             sizeof(record.character_name)) ||
                !db->get_uint(1, damage) ||
                record.character_name[0] == 0 || damage == 0u) {
                return false;
            }
            record.grow_type = static_cast<uint8_t>(grow_type);
            record.best_damage = static_cast<uint32_t>(damage);
            ++payload.record_count;
        }
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

DB_SkillDamageRankEvent::DB_SkillDamageRankEvent()
    : DBDispatcher() {}

DB_SkillDamageRankEvent::~DB_SkillDamageRankEvent() {}

bool DB_SkillDamageRankEvent::dispatch(int signal,
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
    if (request->operation == OPERATION_UPDATE_BEST_DAMAGE) {
        return UpdateBestDamage(db, *request);
    }
    if (request->operation != OPERATION_QUERY_RANKING ||
        request->job_type == 0u || request->second_grow_type == 0u ||
        !IsValidGrowTypeSet(*request)) {
        return false;
    }

    Payload response = *request;
    response.result = 1u;
    response.record_count = 0u;
    std::memset(response.records, 0, sizeof(response.records));
    if (LoadPersonalBest(db,
                         response.request_charac_no,
                         response.best_damage) &&
        LoadRanking(db, response)) {
        response.result = 0u;
    }
    return SendQueryResult(user_idx, response);
}

}  // namespace skill_damage_rank_event_0828
