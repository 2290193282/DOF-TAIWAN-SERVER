#include "old_game/economy/inventory/packet_dispatcher_use_chestnut_stone.h"

#include <stdint.h>
#include <string>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace {

const char kChestnutStonePvf[] = "exchangeitemnpc.etc";
const int kInventoryStackable = 1;
const int kWeightScale = 10000;
const uint8_t kErrorInventoryFull = 4;
const uint8_t kErrorInvalidState = 7;
const uint8_t kErrorInvalidItem = 17;
const uint8_t kErrorNotAuthorized = 23;
const eItemDelReason kChestnutStoneDeleteReason =
    static_cast<eItemDelReason>(3);
const eItemAddReason kChestnutStoneAddReason =
    static_cast<eItemAddReason>(8);

const char kReadSignature[] =
    "virtual int Dispatcher_UseChestnutStone::read(PacketBuf&, MSG_BASE&)";

struct ChestnutRewardRule {
    ChestnutRewardRule() : weight(0), item_id(0), count(0) {}

    int weight;
    int item_id;
    int count;
};

struct ChestnutNpcRule {
    ChestnutNpcRule()
        : npc_id(0),
          list_count(0),
          source_item_id(0),
          source_count(0),
          has_list(false),
          has_source(false),
          has_rewards(false),
          rewards() {}

    int npc_id;
    int list_count;
    int source_item_id;
    int source_count;
    bool has_list;
    bool has_source;
    bool has_rewards;
    std::vector<ChestnutRewardRule> rewards;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool IsValidItemPrototype(int item_id, bool require_stackable) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const item = data_manager ? data_manager->find_item(item_id) : 0;
    return item && item->vtable_ &&
           (!require_stackable || item->is_stackable() == 1);
}

