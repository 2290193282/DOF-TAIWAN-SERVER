#include "old_game/activity/schedule/packet_dispatcher_event_all_user_gift.h"

#include "common/config.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/log_manager.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_data/db/event/db_event_all_user_gift.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <limits>
#include <vector>

namespace {

const char kUniqueRewardPath[] = "Etc/uniquerewardforallusers.etc";
const unsigned char kOperationFailed = 1u;
const int kRewardItemAddReason = 13;

struct GiftConfig {
    GiftConfig() : loaded(false), packages() {}

    bool loaded;
    FairPvpEquipmentMap packages;
};

bool IsValidRewardOption(int job,
                         const STStackablePackageData& option,
                         Inven_Item* generated) {
    if (job < 0 || job >= oldsrv::config::kCharacterJobCount ||
        option.item_index <= 0 ||
        option.item_count != 1 || !G_CDataManager()) {
        return false;
    }
    CItem* const prototype = G_CDataManager()->find_item(option.item_index);
    if (!prototype || !prototype->vtable_ || prototype->is_stackable() ||
        prototype->check_job_type(static_cast<char>(job)) != 1 ||
        prototype->get_rarity() != 3 || prototype->getUsableLevel() != 50 ||
        prototype->vtable_->get_item_type(prototype) !=
            oldsrv::config::kEquipmentTypeWeapon) {
        return false;
    }

    Inven_Item item;
    item.item_id_ = option.item_index;
    prototype->vtable_->make_item(prototype, item);
    // The client promises one Lv50 unique weapon per account.  Require those
    // three prototype properties as well as the generated inventory category,
    // so a malformed replacement PVF cannot silently broaden the reward.
    if (item.item_id_ != option.item_index || item.item_category_ != 1) {
        return false;
    }
    if (generated) {
        generated->setCopy(item);
    }
    return true;
}

bool ValidateConfig(const FairPvpEquipmentMap& packages) {
    if (packages.empty()) {
        return false;
    }
    for (FairPvpEquipmentMap::const_iterator job = packages.begin();
         job != packages.end();
         ++job) {
        if (job->first < 0 ||
            job->first >= oldsrv::config::kCharacterJobCount ||
            job->second.empty() ||
            job->second.size() >
                event_all_user_gift::kMaximumSelectionGroupCount) {
            return false;
        }
        for (FairPvpEquipmentPackageList::const_iterator group =
                 job->second.begin();
             group != job->second.end();
             ++group) {
            if (group->empty() ||
                group->size() >
                    static_cast<size_t>(
                        std::numeric_limits<uint16_t>::max())) {
                return false;
            }
            for (FairPvpEquipmentPackage::const_iterator option =
                     group->begin();
                 option != group->end();
                 ++option) {
                if (!IsValidRewardOption(job->first, *option, 0)) {
                    return false;
                }
            }
        }
    }
    return true;
}

const GiftConfig* GetGiftConfig() {
    static bool attempted = false;
    static GiftConfig config;
    if (attempted) {
        return config.loaded ? &config : 0;
    }
    attempted = true;

    char filename[sizeof(kUniqueRewardPath)] = {};
    std::memcpy(filename, kUniqueRewardPath, sizeof(kUniqueRewardPath));
    const int imported =
        importUniqueRewardForAllUsers(filename, &config.packages);
    // importUniqueRewardForAllUsers owns no persistent lexer state.  Release
    // the process-global RDAR cursor after copying its values into the feature
    // cache, including on a partial parse failure.
    unloadRDARScriptFile();
    config.loaded = imported == 1 && ValidateConfig(config.packages);
    return config.loaded ? &config : 0;
}

const FairPvpEquipmentPackageList* FindJobPackages(int job) {
    const GiftConfig* const config = GetGiftConfig();
    if (!config) {
        return 0;
    }
    const FairPvpEquipmentMap::const_iterator found =
        config->packages.find(job);
    return found == config->packages.end() ? 0 : &found->second;
}

bool IsCurrentCharacter(CUser* user,
                        const event_all_user_gift::Payload& payload) {
    return user && user->get_state() == 3 && user->getCurCharacR() &&
           user->get_acc_id() > 0 &&
           payload.account_id ==
               static_cast<uint32_t>(user->get_acc_id()) &&
           payload.charac_no ==
               static_cast<uint32_t>(user->getCurCharacNo()) &&
           payload.origin_user_idx == user->GetUID() &&
           payload.server_group == user->GetServerGroup() &&
           payload.job == static_cast<uint8_t>(user->get_charac_job());
}

bool FillResolvedRewards(
    int job,
    const std::vector<uint16_t>& selected_indices,
    event_all_user_gift::Payload& payload) {
    const FairPvpEquipmentPackageList* const groups = FindJobPackages(job);
    if (!groups || groups->size() != selected_indices.size() ||
        groups->empty() ||
        groups->size() >
            event_all_user_gift::kMaximumSelectionGroupCount) {
        return false;
    }

    payload.selection_count = static_cast<uint8_t>(groups->size());
    for (size_t group_index = 0u; group_index < groups->size();
         ++group_index) {
        const FairPvpEquipmentPackage& group = groups->at(group_index);
        const uint16_t selected = selected_indices[group_index];
        if (static_cast<size_t>(selected) >= group.size()) {
            return false;
        }
        const STStackablePackageData& reward = group.at(selected);
        if (!IsValidRewardOption(job, reward, 0)) {
            return false;
        }
        payload.selected_indices[group_index] = selected;
        payload.item_ids[group_index] =
            static_cast<uint32_t>(reward.item_index);
        payload.item_counts[group_index] =
            static_cast<uint32_t>(reward.item_count);
    }
    return true;
}

bool ValidateResolvedPayload(
    CUser* user,
    const event_all_user_gift::Payload& payload) {
    if (!IsCurrentCharacter(user, payload)) {
        return false;
    }
    const FairPvpEquipmentPackageList* const groups =
        FindJobPackages(static_cast<int>(payload.job));
    if (!groups || groups->size() != payload.selection_count ||
        groups->empty()) {
        return false;
    }

    for (size_t index = 0u;
         index < event_all_user_gift::kMaximumSelectionGroupCount;
         ++index) {
        if (index >= groups->size()) {
            if (payload.selected_indices[index] != 0u ||
                payload.item_ids[index] != 0u ||
                payload.item_counts[index] != 0u) {
                return false;
            }
            continue;
        }
        const FairPvpEquipmentPackage& group = groups->at(index);
        const uint16_t selected = payload.selected_indices[index];
        if (static_cast<size_t>(selected) >= group.size()) {
            return false;
        }
        const STStackablePackageData& reward = group.at(selected);
        if (!IsValidRewardOption(payload.job, reward, 0) ||
            payload.item_ids[index] !=
                static_cast<uint32_t>(reward.item_index) ||
            payload.item_counts[index] !=
                static_cast<uint32_t>(reward.item_count)) {
            return false;
        }
    }
    return true;
}

bool PreflightInventory(
    CUser* user,
    const event_all_user_gift::Payload& payload) {
    CInventory* const source = user ? user->getCurCharacInvenRefR() : 0;
    if (!source || user->CheckInTrade()) {
        return false;
    }

    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    if (!preview.setCopy(*source)) {
        return false;
    }

    for (unsigned int index = 0u; index < payload.selection_count; ++index) {
        STStackablePackageData option;
        option.item_index = static_cast<int32_t>(payload.item_ids[index]);
        option.item_count = static_cast<int32_t>(payload.item_counts[index]);
        Inven_Item reward;
        if (!IsValidRewardOption(payload.job, option, &reward) ||
            preview.insertItemIntoInventory(
                reward,
                static_cast<eItemAddReason>(kRewardItemAddReason),
                false,
                true) < 0) {
            return false;
        }
    }
    return true;
}

void SendGiftError(CUser* user) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EVENT_ALL_USER_GIFT,
                                 kOperationFailed);
    }
}

