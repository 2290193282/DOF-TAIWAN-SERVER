#include "old_game/activity/schedule/decide_levelup_event.h"

#include "common/manager/global_lifecycle.h"
#include "common/config.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_decide_levelup_event.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/checksum.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace {

const char kDecideLevelupPvf[] = "decidelevelupevent.evt";

struct EventState {
    EventState()
        : charac_no(0), target_level(0), last_reward_level(0),
          session_token(0), delivered_claim_token(0),
          user_idx(-1), main_reward_decided(false), loaded(false),
          db_pending(false), prepared_plan() {
        std::memset(&prepared_plan, 0, sizeof(prepared_plan));
    }

    uint32_t charac_no;
    uint32_t target_level;
    uint32_t last_reward_level;
    uint32_t session_token;
    uint32_t delivered_claim_token;
    int user_idx;
    bool main_reward_decided;
    bool loaded;
    bool db_pending;
    decide_levelup_db::Message prepared_plan;
};

struct Reward {
    Reward() : level(0), item_id(0), count(0), flag(0) {}

    int level;
    int item_id;
    int count;
    // EVIDENCE_BLOCKED: the fourth PVF value is consistently 0/1 and is
    // retained verbatim.  The 0828 client packet state machine does not
    // expose a server-side delivery branch for it, so it must not be
    // invented as a mail/direct-inventory selector.
    int flag;
};

typedef std::map<int, std::vector<Reward> > RewardMap;
typedef std::map<uint32_t, EventState> StateMap;

StateMap& States() {
    static StateMap states;
    return states;
}

uint32_t NextSessionToken() {
    static uint32_t next_token = 0;
    ++next_token;
    if (next_token == 0u) {
        ++next_token;
    }
    return next_token;
}

bool IsAllowedTarget(uint32_t target_level) {
    return target_level == 20u || target_level == 30u ||
           target_level == 40u || target_level == 50u ||
           target_level == 60u || target_level == 70u;
}

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

class DecideLevelupConfig {
public:
    DecideLevelupConfig() : loaded_(false), main_rewards_(), sub_rewards_() {}

    bool available() { return Load(); }

    const std::vector<Reward>* MainRewards(int level) {
        return Find(main_rewards_, level);
    }

    const std::vector<Reward>* SubRewards(int level) {
        return Find(sub_rewards_, level);
    }

private:
    static const std::vector<Reward>* Find(const RewardMap& rewards,
                                           int level) {
        const RewardMap::const_iterator found = rewards.find(level);
        return found == rewards.end() ? 0 : &found->second;
    }

    static bool ParseRewardBlock(const std::string& closing_tag,
                                 RewardMap& destination) {
        bool is_type = false;
        bool success = false;
        std::string token;
        for (;;) {
            Reward reward;
            reward.level = ScanTypeOrInt(is_type, token, success, true);
            if (is_type) {
                return token == closing_tag;
            }
            if (!success) {
                return false;
            }
            reward.item_id = ScanInt(&success);
            if (!success) {
                return false;
            }
            reward.count = ScanInt(&success);
            if (!success) {
                return false;
            }
            reward.flag = ScanInt(&success);
            if (!success || reward.level <= 0 || reward.item_id <= 0 ||
                reward.count <= 0 || (reward.flag != 0 && reward.flag != 1)) {
                return false;
            }
            destination[reward.level].push_back(reward);
        }
    }

    bool Load() {
        if (loaded_) {
            return true;
        }
        if (!loadRDARScriptFile("Event/", kDecideLevelupPvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        RewardMap main_rewards;
        RewardMap sub_rewards;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            if (tag == "[Main Reward]") {
                if (!ParseRewardBlock("[/Main Reward]", main_rewards)) {
                    return false;
                }
            } else if (tag == "[Sub Reward]") {
                if (!ParseRewardBlock("[/Sub Reward]", sub_rewards)) {
                    return false;
                }
            }
        }

        static const int kTargets[] = {20, 30, 40, 50, 60, 70};
        for (unsigned int index = 0;
             index < sizeof(kTargets) / sizeof(kTargets[0]);
             ++index) {
            if (main_rewards.find(kTargets[index]) == main_rewards.end()) {
                return false;
            }
            const RewardMap::const_iterator sub =
                sub_rewards.find(kTargets[index]);
            const size_t same_level_count =
                main_rewards[kTargets[index]].size() +
                (sub == sub_rewards.end() ? 0u : sub->second.size());
            if (same_level_count >
                static_cast<size_t>(
                    decide_levelup_db::kMaximumRewardDeliveries)) {
                // A durable plan may be split between levels, never within
                // one PVF level (which would advance the watermark early).
                return false;
            }
        }
        for (RewardMap::const_iterator sub = sub_rewards.begin();
             sub != sub_rewards.end(); ++sub) {
            if (sub->second.size() >
                static_cast<size_t>(
                    decide_levelup_db::kMaximumRewardDeliveries)) {
                return false;
            }
        }
        if (sub_rewards.find(20) == sub_rewards.end() ||
            sub_rewards.find(85) == sub_rewards.end()) {
            return false;
        }

        main_rewards_.swap(main_rewards);
        sub_rewards_.swap(sub_rewards);
        loaded_ = true;
        return true;
    }

    bool loaded_;
    RewardMap main_rewards_;
    RewardMap sub_rewards_;
};

DecideLevelupConfig& Config() {
    static DecideLevelupConfig config;
    return config;
}

void InitMessage(decide_levelup_db::Message& message,
                 CUser* user,
                 uint8_t operation) {
    std::memset(&message, 0, sizeof(message));
    message.account_id = user && user->get_acc_id() > 0
                             ? static_cast<uint32_t>(user->get_acc_id())
                             : 0u;
    StateMap::const_iterator state = States().find(message.account_id);
    message.session_token = state != States().end()
                                ? state->second.session_token
                                : 0u;
    message.server_group = user ? user->GetServerGroup() : -1;
    message.operation = operation;
    message.result = static_cast<uint8_t>(
        decide_levelup_db::kResultOperationFailed);
}

EventState* FindState(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return 0;
    }
    StateMap::iterator found =
        States().find(static_cast<uint32_t>(user->get_acc_id()));
    return found == States().end() ? 0 : &found->second;
}

