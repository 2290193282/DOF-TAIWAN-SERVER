#include "old_data/db/event/db_take_magic_lamp.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstring>

namespace magic_lamp {
namespace {

// SQL014 owns this activity independently.  event_1306_account_reward belongs
// to CMD420 and sharing its PRIMARY KEY(m_id) would make one event consume the
// other event's account-wide claim.
const char kClaimTable[] = "magic_lamp_claim_0828";
const uint32_t kMagicLampItemId = 690001710u;
const unsigned int kRetryLeaseSeconds = 60u;

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

void InitMessage(ClaimMessage& message,
                 uint32_t account_id,
                 uint32_t request_charac_no,
                 uint32_t reward_charac_no,
                 uint32_t reward_item_id,
                 ClaimOperation operation,
                 int request_server_group,
                 int reward_server_group) {
    std::memset(&message, 0, sizeof(message));
    message.account_id = account_id;
    message.request_charac_no = request_charac_no;
    message.reward_charac_no = reward_charac_no;
    message.reward_item_id = reward_item_id;
    message.request_server_group = request_server_group;
    message.reward_server_group = reward_server_group;
    message.operation = static_cast<uint8_t>(operation);
    message.result = static_cast<uint8_t>(CLAIM_DATABASE_FAILURE);
}

bool QueueRequest(int user_idx, const ClaimMessage& source) {
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
    ClaimMessage* const request = reinterpret_cast<ClaimMessage*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(ClaimMessage))));
    if (!request) {
        return false;
    }
    *request = source;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool SendResult(int user_idx, const ClaimMessage& source) {
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
    ClaimMessage* const result = reinterpret_cast<ClaimMessage*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(ClaimMessage))));
    if (!result) {
        return false;
    }
    *result = source;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

DB_TakeMagicLamp::DB_TakeMagicLamp() : DBDispatcher() {}

DB_TakeMagicLamp::~DB_TakeMagicLamp() {}

bool DB_TakeMagicLamp::makeReserveRequest(int user_idx,
                                              uint32_t account_id,
                                              uint32_t charac_no,
                                              uint32_t reward_item_id,
                                              int server_group) {
    if (user_idx == -1 || account_id == 0u || charac_no == 0u ||
        reward_item_id != kMagicLampItemId ||
        !IsValidServerGroup(server_group)) {
        return false;
    }
    ClaimMessage request;
    InitMessage(request,
                account_id,
                charac_no,
                charac_no,
                reward_item_id,
                CLAIM_OPERATION_RESERVE,
                server_group,
                server_group);
    return QueueRequest(user_idx, request);
}

bool DB_TakeMagicLamp::makeMarkMailPersistedRequest(
    uint32_t account_id,
    uint32_t reward_charac_no,
    uint32_t reward_item_id,
    int server_group) {
    if (account_id == 0u || reward_charac_no == 0u ||
        reward_item_id != kMagicLampItemId ||
        !IsValidServerGroup(server_group)) {
        return false;
    }
    ClaimMessage request;
    InitMessage(request,
                account_id,
                0u,
                reward_charac_no,
                reward_item_id,
                CLAIM_OPERATION_MARK_MAIL_PERSISTED,
                -1,
                server_group);
    // This is a one-way idempotent transition after signal 128 reported a
    // successful postal exec(true).  No live CUser is required.
    return QueueRequest(-1, request);
}