void SendEligibilityNoti(CUser* user) {
    // Also gate the notification sink: a DB837 reply queued before a rolling
    // deployment must not resurrect the disabled client popup.
    if (!oldsrv::policy::kEventAllUserGiftEnabled || !user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_EVENT_ALL_USER_GIFT);
    // CLIENT_IDA_EVIDENCE: DNF.exe sub_B72C30 consumes no payload bytes.
    packet.finalize(true);
    user->Send(packet);
}

void LogNoXa(const char* phase,
             const event_all_user_gift::Payload& payload,
             unsigned int delivered_count) {
    LogManager::logFormat(
        1,
        __FILE__,
        "EVENT_ALL_USER_GIFT NO_XA",
        __LINE__,
        "RECONSTRUCTED_0828 NO_XA phase=%s account=%u charac=%u "
        "operation=%u result=%u delivered=%u",
        phase ? phase : "unknown",
        static_cast<unsigned int>(payload.account_id),
        static_cast<unsigned int>(payload.charac_no),
        static_cast<unsigned int>(payload.operation),
        static_cast<unsigned int>(payload.result),
        delivered_count);
}

void QueueRelease(const event_all_user_gift::Payload& reserved) {
    event_all_user_gift::Payload release = reserved;
    release.operation = static_cast<uint8_t>(
        event_all_user_gift::OPERATION_RELEASE_CLAIM);
    release.result = static_cast<uint8_t>(
        event_all_user_gift::RESULT_DB_ERROR);
    if (!event_all_user_gift::Queue(-1, release)) {
        LogNoXa("release_enqueue_failed", reserved, 0u);
    }
}

