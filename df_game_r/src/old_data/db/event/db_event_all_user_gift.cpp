#include "old_data/db/event/db_event_all_user_gift.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"

#include <cstring>

namespace event_all_user_gift {
namespace {

const char kClaimTable[] = "event_1306_account_reward";

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < 18;
}

bool IsValidRequest(const Payload& payload, int dispatch_user_idx) {
    if (payload.account_id == 0u || payload.charac_no == 0u ||
        payload.origin_user_idx < 0 ||
        !IsValidServerGroup(payload.server_group) ||
        payload.job >= oldsrv::config::kCharacterJobCount ||
        payload.selection_count == 0u ||
        payload.selection_count > kMaximumSelectionGroupCount) {
        return false;
    }

    switch (payload.operation) {
    case OPERATION_QUERY_ELIGIBILITY:
        return dispatch_user_idx == payload.origin_user_idx;
    case OPERATION_RESERVE_CLAIM:
    case OPERATION_RELEASE_CLAIM:
        return dispatch_user_idx == -1;
    default:
        return false;
    }
}

MySQL* GetEventDatabase() {
    if (!GlobalData::s_db_mgr) {
        return 0;
    }
    // taiwan_game_event is a global event database; all restored owners use
    // server group zero for handle 9.
    return GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kEventDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));
}

bool ReadClaim(MySQL* db, uint32_t account_id, bool& claimed) {
    claimed = false;
    if (!db ||
        !db->set_query("seLect charac_no from %s where m_id=%u",
                       kClaimTable,
                       static_cast<unsigned int>(account_id)) ||
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
    unsigned int ignored_charac_no = 0u;
    if (!db->get_uint(0, ignored_charac_no)) {
        return false;
    }
    claimed = true;
    return true;
}

bool ReserveClaim(MySQL* db, const Payload& payload, Result& result) {
    result = RESULT_DB_ERROR;
    if (!db ||
        !db->set_query(
            "inSert ignore into %s(m_id,charac_no,occ_date) "
            "values(%u,%u,now())",
            kClaimTable,
            static_cast<unsigned int>(payload.account_id),
            static_cast<unsigned int>(payload.charac_no)) ||
        !db->exec(true)) {
        return false;
    }

    const unsigned long long affected = db->getAffectedRowCount();
    if (affected == 1u) {
        result = RESULT_RESERVED;
        return true;
    }
    if (affected != 0u) {
        return false;
    }

    // INSERT IGNORE with this table's sole PRIMARY KEY can affect zero rows
    // only when the account already owns the one-time claim.  Read it back so
    // an unexpected schema/query condition is not misreported as idempotence.
    bool claimed = false;
    if (!ReadClaim(db, payload.account_id, claimed) || !claimed) {
        return false;
    }
    result = RESULT_ALREADY_CLAIMED;
    return true;
}

bool ReleaseClaim(MySQL* db, const Payload& payload) {
    if (!db ||
        !db->set_query(
            "deLete from %s where m_id=%u and charac_no=%u",
            kClaimTable,
            static_cast<unsigned int>(payload.account_id),
            static_cast<unsigned int>(payload.charac_no)) ||
        !db->exec(true)) {
        return false;
    }
    // Zero affected rows is an idempotent release (for example after a retry
    // of the release signal).  More than one row is impossible under PRIMARY
    // KEY(m_id) and therefore treated as an integrity failure.
    return db->getAffectedRowCount() <= 1u;
}

bool SendResult(int dispatch_user_idx, const Payload& payload) {
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
    *guard << dispatch_user_idx;
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
    payload.origin_user_idx = -1;
    payload.server_group = -1;
    payload.result = static_cast<uint8_t>(RESULT_DB_ERROR);
}

bool Queue(int dispatch_user_idx, const Payload& payload) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!IsValidRequest(payload, dispatch_user_idx) || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << dispatch_user_idx;
    Payload* const request = reinterpret_cast<Payload*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Payload))));
    if (!request) {
        return false;
    }
    *request = payload;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

DB_EventAllUserGift::DB_EventAllUserGift() : DBDispatcher() {}

DB_EventAllUserGift::~DB_EventAllUserGift() {}

bool DB_EventAllUserGift::dispatch(int signal,
                                        int dispatch_user_idx,
                                        Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, dispatch_user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Payload))) {
        return false;
    }
    const Payload* const request =
        reinterpret_cast<const Payload*>(stream->get_buffer());
    if (!request || !IsValidRequest(*request, dispatch_user_idx)) {
        return false;
    }

    Payload response = *request;
    response.result = static_cast<uint8_t>(RESULT_DB_ERROR);
    MySQL* const db = GetEventDatabase();

    if (request->operation == OPERATION_QUERY_ELIGIBILITY) {
        bool claimed = false;
        if (ReadClaim(db, request->account_id, claimed)) {
            response.result = static_cast<uint8_t>(
                claimed ? RESULT_ALREADY_CLAIMED : RESULT_UNCLAIMED);
        }
        return SendResult(dispatch_user_idx, response);
    }

    if (request->operation == OPERATION_RESERVE_CLAIM) {
        Result result = RESULT_DB_ERROR;
        if (ReserveClaim(db, *request, result)) {
            response.result = static_cast<uint8_t>(result);
        }
        return SendResult(-1, response);
    }

    if (request->operation == OPERATION_RELEASE_CLAIM) {
        if (ReleaseClaim(db, *request)) {
            response.result = static_cast<uint8_t>(RESULT_RELEASED);
        }
        return SendResult(-1, response);
    }

    return false;
}

}  // namespace event_all_user_gift_0828