bool DB_TakeMagicLamp::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(ClaimMessage))) {
        return false;
    }
    const ClaimMessage* const request =
        reinterpret_cast<const ClaimMessage*>(stream->get_buffer());
    if (!request || request->account_id == 0u ||
        request->reward_charac_no == 0u ||
        request->reward_item_id != kMagicLampItemId ||
        !IsValidServerGroup(request->reward_server_group) ||
        (request->operation == CLAIM_OPERATION_RESERVE &&
         !IsValidServerGroup(request->request_server_group)) ||
        !GlobalData::s_db_mgr) {
        return false;
    }

    MySQL* const db = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));

    if (request->operation == CLAIM_OPERATION_MARK_MAIL_PERSISTED) {
        if (!db || user_idx != -1 || request->request_charac_no != 0u ||
            !db->set_query(
                "upDate %s set claim_state=1,mail_submitted_at=now() "
                "where m_id=%u and charac_no=%u and server_group=%d "
                "and reward_item_id=%u "
                "and claim_state=0",
                kClaimTable,
                static_cast<unsigned int>(request->account_id),
                static_cast<unsigned int>(request->reward_charac_no),
                request->reward_server_group,
                static_cast<unsigned int>(request->reward_item_id)) ||
            !db->exec(true)) {
            return false;
        }
        if (db->getAffectedRowCount() == 1u) {
            return true;
        }
        // A repeated mark is idempotent, but only when the exact claim row is
        // already in state 1.  Any other zero-row update is an integrity error.
        if (!db->set_query(
                "seLect claim_state from %s where m_id=%u and charac_no=%u "
                "and server_group=%d and reward_item_id=%u",
                kClaimTable,
                static_cast<unsigned int>(request->account_id),
                static_cast<unsigned int>(request->reward_charac_no),
                request->reward_server_group,
                static_cast<unsigned int>(request->reward_item_id)) ||
            !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
            return false;
        }
        unsigned int state = 0u;
        return db->get_uint(0, state) && state == 1u;
    }

    if (request->operation != CLAIM_OPERATION_RESERVE || user_idx == -1 ||
        request->request_charac_no == 0u ||
        request->request_charac_no != request->reward_charac_no ||
        request->request_server_group != request->reward_server_group) {
        return false;
    }

    ClaimMessage response = *request;
    response.result = static_cast<uint8_t>(CLAIM_DATABASE_FAILURE);
    if (!db ||
        !db->set_query(
            "inSert ignore into %s"
            "(m_id,charac_no,server_group,reward_item_id,claim_state,claimed_at) "
            "values(%u,%u,%d,%u,0,now())",
            kClaimTable,
            static_cast<unsigned int>(request->account_id),
            static_cast<unsigned int>(request->request_charac_no),
            request->request_server_group,
            static_cast<unsigned int>(request->reward_item_id)) ||
        !db->exec(true)) {
        return SendResult(user_idx, response);
    }
    if (db->getAffectedRowCount() == 1u) {
        response.result = static_cast<uint8_t>(CLAIM_GRANTED);
        return SendResult(user_idx, response);
    }

    // Acquire a bounded retry lease before re-sending a reserved reward.
    // This collapses duplicate clicks/replayed packets while still allowing a
    // process-loss reservation to recover without operator DML.
    if (!db->set_query(
            "upDate %s set claimed_at=now() where m_id=%u and claim_state=0 "
            "and claimed_at<=date_sub(now(),interval %u second)",
            kClaimTable,
            static_cast<unsigned int>(request->account_id),
            kRetryLeaseSeconds) ||
        !db->exec(true) || db->getAffectedRowCount() > 1u) {
        return SendResult(user_idx, response);
    }
    const bool retry_lease_acquired = db->getAffectedRowCount() == 1u;

    if (!db->set_query(
            "seLect charac_no,server_group,reward_item_id,claim_state from %s "
            "where m_id=%u",
            kClaimTable,
            static_cast<unsigned int>(request->account_id)) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return SendResult(user_idx, response);
    }
    unsigned int stored_charac_no = 0u;
    unsigned int stored_server_group = 0u;
    unsigned int stored_item_id = 0u;
    unsigned int stored_state = 0u;
    if (!db->get_uint(0, stored_charac_no) ||
        !db->get_uint(1, stored_server_group) ||
        !db->get_uint(2, stored_item_id) ||
        !db->get_uint(3, stored_state) || stored_charac_no == 0u ||
        !IsValidServerGroup(static_cast<int>(stored_server_group)) ||
        stored_item_id != kMagicLampItemId || stored_state > 1u) {
        return SendResult(user_idx, response);
    }
    response.reward_charac_no = static_cast<uint32_t>(stored_charac_no);
    response.reward_server_group = static_cast<int32_t>(stored_server_group);
    response.reward_item_id = static_cast<uint32_t>(stored_item_id);
    response.result = static_cast<uint8_t>(
        stored_state == 1u
            ? CLAIM_ALREADY_SUBMITTED
            : (retry_lease_acquired ? CLAIM_RETRY_PENDING
                                    : CLAIM_PENDING_RECENT));
    return SendResult(user_idx, response);
}

}  // namespace magic_lamp_0828
