#include "old_game/progression/career/packet_dispatcher_monstercard_bind.h"

#include <stddef.h>
#include <stdint.h>

#include <utility>
#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/world/region/game_world.h"


namespace {

const INVEN_TYPE kInventory = static_cast<INVEN_TYPE>(1);
const eItemDelReason kDeleteReason = static_cast<eItemDelReason>(44);
const eItemAddReason kAddReason = static_cast<eItemAddReason>(52);
const eMoneyAddReason kMoneyReason = static_cast<eMoneyAddReason>(0);
const SECURITY_PROTCTION kSecurityProtection =
    static_cast<SECURITY_PROTCTION>(40);

const uint8_t kErrorNoSlot = 4;
const uint8_t kErrorInvalidItem = 17;
const uint8_t kErrorInvalidBind = 19;
const uint8_t kErrorMissingCount = 22;
const uint8_t kErrorForbiddenCard = 23;
const uint8_t kErrorItemLocked = 213;

void SendError(CUser* user, uint8_t error) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MONSTERCARD_BIND, error);
    }
}

unsigned int NativeRoll(unsigned int limit) {
    if (limit == 0u) return 0u;
    // The restored global helper can return `limit` on one legacy boundary;
    // normalize it to the half-open interval used by the original CMTRand
    // bind owner.
    return get_rand_int(static_cast<int>(limit)) % limit;
}

int UpgradeRarity(int rarity, uint16_t rate) {
    if (NativeRoll(10000u) >= static_cast<unsigned int>(rate)) {
        return rarity;
    }
    if (rarity == 0) return 1;
    if (rarity == 1) return 2;
    if (rarity == 2 || rarity == 3) return 3;
    return -1;
}

bool SelectWeightedResult(
    const STExpertJobScript::stMonsterCardBindInfo& info,
    int rarity,
    std::vector<std::pair<int, int> >& rewards) {
    if (rarity < 0 || rarity >= 4) return false;
    const int total_weight = info.group_counts[rarity];
    const std::vector<STExpertJobScript::stMonsterCardBindResult>& candidates =
        info.results[rarity];
    if (total_weight <= 0 || candidates.empty()) return false;

    const unsigned int selected = NativeRoll(
        static_cast<unsigned int>(total_weight)) + 1u;
    unsigned int accumulated = 0u;
    for (std::vector<STExpertJobScript::stMonsterCardBindResult>::const_iterator
             it = candidates.begin();
         it != candidates.end();
         ++it) {
        if (it->item_index == 0u || it->probability == 0u) continue;
        accumulated += static_cast<unsigned int>(it->probability);
        if (selected <= accumulated) {
            rewards.clear();
            rewards.push_back(std::make_pair(
                static_cast<int>(it->item_index), 1));
            return true;
        }
    }
    return false;
}

bool BuildReward(CStackableItem* catalyst,
                 int source_rarity,
                 std::vector<std::pair<int, int> >& rewards) {
    if (!catalyst || source_rarity < 0 || source_rarity >= 4) return false;
    const int bind_rate_index =
        catalyst->private_store_info_.expert_job_value_4;
    if (bind_rate_index == -1) return false;

    // Monster-card binding is part of the enchanter script family.  The same
    // index is used by the recovered enchanter extraction owner.
    STExpertJobScript* const script =
        G_CDataManager()->GetExpertJobScript(1);
    if (!script) return false;

    STExpertJobScript::stMonsterCardBindRate rate;
    if (script->GetMonsterCardBindRate(
            static_cast<uint8_t>(bind_rate_index), rate) != 1 ||
        rate.common_rate > 10000u || rate.special_rate > 10000u) {
        return false;
    }

    const int result_rarity = UpgradeRarity(source_rarity, rate.common_rate);
    if (result_rarity < 0 || result_rarity >= 4) return false;
    const bool special =
        NativeRoll(10000u) < static_cast<unsigned int>(rate.special_rate);
    STExpertJobScript::stMonsterCardBindInfo* const info =
        special ? script->GetSpecialBindInfo() : script->GetCommonBindInfo();
    return info && SelectWeightedResult(*info, result_rarity, rewards);
}

bool IsMonsterCard(CItem* item) {
    return item && item->is_stackable() == 1 &&
           item->hasCategory(STItemScript::ITEM_CATEGORY_MONSTER_CARD) == 1 &&
           item->hasCategory(STItemScript::ITEM_CATEGORY_CANNOT_BIND) == 0;
}

void RestoreInputSnapshots(CInventory* inventory,
                           const uint16_t slots[3],
                           const Inven_Item snapshots[3]) {
    if (!inventory) return;
    for (int index = 0; index < 3; ++index) {
        bool already_restored = false;
        for (int prior = 0; prior < index; ++prior) {
            if (slots[prior] == slots[index]) {
                already_restored = true;
                break;
            }
        }
        if (!already_restored) {
            inventory->update_item(
                kInventory, static_cast<int>(slots[index]), snapshots[index]);
        }
    }
}

}  // namespace


Dispatcher_MonstercardBind::Dispatcher_MonstercardBind()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_MonstercardBind::~Dispatcher_MonstercardBind() {}

