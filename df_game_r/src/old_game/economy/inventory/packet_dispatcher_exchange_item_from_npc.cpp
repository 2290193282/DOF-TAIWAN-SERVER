#include "old_game/economy/inventory/packet_dispatcher_exchange_item_from_npc.h"

#include <stdint.h>
#include <string>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace {

const char kNpcExchangePvf[] = "exchangeitemnpc.etc";
const int kMainInventory = 1;
const int kWeightScale = 10000;
const int kMaximumWireU16 = 0xffff;
const uint8_t kErrorInvalidState = 1;
const uint8_t kErrorInventoryFull = 4;
const uint8_t kErrorUnavailable = 22;
const eItemDelReason kNpcExchangeDeleteReason =
    static_cast<eItemDelReason>(3);
const eItemAddReason kNpcExchangeAddReason =
    static_cast<eItemAddReason>(8);
// insertItemIntoInventory records DBLog items before it knows whether the
// mutation can succeed.  Reason 7 is one of the native no-DBLog paths; the
// intended reason-8 history/DBLog records are emitted only after both live
// mutations have committed below.
const eItemAddReason kNpcExchangeStagingAddReason =
    static_cast<eItemAddReason>(7);

const char kReadSignature[] =
    "virtual int Dispatcher_ExchangeItemFromNpc::read(PacketBuf&, MSG_BASE&)";

struct NpcExchangeRewardRule {
    NpcExchangeRewardRule() : weight(0), item_id(0), count(0) {}

    int weight;
    int item_id;
    int count;
};

struct NpcExchangeRule {
    NpcExchangeRule()
        : npc_id(0),
          list_id(0),
          source_item_id(0),
          source_count(0),
          has_list(false),
          has_source(false),
          has_rewards(false),
          rewards() {}

    int npc_id;
    int list_id;
    int source_item_id;
    int source_count;
    bool has_list;
    bool has_source;
    bool has_rewards;
    std::vector<NpcExchangeRewardRule> rewards;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool IsValidStackablePrototype(int item_id) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const item = data_manager ? data_manager->find_item(item_id) : 0;
    return item && item->vtable_ && item->is_stackable() == 1;
}

bool IsValidRuleSet(const std::vector<NpcExchangeRule>& rules) {
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager || rules.empty()) {
        return false;
    }

    for (size_t index = 0; index < rules.size(); ++index) {
        const NpcExchangeRule& rule = rules[index];
        if (rule.npc_id <= 0 || rule.npc_id > kMaximumWireU16 ||
            rule.list_id <= 0 ||
            rule.list_id > kMaximumWireU16 ||
            rule.source_item_id <= 0 || rule.source_count != 1 ||
            !rule.has_list || !rule.has_source || !rule.has_rewards ||
            rule.rewards.empty() || !data_manager->find_npc(rule.npc_id) ||
            !IsValidStackablePrototype(rule.source_item_id)) {
            return false;
        }

        int total_weight = 0;
        for (size_t reward_index = 0;
             reward_index < rule.rewards.size();
             ++reward_index) {
            const NpcExchangeRewardRule& reward =
                rule.rewards[reward_index];
            if (reward.weight <= 0 || reward.item_id <= 0 ||
                reward.count <= 0 ||
                reward.count > kMaximumWireU16 ||
                total_weight > kWeightScale - reward.weight ||
                !IsValidStackablePrototype(reward.item_id)) {
                return false;
            }
            total_weight += reward.weight;
        }
        if (total_weight != kWeightScale) {
            return false;
        }

        for (size_t previous = 0; previous < index; ++previous) {
            if (rules[previous].npc_id == rule.npc_id ||
                rules[previous].source_item_id == rule.source_item_id) {
                return false;
            }
        }
    }
    return true;
}

// Typed, lazy importer for etc/exchangeitemnpc.etc.  The current 0828 table
// contains only stackable source/reward rows.  Equipment is rejected instead
// of fabricating instance fields that CMD718 never serializes back to client.
class NpcExchangeConfig {
public:
    NpcExchangeConfig() : loaded_(false), rules_() {}

    const NpcExchangeRule* find_by_npc(int npc_id) {
        if (!load()) {
            return 0;
        }
        for (std::vector<NpcExchangeRule>::const_iterator it = rules_.begin();
             it != rules_.end();
             ++it) {
            if (it->npc_id == npc_id) {
                return &*it;
            }
        }
        return 0;
    }

