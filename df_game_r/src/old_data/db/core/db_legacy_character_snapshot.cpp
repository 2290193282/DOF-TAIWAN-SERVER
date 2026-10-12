#include "old_data/db/core/db_legacy_character_snapshot.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/event/db_exp_keeping_event.h"
#include "old_data/db/event/db_zombie_mode_result.h"

#include <cstddef>
#include <cstring>


namespace legacy_character_snapshot {
namespace {

const uint32_t kMagic = 0x8280828u;
const uint32_t kVersion = 1u;

uint32_t Checksum(const uint8_t* bytes, uint32_t size) {
    uint32_t value = 2166136261u;
    for (uint32_t index = 0u; index < size; ++index) {
        value ^= static_cast<uint32_t>(bytes[index]);
        value *= 16777619u;
    }
    return value;
}

bool EqualSnapshot(const CharacterSnapshot& lhs,
                   const CharacterSnapshot& rhs) {
    return lhs.level == rhs.level &&
           lhs.experience == rhs.experience &&
           lhs.remain_sp_first == rhs.remain_sp_first &&
           lhs.remain_sp_second == rhs.remain_sp_second &&
           lhs.remain_sfp_first == rhs.remain_sfp_first &&
           lhs.remain_sfp_second == rhs.remain_sfp_second;
}

template <typename T>
CharacterSnapshot CopySnapshot(const T& source) {
    CharacterSnapshot result;
    result.level = source.level;
    result.experience = source.experience;
    result.remain_sp_first = source.remain_sp_first;
    result.remain_sp_second = source.remain_sp_second;
    result.remain_sfp_first = source.remain_sfp_first;
    result.remain_sfp_second = source.remain_sfp_second;
    return result;
}

bool IsPlausiblePair(const CharacterSnapshot& baseline,
                     const CharacterSnapshot& target) {
    return baseline.level != 0u && baseline.level <= 85u &&
           target.level >= baseline.level && target.level <= 85u &&
           target.experience >= baseline.experience &&
           target.remain_sp_first >= baseline.remain_sp_first &&
           target.remain_sp_second >= baseline.remain_sp_second &&
           target.remain_sfp_first >= baseline.remain_sfp_first &&
           target.remain_sfp_second >= baseline.remain_sfp_second;
}

bool IsRepairableField(uint32_t current,
                       uint32_t baseline,
                       uint32_t target) {
    return current == baseline || current == target;
}

bool IsRepairableSnapshot(const CharacterSnapshot& current,
                          const CharacterSnapshot& baseline,
                          const CharacterSnapshot& target) {
    // A prior MyISAM attempt may have stopped between the three table
    // updates.  Accept only an exact baseline/target value per scalar; any
    // third value is newer or foreign state and must never be overwritten.
    return IsRepairableField(current.level, baseline.level, target.level) &&
           IsRepairableField(current.experience,
                             baseline.experience,
                             target.experience) &&
           IsRepairableField(current.remain_sp_first,
                             baseline.remain_sp_first,
                             target.remain_sp_first) &&
           IsRepairableField(current.remain_sp_second,
                             baseline.remain_sp_second,
                             target.remain_sp_second) &&
           IsRepairableField(current.remain_sfp_first,
                             baseline.remain_sfp_first,
                             target.remain_sfp_first) &&
           IsRepairableField(current.remain_sfp_second,
                             baseline.remain_sfp_second,
                             target.remain_sfp_second);
}

bool IsZeroTail(const Message& request) {
    if (request.continuation_size > kMaximumContinuationBytes) {
        return false;
    }
    for (uint32_t index = request.continuation_size;
         index < kMaximumContinuationBytes;
         ++index) {
        if (request.continuation[index] != 0u) {
            return false;
        }
    }
    return true;
}

bool ValidateExpKeeping(const Message& request) {
    if (request.feature_kind != kFeatureExpKeeping ||
        request.continuation_size !=
            sizeof(exp_keeping::Message)) {
        return false;
    }
    exp_keeping::Message continuation;
    std::memcpy(&continuation,
                request.continuation,
                sizeof(continuation));
    return continuation.account_id == request.account_id &&
           continuation.charac_no == request.charac_no &&
           continuation.server_group == request.server_group &&
           continuation.event_id == exp_keeping::kEventId &&
           continuation.operation ==
               exp_keeping::OPERATION_COMMIT &&
           continuation.claim_state ==
               exp_keeping::CLAIM_STATE_TARGET_STORED &&
           continuation.result == exp_keeping::RESULT_SUCCESS &&
           continuation.request_token != 0u &&
           (continuation.claim_token.low != 0u ||
            continuation.claim_token.high != 0u) &&
           EqualSnapshot(request.baseline,
                         CopySnapshot(continuation.baseline)) &&
           EqualSnapshot(request.target,
                         CopySnapshot(continuation.target));
}

bool ValidateZombieMode(const Message& request) {
    if (request.feature_kind != kFeatureZombieMode ||
        request.continuation_size != sizeof(zombie_mode::Message)) {
        return false;
    }
    zombie_mode::Message continuation;
    std::memcpy(&continuation,
                request.continuation,
                sizeof(continuation));
    return continuation.account_id == request.account_id &&
           continuation.charac_no == request.charac_no &&
           continuation.server_group == request.server_group &&
           continuation.reply_user_idx == request.reply_user_idx &&
           continuation.operation == zombie_mode::OPERATION_COMMIT &&
           continuation.claim_state ==
               zombie_mode::CLAIM_STATE_TARGET_STORED &&
           continuation.result == zombie_mode::RESULT_SUCCESS &&
           continuation.request_token != 0u &&
           continuation.round_token_hi != 0u &&
           continuation.round_token_lo != 0u &&
           (continuation.delivery_key_hi != 0u ||
            continuation.delivery_key_lo != 0u) &&
           EqualSnapshot(request.baseline,
                         CopySnapshot(continuation.baseline)) &&
           EqualSnapshot(request.target,
                         CopySnapshot(continuation.target));
}

bool ValidateRequest(const Message& request) {
    if (request.magic != kMagic || request.version != kVersion ||
        request.apply_status != kApplyRequest ||
        request.reply_user_idx < 0 || request.server_group < 0 ||
        request.server_group >= kMaximumServerGroupCount ||
        request.account_id == 0u || request.charac_no == 0u ||
        request.continuation_size == 0u ||
        request.continuation_size > kMaximumContinuationBytes ||
        !IsZeroTail(request) ||
        Checksum(request.continuation, request.continuation_size) !=
            request.continuation_checksum ||
        !IsPlausiblePair(request.baseline, request.target)) {
        return false;
    }
    return ValidateExpKeeping(request) || ValidateZombieMode(request);
}

template <typename FeatureMessage, typename FeatureSnapshot>
bool QueueApply(FeatureKind kind,
                int reply_user_idx,
                int server_group,
                uint32_t account_id,
                uint32_t charac_no,
                const FeatureSnapshot& baseline,
                const FeatureSnapshot& target,
                const FeatureMessage& continuation) {
    if (reply_user_idx < 0 || server_group < 0 ||
        server_group >= kMaximumServerGroupCount || account_id == 0u ||
        charac_no == 0u || sizeof(FeatureMessage) >
                               static_cast<size_t>(
                                   kMaximumContinuationBytes)) {
        return false;
    }
    Message request;
    std::memset(&request, 0, sizeof(request));
    request.magic = kMagic;
    request.version = kVersion;
    request.feature_kind = static_cast<uint32_t>(kind);
    request.apply_status = kApplyRequest;
    request.reply_user_idx = reply_user_idx;
    request.server_group = server_group;
    request.account_id = account_id;
    request.charac_no = charac_no;
    request.continuation_size = sizeof(FeatureMessage);
    request.baseline = CopySnapshot(baseline);
    request.target = CopySnapshot(target);
    std::memcpy(request.continuation,
                &continuation,
                sizeof(continuation));
    request.continuation_checksum =
        Checksum(request.continuation, request.continuation_size);
    if (!ValidateRequest(request)) {
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
    *guard << -1;
    if (guard.put_binary(&request, static_cast<int>(sizeof(request))) != 1) {
        return false;
    }
    return queues->put(MsgQueueMgr::DB_Q, guard) == 1;
}

MySQL* GetCanonicalDatabase(int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || DBMgr::IsFeatureDBThreadContext() ||
        server_group < 0 || server_group >= kMaximumServerGroupCount) {
        return 0;
    }
    return manager->GetDBHandleExact(
        DB_HANDLE_2, static_cast<ENUM_SERVER_GROUP>(server_group));
}

int LoadSnapshot(MySQL* database,
                 uint32_t account_id,
                 uint32_t charac_no,
                 CharacterSnapshot& snapshot) {
    if (!database ||
        !database->set_query(
            "seLect ci.lev,cs.exp,s.remain_sp,s.remain_sp_2nd,"
            "s.remain_sfp_1st,s.remain_sfp_2nd from charac_info ci "
            "inner join charac_stat cs on cs.charac_no=ci.charac_no "
            "inner join skill s on s.charac_no=ci.charac_no "
            "where ci.charac_no=%u and ci.m_id=%u and ci.delete_flag=0 "
            "limit 2",
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(account_id)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, snapshot.level) ||
        !database->get_uint(1, snapshot.experience) ||
        !database->get_uint(2, snapshot.remain_sp_first) ||
        !database->get_uint(3, snapshot.remain_sp_second) ||
        !database->get_uint(4, snapshot.remain_sfp_first) ||
        !database->get_uint(5, snapshot.remain_sfp_second)) {
        return -1;
    }
    return 1;
}

bool ApplyAndVerify(MySQL* database, const Message& request) {
    CharacterSnapshot current;
    if (LoadSnapshot(database,
                     request.account_id,
                     request.charac_no,
                     current) != 1 ||
        !IsRepairableSnapshot(current, request.baseline, request.target)) {
        return false;
    }

    const CharacterSnapshot& target = request.target;
    if (!database->set_query(
            "upDate charac_info set lev=%u where charac_no=%u and m_id=%u "
            "and delete_flag=0",
            static_cast<unsigned int>(target.level),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.account_id)) ||
        !database->exec(true) || database->getAffectedRowCount() > 1u ||
        !database->set_query(
            "upDate charac_stat set exp=%u where charac_no=%u",
            static_cast<unsigned int>(target.experience),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true) || database->getAffectedRowCount() > 1u ||
        !database->set_query(
            "upDate skill set remain_sp=%u,remain_sp_2nd=%u,"
            "remain_sfp_1st=%u,remain_sfp_2nd=%u where charac_no=%u",
            static_cast<unsigned int>(target.remain_sp_first),
            static_cast<unsigned int>(target.remain_sp_second),
            static_cast<unsigned int>(target.remain_sfp_first),
            static_cast<unsigned int>(target.remain_sfp_second),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true) || database->getAffectedRowCount() > 1u) {
        return false;
    }