bool IsCurrentResponse(CUser* user,
                       const decide_levelup_db::Message& message) {
    EventState* const state = FindState(user);
    return state && message.session_token != 0u &&
           state->session_token == message.session_token &&
           message.server_group == user->GetServerGroup();
}

void ApplyState(const decide_levelup_db::Message& message,
                bool finalized_current_level) {
    StateMap::iterator found = States().find(message.account_id);
    if (found == States().end() ||
        found->second.session_token != message.session_token) {
        return;
    }
    EventState& state = found->second;
    state.charac_no = message.charac_no;
    state.target_level = message.target_level;
    state.last_reward_level = finalized_current_level
                                  ? message.current_reward_level
                                  : message.previous_reward_level;
    state.main_reward_decided = message.main_reward_decided != 0;
    state.loaded = true;
    if (message.plan_state == decide_levelup_db::kPlanStatePrepared) {
        state.prepared_plan = message;
    } else if (finalized_current_level) {
        std::memset(&state.prepared_plan, 0,
                    sizeof(state.prepared_plan));
    }
}

void SendStateNoti(CUser* user, const EventState& state) {
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: NOTI493 handler sub_5EEFB0 consumes exactly u32
    // target level followed by u8 main-reward-decided.
    packet.put_header(0, ENUM_NOTIPACKET_DECIDE_LEVEL);
    packet.put_int(static_cast<int>(state.target_level));
    packet.put_byte(state.main_reward_decided ? 1 : 0);
    packet.finalize(true);
    user->Send(packet);
}

void SendDecideSuccess(CUser* user, uint32_t target_level) {
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: CMD561 handler sub_5EEDE0 reads one u32 only on
    // the framework-success branch.
    packet.put_header(1, ENUM_CMDPACKET_DECIDE_LEVELUP);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(target_level));
    packet.finalize(true);
    user->Send(packet);
}

void SendDecideError(CUser* user, uint8_t result) {
    if (!user) {
        return;
    }
    uint8_t error = result;
    if (error != decide_levelup_db::kResultInvalidTarget &&
        error != decide_levelup_db::kResultAlreadyDecided &&
        error != decide_levelup_db::kResultReservationRequired) {
        error = static_cast<uint8_t>(
            decide_levelup_db::kResultInvalidTarget);
    }
    user->SendCmdErrorPacket(ENUM_CMDPACKET_DECIDE_LEVELUP, error);
}

void SendMainRewardDecidedNoti(CUser* user) {
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: NOTI494 handler sub_5EF000 has no packet reader.
    packet.put_header(0, ENUM_NOTIPACKET_DECIDE_MAIN_REWARD);
    packet.finalize(true);
    user->Send(packet);
}

