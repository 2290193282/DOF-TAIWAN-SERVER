#include "old_data/db/core/db_legacy_feature_fence.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/event/db_event_account_fatigue_stat.h"
#include "old_data/db/event/db_collect_box.h"
#include "old_data/db/event/db_decide_levelup_event.h"
#include "old_data/db/event/db_freely_give_item.h"
#include "old_data/db/event/db_shooting_star_dimension_pocket.h"
#include "old_data/db/event/db_sea_bottom_event.h"
#include "old_data/db/core/db_shutdown_quiescence.h"

#include <cstddef>
#include <cstring>


namespace legacy_feature_fence {
namespace {

const uint32_t kMagic = 0x8230828u;
const uint32_t kVersion = 1u;
const int kTaiwanOuterSignal = 846;

static_assert(8u + sizeof(decide_levelup_db::Message) <=
                  kMaximumForwardBytes,
              "decide-level finalize must fit the fixed DB_Q fence");

uint32_t Checksum(const unsigned char* bytes, uint32_t size) {
    uint32_t value = 2166136261u;
    for (uint32_t index = 0u; index < size; ++index) {
        value ^= static_cast<uint32_t>(bytes[index]);
        value *= 16777619u;
    }
    return value;
}

bool ReadInt32(const unsigned char* bytes,
               uint32_t size,
               uint32_t offset,
               int32_t& value) {
    if (!bytes || offset > size || size - offset < sizeof(value)) {
        return false;
    }
    std::memcpy(&value, bytes + offset, sizeof(value));
    return true;
}

Stream* AcquireFenceStream() {
    StreamPool* const default_pool = GlobalData::stream_pool();
    Stream* stream = default_pool
                         ? default_pool->Acquire(__FILE__, __LINE__)
                         : 0;
    if (!stream) {
        SmallStreamPool* const small_pool = GlobalData::small_stream_pool();
        stream = small_pool
                     ? small_pool->Acquire(__FILE__, __LINE__)
                     : 0;
    }
    return stream;
}

bool ValidateTarget(FenceKind kind,
                    const unsigned char* bytes,
                    uint32_t size) {
    int32_t signal = 0;
    int32_t user_idx = -1;
    if (!ReadInt32(bytes, size, 0u, signal) ||
        !ReadInt32(bytes, size, 4u, user_idx)) {
        return false;
    }

    // Shutdown is the sole fence kind whose target has no user owner.  Keep
    // the -1 sentinel out of every business branch and validate the complete
    // pointer-free ACK envelope before DB_Q is allowed to forward it.
    if (kind == kFenceShutdownQuiescence) {
        return db_shutdown_quiescence::ValidateFenceAckTarget(bytes,
                                                                   size);
    }
    if (user_idx < 0) {
        return false;
    }

    if (kind == kFenceFreelyGiveLevelUp) {
        int32_t inner = 0;
        if (signal != kTaiwanOuterSignal ||
            size != 12u + sizeof(freely_give_item_db::Message) ||
            !ReadInt32(bytes, size, 8u, inner) ||
            inner != freely_give_item_db::kTaiwanInnerCode) {
            return false;
        }
        freely_give_item_db::Message message;
        std::memcpy(&message, bytes + 12u, sizeof(message));
        return message.operation ==
               freely_give_item_db::OPERATION_LEVEL_UP;
    }
    if (kind == kFenceFreelyGiveInventoryMoneyCommit) {
        int32_t inner = 0;
        if (signal != kTaiwanOuterSignal ||
            size != 12u + sizeof(freely_give_item_db::Message) ||
            !ReadInt32(bytes, size, 8u, inner) ||
            inner != freely_give_item_db::kTaiwanInnerCode) {
            return false;
        }
        freely_give_item_db::Message message;
        std::memcpy(&message, bytes + 12u, sizeof(message));
        uint8_t pre_or = 0u;
        uint8_t post_or = 0u;
        for (unsigned int index = 0u; index < 32u; ++index) {
            pre_or = static_cast<uint8_t>(
                pre_or | message.pre_inventory_sha256[index]);
            post_or = static_cast<uint8_t>(
                post_or | message.post_inventory_sha256[index]);
        }
        const bool local_action =
            message.action == freely_give_item_db::ACTION_CLAIM_BOX ||
            message.action ==
                freely_give_item_db::ACTION_CLAIM_COUPON_LOCAL ||
            message.action == freely_give_item_db::ACTION_USE_COUPON ||
            message.action ==
                freely_give_item_db::ACTION_CONVERT_UNLIMITED ||
            message.action == freely_give_item_db::ACTION_RESET ||
            message.action ==
                freely_give_item_db::ACTION_SELECT_WEAPON;
        return (message.operation ==
                    freely_give_item_db::OPERATION_COMMIT_ACTION ||
                message.operation ==
                    freely_give_item_db::OPERATION_CANCEL_ACTION) &&
               local_action && message.account_id != 0u &&
               message.charac_no != 0u && message.session_token != 0u &&
               message.request_token != 0u && message.action_token != 0u &&
               message.server_group >= 0 && message.server_group < 18 &&
               message.pending_server_group == message.server_group &&
               message.pending_charac_no == message.charac_no &&
               message.pending_token == message.action_token &&
               message.pending_action == message.action &&
               message.proof_kind == freely_give_item_db::
                                         PERSISTENCE_PROOF_INVENTORY_MONEY &&
               ((message.operation == freely_give_item_db::
                                          OPERATION_COMMIT_ACTION &&
                 message.saga_state == freely_give_item_db::
                                           SAGA_STATE_STAGED) ||
                (message.operation == freely_give_item_db::
                                          OPERATION_CANCEL_ACTION &&
                 (message.saga_state == freely_give_item_db::
                                            SAGA_STATE_RESERVED ||
                  message.saga_state == freely_give_item_db::
                                            SAGA_STATE_STAGED))) &&
               pre_or != 0u && post_or != 0u &&
               message.reserved_tail[0] == 0u;
    }

    if (kind == kFenceEventAccountFatigueRead) {
        return signal == event_account_fatigue_stat::kDbSignal &&
               size == 8u + sizeof(event_account_fatigue_stat::Result);
    }
    if (kind == kFenceCollectBoxFinalize) {
        if (signal != collect_box_db::kDbSignal ||
            size != 8u + sizeof(collect_box_db::Message)) {
            return false;
        }
        collect_box_db::Message message;
        std::memcpy(&message, bytes + 8u, sizeof(message));
        return message.operation == collect_box_db::kOperationFinalize &&
               message.saga_operation >=
                   collect_box_db::kOperationPrepareAdd &&
               message.saga_operation <=
                   collect_box_db::kOperationPrepareExtend &&
               message.result == collect_box_db::kResultRequest &&
               message.phase == collect_box_db::kPhaseNone &&
               message.box_id == 1u && message.saga_id != 0u;
    }
    if (kind == kFenceDecideLevelupFinalize) {
        if (signal != decide_levelup_db::kDbSignal ||
            size != 8u + sizeof(decide_levelup_db::Message)) {
            return false;
        }
        decide_levelup_db::Message message;
        std::memcpy(&message, bytes + 8u, sizeof(message));
        return message.operation ==
                   decide_levelup_db::kOperationFinalizeRewards &&
               message.plan_state == decide_levelup_db::kPlanStatePrepared &&
               message.account_id != 0u && message.charac_no != 0u &&
               message.claim_token != 0u && message.session_token != 0u &&
               message.server_group >= 0 && message.server_group < 18 &&
               message.reward_count != 0u &&
               message.reward_count <=
                   decide_levelup_db::kMaximumRewardDeliveries &&
               message.mail_count <= message.reward_count;
    }
    if (kind == kFenceTwEvent720721InventoryCommit) {
        int32_t inner = 0;
        if (signal != kTaiwanOuterSignal ||
            size != 12u + sizeof(shooting_star_dimension_pocket_db::Message) ||
            !ReadInt32(bytes, size, 8u, inner) ||
            inner != shooting_star_dimension_pocket_db::kTaiwanInnerCode) {
            return false;
        }
        shooting_star_dimension_pocket_db::Message message;
        std::memcpy(&message, bytes + 12u, sizeof(message));
        return shooting_star_dimension_pocket_db::IsLegacyFencedInventoryCommit(
            message);
    }
    if (kind == kFenceTwSeaBottomInventoryCommit) {
        int32_t inner = 0;
        if (signal != kTaiwanOuterSignal ||
            size != 12u + sizeof(sea_bottom_event_db::Message) ||
            !ReadInt32(bytes, size, 8u, inner) ||
            inner != sea_bottom_event_db::kTaiwanInnerCode) {
            return false;
        }
        sea_bottom_event_db::Message message;
        std::memcpy(&message, bytes + 12u, sizeof(message));
        return sea_bottom_event_db::IsLegacyFencedInventoryCommit(message);
    }
    return false;
}

}  // namespace


bool QueueAfterLegacy(FenceKind kind, CStreamGuard& target_guard) {
    Stream* const target = target_guard.Get();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!target || !queues || target->length() <= 0 ||
        target->length() > kMaximumForwardBytes) {
        return false;
    }