    CharacterSnapshot verified;
    return LoadSnapshot(database,
                        request.account_id,
                        request.charac_no,
                        verified) == 1 &&
           EqualSnapshot(verified, target);
}

template <typename FeatureMessage>
bool QueueNativeContinuation(int reply_user_idx,
                             int inner_code,
                             const FeatureMessage& continuation) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues || reply_user_idx < 0) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, inner_code, reply_user_idx);
    FeatureMessage* const output = reinterpret_cast<FeatureMessage*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(FeatureMessage))));
    if (!output) {
        return false;
    }
    *output = continuation;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool QueueApplyAck(const Message& request, bool applied) {
    if (request.feature_kind == kFeatureExpKeeping) {
        exp_keeping::Message continuation;
        std::memcpy(&continuation,
                    request.continuation,
                    sizeof(continuation));
        continuation.operation = exp_keeping::OPERATION_FINALIZE;
        continuation.result = applied
                                  ? exp_keeping::RESULT_SUCCESS
                                  : exp_keeping::RESULT_DATABASE_FAILURE;
        continuation.replayed = 0u;
        return QueueNativeContinuation(request.reply_user_idx,
                                       exp_keeping::kTaiwanInnerCode,
                                       continuation);
    }
    if (request.feature_kind == kFeatureZombieMode) {
        zombie_mode::Message continuation;
        std::memcpy(&continuation,
                    request.continuation,
                    sizeof(continuation));
        continuation.operation = zombie_mode::OPERATION_FINALIZE;
        continuation.result = applied
                                  ? zombie_mode::RESULT_SUCCESS
                                  : zombie_mode::RESULT_DATABASE;
        continuation.replayed = 0u;
        return QueueNativeContinuation(request.reply_user_idx,
                                       zombie_mode::kTaiwanInnerCode,
                                       continuation);
    }
    return false;
}

}  // namespace