bool HashMainInventory(const CInventory* inventory, uint8_t digest[32]) {
    if (!inventory || !digest) return false;
    std::vector<unsigned char> raw(
        static_cast<size_t>(oldsrv::config::kMainInventoryItemCount) *
        sizeof(Inven_Item));
    for (int index = 0; index < oldsrv::config::kMainInventoryItemCount;
         ++index) {
        const Inven_Item* const item = inventory->GetInvenRef(1, index);
        if (!item) return false;
        std::memcpy(&raw[static_cast<size_t>(index) * sizeof(Inven_Item)],
                    item, sizeof(Inven_Item));
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
               CHECKSUM_RESULT_OK;
}

bool HashRewardPlan(const decide_levelup_db::Message& message,
                    uint8_t digest[32]) {
    if (!digest || message.reward_count == 0u ||
        message.reward_count > decide_levelup_db::kMaximumRewardDeliveries) {
        return false;
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               reinterpret_cast<const unsigned char*>(message.deliveries),
               static_cast<int>(message.reward_count *
                                sizeof(decide_levelup_db::RewardDelivery)),
               digest,
               32) == CHECKSUM_RESULT_OK;
}

bool SameItemExceptStackCount(const Inven_Item& before,
                              const Inven_Item& after) {
    const size_t count_offset = offsetof(Inven_Item, add_info_);
    const size_t count_end = count_offset + sizeof(before.add_info_);
    return std::memcmp(&before, &after, count_offset) == 0 &&
           std::memcmp(reinterpret_cast<const uint8_t*>(&before) + count_end,
                       reinterpret_cast<const uint8_t*>(&after) + count_end,
                       sizeof(Inven_Item) - count_end) == 0;
}

bool IsDeterministicPrimaryStackMerge(const CInventory& before,
                                      const CInventory& after,
                                      int projected_slot,
                                      uint32_t item_id,
                                      uint32_t count) {
    if (projected_slot < 0 ||
        projected_slot >=
            oldsrv::config::kMainInventoryPrimaryItemCount ||
        item_id == 0u || item_id > 0x7fffffffu || count == 0u ||
        count > 0x7fffffffu) {
        return false;
    }
    bool saw_expected_delta = false;
    for (int slot = 0; slot < oldsrv::config::kMainInventoryItemCount;
         ++slot) {
        const Inven_Item* const old_item = before.GetInvenRef(1, slot);
        const Inven_Item* const new_item = after.GetInvenRef(1, slot);
        if (!old_item || !new_item) return false;
        if (std::memcmp(old_item, new_item, sizeof(Inven_Item)) == 0) continue;
        if (slot != projected_slot || saw_expected_delta ||
            old_item->item_id_ != static_cast<int32_t>(item_id) ||
            new_item->item_id_ != static_cast<int32_t>(item_id) ||
            old_item->add_info_ <= 0 ||
            count > static_cast<uint32_t>(
                        std::numeric_limits<int32_t>::max() -
                        old_item->add_info_) ||
            new_item->add_info_ !=
                old_item->add_info_ + static_cast<int32_t>(count) ||
            !SameItemExceptStackCount(*old_item, *new_item)) {
            return false;
        }
        saw_expected_delta = true;
    }
    return saw_expected_delta;
}

bool DeliveryTokenIsZero(const uint8_t token[16]) {
    if (!token) return true;
    for (size_t index = 0u; index < 16u; ++index) {
        if (token[index] != 0u) return false;
    }
    return true;
}

bool DeliveryTokenUnique(const decide_levelup_db::Message& message,
                         const uint8_t token[16]) {
    if (!token || DeliveryTokenIsZero(token)) return false;
    for (uint8_t index = 0u; index < message.reward_count; ++index) {
        if (std::memcmp(message.deliveries[index].delivery_token,
                        token, 16u) == 0) {
            return false;
        }
    }
    return true;
}

bool MakeDeliveryToken(const decide_levelup_db::Message& plan,
                       uint32_t ordinal,
                       uint32_t item_id,
                       uint32_t count,
                       uint32_t collision_salt,
                       uint8_t token[16]) {
    if (!token) return false;
    const uint32_t words[] = {
        plan.account_id,
        plan.charac_no,
        plan.claim_token,
        plan.current_reward_level,
        static_cast<uint32_t>(plan.server_group),
        ordinal,
        item_id,
        count,
        collision_salt,
        0x8440828u,
    };
    uint8_t digest[32] = {};
    CNChecksumSha256 checksum;
    if (checksum.MakeChecksumFromMemory(
            reinterpret_cast<const unsigned char*>(words),
            static_cast<int>(sizeof(words)), digest, 32) !=
        CHECKSUM_RESULT_OK) {
        return false;
    }
    std::memcpy(token, digest, 16u);
    return !DeliveryTokenIsZero(token);
}

void StoreMailProjection(decide_levelup_db::RewardDelivery& delivery,
                         const InvenItemMailRecord& record) {
    delivery.seal_flag = record.seal_flag;
    delivery.upgrade = record.item_attr;
    delivery.amplify_option = record.amplify_option;
    delivery.endurance = record.endurance;
    delivery.amplify_value = record.amplify_value;
    delivery.separate_upgrade = record.separate_upgrade;
    delivery.avatar_flag = record.avatar_flag ? 1u : 0u;
    delivery.creature_flag = record.creature_flag ? 1u : 0u;
    delivery.reserved = 0u;
    delivery.extend_info = record.extend_info;
    delivery.chronicle_value = record.chronicle_value;
    std::memcpy(delivery.item_guid, record.item_guid,
                sizeof(delivery.item_guid));
    std::memcpy(delivery.random_option, record.random_option,
                sizeof(delivery.random_option));
}

void LoadMailProjection(const decide_levelup_db::RewardDelivery& delivery,
                        InvenItemMailRecord& record) {
    std::memset(&record, 0, sizeof(record));
    record.seal_flag = delivery.seal_flag;
    record.item_id = delivery.item_id;
    record.add_info = delivery.count;
    record.item_attr = delivery.upgrade;
    record.endurance = delivery.endurance;
    record.extend_info = delivery.extend_info;
    std::memcpy(record.item_guid, delivery.item_guid,
                sizeof(record.item_guid));
    record.amplify_option = delivery.amplify_option;
    record.amplify_value = delivery.amplify_value;
    record.chronicle_value = delivery.chronicle_value;
    std::memcpy(record.random_option, delivery.random_option,
                sizeof(record.random_option));
    record.separate_upgrade = delivery.separate_upgrade;
    record.avatar_flag = delivery.avatar_flag != 0u;
    record.creature_flag = delivery.creature_flag != 0u;
}

bool AppendRewardPlan(decide_levelup_db::Message& plan,
                      CInventory& shadow,
                      const Reward& reward) {
    if (reward.item_id <= 0 || reward.count <= 0 ||
        plan.reward_count >= decide_levelup_db::kMaximumRewardDeliveries) {
        return false;
    }
    decide_levelup_db::RewardDelivery& delivery =
        plan.deliveries[plan.reward_count];
    std::memset(&delivery, 0, sizeof(delivery));
    delivery.item_id = static_cast<uint32_t>(reward.item_id);
    delivery.count = static_cast<uint32_t>(reward.count);

    if (reward.item_id == 1) {
        if (plan.event_coin_after > static_cast<uint32_t>(
                std::numeric_limits<int>::max() - reward.count)) {
            return false;
        }
        plan.event_coin_after += static_cast<uint32_t>(reward.count);
        delivery.kind = decide_levelup_db::kDeliveryEventCoin;
    } else {
        CItem* const prototype = G_CDataManager()
                                     ? G_CDataManager()->find_item(
                                           reward.item_id)
                                     : 0;
        if (!prototype || !prototype->vtable_) return false;
        Inven_Item item;
        item.item_id_ = prototype->get_index();
        prototype->vtable_->make_item(prototype, item);
        if (prototype->is_stackable()) item.set_add_info(reward.count);

        // Only deterministic stackables are projected as direct inventory
        // mutations.  Equipment/creature instances can carry generated state
        // and therefore use the tracked, idempotent postal path.
        int projected_slot = -1;
        InventoryMemory candidate_memory;
        CInventory candidate;
        candidate.SetInventoryMemory(&candidate_memory);
        if (prototype->is_stackable() && !item.IsCreatureItemType()) {
            if (!candidate.setCopy(shadow)) return false;
            projected_slot = candidate.insertItemIntoInventory(
                item, static_cast<eItemAddReason>(13), false, false);
        }
        // Signal 35 writes the primary inventory/EventCoin row before its
        // separate expanded-inventory statement.  Keep direct rewards in the
        // primary image so a failed second statement cannot leave a half-plan
        // that is neither the pre-image nor the replayable post-image.
        if (IsDeterministicPrimaryStackMerge(
                shadow, candidate, projected_slot, delivery.item_id,
                delivery.count)) {
            if (!shadow.setCopy(candidate)) return false;
            delivery.kind = decide_levelup_db::kDeliveryDirectInventory;
        } else {
            delivery.kind = decide_levelup_db::kDeliveryTrackedMail;
            // Capture the exact request projection once.  Equipment GUIDs
            // and usable-period stackables are deliberately not regenerated
            // on replay; the same token therefore always carries the same
            // payload SHA to signal 128's idempotent consumer.
            item.set_add_info(static_cast<int>(delivery.count));
            InvenItemMailRecord record;
            std::memset(&record, 0, sizeof(record));
            item.ExportMailRecord(record);
            if (record.item_id != delivery.item_id ||
                record.add_info != delivery.count) {
                return false;
            }
            StoreMailProjection(delivery, record);
            uint32_t collision_salt = 0u;
            for (;;) {
                if (!MakeDeliveryToken(
                        plan,
                        static_cast<uint32_t>(plan.reward_count) + 1u,
                        delivery.item_id,
                        delivery.count,
                        collision_salt,
                        delivery.delivery_token)) {
                    return false;
                }
                if (DeliveryTokenUnique(plan, delivery.delivery_token)) break;
                ++collision_salt;
                if (collision_salt == 0u) return false;
            }
            if (plan.mail_count == 0xffu) return false;
            ++plan.mail_count;
        }
    }
    ++plan.reward_count;
    return true;
}

bool AppendRewardList(decide_levelup_db::Message& plan,
                      CInventory& shadow,
                      const std::vector<Reward>* rewards) {
    if (!rewards) return true;
    for (std::vector<Reward>::const_iterator reward = rewards->begin();
         reward != rewards->end(); ++reward) {
        if (!AppendRewardPlan(plan, shadow, *reward)) return false;
    }
    return true;
}

bool HasRewardInRange(const EventState& state, uint32_t current_level) {
    if (current_level <= state.last_reward_level) return false;
    for (uint32_t level = state.last_reward_level + 1u;
         level <= current_level && level <= 255u; ++level) {
        const std::vector<Reward>* const rewards =
            Config().SubRewards(static_cast<int>(level));
        if (rewards && !rewards->empty()) return true;
    }
    const std::vector<Reward>* const main_rewards =
        Config().MainRewards(static_cast<int>(state.target_level));
    return !state.main_reward_decided &&
           state.last_reward_level < state.target_level &&
           current_level >= state.target_level && main_rewards &&
           !main_rewards->empty();
}

uint32_t ChooseRewardBatchEnd(const EventState& state,
                              uint32_t current_level) {
    if (current_level <= state.last_reward_level ||
        current_level > 255u || !IsAllowedTarget(state.target_level)) {
        return 0u;
    }
    size_t reward_count = 0u;
    uint32_t batch_end = state.last_reward_level;
    for (uint32_t level = state.last_reward_level + 1u;
         level <= current_level; ++level) {
        size_t level_count = 0u;
        const std::vector<Reward>* const sub =
            Config().SubRewards(static_cast<int>(level));
        if (sub) level_count += sub->size();
        if (!state.main_reward_decided &&
            state.last_reward_level < state.target_level &&
            level == state.target_level) {
            const std::vector<Reward>* const main =
                Config().MainRewards(static_cast<int>(state.target_level));
            if (main) level_count += main->size();
        }
        if (level_count > static_cast<size_t>(
                              decide_levelup_db::
                                  kMaximumRewardDeliveries) ||
            reward_count + level_count >
                static_cast<size_t>(
                    decide_levelup_db::kMaximumRewardDeliveries)) {
            break;
        }
        reward_count += level_count;
        batch_end = level;
    }
    return reward_count != 0u ? batch_end : 0u;
}

bool BuildRewardPlan(CUser* user,
                     const decide_levelup_db::Message& reservation,
                     decide_levelup_db::Message& plan) {
    const CInventory* const inventory = user ? user->getCurCharacInvenR() : 0;
    if (!inventory || !G_CDataManager()) return false;
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    if (!shadow.setCopy(*inventory)) return false;

    plan = reservation;
    plan.operation = decide_levelup_db::kOperationPrepareRewardPlan;
    plan.result = decide_levelup_db::kResultOperationFailed;
    plan.plan_state = decide_levelup_db::kPlanStatePrepared;
    plan.reward_count = 0u;
    plan.mail_count = 0u;
    plan.mail_persisted = 0u;
    std::memset(plan.ack_delivery_token, 0,
                sizeof(plan.ack_delivery_token));
    plan.ack_reward_item_id = 0u;
    std::memset(plan.inventory_pre_sha256, 0,
                sizeof(plan.inventory_pre_sha256));
    std::memset(plan.inventory_post_sha256, 0,
                sizeof(plan.inventory_post_sha256));
    std::memset(plan.reward_plan_sha256, 0,
                sizeof(plan.reward_plan_sha256));
    std::memset(plan.deliveries, 0, sizeof(plan.deliveries));
    const int event_coin = inventory->GetEventCoin();
    if (event_coin < 0 ||
        !HashMainInventory(inventory, plan.inventory_pre_sha256)) {
        return false;
    }
    plan.event_coin_before = static_cast<uint32_t>(event_coin);
    plan.event_coin_after = static_cast<uint32_t>(event_coin);

    for (uint32_t level = reservation.previous_reward_level + 1u;
         level <= reservation.current_reward_level && level <= 255u;
         ++level) {
        if (!AppendRewardList(plan, shadow,
                              Config().SubRewards(static_cast<int>(level)))) {
            return false;
        }
    }
    if (reservation.main_reward_newly_decided &&
        !AppendRewardList(
            plan,
            shadow,
            Config().MainRewards(static_cast<int>(
                reservation.target_level)))) {
        return false;
    }
    return plan.reward_count != 0u &&
           HashMainInventory(&shadow, plan.inventory_post_sha256) &&
           HashRewardPlan(plan, plan.reward_plan_sha256);
}

bool InventoryMatchesPlan(CUser* user,
                          const uint8_t expected_sha256[32],
                          uint32_t expected_event_coin) {
    const CInventory* const inventory = user ? user->getCurCharacInvenR() : 0;
    uint8_t digest[32] = {};
    return inventory && inventory->GetEventCoin() >= 0 &&
           static_cast<uint32_t>(inventory->GetEventCoin()) ==
               expected_event_coin &&
           HashMainInventory(inventory, digest) &&
           std::memcmp(digest, expected_sha256, 32u) == 0;
}

bool SubmitTrackedRewardMail(
    CUser* user,
    const decide_levelup_db::RewardDelivery& delivery,
    int server_group) {
    if (!user || delivery.kind != decide_levelup_db::kDeliveryTrackedMail ||
        !delivery.item_id || !delivery.count ||
        DeliveryTokenIsZero(delivery.delivery_token) ||
        delivery.avatar_flag > 1u || delivery.creature_flag > 1u) {
        return false;
    }
    InvenItemMailRecord record;
    LoadMailProjection(delivery, record);
    SIG_MAILBOX_PERSISTENCE_CONTEXT persistence;
    std::memset(&persistence, 0, sizeof(persistence));
    persistence.magic = MAILBOX_PERSISTENCE_CONTEXT_MAGIC;
    persistence.version = MAILBOX_PERSISTENCE_CONTEXT_VERSION;
    persistence.consumer =
        MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP;
    std::memcpy(persistence.delivery_token, delivery.delivery_token,
                sizeof(persistence.delivery_token));
    persistence.receiver_charac_no =
        static_cast<uint32_t>(user->getCurCharacNo());
    persistence.reward_item_id = delivery.item_id;
    persistence.server_group = server_group;
    // These native-only labels are intentionally stable across PVF reloads;
    // sender/body bytes are part of the persisted Request SHA.
    static const char kRewardMailTitle[] = "Decide Level-Up";
    static const char kRewardMailContents[] = "Decide Level-Up Reward";
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMailTrackedRecord(
               kRewardMailTitle,
               record,
               0u,
               persistence.receiver_charac_no,
               kRewardMailContents,
               static_cast<int>(sizeof(kRewardMailContents) - 1u),
               15u,
               static_cast<ENUM_SERVER_GROUP>(server_group),
               false,
               false,
               persistence) == 0;
}

bool ApplyDirectPlan(CUser* user,
                      const decide_levelup_db::Message& plan) {
    CInventory* const inventory = user ? user->getCurCharacInvenW() : 0;
    if (!inventory) return false;
    InventoryMemory rollback_memory;
    CInventory rollback_image;
    rollback_image.SetInventoryMemory(&rollback_memory);
    if (!rollback_image.setCopy(*inventory)) return false;

    struct UpdatedSlot {
        ENUM_ITEMSPACE item_space;
        int slot;
    };
    std::vector<UpdatedSlot> updated_slots;
    for (uint8_t index = 0u; index < plan.reward_count; ++index) {
        const decide_levelup_db::RewardDelivery& delivery =
            plan.deliveries[index];
        if (delivery.kind == decide_levelup_db::kDeliveryEventCoin) {
            const int current = inventory->GetEventCoin();
            if (current < 0 || delivery.count >
                    static_cast<uint32_t>(
                        std::numeric_limits<int>::max() - current)) {
                (void)inventory->setCopy(rollback_image);
                return false;
            }
            inventory->SetEventCoin(
                static_cast<unsigned int>(current) + delivery.count);
        } else if (delivery.kind ==
                   decide_levelup_db::kDeliveryDirectInventory) {
            ENUM_ITEMSPACE item_space = static_cast<ENUM_ITEMSPACE>(0);
            const int slot = user->AddItem(
                static_cast<int>(delivery.item_id),
                static_cast<int>(delivery.count),
                static_cast<eItemAddReason>(13), item_space, 0);
            if (slot < 0 || static_cast<int>(item_space) != 0) {
                (void)inventory->setCopy(rollback_image);
                return false;
            }
            UpdatedSlot updated;
            updated.item_space = item_space;
            updated.slot = slot;
            updated_slots.push_back(updated);
        }
    }
    if (!InventoryMatchesPlan(user, plan.inventory_post_sha256,
                              plan.event_coin_after)) {
        (void)inventory->setCopy(rollback_image);
        return false;
    }

    int logged_event_coin = static_cast<int>(plan.event_coin_before);
    for (uint8_t index = 0u; index < plan.reward_count; ++index) {
        const decide_levelup_db::RewardDelivery& delivery =
            plan.deliveries[index];
        if (delivery.kind != decide_levelup_db::kDeliveryEventCoin) continue;
        logged_event_coin += static_cast<int>(delivery.count);
        user->history_log().EventCoinAdd(
            logged_event_coin, static_cast<int>(delivery.count),
            static_cast<eCoinAddReason>(2));
    }
    for (std::vector<UpdatedSlot>::const_iterator updated =
             updated_slots.begin();
         updated != updated_slots.end(); ++updated) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             updated->item_space, updated->slot);
    }
    user->SendCoin();
    return true;
}

