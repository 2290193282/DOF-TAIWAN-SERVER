#include "old_data/db/event/db_event_account_fatigue_stat.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"


namespace event_account_fatigue_stat {
namespace {

// DEPLOYMENT_SCHEMA_EVIDENCE: the authorized remote SHOW CREATE on 2026-08-09
// proves event_account_fatigue(m_id,fatigue), MyISAM/utf8, primary key m_id.
// This reader intentionally has no CREATE/ALTER fallback.
const char kAccountFatigueTable[] = "event_account_fatigue";

void InitResult(Result& result, uint32_t account_id) {
    result.account_id = account_id;
    result.fatigue_quantity = 0;
    result.result = static_cast<uint8_t>(kErrorOperationFailed);
    result.reserved[0] = 0;
    result.reserved[1] = 0;
    result.reserved[2] = 0;
}

bool LoadAccountFatigue(uint32_t account_id, uint32_t& fatigue_quantity) {
    fatigue_quantity = 0;
    if (!GlobalData::s_db_mgr || !account_id) {
        return false;
    }

    MySQL* const db = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!db ||
        !db->set_query(
            "seLect fatigue from %s where m_id=%u",
            kAccountFatigueTable,
            static_cast<unsigned int>(account_id)) ||
        !db->exec(true)) {
        return false;
    }

    const int row_count = db->get_n_rows();
    // A new account can legitimately have no row until its first fatigue
    // update.  More than one row violates the account-owner contract.
    if (row_count == 0) {
        return true;
    }
    if (row_count != 1 || !db->fetch()) {
        return false;
    }

    unsigned int quantity = 0;
    if (!db->get_uint(0, quantity)) {
        return false;
    }
    fatigue_quantity = static_cast<uint32_t>(quantity);
    return true;
}

bool SendResult(int user_idx, const Result& source) {
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
    // Compatibility payloads do not add specializations to the shared
    // original Stream template registry; the checked untyped span is local
    // to this fixed 12-byte ABI owner.
    Result* const result = reinterpret_cast<Result*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Result))));
    if (!result) {
        return false;
    }
    *result = source;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace


DB_EventAccountFatigueStat::DB_EventAccountFatigueStat()
    : DBDispatcher() {}

DB_EventAccountFatigueStat::~DB_EventAccountFatigueStat() {}

bool DB_EventAccountFatigueStat::makeRequest(int user_idx,
                                             uint32_t account_id) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || !account_id || !pool || !queues) {
        return false;
    }

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Result* const request = reinterpret_cast<Result*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Result))));
    if (!request) {
        return false;
    }
    InitResult(*request, account_id);
    return legacy_feature_fence::QueueAfterLegacy(
        legacy_feature_fence::kFenceEventAccountFatigueRead, guard);
}

bool DB_EventAccountFatigueStat::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream)) {
        return false;
    }
    if (stream->length() != static_cast<int>(sizeof(Result))) {
        return false;
    }

    const Result* const request =
        reinterpret_cast<const Result*>(stream->get_buffer());
    if (!request || !request->account_id) {
        return false;
    }

    Result response;
    InitResult(response, request->account_id);
    if (LoadAccountFatigue(response.account_id,
                           response.fatigue_quantity)) {
        response.result = 0;
    }
    return SendResult(user_idx, response);
}

}  // namespace event_account_fatigue_stat