    const uint32_t target_size = static_cast<uint32_t>(target->length());
    const unsigned char* const target_bytes =
        reinterpret_cast<const unsigned char*>(target->get_buffer());
    if (!ValidateTarget(kind, target_bytes, target_size)) {
        return false;
    }

    Stream* const wrapper = AcquireFenceStream();
    if (!wrapper) {
        return false;
    }
    CStreamGuard wrapper_guard(wrapper, true);
    *wrapper_guard << static_cast<int>(kDbSignal);
    *wrapper_guard << -1;

    Header header;
    header.magic = kMagic;
    header.version = kVersion;
    header.fence_kind = static_cast<uint32_t>(kind);
    header.target_size = target_size;
    header.target_checksum = Checksum(target_bytes, target_size);
    if (wrapper_guard.put_binary(&header, static_cast<int>(sizeof(header))) !=
            1 ||
        wrapper_guard.put_binary(
            const_cast<unsigned char*>(target_bytes),
            static_cast<int>(target_size)) != 1) {
        return false;
    }
    return queues->put(MsgQueueMgr::DB_Q, wrapper_guard) == 1;
}

DB_LegacyFeatureFence::DB_LegacyFeatureFence() : DBDispatcher() {}

DB_LegacyFeatureFence::~DB_LegacyFeatureFence() {}