bool DeliverPreparedPlan(CUser* user,
                         const decide_levelup_db::Message& plan) {
    EventState* const state = FindState(user);
    if (!state || !IsCurrentResponse(user, plan) ||
        plan.plan_state != decide_levelup_db::kPlanStatePrepared ||
        plan.charac_no != static_cast<uint32_t>(user->getCurCharacNo())) {
        return false;
    }
    const bool already_post = InventoryMatchesPlan(
        user, plan.inventory_post_sha256, plan.event_coin_after);
    if (!already_post &&
        (!InventoryMatchesPlan(user, plan.inventory_pre_sha256,
                               plan.event_coin_before) ||
         !ApplyDirectPlan(user, plan))) {
        return false;
    }

    state->prepared_plan = plan;
    state->delivered_claim_token = plan.claim_token;
    bool all_mail_submitted = true;
    for (uint8_t index = 0u; index < plan.reward_count; ++index) {
        if (plan.deliveries[index].kind ==
                decide_levelup_db::kDeliveryTrackedMail &&
            !SubmitTrackedRewardMail(user, plan.deliveries[index],
                                     plan.server_group)) {
            all_mail_submitted = false;
        }
    }

    // A full signal-35 snapshot persists direct stacks and EventCoin.  The
    // strict signal-823 envelope is queued behind both tracked signal-128
    // requests and this snapshot; queue6 still re-reads DB3 and all ACK rows.
    if (user->SaveInventory() != 1) return false;
    decide_levelup_db::Message finalize = plan;
    finalize.operation = decide_levelup_db::kOperationFinalizeRewards;
    finalize.result = decide_levelup_db::kResultOperationFailed;
    finalize.mail_persisted = 0u;
    std::memset(finalize.ack_delivery_token, 0,
                sizeof(finalize.ack_delivery_token));
    finalize.ack_reward_item_id = 0u;
    if (!decide_levelup_db::DB_DecideLevelupEvent::
            QueueAfterLegacyInventory(user->GetUID(), finalize)) {
        return false;
    }
    state->db_pending = true;
    return all_mail_submitted;
}