bool QueueExpKeepingApply(
    int reply_user_idx,
    const exp_keeping::Message& continuation) {
    return QueueApply(kFeatureExpKeeping,
                      reply_user_idx,
                      continuation.server_group,
                      continuation.account_id,
                      continuation.charac_no,
                      continuation.baseline,
                      continuation.target,
                      continuation);
}

bool QueueZombieModeApply(
    int reply_user_idx,
    const zombie_mode::Message& continuation) {
    return QueueApply(kFeatureZombieMode,
                      reply_user_idx,
                      continuation.server_group,
                      continuation.account_id,
                      continuation.charac_no,
                      continuation.baseline,
                      continuation.target,
                      continuation);
}

DB_LegacyCharacterSnapshot::DB_LegacyCharacterSnapshot()
    : DBDispatcher() {}

DB_LegacyCharacterSnapshot::~DB_LegacyCharacterSnapshot() {}

bool DB_LegacyCharacterSnapshot::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    if (signal != kDbSignal || user_idx != -1 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    Message request;
    std::memset(&request, 0, sizeof(request));
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !ValidateRequest(request)) {
        return false;
    }

    MySQL* const database = GetCanonicalDatabase(request.server_group);
    const bool applied = database && ApplyAndVerify(database, request);
    // A negative ACK is still a real DB execution result.  The native owner
    // keeps its durable target pending and returns a retryable failure to the
    // correlated game session; enqueueing work is never treated as success.
    return QueueApplyAck(request, applied);
}

}  // namespace legacy_character_snapshot_0828
