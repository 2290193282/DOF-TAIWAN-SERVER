#include "old_game/activity/schedule/advance_altar_data.h"

#include "old_game/activity/schedule/advance_altar_dispatchers.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/activity/reward/advance_altar.h"
#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "old_game/activity/reward/advance_altar_config_parameter.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/world_value_types.h"

#include <cstring>

namespace advancealtar {

typedef char SlotItem_Size_Must_Be_0x0c[
    (sizeof(_SlotItem) == 0x0c) ? 1 : -1];
typedef char Slot_Size_Must_Be_0x0a[(sizeof(_Slot) == 0x0a) ? 1 : -1];
typedef char Stage_Size_Must_Be_0x08[(sizeof(_Stage) == 0x08) ? 1 : -1];
typedef char BuyItem_Size_Must_Be_0x0c[
    (sizeof(_BuyItem) == 0x0c) ? 1 : -1];
typedef char Reward_Size_Must_Be_0x05[
    (sizeof(_Reward) == 0x05) ? 1 : -1];
typedef char CharacAdvanceAltarDbData_Size_Must_Be_0x51c[
    (sizeof(_CharacAdvanceAltarDbData) == 0x51c) ? 1 : -1];
typedef char AdvanceAltarDbData_Size_Must_Be_0x518[
    (sizeof(_AdvanceAltarDbData) == 0x518) ? 1 : -1];
typedef char AdvanceAltarData_Size_Must_Be_0xd0[
    (sizeof(AdvanceAltarData) == 0xd0) ? 1 : -1];
typedef char CharacAdvanceAltarManager_Size_Must_Be_0xd4[
    (sizeof(CharacAdvanceAltarManager) == 0xd4) ? 1 : -1];

namespace {

const int kAdvanceAltarStageCount = 50;
const int kAdvanceAltarSlotCount = 12;
const int kAdvanceAltarBuyItemCount = 50;
const int kAdvanceAltarRewardCount = 30;
// IDA data _ZN12advancealtarL11kRewardStarE at 0x8AF2E40:
// difficulty 0/1/2 yields 3/2/1 stars respectively.
const int kRewardStar[] = {3, 2, 1};

// IDA data: _ZN12advancealtarL15kUnitSlotNoLockE @ 0x8AF2E58 and
// _ZN12advancealtarL16kSkillSlotNoLockE @ 0x8AF2E5E.  These six-entry
// unlock masks are consumed by setDefaultSlot at 0x8132F8E.
const uint8_t kUnitSlotNoLock[6] = {0, 0, 0, 0, 0, 0};
const uint8_t kSkillSlotNoLock[6] = {0, 0, 0, 0, 1, 1};

AdvanceAltarShopParameter* GetAdvanceAltarShopParameter() {
    CDataManager* data_manager = G_CDataManager();
    if (!data_manager) {
        return 0;
    }
    return data_manager->get_advance_altar_shop_parameter();
}

}  // namespace

// IDA_EA: 0x81347E2
// IDA_NAME: _ZN12advancealtar9_SlotItemC2Ev
// DEMANGLED: advancealtar::_SlotItem::_SlotItem(void)
_SlotItem::_SlotItem() {
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x813449A
// IDA_NAME: _ZN12advancealtar5_SlotC2Ev
// DEMANGLED: advancealtar::_Slot::_Slot(void)
_Slot::_Slot() {
    std::memset(this, 0, 0x0au);
}

// IDA_EA: 0x8134452
// IDA_NAME: _ZN12advancealtar6_StageC2Ev
// DEMANGLED: advancealtar::_Stage::_Stage(void)
_Stage::_Stage() {
    std::memset(this, 0, 0x08u);
}

// IDA_EA: 0x8134476
// IDA_NAME: _ZN12advancealtar8_BuyItemC2Ev
// DEMANGLED: advancealtar::_BuyItem::_BuyItem(void)
_BuyItem::_BuyItem() {
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x813B4CC
// IDA_NAME: _ZN12advancealtar8_BuyItemD2Ev
// DEMANGLED: advancealtar::_BuyItem::~_BuyItem()
_BuyItem::~_BuyItem() {}

// IDA_EA: 0x848F560
// IDA_NAME: _ZN12advancealtar25_CharacAdvanceAltarDbDataC2Ev
// DEMANGLED: advancealtar::_CharacAdvanceAltarDbData::_CharacAdvanceAltarDbData(void)
_CharacAdvanceAltarDbData::_CharacAdvanceAltarDbData() {
    std::memset(this, 0, 0x51cu);
}

// IDA_EA: 0x889911E
// IDA_NAME: _ZN12advancealtar16AdvanceAltarDataC2Ev
// DEMANGLED: advancealtar::AdvanceAltarData::AdvanceAltarData(void)
AdvanceAltarData::AdvanceAltarData()
    : ridable_id(0),
      free_ticket_count(0),
      charged_ticket_count(0),
      star_game(0),
      star_cera(0),
      star_usable(0),
      survival_best(0),
      star_reset_count(0),
      unknown_1c(0),
      is_unlock_stage_effect(0),
      stages_24(),
      unit_buy_items_3c(),
      skill_buy_items_54(),
      tower_buy_items_6c(),
      unit_slots_84(),
      skill_slots_9c(),
      rewards_b4(),
      updated(0) {}

// IDA_EA: 0x8134532
// IDA_NAME: _ZN12advancealtar16AdvanceAltarDataD2Ev
// DEMANGLED: advancealtar::AdvanceAltarData::~AdvanceAltarData()
AdvanceAltarData::~AdvanceAltarData() {}

// IDA_EA: 0x8899244
// IDA_NAME: _ZN12advancealtar16AdvanceAltarData5resetEv
// DEMANGLED: advancealtar::AdvanceAltarData::reset(void)
int AdvanceAltarData::reset() {
    ridable_id = 0;
    free_ticket_count = 0;
    charged_ticket_count = 0;
    star_game = 0;
    star_cera = 0;
    star_usable = 0;
    survival_best = 0;
    star_reset_count = 0;
    unknown_1c = 0;
    is_unlock_stage_effect = 0;
    updated = 0;
    stages_24.clear();
    unit_buy_items_3c.clear();
    skill_buy_items_54.clear();
    tower_buy_items_6c.clear();
    unit_slots_84.clear();
    skill_slots_9c.clear();
    rewards_b4.clear();
    return 0;
}

// IDA_EA: 0x88992DA
// IDA_NAME: _ZN12advancealtar16AdvanceAltarData3setERKNS_19_AdvanceAltarDbDataE
// DEMANGLED: advancealtar::AdvanceAltarData::set(advancealtar::_AdvanceAltarDbData const&)
bool AdvanceAltarData::set(const _AdvanceAltarDbData& db_data) {
    updated = 0;
    ridable_id = db_data.ridable_id;
    free_ticket_count = db_data.free_ticket_count;
    charged_ticket_count = db_data.charged_ticket_count;
    star_game = db_data.star_game;
    star_cera = db_data.star_cera;
    star_usable = db_data.star_usable;
    survival_best = db_data.survival_best;
    star_reset_count = db_data.star_reset_count;
    unknown_1c = db_data.unknown_1c;
    is_unlock_stage_effect = db_data.is_unlock_stage_effect;

    const _Stage* stages = db_data.stages;
    for (int index = 0; index < kAdvanceAltarStageCount; ++index) {
        if (stages[index].stage_id > 0) {
            stages_24.insert(std::make_pair(stages[index].stage_id,
                                            stages[index]));
        }
    }

    // RECONSTRUCTED_0828_MIGRATION: the old StageMapList loader used the
    // stage id as its ordering ordinal.  For the first ridable that made the
    // survival node sort before every normal node, and one historical default
    // row could therefore be persisted with only that survival node.  Repair
    // only that exact legacy shape.  Normal progress (including sparse or
    // future PVF-defined records) belongs to the official transition logic and
    // must never be normalized or discarded here.
    bool repaired_survival_only_stage = false;
    CDataManager* const data_manager = G_CDataManager();
    const StageMapList* const stage_maps =
        data_manager ? data_manager->get_stage_map_list() : 0;
    if (ridable_id == RidableId::FIRST &&
        stage_maps &&
        stages_24.size() == 1u) {
        const int first_stage_index = stage_maps->getFirstStageIndex();
        const int stored_stage_index = stages_24.begin()->first;
        const std::map<int, StageMapList::_StageMapSummary>::const_iterator
            first_summary = stage_maps->stage_map.find(first_stage_index);
        const std::map<int, StageMapList::_StageMapSummary>::const_iterator
            stored_summary = stage_maps->stage_map.find(stored_stage_index);
        if (first_stage_index > 0 &&
            first_stage_index != stored_stage_index &&
            first_summary != stage_maps->stage_map.end() &&
            first_summary->second.is_survival_stage == 0 &&
            stored_summary != stage_maps->stage_map.end() &&
            stored_summary->second.is_survival_stage != 0 &&
            stages_24.find(first_stage_index) == stages_24.end()) {
            _Stage first_stage;
            first_stage.stage_id = first_stage_index;
            first_stage.star = StageDifficulty::NONE;
            stages_24.insert(std::make_pair(first_stage.stage_id,
                                            first_stage));
            repaired_survival_only_stage = true;
        }
    }

    const _Slot* slots = db_data.slots;
    for (int index = 0; index < kAdvanceAltarSlotCount; ++index) {
        if (slots[index].slot_type == 1) {
            unit_slots_84.insert(std::make_pair(slots[index].slot_no,
                                                slots[index]));
        } else if (slots[index].slot_type == 2) {
            skill_slots_9c.insert(std::make_pair(slots[index].slot_no,
                                                 slots[index]));
        }
    }

    // The reconstructed serializer used to consume every skill source entry
    // while scanning the six occupied unit destinations.  Repair that exact
    // historical blob shape during normal load: a live altar record with unit
    // slots must also own skill slots 0..5.  Existing skill slot/item state is
    // never overwritten, and updated=1 lets the regular checkout persist the
    // repaired invariant without a direct DB rewrite.
    bool repaired_missing_skill_slot = false;
    if (!unit_slots_84.empty()) {
        for (uint8_t slot_no = 0; slot_no <= 5; ++slot_no) {
            if (skill_slots_9c.find(slot_no) != skill_slots_9c.end()) {
                continue;
            }
            _Slot slot;
            slot.locked = kSkillSlotNoLock[slot_no];
            slot.slot_type = AdvanceAltarSlotType::SKILL;
            slot.slot_no = slot_no;
            slot.item_id = 0;
            skill_slots_9c.insert(std::make_pair(slot_no, slot));
            repaired_missing_skill_slot = true;
        }
    }

    const _BuyItem* buy_items = db_data.buy_items;
    for (int index = 0; index < kAdvanceAltarBuyItemCount; ++index) {
        if (buy_items[index].item_id > 0) {
            std::map<int, _BuyItem>* target =
                getBuyItemMap(static_cast<AdvanceAltarShopType::T>(
                    buy_items[index].shop_type));
            if (target) {
                target->insert(std::make_pair(buy_items[index].item_id,
                                              buy_items[index]));
            }
        }
    }

    const _Reward* rewards = db_data.rewards;
    for (int index = 0; index < kAdvanceAltarRewardCount; ++index) {
        if (rewards[index].reward_id > 0) {
            rewards_b4.insert(std::make_pair(rewards[index].reward_id,
                                             rewards[index]));
        }
    }
    if (repaired_missing_skill_slot) {
        updated = 1;
    }
    if (repaired_survival_only_stage) {
        updated = 1;
    }
    return true;
}

// IDA_EA: 0x88996D8
// IDA_NAME: _ZNK12advancealtar16AdvanceAltarData3getERNS_19_AdvanceAltarDbDataE
// DEMANGLED: advancealtar::AdvanceAltarData::get(advancealtar::_AdvanceAltarDbData &)const
bool AdvanceAltarData::get(_AdvanceAltarDbData& db_data) const {
    std::memset(&db_data, 0, sizeof(db_data));
    db_data.ridable_id = ridable_id;
    db_data.free_ticket_count = free_ticket_count;
    db_data.charged_ticket_count = charged_ticket_count;
    db_data.star_game = star_game;
    db_data.star_cera = star_cera;
    db_data.star_usable = star_usable;
    db_data.survival_best = survival_best;
    db_data.star_reset_count = star_reset_count;
    db_data.unknown_1c = unknown_1c;
    db_data.is_unlock_stage_effect = is_unlock_stage_effect;
    db_data.updated = updated;

    _Stage* stages = db_data.stages;
    int stage_count = 0;
    for (std::map<int, _Stage>::const_iterator it = stages_24.begin();
         it != stages_24.end() && stage_count < kAdvanceAltarStageCount;
         ++it) {
        if (it->second.stage_id > 0) {
            stages[stage_count++] = it->second;
        }
    }

    getSlot(unit_slots_84, db_data.slots,
            kAdvanceAltarSlotCount);
    getSlot(skill_slots_9c, db_data.slots,
            kAdvanceAltarSlotCount);
    getBuyItem(unit_buy_items_3c, db_data.buy_items,
               kAdvanceAltarBuyItemCount);
    getBuyItem(skill_buy_items_54, db_data.buy_items,
               kAdvanceAltarBuyItemCount);
    getBuyItem(tower_buy_items_6c, db_data.buy_items,
               kAdvanceAltarBuyItemCount);

    _Reward* rewards = db_data.rewards;
    int reward_count = 0;
    for (std::map<int, _Reward>::const_iterator it = rewards_b4.begin();
         it != rewards_b4.end() && reward_count < kAdvanceAltarRewardCount;
         ++it) {
        if (it->second.reward_id > 0) {
            rewards[reward_count++] = it->second;
        }
    }
    return true;
}

// IDA_EA: 0x88999D8
// IDA_NAME: _ZN12advancealtar16AdvanceAltarData13getBuyItemMapENS_20AdvanceAltarShopType1TE
// DEMANGLED: advancealtar::AdvanceAltarData::getBuyItemMap(advancealtar::AdvanceAltarShopType::T)
std::map<int, _BuyItem>* AdvanceAltarData::getBuyItemMap(
    AdvanceAltarShopType::T shop_type) {
    if (shop_type == AdvanceAltarShopType::SKILL) {
        return &skill_buy_items_54;
    }
    if (shop_type == AdvanceAltarShopType::TOWER) {
        return &tower_buy_items_6c;
    }
    if (shop_type != AdvanceAltarShopType::UNIT) {
        return 0;
    }
    return &unit_buy_items_3c;
}

// IDA_EA: 0x8899A0C
// IDA_NAME: _ZN12advancealtar16AdvanceAltarData10getSlotMapENS_20AdvanceAltarSlotType1TE
// DEMANGLED: advancealtar::AdvanceAltarData::getSlotMap(advancealtar::AdvanceAltarSlotType::T)
std::map<uint8_t, _Slot>* AdvanceAltarData::getSlotMap(
    AdvanceAltarSlotType::T slot_type) {
    if (slot_type == AdvanceAltarSlotType::UNIT) {
        return &unit_slots_84;
    }
    if (slot_type == AdvanceAltarSlotType::SKILL) {
        return &skill_slots_9c;
    }
    return 0;
}

// IDA_EA: 0x8899A3A
// IDA_NAME: _ZNK12advancealtar16AdvanceAltarData10getBuyItemERKSt3mapIiNS_8_BuyItemESt4lessIiESaISt4pairIKiS2_EEEPS2_i
// DEMANGLED: advancealtar::AdvanceAltarData::getBuyItem(std::map<int,advancealtar::_BuyItem,std::less<int>,std::allocator<std::pair<int const,advancealtar::_BuyItem>>> const&,advancealtar::_BuyItem*,int)const
bool AdvanceAltarData::getBuyItem(
    const std::map<int, _BuyItem>& source,
    _BuyItem* dest,
    int count) const {
    int out_index = 0;
    std::map<int, _BuyItem>::const_iterator it = source.begin();
    while (it != source.end() && out_index < count) {
        if (it->second.item_id <= 0) {
            ++it;
            continue;
        }
        if (dest[out_index].item_id <= 0) {
            dest[out_index] = it->second;
            ++it;
        }
        ++out_index;
    }
    return out_index < count;
}

// IDA_EA: 0x8899B4E
// IDA_NAME: _ZNK12advancealtar16AdvanceAltarData7getSlotERKSt3mapIhNS_5_SlotESt4lessIhESaISt4pairIKhS2_EEEPS2_i
// DEMANGLED: advancealtar::AdvanceAltarData::getSlot(std::map<unsigned char,advancealtar::_Slot,std::less<unsigned char>,std::allocator<std::pair<unsigned char const,advancealtar::_Slot>>> const&,advancealtar::_Slot*,int)const
bool AdvanceAltarData::getSlot(
    const std::map<uint8_t, _Slot>& source,
    _Slot* dest,
    int count) const {
    int out_index = 0;
    std::map<uint8_t, _Slot>::const_iterator it = source.begin();
    while (it != source.end() && out_index < count) {
        if (!it->second.slot_type) {
            ++it;
            continue;
        }
        if (!dest[out_index].slot_type) {
            dest[out_index] = it->second;
            ++it;
        }
        ++out_index;
    }
    return out_index < count;
}

// IDA_EA: 0x813119E
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManagerC2Ev
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::CharacAdvanceAltarManager(void)
CharacAdvanceAltarManager::CharacAdvanceAltarManager()
    : current(0),
      data() {}

// IDA_EA: 0x813121A
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManagerD2Ev
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::~CharacAdvanceAltarManager()
CharacAdvanceAltarManager::~CharacAdvanceAltarManager() {}

// IDA_EA: 0x8131254
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager26setCurrentAdvanceAltarDataENS_9RidableId1TE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::setCurrentAdvanceAltarData(advancealtar::RidableId::T)
int CharacAdvanceAltarManager::setCurrentAdvanceAltarData(
    RidableId::T ridable_id) {
    if (!isValidRidableId(ridable_id)) {
        return 0;
    }
    for (int index = 0; index <= 0; ++index) {
        if (data[index].ridable_id == ridable_id) {
            current = &data[index];
            return 1;
        }
    }
    current = 0;
    return 0;
}

// IDA_EA: 0x81312D0
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager19getCurrentRidableIdEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getCurrentRidableId(void)const
int CharacAdvanceAltarManager::getCurrentRidableId() const {
    if (current) {
        return current->ridable_id;
    }
    return 0;
}

// IDA_EA: 0x81312EC
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager17isAvailableTciketEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::isAvailableTciket(void)const
bool CharacAdvanceAltarManager::isAvailableTciket() const {
    return current && current->free_ticket_count + current->charged_ticket_count > 0;
}

// IDA_EA: 0x813131A
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager12isEnterStageEi
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::isEnterStage(int)const
bool CharacAdvanceAltarManager::isEnterStage(int stage_id) const {
    if (!current) {
        return false;
    }
    return current->stages_24.find(stage_id) != current->stages_24.end();
}

// IDA_EA: 0x8132638
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager11getSlotItemERSt3mapIhNS_9_SlotItemESt4lessIhESaISt4pairIKhS2_EEE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getSlotItem(std::map<unsigned char,advancealtar::_SlotItem> &)const
char CharacAdvanceAltarManager::getSlotItem(
    std::map<uint8_t, _SlotItem>& slot_items) const {
    if (!current) {
        return 0;
    }

    AdvanceAltarShopParameter* const parameter =
        GetAdvanceAltarShopParameter();
    if (!parameter) {
        return 0;
    }
    std::vector<BuyUpgradeData>* const upgrades =
        parameter->getBuyUpgradeDataVec(current->ridable_id,
                                        AdvanceAltarShopType::UNIT);
    if (!upgrades) {
        return 0;
    }

    char result = 0;
    for (std::map<uint8_t, _Slot>::const_iterator slot =
             current->unit_slots_84.begin();
         slot != current->unit_slots_84.end(); ++slot) {
        const std::map<int, _BuyItem>::const_iterator bought =
            current->unit_buy_items_3c.find(slot->second.item_id);
        if (bought == current->unit_buy_items_3c.end()) {
            continue;
        }

        BuyUpgradeData* upgrade = 0;
        for (std::vector<BuyUpgradeData>::iterator it = upgrades->begin();
             it != upgrades->end(); ++it) {
            if (it->id == bought->second.item_id) {
                upgrade = &*it;
                break;
            }
        }
        if (!upgrade) {
            continue;
        }

        FieldData* const item_field = upgrade->getFieldDataPoint(
            bought->second.buy_count, FieldType::INDEX);
        if (!item_field) {
            continue;
        }
        CItem* const item = G_CDataManager()->find_item(item_field->int_value);
        if (!item) {
            continue;
        }

        _SlotItem slot_item;
        slot_item.slot_no = slot->second.slot_no;
        slot_item.item_id = item_field->int_value;
        slot_item.required_gage = item->getAdvanceAltarNeedGage();
        slot_items.insert(std::make_pair(slot_item.slot_no, slot_item));
        result = 1;
    }
    return result;
}

// IDA_EA: 0x81328EA
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager12getTowerInfoEiRiS1_
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getTowerInfo(int,int &,int &)const
int CharacAdvanceAltarManager::getTowerInfo(int monster_id,
                                            int& tower_index,
                                            int& tower_level) const {
    if (!current) {
        return 0;
    }

    const std::map<int, _BuyItem>::const_iterator bought =
        current->tower_buy_items_6c.find(monster_id);
    if (bought == current->tower_buy_items_6c.end()) {
        return 0;
    }

    AdvanceAltarShopParameter* const parameter =
        GetAdvanceAltarShopParameter();
    if (!parameter) {
        return 0;
    }
    std::vector<BuyUpgradeData>* const upgrades =
        parameter->getBuyUpgradeDataVec(current->ridable_id,
                                        AdvanceAltarShopType::TOWER);
    if (!upgrades) {
        return 0;
    }

    BuyUpgradeData* tower = 0;
    for (std::vector<BuyUpgradeData>::iterator it = upgrades->begin();
         it != upgrades->end(); ++it) {
        if (it->id == monster_id) {
            tower = &*it;
            break;
        }
    }
    if (!tower) {
        return 0;
    }

    FieldData* const field = tower->getFieldDataPoint(
        bought->second.buy_count, FieldType::INDEX);
    if (!field || !bought->second.buy_count) {
        return 0;
    }
    tower_index = field->int_value;
    tower_level = bought->second.buy_count;
    return 1;
}

// IDA_EA: 0x81313B6
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager9decTicketEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::decTicket(void)
int CharacAdvanceAltarManager::decTicket() {
    if (!current) {
        return 0;
    }

    uint8_t dec_succeeded = 0;
    if (current->free_ticket_count <= 0) {
        if (current->charged_ticket_count > 0) {
            --current->charged_ticket_count;
            dec_succeeded = 1;
        }
    } else {
        --current->free_ticket_count;
        dec_succeeded = 1;
    }

    if (dec_succeeded) {
        current->updated = 1;
    }
    return dec_succeeded;
}

// IDA_EA: 0x8131426
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager28checkAndCreateDefaultRidableEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::checkAndCreateDefaultRidable(void)
char CharacAdvanceAltarManager::checkAndCreateDefaultRidable() {
    bool has_existing_ridable = false;
    AdvanceAltarData* empty_ridable = 0;
    char result = 0;

    for (int index = 0; index <= 0; ++index) {
        result = index <= 0;
        AdvanceAltarData& altar_data = data[index];
        if (altar_data.ridable_id != RidableId::FIRST) {
            continue;
        }
        if (altar_data.unit_buy_items_3c.empty() &&
            altar_data.skill_buy_items_54.empty() &&
            altar_data.tower_buy_items_6c.empty()) {
            empty_ridable = &altar_data;
            break;
        }
        has_existing_ridable = true;
    }

    if (has_existing_ridable) {
        return result;
    }

    if (!empty_ridable) {
        empty_ridable = &data[0];
    }
    empty_ridable->updated = 1;
    empty_ridable->ridable_id = RidableId::FIRST;
    setCurrentAdvanceAltarData(RidableId::FIRST);
    setDefaultSlot();
    setDefaultStage();
    setDefaultBuyItem(AdvanceAltarShopType::UNIT);
    setDefaultBuyItem(AdvanceAltarShopType::SKILL);
    setDefaultBuyItem(AdvanceAltarShopType::TOWER);
    empty_ridable->free_ticket_count =
        G_CDataManager()->get_advance_altar_config_parameter()
            ->daily_refill_ticket;
    return static_cast<char>(reinterpret_cast<uintptr_t>(empty_ridable));
}

// IDA_EA: 0x8132CB8
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager17setDefaultBuyItemENS_20AdvanceAltarShopType1TE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::setDefaultBuyItem(advancealtar::AdvanceAltarShopType::T)
int CharacAdvanceAltarManager::setDefaultBuyItem(
    AdvanceAltarShopType::T shop_type) {
    if (!current) {
        return 0;
    }
    std::map<int, _BuyItem>* const buy_item_map =
        current->getBuyItemMap(shop_type);
    if (!buy_item_map) {
        return static_cast<int>(reinterpret_cast<uintptr_t>(current));
    }
    buy_item_map->clear();
    return setUnlockBuyItem(shop_type, 0, 0);
}

// IDA_EA: 0x8132F72
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager15setDefaultStageEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::setDefaultStage(void)
int CharacAdvanceAltarManager::setDefaultStage() {
    return setNextStage(0);
}

// IDA_EA: 0x8132F8E
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager14setDefaultSlotEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::setDefaultSlot(void)
char CharacAdvanceAltarManager::setDefaultSlot() {
    if (!current) {
        return 0;
    }

    for (uint8_t slot_no = 0; slot_no <= 5; ++slot_no) {
        std::map<uint8_t, _Slot>::iterator it =
            current->unit_slots_84.find(slot_no);
        if (it == current->unit_slots_84.end()) {
            _Slot slot;
            slot.locked = kUnitSlotNoLock[slot_no];
            slot.slot_type = AdvanceAltarSlotType::UNIT;
            slot.slot_no = slot_no;
            slot.item_id = 0;
            current->unit_slots_84.insert(std::make_pair(slot_no, slot));
        } else {
            it->second.item_id = 0;
        }
    }

    for (uint8_t slot_no = 0; slot_no <= 5; ++slot_no) {
        std::map<uint8_t, _Slot>::iterator it =
            current->skill_slots_9c.find(slot_no);
        if (it == current->skill_slots_9c.end()) {
            _Slot slot;
            slot.locked = kSkillSlotNoLock[slot_no];
            slot.slot_type = AdvanceAltarSlotType::SKILL;
            slot.slot_no = slot_no;
            slot.item_id = 0;
            current->skill_slots_9c.insert(std::make_pair(slot_no, slot));
        } else {
            it->second.item_id = 0;
        }
    }
    // The old C2 keeps the low byte of its initial current-data pointer in
    // EAX through both loops and returns that byte.  Callers ignore it, but
    // retaining it avoids normalizing this member into a synthetic success
    // predicate.
    return static_cast<char>(reinterpret_cast<uintptr_t>(current));
}

// IDA_EA: 0x813175A
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager10clearStageEiNS_15StageDifficulty1TENS_9StageType1TEiRiRbS6_
int CharacAdvanceAltarManager::clearStage(
    int stage_index,
    StageDifficulty::T difficulty,
    StageType::T stage_type,
    int clear_survival_round_no,
    int& reward_star,
    bool& unlocked_stage,
    bool& improved_survival_record) {
    unlocked_stage = false;
    if (!current) {
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    const StageMapList* const stage_maps =
        data_manager ? data_manager->get_stage_map_list() : 0;
    if (!stage_maps || !stage_maps->isExist(stage_index)) {
        return 0;
    }

    std::map<int, _Stage>::iterator stage = current->stages_24.find(stage_index);
    if (stage == current->stages_24.end()) {
        return 0;
    }

    if (stage_type == StageType::SURVIVAL) {
        if (current->survival_best < clear_survival_round_no) {
            current->survival_best =
                static_cast<uint16_t>(clear_survival_round_no);
            improved_survival_record = true;
            current->updated = 1;
        }
        return 1;
    }

    if (stage->second.star == StageDifficulty::NONE) {
        unlocked_stage = true;
    }
    if (stage->second.star < static_cast<int>(difficulty)) {
        stage->second.star = static_cast<int>(
            StageDifficulty::getNextDifficulty(
                static_cast<StageDifficulty::T>(stage->second.star)));
        reward_star = kRewardStar[static_cast<int>(difficulty)];
        current->star_game += reward_star;
        current->star_usable += reward_star;
        current->updated = 1;
    }
    return 1;
}

// IDA_EA: 0x81318E2
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager20getClearStageSummaryEPi
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getClearStageSummary(int *)const
char CharacAdvanceAltarManager::getClearStageSummary(int summary[3]) const {
    if (!current) {
        return 0;
    }

    for (std::map<int, _Stage>::const_iterator it =
             current->stages_24.begin();
         it != current->stages_24.end();
         ++it) {
        switch (it->second.star) {
            case StageDifficulty::HARD:
                ++summary[2];
                // Deliberate fall-through: hard completions count toward
                // medium and easy achievement thresholds in the original.
            case StageDifficulty::MEDIUM:
                ++summary[1];
                // Deliberate fall-through: medium completions count toward
                // the easy achievement threshold.
            case StageDifficulty::EASY:
                ++summary[0];
                break;
            default:
                break;
        }
    }
    return 0;
}

// IDA_EA: 0x8132ABE
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager13addRewardDataEiRSt3mapIiNS_7_RewardESt4lessIiESaISt4pairIKiS2_EEE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::addRewardData(int,std::map<int,advancealtar::_Reward> &)
int CharacAdvanceAltarManager::addRewardData(
    int achievement_id,
    std::map<int, _Reward>& rewards) {
    if (!current) {
        return 0;
    }
    if (current->rewards_b4.find(achievement_id) !=
        current->rewards_b4.end()) {
        return 0;
    }

    _Reward reward;
    reward.reward_id = achievement_id;
    reward.received = 0;
    current->rewards_b4.insert(std::make_pair(achievement_id, reward));
    current->updated = 1;
    rewards.insert(std::make_pair(achievement_id, reward));
    return 1;
}

// IDA_EA: 0x81331D4
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager12setNextStageEi
int CharacAdvanceAltarManager::setNextStage(int stage_index) {
    if (!current) {
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    const StageMapList* const stage_maps =
        data_manager ? data_manager->get_stage_map_list() : 0;
    if (!stage_maps) {
        return 0;
    }

    if (stage_index == 0) {
        _Stage first_stage;
        first_stage.stage_id = stage_maps->getFirstStageIndex();
        first_stage.star = StageDifficulty::NONE;
        current->stages_24.insert(std::make_pair(first_stage.stage_id,
                                                  first_stage));
        return first_stage.stage_id;
    }

    bool next_is_survival_stage = false;
    const int next_stage =
        stage_maps->getNextStage(stage_index, next_is_survival_stage);
    if (next_stage == 0) {
        return 0;
    }

    if (current->stages_24.find(next_stage) != current->stages_24.end()) {
        return next_stage;
    }

    current->is_unlock_stage_effect = 1;
    _Stage stage;
    stage.stage_id = next_stage;
    stage.star = StageDifficulty::NONE;
    current->stages_24.insert(std::make_pair(stage.stage_id, stage));
    current->updated = 1;
    return next_is_survival_stage ? setNextStage(next_stage) : next_stage;
}

// IDA_EA: 0x8132234
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager25setUnlockItemToEmptySlotENS_20AdvanceAltarShopType1TEi
int CharacAdvanceAltarManager::setUnlockItemToEmptySlot(
    AdvanceAltarShopType::T shop_type,
    int item_id) {
    if (!current) {
        return 0;
    }

    AdvanceAltarSlotType::T slot_type;
    if (shop_type == AdvanceAltarShopType::UNIT) {
        slot_type = AdvanceAltarSlotType::UNIT;
    } else if (shop_type == AdvanceAltarShopType::SKILL) {
        slot_type = AdvanceAltarSlotType::SKILL;
    } else {
        return static_cast<int>(shop_type);
    }

    std::map<uint8_t, _Slot>* const slots = current->getSlotMap(slot_type);
    if (!slots) {
        return 0;
    }
    for (std::map<uint8_t, _Slot>::iterator it = slots->begin();
         it != slots->end(); ++it) {
        _Slot& slot = it->second;
        if (slot.locked != 1 && slot.item_id == 0) {
            slot.item_id = item_id;
            current->updated = 1;
            return static_cast<int>(
                reinterpret_cast<uintptr_t>(current));
        }
    }
    return 0;
}

// IDA_EA: 0x8132D20
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager16setUnlockBuyItemENS_20AdvanceAltarShopType1TEiPSt6vectorINS_8_BuyItemESaIS4_EE
int CharacAdvanceAltarManager::setUnlockBuyItem(
    AdvanceAltarShopType::T shop_type,
    int stage_index,
    std::vector<_BuyItem>* unlocked_items) {
    if (!current) {
        return 0;
    }

    AdvanceAltarShopParameter* const shop_parameter =
        GetAdvanceAltarShopParameter();
    std::vector<BuyUpgradeData>* const upgrades =
        shop_parameter ? shop_parameter->getBuyUpgradeDataVec(
                             current->ridable_id, shop_type)
                       : 0;
    std::map<int, _BuyItem>* const buy_items =
        current->getBuyItemMap(shop_type);
    if (!upgrades || !buy_items) {
        return static_cast<int>(reinterpret_cast<uintptr_t>(current));
    }

    for (std::vector<BuyUpgradeData>::iterator it = upgrades->begin();
         it != upgrades->end(); ++it) {
        BuyUpgradeData& upgrade = *it;
        if (upgrade.unlock_stage_index != stage_index) {
            continue;
        }

        int minimum_level = 0;
        FieldData* const level = upgrade.getMinLevelFieldDataPoint(
            FieldType::INDEX, minimum_level);
        FieldData* const price = upgrade.getFieldDataPoint(
            minimum_level, FieldType::PRICE);
        if (!level || !price) {
            continue;
        }

        _BuyItem item;
        item.shop_type = static_cast<int>(shop_type);
        item.item_id = upgrade.id;
        item.buy_count = static_cast<int16_t>(minimum_level);
        item.buy_limit = static_cast<int16_t>(price->int_value);
        buy_items->insert(std::make_pair(item.item_id, item));

        if (minimum_level > 0 && upgrade.is_settable_slot) {
            setUnlockItemToEmptySlot(shop_type, item.item_id);
        }
        if (unlocked_items) {
            unlocked_items->push_back(item);
        }
    }

    current->updated = 1;
    return static_cast<int>(reinterpret_cast<uintptr_t>(current));
}

// IDA_EA: 0x8131616
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager3setERKNS_25_CharacAdvanceAltarDbDataE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::set(advancealtar::_CharacAdvanceAltarDbData const&)
bool CharacAdvanceAltarManager::set(const _CharacAdvanceAltarDbData& db_data) {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 0;
        if (index > 0) {
            break;
        }
        data[index].set(db_data.records[index]);
    }
    return result;
}

// IDA_EA: 0x8131B58
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager7setSlotEbRKN15CMDPacketStruct12_SetSlotDataE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::setSlot(bool,CMDPacketStruct::_SetSlotData const&)
int CharacAdvanceAltarManager::setSlot(
    bool set_flag,
    const CMDPacketStruct::_SetSlotData& slot_data) {
    AdvanceAltarData* altar_data = current;
    if (!altar_data) {
        return 2;
    }

    int shop_type = 0;
    if (slot_data.dword_00 == 1) {
        shop_type = 0;
        if (slot_data.byte_04 > 5u) {
            return 2;
        }
    } else if (slot_data.dword_00 == 2) {
        shop_type = 1;
        if (slot_data.byte_04 > 5u) {
            return 2;
        }
    } else {
        return 2;
    }

    AdvanceAltarShopParameter* shop_parameter = GetAdvanceAltarShopParameter();
    if (!shop_parameter) {
        return 3;
    }
    std::vector<BuyUpgradeData>* upgrade_data =
        shop_parameter->getBuyUpgradeDataVec(
            altar_data->ridable_id,
            static_cast<AdvanceAltarShopType::T>(shop_type));
    if (!upgrade_data) {
        return 3;
    }

    const BuyUpgradeData* selected = 0;
    for (std::vector<BuyUpgradeData>::const_iterator it =
             upgrade_data->begin();
         it != upgrade_data->end(); ++it) {
        if (it->id == static_cast<int>(slot_data.dword_08)) {
            selected = &*it;
            break;
        }
    }
    if (!selected || selected->is_settable_slot != 1) {
        return 3;
    }

    if (!set_flag) {
        return resetSlot(*altar_data, slot_data);
    }

    std::map<int, _BuyItem>* buy_items =
        altar_data->getBuyItemMap(static_cast<AdvanceAltarShopType::T>(
            shop_type));
    if (!buy_items) {
        return 3;
    }
    if (buy_items->find(static_cast<int>(slot_data.dword_08)) ==
        buy_items->end()) {
        return 3;
    }

    std::map<uint8_t, _Slot>* slots =
        altar_data->getSlotMap(
            static_cast<AdvanceAltarSlotType::T>(slot_data.dword_00));
    if (!slots) {
        return 3;
    }
    for (std::map<uint8_t, _Slot>::iterator it = slots->begin();
         it != slots->end(); ++it) {
        if (it->second.item_id == slot_data.dword_08) {
            return 4;
        }
    }
    for (std::map<uint8_t, _Slot>::iterator it = slots->begin();
         it != slots->end(); ++it) {
        if (it->second.slot_no == slot_data.byte_04 &&
            it->second.slot_type == slot_data.dword_00 &&
            it->second.locked != 1) {
            it->second.item_id = slot_data.dword_08;
            altar_data->updated = 1;
            return 0;
        }
    }
    return 2;
}

// IDA_EA: 0x8131F0A
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager12exchangeSlotERN15CMDPacketStruct12_SetSlotDataES3_
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::exchangeSlot(CMDPacketStruct::_SetSlotData &,CMDPacketStruct::_SetSlotData &)
int CharacAdvanceAltarManager::exchangeSlot(
    CMDPacketStruct::_SetSlotData& from,
    CMDPacketStruct::_SetSlotData& to) {
    if (from.dword_00 != to.dword_00) {
        return 2;
    }
    AdvanceAltarData* altar_data = current;
    if (!altar_data) {
        return 3;
    }
    std::map<uint8_t, _Slot>* slots =
        altar_data->getSlotMap(
            static_cast<AdvanceAltarSlotType::T>(from.dword_00));
    if (!slots) {
        return 4;
    }

    std::map<uint8_t, _Slot>::iterator from_it = slots->end();
    for (std::map<uint8_t, _Slot>::iterator it = slots->begin();
         it != slots->end(); ++it) {
        if (it->second.slot_no == from.byte_04 &&
            it->second.slot_type == from.dword_00 &&
            it->second.item_id == from.dword_08) {
            from_it = it;
            break;
        }
    }

    std::map<uint8_t, _Slot>::iterator to_it = slots->end();
    for (std::map<uint8_t, _Slot>::iterator it = slots->begin();
         it != slots->end(); ++it) {
        if (it->second.slot_no == to.byte_04 &&
            it->second.slot_type == to.dword_00 &&
            it->second.item_id == to.dword_08) {
            to_it = it;
            break;
        }
    }

    if (from_it == slots->end() || to_it == slots->end()) {
        return 3;
    }
    if (from_it->second.locked || to_it->second.locked) {
        return 3;
    }
    from_it->second.item_id = to.dword_08;
    to_it->second.item_id = from.dword_08;
    altar_data->updated = 1;
    return 0;
}

// IDA_EA: 0x8132BFA
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager9resetSlotERNS_16AdvanceAltarDataERKN15CMDPacketStruct12_SetSlotDataE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::resetSlot(advancealtar::AdvanceAltarData &,CMDPacketStruct::_SetSlotData const&)
int CharacAdvanceAltarManager::resetSlot(
    AdvanceAltarData& altar_data,
    const CMDPacketStruct::_SetSlotData& slot_data) {
    std::map<uint8_t, _Slot>* slots =
        altar_data.getSlotMap(
            static_cast<AdvanceAltarSlotType::T>(slot_data.dword_00));
    if (!slots) {
        return 3;
    }
    std::map<uint8_t, _Slot>::iterator it = slots->find(slot_data.byte_04);
    if (it == slots->end()) {
        return 2;
    }
    if (it->second.locked == 1) {
        return 2;
    }
    it->second.item_id = 0;
    altar_data.updated = 1;
    return 0;
}

// IDA_EA: 0x8131660
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager5resetEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::reset(void)
bool CharacAdvanceAltarManager::reset() {
    current = 0;
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 0;
        if (index > 0) {
            break;
        }
        data[index].reset();
    }
    return result;
}

// IDA_EA: 0x81316A0
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager9isUpdatedEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::isUpdated(void)const
int CharacAdvanceAltarManager::isUpdated() const {
    for (int index = 0; index <= 0; ++index) {
        if (data[index].updated) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x81316E8
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager26getAdvanceAltarDataByIndexEi
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getAdvanceAltarDataByIndex(int)
AdvanceAltarData* CharacAdvanceAltarManager::getAdvanceAltarDataByIndex(
    int index) {
    if (index) {
        return 0;
    }
    return data;
}

// IDA_EA: 0x8131710
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager27getCharacAdvanceAltarDbDataERNS_25_CharacAdvanceAltarDbDataE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getCharacAdvanceAltarDbData(advancealtar::_CharacAdvanceAltarDbData &)const
bool CharacAdvanceAltarManager::getCharacAdvanceAltarDbData(
    _CharacAdvanceAltarDbData& db_data) const {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 0;
        if (index > 0) {
            break;
        }
        data[index].get(db_data.records[index]);
    }
    return result;
}

// IDA_EA: 0x81319AE
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager7buyItemENS_20AdvanceAltarShopType1TEissiRi
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::buyItem(advancealtar::AdvanceAltarShopType::T,int,short,short,int,int &)
int CharacAdvanceAltarManager::buyItem(AdvanceAltarShopType::T shop_type,
                                       int upgrade_id,
                                       int16_t first_level,
                                       int16_t last_level,
                                       int price,
                                       int& remaining_star) {
    if (!current) {
        return 3;
    }
    if (current->star_usable < price) {
        return 2;
    }

    std::map<int, _BuyItem>* const buy_items =
        current->getBuyItemMap(shop_type);
    if (!buy_items) {
        return 3;
    }

    std::map<int, _BuyItem>::iterator found = buy_items->find(upgrade_id);
    if (found == buy_items->end()) {
        _BuyItem& item = (*buy_items)[upgrade_id];
        item.shop_type = static_cast<int32_t>(shop_type);
        item.item_id = upgrade_id;
        item.buy_count = last_level;
        item.buy_limit = static_cast<int16_t>(price);
    } else {
        found->second.buy_count = last_level;
        found->second.buy_limit =
            static_cast<int16_t>(found->second.buy_limit + price);
    }

    current->star_usable -= price;
    current->updated = 1;
    remaining_star = current->star_usable;
    HistoryLog::buyItem(shop_type, upgrade_id, first_level, last_level,
                        price);
    return 0;
}

// IDA_EA: 0x8133720
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager9gmSetStarEi
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::gmSetStar(int)
int CharacAdvanceAltarManager::gmSetStar(int star) {
    if (current) {
        current->star_game = star;
        current->star_usable = star + current->star_cera;
        current->updated = 1;
    }
    return reinterpret_cast<int>(current);
}

// IDA_EA: 0x81333B2
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager9resetStarEP5CUserN15CMDPacketStruct13ResetStarType1TE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::resetStar(CUser *,CMDPacketStruct::ResetStarType::T)
int CharacAdvanceAltarManager::resetStar(
    CUser* user,
    CMDPacketStruct::ResetStarType::T type) {
    if (!user || !current || type != CMDPacketStruct::ResetStarType::BY_GOLD) {
        return 2;
    }

    AdvanceAltarShopParameter* const parameter =
        GetAdvanceAltarShopParameter();
    if (!parameter) {
        return 2;
    }

    const int reset_count = static_cast<int>(current->star_reset_count);
    std::map<int, _StarRestFeeByGold>::const_iterator fee =
        parameter->star_reset_fee_by_gold.find(reset_count);
    const int gold_fee =
        fee == parameter->star_reset_fee_by_gold.end()
            ? parameter->getMaxStarResetGoldFee()
            : fee->second.gold_fee;

    if (user->getCurCharacInvenR()->get_money() < gold_fee) {
        return 3;
    }
    if (user->getCurCharacInvenW()->use_money(
            gold_fee, static_cast<eMoneySubReason>(50), true) != 1) {
        return 3;
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), 0);

    ++current->star_reset_count;
    setDefaultSlot();
    setDefaultBuyItem(static_cast<AdvanceAltarShopType::T>(0));
    setDefaultBuyItem(static_cast<AdvanceAltarShopType::T>(1));
    setDefaultBuyItem(static_cast<AdvanceAltarShopType::T>(2));
    for (std::map<int, _Stage>::const_iterator it = current->stages_24.begin();
         it != current->stages_24.end(); ++it) {
        if (it->second.star != StageDifficulty::NONE) {
            setUnlockBuyItem(static_cast<AdvanceAltarShopType::T>(0),
                             it->second.stage_id, 0);
            setUnlockBuyItem(static_cast<AdvanceAltarShopType::T>(1),
                             it->second.stage_id, 0);
            setUnlockBuyItem(static_cast<AdvanceAltarShopType::T>(2),
                             it->second.stage_id, 0);
        }
    }

    const int restored_star =
        current->star_cera + current->star_game - current->star_usable;
    current->star_usable = current->star_game + current->star_cera;
    current->updated = 1;
    if (restored_star > 0) {
        HistoryLog::starAdd(*user, AddStarType::RESET_STAR, 0,
                            restored_star);
    }
    return 0;
}

// IDA_EA: 0x813375C
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager11gmSetTicketEi
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::gmSetTicket(int)
int CharacAdvanceAltarManager::gmSetTicket(int ticket_count) {
    if (current) {
        current->free_ticket_count = ticket_count;
        current->updated = 1;
    }
    return reinterpret_cast<int>(current);
}

// IDA_EA: 0x81348C4
// IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager26getCurrentAdvanceAltarDataEv
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::getCurrentAdvanceAltarData(void)const
AdvanceAltarData* CharacAdvanceAltarManager::getCurrentAdvanceAltarData()
    const {
    return current;
}

}  // namespace advancealtar