bool ProcessReservedRewards(
    CUser* user,
    const decide_levelup_db::Message& message) {
    EventState* const state = FindState(user);
    if (!state || !IsCurrentResponse(user, message) ||
        message.charac_no != static_cast<uint32_t>(user->getCurCharacNo()) ||
        !Config().available() || message.claim_token == 0u ||
        message.current_reward_level <= message.previous_reward_level) {
        return false;
    }

    if (message.plan_state == decide_levelup_db::kPlanStatePrepared) {
        state->db_pending = false;
        return DeliverPreparedPlan(user, message);
    }

    decide_levelup_db::Message plan;
    if (!BuildRewardPlan(user, message, plan) ||
        !decide_levelup_db::DB_DecideLevelupEvent::Queue(user->GetUID(),
                                                         plan)) {
        // Never roll back the SQL007 reservation here.  No durable plan means
        // no local mutation; a later load/level-up can retry safely.
        state->db_pending = false;
        return false;
    }
    state->prepared_plan = plan;
    state->db_pending = true;
    return true;
}

}  // namespace

namespace decide_levelup_event {

void RequestLoginState(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return;
    }
    EventState state;
    state.session_token = NextSessionToken();
    state.user_idx = user->GetUID();
    States()[static_cast<uint32_t>(user->get_acc_id())] = state;
    decide_levelup_db::Message request;
    InitMessage(request, user, decide_levelup_db::kOperationLoad);
    EventState* const installed = FindState(user);
    if (installed &&
        decide_levelup_db::DB_DecideLevelupEvent::Queue(user->GetUID(),
                                                        request)) {
        installed->db_pending = true;
    }
}