    bool select_reward(const NpcExchangeRule& rule,
                       NpcExchangeRewardRule& selected) const {
        if (rule.rewards.empty()) {
            return false;
        }
        // The restored legacy RNG has an equality edge that may return the
        // upper limit.  Normalize it to the PVF's proven 0..9999 domain.
        const int roll = static_cast<int>(
            get_rand_int(kWeightScale) %
            static_cast<unsigned int>(kWeightScale));
        int accumulated = 0;
        for (std::vector<NpcExchangeRewardRule>::const_iterator it =
                 rule.rewards.begin();
             it != rule.rewards.end();
             ++it) {
            accumulated += it->weight;
            if (roll < accumulated) {
                selected = *it;
                return true;
            }
        }
        return false;
    }

private:
    bool load() {
        if (loaded_) {
            return true;
        }
        if (!loadRDARScriptFile("Etc/", kNpcExchangePvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        std::vector<NpcExchangeRule> parsed;
        NpcExchangeRule current;
        bool in_npc = false;
        std::string tag;
        bool success = false;

        while (ScanType(tag, true) == 1) {
            if (tag == "[npc]") {
                if (in_npc) {
                    return false;
                }
                current = NpcExchangeRule();
                current.npc_id = ScanInt(&success);
                if (!success) {
                    return false;
                }
                in_npc = true;
            } else if (tag == "[list]" && in_npc) {
                if (current.has_list) {
                    return false;
                }
                current.list_id = ScanInt(&success);
                if (!success) {
                    return false;
                }
                current.has_list = true;
            } else if (tag == "[exchange item]" && in_npc) {
                if (current.has_source) {
                    return false;
                }
                current.source_item_id = ScanInt(&success);
                if (!success) {
                    return false;
                }
                current.source_count = ScanInt(&success);
                if (!success) {
                    return false;
                }
                current.has_source = true;
            } else if (tag == "[random exchange item]" && in_npc) {
                if (current.has_rewards) {
                    return false;
                }
                for (;;) {
                    NpcExchangeRewardRule reward;
                    reward.weight = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    reward.item_id = ScanInt(&success);
                    if (!success) {
                        return false;
                    }
                    reward.count = ScanInt(&success);
                    if (!success) {
                        return false;
                    }
                    current.rewards.push_back(reward);
                }
                current.has_rewards = true;
            } else if (tag == "[/npc]") {
                if (!in_npc) {
                    return false;
                }
                parsed.push_back(current);
                current = NpcExchangeRule();
                in_npc = false;
            }
        }

        if (in_npc || !IsValidRuleSet(parsed)) {
            return false;
        }
        rules_.swap(parsed);
        loaded_ = true;
        return true;
    }

    bool loaded_;
    std::vector<NpcExchangeRule> rules_;
};

NpcExchangeConfig& GetNpcExchangeConfig() {
    static NpcExchangeConfig config;
    return config;
}

bool IsAuthorizedTownExchange(CUser* user, uint16_t npc_id) {
    // The restored tree has no current-dialogue token or proven distance
    // threshold, but it does have the authoritative user village/area and the
    // area's exact NPC set.  Require the requested typed NPC to exist in the
    // user's current area instead of accepting it from any town map.
    GameWorld* const world = G_GameWorld();
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !user->getCurCharacInvenR() || user->getMoveSpace() == 1 || !world ||
        world->IsPVPChannel()) {
        return false;
    }

    const int village_index =
        static_cast<signed char>(user->getCurCharacVill());
    const int area_index = user->get_area(false);
    if (!world->check_valid_area(village_index, area_index)) {
        return false;
    }
    Village* const village = world->getUserVillage(user);
    Area* const area = village ? village->getArea(area_index) : 0;
    return area && area->hasNPC(static_cast<uint32_t>(npc_id)) == 1;
}

bool FindUnlockedSource(CUser* user,
                        const NpcExchangeRule& rule,
                        int& source_slot,
                        const Inven_Item*& source_item) {
    source_slot = -1;
    source_item = 0;
    if (!user || !user->getCurCharacInvenR()) {
        return false;
    }

    const CInventory* const inventory = user->getCurCharacInvenR();
    for (int slot = 3; slot <= 8; ++slot) {
        const Inven_Item* const item =
            inventory->GetInvenRef(kMainInventory, slot);
        if (item && item->item_id_ == rule.source_item_id &&
            item->get_add_info() >= rule.source_count &&
            !user->CheckItemLock(kMainInventory, slot)) {
            source_slot = slot;
            source_item = item;
            return true;
        }
    }

    const int item_type = inventory->GetItemType(rule.source_item_id);
    if (item_type <= 0) {
        return false;
    }
    int start = 0;
    int end = 0;
    inventory->StartEndPos(
        static_cast<Inven_Item::ITEM_TYPE>(item_type), start, end);
    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* const item =
            inventory->GetInvenRef(kMainInventory, slot);
        if (item && item->item_id_ == rule.source_item_id &&
            item->get_add_info() >= rule.source_count &&
            !user->CheckItemLock(kMainInventory, slot)) {
            source_slot = slot;
            source_item = item;
            return true;
        }
    }
    return false;
}