int Dispatcher_MonstercardBind::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    uint16_t slots[3] = {0u, 0u, 0u};
    // CLIENT_IDA_EVIDENCE: sub_1150C40 writes slots[0] once at 0x1150E15,
    // then executes the 0x1150E3D u16 writer twice for the two map entries.
    if (packet.get_short(slots[0]) != 1 ||
        packet.get_short(slots[1]) != 1 ||
        packet.get_short(slots[2]) != 1) {
        return LineFunc(
            0,
            "virtual int Dispatcher_MonstercardBind0828::dispatch_sig(CUser*, PacketBuf&)",
            0,
            user ? static_cast<unsigned int>(user->get_acc_id()) : 0u);
    }

    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !user->getCurCharacInvenR() || !user->getCurCharacInvenW() ||
        user->CheckInTrade() || !G_GameWorld() ||
        G_GameWorld()->IsPVPChannel()) {
        return 0;
    }

    for (int index = 0; index < 3; ++index) {
        if (user->CheckItemLock(1, static_cast<int>(slots[index]))) {
            SendError(user, kErrorItemLocked);
            return 0;
        }
    }

    const CInventory* const readable = user->getCurCharacInvenR();
    Inven_Item snapshots[3];
    CItem* prototypes[3] = {0, 0, 0};
    for (int index = 0; index < 3; ++index) {
        const Inven_Item* const item = readable->GetInvenRef(
            1, static_cast<int>(slots[index]));
        if (!item || item->item_id_ == 0) {
            SendError(user, kErrorInvalidItem);
            return 0;
        }
        snapshots[index] = *item;
        prototypes[index] = G_CDataManager()->find_item(item->item_id_);
        if (!prototypes[index] || prototypes[index]->is_stackable() != 1) {
            SendError(user, kErrorInvalidItem);
            return 0;
        }
        if (item->get_add_info() <= 0) {
            SendError(user, kErrorMissingCount);
            return 0;
        }
    }

    CStackableItem* const catalyst =
        static_cast<CStackableItem*>(prototypes[0]);
    if (catalyst->private_store_info_.expert_job_value_4 == -1) {
        SendError(user, kErrorInvalidItem);
        return 0;
    }
    if (!IsMonsterCard(prototypes[1]) || !IsMonsterCard(prototypes[2])) {
        SendError(user, kErrorForbiddenCard);
        return 0;
    }
    const int rarity = prototypes[1]->get_rarity();
    if (rarity < 0 || rarity >= 4 ||
        rarity != prototypes[2]->get_rarity()) {
        SendError(user, kErrorInvalidBind);
        return 0;
    }

    if (!GlobalData::s_pSecuProtectionField) {
        SendError(user, kErrorInvalidItem);
        return 0;
    }
    const int security_error =
        GlobalData::s_pSecuProtectionField->CheckItemRarity(
            user, kSecurityProtection, prototypes[1], &snapshots[1]);
    if (security_error != 0) {
        SendError(user, static_cast<uint8_t>(security_error));
        return 0;
    }

    std::vector<std::pair<int, int> > rewards;
    if (!BuildReward(catalyst, rarity, rewards) || rewards.size() != 1u ||
        !G_CDataManager()->find_item(rewards[0].first)) {
        SendError(user, kErrorInvalidBind);
        return 0;
    }

    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*readable);
    for (int index = 0; index < 3; ++index) {
        if (preview.delete_item(kInventory,
                                static_cast<int>(slots[index]),
                                1,
                                kDeleteReason,
                                false) != 1) {
            SendError(user, kErrorInvalidItem);
            return 0;
        }
    }
    if (preview.tryInsertEventItems(rewards) !=
        static_cast<int>(rewards.size())) {
        SendError(user, kErrorNoSlot);
        return 0;
    }

    CInventory* const writable = user->getCurCharacInvenW();
    for (int index = 0; index < 3; ++index) {
        if (writable->delete_item(kInventory,
                                  static_cast<int>(slots[index]),
                                  1,
                                  kDeleteReason,
                                  true) != 1) {
            RestoreInputSnapshots(writable, slots, snapshots);
            SendError(user, kErrorInvalidItem);
            return 0;
        }
    }

    std::vector<int> inserted_slots;
    const int inserted = writable->insert_event_items(
        rewards, inserted_slots, kMoneyReason, kAddReason, false, "", "");
    if (inserted != static_cast<int>(rewards.size()) ||
        inserted_slots.size() != rewards.size() * 3u ||
        rewards.size() > 0xffu) {
        RestoreInputSnapshots(writable, slots, snapshots);
        SendError(user, kErrorInvalidItem);
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_MONSTERCARD_BIND);
    response.put_byte(1);
    response.put_short(slots[0]);
    response.put_short(slots[1]);
    response.put_short(slots[2]);
    response.put_byte(static_cast<int>(rewards.size()));
    for (size_t offset = 0; offset < inserted_slots.size(); offset += 3u) {
        response.put_short(inserted_slots[offset]);
        response.put_int(inserted_slots[offset + 1u]);
        response.put_int(inserted_slots[offset + 2u]);
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

int Dispatcher_MonstercardBind::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_MonstercardBind::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