void ResetSession(CUser* user) {
    if (user && user->get_acc_id() > 0) {
        States().erase(static_cast<uint32_t>(user->get_acc_id()));
    }
}

void OnTrackedMailResult(const SIG_MAILBOX_PERSISTENCE_CONTEXT& context,
                         bool persisted) {
    if (context.magic != MAILBOX_PERSISTENCE_CONTEXT_MAGIC ||
        context.version != MAILBOX_PERSISTENCE_CONTEXT_VERSION ||
        context.consumer !=
            MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP ||
        DeliveryTokenIsZero(context.delivery_token) ||
        context.receiver_charac_no == 0u || context.reward_item_id == 0u ||
        context.server_group < 0 || context.server_group >= 18) {
        return;
    }

    decide_levelup_db::Message ack;
    std::memset(&ack, 0, sizeof(ack));
    ack.charac_no = context.receiver_charac_no;
    ack.server_group = context.server_group;
    ack.operation = decide_levelup_db::kOperationMailPersistenceAck;
    ack.result = decide_levelup_db::kResultOperationFailed;
    std::memcpy(ack.ack_delivery_token, context.delivery_token,
                sizeof(ack.ack_delivery_token));
    ack.ack_reward_item_id = context.reward_item_id;
    ack.mail_persisted = persisted ? 1u : 0u;
    int user_idx = -1;

    for (StateMap::const_iterator state = States().begin();
         state != States().end(); ++state) {
        const EventState& session = state->second;
        const decide_levelup_db::Message& plan = session.prepared_plan;
        if (session.charac_no != context.receiver_charac_no ||
            plan.plan_state != decide_levelup_db::kPlanStatePrepared ||
            plan.server_group != context.server_group) {
            continue;
        }
        bool matched = false;
        for (uint8_t index = 0u; index < plan.reward_count; ++index) {
            const decide_levelup_db::RewardDelivery& delivery =
                plan.deliveries[index];
            if (delivery.kind == decide_levelup_db::kDeliveryTrackedMail &&
                std::memcmp(delivery.delivery_token, context.delivery_token,
                            sizeof(delivery.delivery_token)) == 0 &&
                delivery.item_id == context.reward_item_id) {
                matched = true;
                break;
            }
        }
        if (!matched) continue;
        ack.account_id = state->first;
        ack.claim_token = plan.claim_token;
        ack.session_token = session.session_token;
        user_idx = session.user_idx;
        break;
    }
    (void)decide_levelup_db::DB_DecideLevelupEvent::Queue(user_idx, ack);
}