bool MakeStackableReward(const NpcExchangeRewardRule& rule,
                         Inven_Item& item) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const prototype = data_manager
                                 ? data_manager->find_item(rule.item_id)
                                 : 0;
    if (!prototype || !prototype->vtable_ ||
        prototype->is_stackable() != 1) {
        return false;
    }

    item.reset();
    item.item_id_ = rule.item_id;
    item.set_add_info(rule.count);
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = rule.item_id;
    item.set_add_info(rule.count);
    return item.item_category_ != 1 && !item.IsCreatureItemType() &&
           item.item_category_ != 8;
}

void CommitNpcExchangeAudit(CUser* user,
                            CInventory* inventory,
                            const Inven_Item& source_snapshot,
                            int source_count,
                            int inserted_slot,
                            const Inven_Item& reward_item) {
    const int source_after_count =
        source_snapshot.get_add_info() - source_count;
    Inven_Item source_log_item = source_snapshot;
    if (source_after_count > 0) {
        source_log_item.set_add_info(source_after_count);
    }

    const Inven_Item* const final_reward = inventory
                                               ? inventory->GetInvenRef(
                                                     kMainInventory,
                                                     inserted_slot)
                                               : 0;
    const int resulting_count =
        final_reward && final_reward->item_id_ == reward_item.item_id_
            ? final_reward->get_add_info()
            : reward_item.get_add_info();

    cUserHistoryLog* const history = CUser_history_log(user);
    if (history) {
        history->ItemDel(static_cast<INVEN_TYPE>(kMainInventory),
                         source_after_count,
                         source_count,
                         source_log_item,
                         kNpcExchangeDeleteReason);
    }

    // Match the native successful path's observable audit ordering:
    // source ItemDel, reward DBLog, then reward ItemAdd.
    CDataManager* const data_manager = G_CDataManager();
    const int log_type = data_manager
                             ? data_manager->IsDBLogItem(
                                   static_cast<uint32_t>(reward_item.item_id_))
                             : -1;
    if (user && reward_item.item_category_ == 2 && log_type != -1) {
        (void)user->AddDBLogItem(
            static_cast<uint32_t>(reward_item.item_id_),
            static_cast<uint32_t>(reward_item.get_add_info()),
            static_cast<ENUM_DBLOG_ITEM_TYPE>(log_type));
    }

    if (history) {
        history->ItemAdd(static_cast<INVEN_TYPE>(kMainInventory),
                         resulting_count,
                         reward_item.get_add_info(),
                         reward_item,
                         kNpcExchangeAddReason);
    }
}

}  // namespace

MSG_EXCHANGE_ITEM_FROM_NPC::MSG_EXCHANGE_ITEM_FROM_NPC()
    : MSG_BASE(), npc_id(0), exchange_list_id(0) {}

NpcExchangeRewardResult::NpcExchangeRewardResult() : item_id(0), count(0) {}

stExchangeItemFromNpc::stExchangeItemFromNpc()
    : ParamBase(), error_code(0), rewards() {}

stExchangeItemFromNpc::~stExchangeItemFromNpc() {}

template class IPacketDispatcher<MSG_EXCHANGE_ITEM_FROM_NPC,
                                 stExchangeItemFromNpc,
                                 static_cast<ch_state>(3)>;

Dispatcher_ExchangeItemFromNpc::Dispatcher_ExchangeItemFromNpc() {}

Dispatcher_ExchangeItemFromNpc::~Dispatcher_ExchangeItemFromNpc() {}

int Dispatcher_ExchangeItemFromNpc::read(PacketBuf& packet,
                                         MSG_BASE& base_message) {
    MSG_EXCHANGE_ITEM_FROM_NPC& message =
        static_cast<MSG_EXCHANGE_ITEM_FROM_NPC&>(base_message);
    if (packet.get_short(message.npc_id) != 1) {
        return LineFunc(71801, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.exchange_list_id) != 1) {
        return LineFunc(71802, kReadSignature, 0, 0);
    }
    return 0;
}