CUser* FindOriginUser(
    const event_all_user_gift::Payload& payload) {
    CGameManager* const manager = G_CGameManager();
    return manager && payload.origin_user_idx >= 0
               ? manager->getUser(payload.origin_user_idx, 0)
               : 0;
}

unsigned int GrantRewards(
    CUser* user,
    const event_all_user_gift::Payload& payload) {
    unsigned int delivered = 0u;
    for (unsigned int index = 0u; index < payload.selection_count; ++index) {
        ENUM_ITEMSPACE item_space = static_cast<ENUM_ITEMSPACE>(0);
        const int slot = user->AddItem(
            static_cast<int>(payload.item_ids[index]),
            static_cast<int>(payload.item_counts[index]),
            static_cast<eItemAddReason>(kRewardItemAddReason),
            item_space,
            0);
        if (slot < 0) {
            break;
        }
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             item_space,
                             slot);
        ++delivered;
    }
    return delivered;
}

}  // namespace

namespace event_all_user_gift {

void QueueEligibilityCheck(CUser* user) {
    if (!oldsrv::policy::kEventAllUserGiftEnabled || !user ||
        user->get_state() != 3 || !user->getCurCharacR() ||
        user->get_acc_id() <= 0 || user->getCurCharacNo() <= 0) {
        return;
    }
    const FairPvpEquipmentPackageList* const groups =
        FindJobPackages(user->get_charac_job());
    if (!groups || groups->empty() ||
        groups->size() > kMaximumSelectionGroupCount) {
        return;
    }

    Payload request;
    ClearPayload(request);
    request.account_id = static_cast<uint32_t>(user->get_acc_id());
    request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    request.origin_user_idx = user->GetUID();
    request.server_group = user->GetServerGroup();
    request.operation = static_cast<uint8_t>(OPERATION_QUERY_ELIGIBILITY);
    request.job = static_cast<uint8_t>(user->get_charac_job());
    request.selection_count = static_cast<uint8_t>(groups->size());
    (void)Queue(user->GetUID(), request);
}

}  // namespace event_all_user_gift_0828

Dispatcher_EventAllUserGift::Dispatcher_EventAllUserGift()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_EventAllUserGift::~Dispatcher_EventAllUserGift() {}

int Dispatcher_EventAllUserGift::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    using namespace event_all_user_gift;

    // Retain CMD420 registration for protocol compatibility, but never parse,
    // reserve, grant, or persist this retired automatic artifact reward.
    if (!oldsrv::policy::kEventAllUserGiftEnabled) {
        SendGiftError(user);
        return 0;
    }

    uint16_t selection_count = 0u;
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        packet.get_short(selection_count) != 1 || selection_count == 0u ||
        selection_count > kMaximumSelectionGroupCount) {
        SendGiftError(user);
        return 0;
    }

    std::vector<uint16_t> selected_indices;
    selected_indices.reserve(selection_count);
    for (uint16_t index = 0u; index < selection_count; ++index) {
        uint16_t selected = 0u;
        if (packet.get_short(selected) != 1) {
            SendGiftError(user);
            return 0;
        }
        selected_indices.push_back(selected);
    }

    Payload request;
    ClearPayload(request);
    request.account_id = static_cast<uint32_t>(user->get_acc_id());
    request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    request.origin_user_idx = user->GetUID();
    request.server_group = user->GetServerGroup();
    request.operation = static_cast<uint8_t>(OPERATION_RESERVE_CLAIM);
    request.job = static_cast<uint8_t>(user->get_charac_job());

    // CLIENT_IDA_EVIDENCE: sub_B707D0 accepts only 0 <= index < vector size,
    // sub_B70ED0 copies that zero-based value from +0x40, and sub_7CB190
    // serializes it with the same u16 writer used for the vector count.
    if (request.account_id == 0u || request.charac_no == 0u ||
        !FillResolvedRewards(user->get_charac_job(),
                             selected_indices,
                             request) ||
        request.selection_count != selection_count ||
        !ValidateResolvedPayload(user, request) ||
        !PreflightInventory(user, request) || !Queue(-1, request)) {
        SendGiftError(user);
    }
    // The 0828 client has no CMD420 success handler.  Success is published by
    // the canonical NOTI14 item update after the DB reservation wins.
    return 0;
}