bool HasPendingReservation(CUser* user) {
    EventState* const state = FindState(user);
    return state && state->loaded && !state->db_pending &&
           IsAllowedTarget(state->target_level) &&
           state->charac_no == 0u && !state->main_reward_decided &&
           Config().available();
}

void BindCreatedCharacter(CUser* user,
                          unsigned int charac_no,
                          int event_character_grow_type) {
    if (!user || event_character_grow_type <= 0 || !charac_no ||
        !HasPendingReservation(user)) {
        return;
    }
    decide_levelup_db::Message request;
    InitMessage(request, user, decide_levelup_db::kOperationBindCharacter);
    request.charac_no = static_cast<uint32_t>(charac_no);
    EventState* const state = FindState(user);
    if (state &&
        decide_levelup_db::DB_DecideLevelupEvent::Queue(user->GetUID(),
                                                        request)) {
        state->db_pending = true;
    }
}

void OnLevelUp(CUser* user) {
    EventState* const state = FindState(user);
    if (!user || !state || !state->loaded || state->db_pending ||
        !Config().available() ||
        state->charac_no == 0u ||
        state->charac_no != static_cast<uint32_t>(user->getCurCharacNo()) ||
        user->get_charac_level() <= static_cast<int>(state->last_reward_level) ||
        !HasRewardInRange(*state,
                          static_cast<uint32_t>(user->get_charac_level()))) {
        return;
    }
    decide_levelup_db::Message request;
    InitMessage(request, user, decide_levelup_db::kOperationReserveRewards);
    request.charac_no = state->charac_no;
    request.target_level = state->target_level;
    request.current_reward_level = ChooseRewardBatchEnd(
        *state, static_cast<uint32_t>(user->get_charac_level()));
    if (request.current_reward_level == 0u) return;
    if (decide_levelup_db::DB_DecideLevelupEvent::Queue(user->GetUID(),
                                                        request)) {
        state->db_pending = true;
    }
}

}  // namespace decide_levelup_event