int Dispatcher_ExchangeItemFromNpc::process(CUser* user,
                                            MSG_BASE& base_message,
                                            ParamBase& base_param) {
    const MSG_EXCHANGE_ITEM_FROM_NPC& message =
        static_cast<const MSG_EXCHANGE_ITEM_FROM_NPC&>(base_message);
    stExchangeItemFromNpc& result =
        static_cast<stExchangeItemFromNpc&>(base_param);

    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !user->getCurCharacInvenR()) {
        result.error_code = kErrorInvalidState;
        return 0;
    }
    if (!IsAuthorizedTownExchange(user, message.npc_id)) {
        result.error_code = kErrorUnavailable;
        return 0;
    }

    const NpcExchangeRule* const rule =
        GetNpcExchangeConfig().find_by_npc(message.npc_id);
    if (!rule || message.exchange_list_id != rule->list_id) {
        result.error_code = kErrorUnavailable;
        return 0;
    }

    int source_slot = -1;
    const Inven_Item* source = 0;
    if (!FindUnlockedSource(user, *rule, source_slot, source)) {
        result.error_code = kErrorUnavailable;
        return 0;
    }

    // Delete the exact source on a shadow copy, then use the side-effect-free
    // try-insert path.  Check every possible reward before rolling so a full
    // inventory cannot bias the weighted distribution through free retries.
    // This accounts for the freed source slot and stack merging without
    // emitting history/DB-log records during the preview.
    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*user->getCurCharacInvenRefR());
    if (preview.delete_item(static_cast<INVEN_TYPE>(kMainInventory),
                            source_slot,
                            rule->source_count,
                            kNpcExchangeDeleteReason,
                            false) != 1) {
        result.error_code = kErrorUnavailable;
        return 0;
    }
    for (std::vector<NpcExchangeRewardRule>::const_iterator it =
             rule->rewards.begin();
         it != rule->rewards.end();
         ++it) {
        Inven_Item possible_reward;
        if (!MakeStackableReward(*it, possible_reward)) {
            result.error_code = kErrorInvalidState;
            return 0;
        }
        InventoryMemory candidate_memory;
        CInventory candidate;
        candidate.SetInventoryMemory(&candidate_memory);
        candidate.setCopy(preview);
        if (candidate.tryInsertItemIntoInventory(possible_reward) < 0) {
            result.error_code = kErrorInventoryFull;
            return 0;
        }
    }

    NpcExchangeRewardRule selected;
    Inven_Item reward_item;
    if (!GetNpcExchangeConfig().select_reward(*rule, selected) ||
        !MakeStackableReward(selected, reward_item)) {
        result.error_code = kErrorInvalidState;
        return 0;
    }

    CInventory* const live = user->getCurCharacInvenW();
    if (!live) {
        result.error_code = kErrorInvalidState;
        return 0;
    }
    const Inven_Item source_snapshot = *source;
    if (live->delete_item(static_cast<INVEN_TYPE>(kMainInventory),
                          source_slot,
                          rule->source_count,
                          kNpcExchangeDeleteReason,
                          false) != 1) {
        result.error_code = kErrorUnavailable;
        return 0;
    }

    const int inserted_slot = live->insertItemIntoInventory(
        reward_item, kNpcExchangeStagingAddReason, false, true);
    if (inserted_slot < 0) {
        // Neither mutation emitted history/DBLog yet.  Restoring the source is
        // therefore a complete compensation, rather than a state-only repair
        // that leaves a phantom delete/add audit record behind.
        if (live->update_item(static_cast<INVEN_TYPE>(kMainInventory),
                              source_slot,
                              source_snapshot) != 1) {
            cMyTrace trace(
                "Dispatcher_ExchangeItemFromNpc::process rollback",
                __LINE__,
                5);
            trace("CMD718 source rollback failed charac(%d) slot(%d) item(%d)",
                  user->get_charac_no(-1),
                  source_slot,
                  source_snapshot.item_id_);
        }
        result.error_code = kErrorInventoryFull;
        return 0;
    }

    CommitNpcExchangeAudit(user,
                           live,
                           source_snapshot,
                           rule->source_count,
                           inserted_slot,
                           reward_item);

    NpcExchangeRewardResult reward;
    reward.item_id = static_cast<uint32_t>(selected.item_id);
    reward.count = static_cast<uint16_t>(selected.count);
    result.rewards.push_back(reward);
    result.error_code = 0;
    return 0;
}

void Dispatcher_ExchangeItemFromNpc::send(CUser* user,
                                          ParamBase& base_param) {
    if (!user) {
        return;
    }
    const stExchangeItemFromNpc& result =
        static_cast<const stExchangeItemFromNpc&>(base_param);
    if (result.error_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EXCHANGE_ITEM_FROM_NPC,
                                 result.error_code);
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_EXCHANGE_ITEM_FROM_NPC);
    packet.put_byte(1);
    packet.put_short(static_cast<int>(result.rewards.size()));
    for (std::vector<NpcExchangeRewardResult>::const_iterator it =
             result.rewards.begin();
         it != result.rewards.end();
         ++it) {
        packet.put_int(static_cast<int>(it->item_id));
        packet.put_short(static_cast<int>(it->count));
    }
    packet.finalize(true);
    user->Send(packet);
}