bool DB_LegacyFeatureFence::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    if (signal != kDbSignal || user_idx != -1 || !stream ||
        stream->length() < static_cast<int>(sizeof(Header))) {
        return false;
    }

    Header header;
    if (stream->get_binary(&header, static_cast<int>(sizeof(header))) != 1 ||
        header.magic != kMagic || header.version != kVersion ||
        header.target_size == 0u ||
        header.target_size > kMaximumForwardBytes ||
        stream->length() != static_cast<int>(header.target_size)) {
        return false;
    }

    unsigned char target_bytes[kMaximumForwardBytes];
    std::memset(target_bytes, 0, sizeof(target_bytes));
    if (stream->get_binary(target_bytes,
                           static_cast<int>(header.target_size)) != 1 ||
        Checksum(target_bytes, header.target_size) != header.target_checksum ||
        !ValidateTarget(static_cast<FenceKind>(header.fence_kind),
                        target_bytes,
                        header.target_size)) {
        return false;
    }

    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!queues) {
        return false;
    }
    Stream* const target = AcquireFenceStream();
    if (!target) {
        return false;
    }
    CStreamGuard target_guard(target, true);
    if (target_guard.put_binary(target_bytes,
                                static_cast<int>(header.target_size)) != 1) {
        return false;
    }
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, target_guard) == 1;
}

}  // namespace legacy_native_fence_0828