Dispatcher_DecideLevelUp::Dispatcher_DecideLevelUp()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>() {}

Dispatcher_DecideLevelUp::~Dispatcher_DecideLevelUp() {}

int Dispatcher_DecideLevelUp::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint32_t target_level = 0;
    if (!user || packet.get_int(target_level) != 1) {
        SendDecideError(user, decide_levelup_db::kResultInvalidTarget);
        return 0;
    }
    if (!IsAllowedTarget(target_level)) {
        SendDecideError(user, decide_levelup_db::kResultInvalidTarget);
        return 0;
    }
    if (!Config().available()) {
        // CLIENT_IDA_EVIDENCE: error 178 maps to "目前未开放预约等级活动".
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DECIDE_LEVELUP, 178);
        return 0;
    }

    EventState* const state = FindState(user);
    if (!state || !state->loaded || state->db_pending) {
        SendDecideError(user, decide_levelup_db::kResultInvalidTarget);
        return 0;
    }
    if (state->target_level != 0u) {
        SendDecideError(user, decide_levelup_db::kResultAlreadyDecided);
        return 0;
    }

    decide_levelup_db::Message request;
    InitMessage(request, user, decide_levelup_db::kOperationDecide);
    request.target_level = target_level;
    if (!request.account_id ||
        !decide_levelup_db::DB_DecideLevelupEvent::Queue(user->GetUID(),
                                                         request)) {
        SendDecideError(user, decide_levelup_db::kResultInvalidTarget);
    } else {
        state->db_pending = true;
    }
    return 0;
}

int Dispatcher_DecideLevelUp::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DecideLevelUp::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_DecideLevelUp::Inter_DecideLevelUp() : ISigDispatcher() {}
Inter_DecideLevelUp::~Inter_DecideLevelUp() {}

int Inter_DecideLevelUp::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(decide_levelup_db::Message))) {
        return 0;
    }
    const decide_levelup_db::Message& message =
        *reinterpret_cast<const decide_levelup_db::Message*>(payload);
    if (user->get_acc_id() <= 0 ||
        message.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        !IsCurrentResponse(user, message)) {
        return 0;
    }

    EventState* state = FindState(user);
    state->db_pending = false;

    switch (message.operation) {
    case decide_levelup_db::kOperationLoad:
        if (message.result == decide_levelup_db::kResultOk) {
            ApplyState(message, false);
            state = FindState(user);
            if (state) {
                SendStateNoti(user, *state);
            }
            if (message.claim_token != 0u) {
                ProcessReservedRewards(user, message);
            }
        }
        break;
    case decide_levelup_db::kOperationDecide:
        if (message.result == decide_levelup_db::kResultOk) {
            ApplyState(message, false);
            SendDecideSuccess(user, message.target_level);
            state = FindState(user);
            if (state) {
                SendStateNoti(user, *state);
            }
        } else {
            if (message.target_level != 0u) {
                ApplyState(message, false);
            }
            SendDecideError(user, message.result);
        }
        break;
    case decide_levelup_db::kOperationBindCharacter:
        if (message.result == decide_levelup_db::kResultOk) {
            ApplyState(message, false);
            decide_levelup_event::OnLevelUp(user);
        }
        break;
    case decide_levelup_db::kOperationReserveRewards:
        if (message.result == decide_levelup_db::kResultOk) {
            if (message.claim_token != 0u) {
                ProcessReservedRewards(user, message);
            } else {
                ApplyState(message, false);
            }
        }
        break;
    case decide_levelup_db::kOperationPrepareRewardPlan:
        if (message.result == decide_levelup_db::kResultOk &&
            message.plan_state == decide_levelup_db::kPlanStatePrepared) {
            ProcessReservedRewards(user, message);
        }
        break;
    case decide_levelup_db::kOperationFinalizeRewards:
        if (message.result == decide_levelup_db::kResultOk) {
            ApplyState(message, true);
            state = FindState(user);
            if (state &&
                state->delivered_claim_token == message.claim_token) {
                state->delivered_claim_token = 0;
            }
            if (message.main_reward_newly_decided) {
                SendMainRewardDecidedNoti(user);
            }
            decide_levelup_event::OnLevelUp(user);
        }
        break;
    case decide_levelup_db::kOperationRollbackRewards:
        if (message.result == decide_levelup_db::kResultOk) {
            ApplyState(message, false);
            state = FindState(user);
            if (state &&
                state->delivered_claim_token == message.claim_token) {
                state->delivered_claim_token = 0;
            }
        }
        break;
    case decide_levelup_db::kOperationMailPersistenceAck:
        // Positive ACKs may be promoted to FinalizeRewards by queue6.  A
        // remaining ACK result only means another mail or DB3 proof is still
        // pending; the reservation deliberately stays intact.
        break;
    default:
        break;
    }
    return 0;
}