bool IsValidRuleSet(const std::vector<ChestnutNpcRule>& rules) {
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager || rules.empty()) {
        return false;
    }

    for (size_t index = 0; index < rules.size(); ++index) {
        const ChestnutNpcRule& rule = rules[index];
        if (rule.npc_id <= 0 || rule.list_count != 1 ||
            rule.source_item_id <= 0 || rule.source_count != 1 ||
            !rule.has_list || !rule.has_source || !rule.has_rewards ||
            rule.rewards.empty() || !data_manager->find_npc(rule.npc_id) ||
            !IsValidItemPrototype(rule.source_item_id, true)) {
            return false;
        }

        int total_weight = 0;
        for (size_t reward_index = 0;
             reward_index < rule.rewards.size();
             ++reward_index) {
            const ChestnutRewardRule& reward = rule.rewards[reward_index];
            if (reward.weight <= 0 || reward.item_id <= 0 ||
                reward.count <= 0 ||
                total_weight > kWeightScale - reward.weight ||
                !IsValidItemPrototype(reward.item_id, true)) {
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

// Typed, lazy importer for the 0828-only etc/exchangeitemnpc.etc container.
// It deliberately rejects non-stackable rewards because the currently
// proven client/PVF closure has equipment_extra=0 for all configured rows.
// A future PVF that introduces equipment must first restore that equipment
// instance field instead of silently serializing a zero.
class ChestnutExchangeConfig {
public:
    ChestnutExchangeConfig() : loaded_(false), rules_() {}

    const ChestnutNpcRule* find_by_source(int source_item_id) {
        if (!load()) {
            return 0;
        }
        for (std::vector<ChestnutNpcRule>::const_iterator it = rules_.begin();
             it != rules_.end();
             ++it) {
            if (it->source_item_id == source_item_id) {
                return &*it;
            }
        }
        return 0;
    }

    bool select_reward(const ChestnutNpcRule& rule,
                       ChestnutRewardRule& selected) const {
        if (rule.rewards.empty()) {
            return false;
        }
        // The restored get_rand_int(limit) can return limit on its equality
        // branch.  Normalize that legacy edge back to the PVF's 0..9999
        // domain before applying cumulative weights.
        const int roll = static_cast<int>(
            get_rand_int(kWeightScale) %
            static_cast<unsigned int>(kWeightScale));
        int accumulated = 0;
        for (std::vector<ChestnutRewardRule>::const_iterator it =
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
        if (!loadRDARScriptFile("Etc/", kChestnutStonePvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        std::vector<ChestnutNpcRule> parsed;
        ChestnutNpcRule current;
        bool in_npc = false;
        std::string tag;
        bool success = false;

        while (ScanType(tag, true) == 1) {
            if (tag == "[npc]") {
                if (in_npc) {
                    return false;
                }
                current = ChestnutNpcRule();
                current.npc_id = ScanInt(&success);
                if (!success) {
                    return false;
                }
                in_npc = true;
            } else if (tag == "[list]" && in_npc) {
                if (current.has_list) {
                    return false;
                }
                current.list_count = ScanInt(&success);
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
                    ChestnutRewardRule reward;
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
                current = ChestnutNpcRule();
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
    std::vector<ChestnutNpcRule> rules_;
};

ChestnutExchangeConfig& GetChestnutExchangeConfig() {
    static ChestnutExchangeConfig config;
    return config;
}

bool IsAuthorizedTownUse(CUser* user) {
    // RECONSTRUCTED_AUTHORIZATION_RULE: CMD431 contains no NPC index and the
    // restored user layout has no current-NPC interaction owner.  The narrow
    // server-verifiable gate is therefore: character-ready town session,
    // outside dungeon/PVP, plus an exact source item selected from a typed PVF
    // node whose NPC prototype exists.  This is not claimed as the official
    // 0828 event-window predicate.
    return user && user->get_state() == 3 && user->getCurCharacR() &&
           user->getCurCharacInvenR() && user->getMoveSpace() != 1 &&
           G_GameWorld() && !G_GameWorld()->IsPVPChannel();
}

bool MakeStackableReward(const ChestnutRewardRule& rule,
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
    return item.item_category_ == kInventoryStackable;
}

}  // namespace

MSG_USE_CHESTNUT_STONE::MSG_USE_CHESTNUT_STONE()
    : MSG_BASE(), inventory_slot(0) {}

ChestnutStoneRewardResult::ChestnutStoneRewardResult()
    : inventory_slot(0),
      item_id(0),
      final_stack_count(0),
      equipment_extra(0) {}

stUseChestnutStone::stUseChestnutStone()
    : ParamBase(), error_code(0), consumed_item_id(0), rewards() {}

stUseChestnutStone::~stUseChestnutStone() {}

template class IPacketDispatcher<MSG_USE_CHESTNUT_STONE,
                                 stUseChestnutStone,
                                 static_cast<ch_state>(3)>;

Dispatcher_UseChestnutStone::Dispatcher_UseChestnutStone() {}

Dispatcher_UseChestnutStone::~Dispatcher_UseChestnutStone() {}

int Dispatcher_UseChestnutStone::read(PacketBuf& packet,
                                      MSG_BASE& base_message) {
    MSG_USE_CHESTNUT_STONE& message =
        static_cast<MSG_USE_CHESTNUT_STONE&>(base_message);
    if (packet.get_short(message.inventory_slot) != 1) {
        return LineFunc(43101, kReadSignature, 0, 0);
    }
    return 0;
}

int Dispatcher_UseChestnutStone::process(CUser* user,
                                         MSG_BASE& base_message,
                                         ParamBase& base_param) {
    const MSG_USE_CHESTNUT_STONE& message =
        static_cast<const MSG_USE_CHESTNUT_STONE&>(base_message);
    stUseChestnutStone& result =
        static_cast<stUseChestnutStone&>(base_param);

    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !user->getCurCharacInvenR()) {
        result.error_code = kErrorInvalidState;
        return 0;
    }
    if (!IsAuthorizedTownUse(user)) {
        result.error_code = kErrorNotAuthorized;
        return 0;
    }

    const CInventory* const read_inventory = user->getCurCharacInvenR();
    const Inven_Item* const source = read_inventory->GetInvenRef(
        kInventoryStackable, static_cast<int>(message.inventory_slot));
    if (!source || source->item_id_ <= 0 || source->get_add_info() <= 0) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }

    const ChestnutNpcRule* const rule =
        GetChestnutExchangeConfig().find_by_source(source->item_id_);
    if (!rule || source->get_add_info() < rule->source_count) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }
    if (user->CheckItemLock(kInventoryStackable,
                            static_cast<int>(message.inventory_slot))) {
        result.error_code = kErrorNotAuthorized;
        return 0;
    }

    ChestnutRewardRule selected;
    Inven_Item reward_item;
    if (!GetChestnutExchangeConfig().select_reward(*rule, selected) ||
        !MakeStackableReward(selected, reward_item)) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }

    // First execute the exact delete/add pair on an isolated inventory copy.
    // This proves capacity (including stack merge behavior) before the live
    // source is consumed and keeps a normal full-inventory failure retryable.
    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*user->getCurCharacInvenRefR());
    if (preview.delete_item(
            static_cast<INVEN_TYPE>(kInventoryStackable),
            static_cast<int>(message.inventory_slot),
            rule->source_count,
            kChestnutStoneDeleteReason,
            false) != 1) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }
    if (preview.insertItemIntoInventory(
            reward_item, kChestnutStoneAddReason, false, true) < 0) {
        result.error_code = kErrorInventoryFull;
        return 0;
    }

    CInventory* const live = user->getCurCharacInvenW();
    if (!live) {
        result.error_code = kErrorInvalidState;
        return 0;
    }
    const Inven_Item source_snapshot = *source;
    if (live->delete_item(
            static_cast<INVEN_TYPE>(kInventoryStackable),
            static_cast<int>(message.inventory_slot),
            rule->source_count,
            kChestnutStoneDeleteReason,
            true) != 1) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }

    const int inserted_slot = live->insertItemIntoInventory(
        reward_item, kChestnutStoneAddReason, true, true);
    if (inserted_slot < 0) {
        // NO_XA_BOUNDARY: inventory is an in-memory blob persisted later, not
        // a database transaction.  Restore the exact source record so the
        // failed command is not material-destructive.  History logging may
        // still contain the attempted delete; no official cross-log XA owner
        // exists in the restored server.
        (void)live->update_item(
            static_cast<INVEN_TYPE>(kInventoryStackable),
            static_cast<int>(message.inventory_slot),
            source_snapshot);
        result.error_code = kErrorInventoryFull;
        return 0;
    }

    const Inven_Item* const final_item =
        live->GetInvenRef(kInventoryStackable, inserted_slot);
    uint32_t final_stack_count = static_cast<uint32_t>(selected.count);
    if (final_item && final_item->item_id_ == selected.item_id &&
        final_item->get_add_info() >= 0) {
        final_stack_count =
            static_cast<uint32_t>(final_item->get_add_info());
    } else {
        // The live insert has already committed.  Do not turn a diagnostic
        // lookup anomaly into a retryable error and duplicate the reward.
        cMyTrace trace(
            "Dispatcher_UseChestnutStone::process(CUser*,MSG_BASE&,ParamBase&)",
            43102,
            4);
        trace("CMD431 final slot mismatch charac_no(%d) slot(%d) item(%d)",
              user->getCurCharacNo(), inserted_slot, selected.item_id);
    }

    ChestnutStoneRewardResult reward_result;
    reward_result.inventory_slot = static_cast<uint16_t>(inserted_slot);
    reward_result.item_id = static_cast<uint32_t>(selected.item_id);
    reward_result.final_stack_count = final_stack_count;
    // CLIENT_IDA_EVIDENCE: sub_997090 consumes this u16 only for equipment.
    // The typed importer rejects equipment for the current 0828 table, so the
    // business value is genuinely zero rather than unknown padding.
    reward_result.equipment_extra = 0;

    result.consumed_item_id =
        static_cast<uint32_t>(rule->source_item_id);
    result.rewards.push_back(reward_result);
    result.error_code = 0;
    return 0;
}

void Dispatcher_UseChestnutStone::send(CUser* user,
                                       ParamBase& base_param) {
    if (!user) {
        return;
    }
    const stUseChestnutStone& result =
        static_cast<const stUseChestnutStone&>(base_param);
    if (result.error_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_CHESTNUT_STONE_ITEM,
                                 result.error_code);
        return;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_CHESTNUT_STONE_ITEM);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(result.consumed_item_id));
    packet.put_byte(static_cast<int>(result.rewards.size()));
    for (std::vector<ChestnutStoneRewardResult>::const_iterator it =
             result.rewards.begin();
         it != result.rewards.end();
         ++it) {
        packet.put_short(static_cast<int>(it->inventory_slot));
        packet.put_int(static_cast<int>(it->item_id));
        packet.put_int(static_cast<int>(it->final_stack_count));
        packet.put_short(static_cast<int>(it->equipment_extra));
    }
    packet.finalize(true);
    user->Send(packet);
}