int Dispatcher_EventAllUserGift::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_EventAllUserGift::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_EventAllUserGift::Inter_EventAllUserGift()
    : ISigDispatcher() {}

Inter_EventAllUserGift::~Inter_EventAllUserGift() {}

int Inter_EventAllUserGift::dispatch_sig(CUser* user,
                                              char* raw_payload,
                                              int payload_size) {
    using namespace event_all_user_gift;

    // Keep the DB reply owner registered for wire compatibility, but fail
    // closed before interpreting any queued payload.  In particular, an old
    // RESULT_RESERVED reply must never reach GrantRewards after the automatic
    // Lv50 artifact selector has been retired.
    if (!oldsrv::policy::kEventAllUserGiftEnabled) {
        return 0;
    }

    if (!raw_payload || payload_size != static_cast<int>(sizeof(Payload))) {
        return 0;
    }
    const Payload& payload =
        *reinterpret_cast<const Payload*>(raw_payload);

    if (payload.operation == OPERATION_QUERY_ELIGIBILITY) {
        if (payload.result == RESULT_UNCLAIMED &&
            IsCurrentCharacter(user, payload)) {
            const FairPvpEquipmentPackageList* const groups =
                FindJobPackages(payload.job);
            if (groups && groups->size() == payload.selection_count) {
                SendEligibilityNoti(user);
            }
        }
        return 0;
    }

    CUser* const origin = FindOriginUser(payload);
    if (payload.operation == OPERATION_RELEASE_CLAIM) {
        // Re-query instead of directly re-opening the dialog.  This preserves
        // the invariant that NOTI368 is sent only after a DB read proves the
        // account absent from the claim table.
        if (payload.result == RESULT_RELEASED &&
            IsCurrentCharacter(origin, payload)) {
            QueueEligibilityCheck(origin);
        } else if (payload.result != RESULT_RELEASED) {
            LogNoXa("release_db_failed", payload, 0u);
        }
        return 0;
    }

    if (payload.operation != OPERATION_RESERVE_CLAIM) {
        return 0;
    }

    if (payload.result == RESULT_ALREADY_CLAIMED ||
        payload.result == RESULT_DB_ERROR) {
        if (IsCurrentCharacter(origin, payload)) {
            SendGiftError(origin);
        }
        return 0;
    }
    if (payload.result != RESULT_RESERVED) {
        return 0;
    }

    if (!IsCurrentCharacter(origin, payload) ||
        !ValidateResolvedPayload(origin, payload) ||
        !PreflightInventory(origin, payload)) {
        // No inventory mutation has happened.  A conditional DELETE by the
        // same (m_id,charac_no) makes this reservation retryable.
        QueueRelease(payload);
        if (IsCurrentCharacter(origin, payload)) {
            SendGiftError(origin);
        }
        return 0;
    }

    const unsigned int delivered = GrantRewards(origin, payload);
    if (delivered == payload.selection_count) {
        return 0;
    }

    SendGiftError(origin);
    if (delivered == 0u) {
        QueueRelease(payload);
    } else {
        // NO_XA: event_1306_account_reward is MyISAM and inventory mutation
        // belongs to a different persistence owner.  Releasing after a
        // partial grant would enable duplicate weapons, so retain the claim
        // and make the exceptional partial delivery auditable instead.
        LogNoXa("partial_inventory_grant", payload, delivered);
    }
    return 0;
}
