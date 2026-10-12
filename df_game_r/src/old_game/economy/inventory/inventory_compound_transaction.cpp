#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "old_game/player/core/cuser.h"

namespace {

bool IsMatchingItem(const Inven_Item* item, int item_id) {
    return item && item->item_id_ == item_id;
}

bool ConsumeEquipmentSlots(CInventory* inventory,
                           int item_id,
                           int start,
                           int end,
                           int& remaining,
                           int& completed) {
    for (int slot = start; slot <= end && remaining > 0; ++slot) {
        Inven_Item* const item = inventory->GetInvenRef(1, slot);
        if (!IsMatchingItem(item, item_id)) {
            continue;
        }
        item->reset();
        --remaining;
        ++completed;
    }
    return remaining == 0;
}

}  // namespace

// IDA_EA: 0x85058DA
// IDA_NAME: _ZN10CInventory19tryDeleteEventItemsERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CInventory::tryDeleteEventItems(std::vector<std::pair<int,int> > const&)
int CInventory::tryDeleteEventItems(
    const std::vector<std::pair<int, int> >& items) {
    CUser* const user = parent_;
    if (user && user->getGmQuestFlag()) {
        return 1;
    }
    if (items.empty()) {
        return 0;
    }

    int completed = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        const int item_id = it->first;
        int remaining = it->second;
        if (item_id == 0) {
            if (use_money(remaining, static_cast<eMoneySubReason>(4), false)) {
                ++completed;
            }
            continue;
        }
        if (item_id == 2) {
            if (user && user->GetWinPoint() >= remaining) {
                ++completed;
            }
            continue;
        }

        const int item_type = GetItemType(item_id);
        if (!item_type) {
            continue;
        }

        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), start, end);
        bool consumed = false;
        if (item_type == 1) {
            ConsumeEquipmentSlots(this, item_id, start, end, remaining, completed);
            ConsumeEquipmentSlots(this, item_id, 3, 8, remaining, completed);
            for (int slot = 0;
                 slot < oldsrv::config::kEquipmentTypeCount &&
                     remaining > 0;
                 ++slot) {
                Inven_Item* const item = GetInvenRef(0, slot);
                if (!IsMatchingItem(item, item_id)) {
                    continue;
                }
                item->reset();
                --remaining;
                ++completed;
            }
            consumed = remaining == 0;
        } else {
            for (int slot = 3; slot <= 8 && !consumed; ++slot) {
                Inven_Item* const item = GetInvenRef(1, slot);
                if (IsMatchingItem(item, item_id) &&
                    item->add_info_ >= remaining) {
                    if (item->add_info_ == remaining) {
                        item->reset();
                    } else {
                        item->add_info_ -= remaining;
                    }
                    consumed = true;
                    ++completed;
                }
            }
            for (int slot = start; slot <= end && !consumed; ++slot) {
                Inven_Item* const item = GetInvenRef(1, slot);
                if (IsMatchingItem(item, item_id) &&
                    item->add_info_ >= remaining) {
                    if (item->add_info_ == remaining) {
                        item->reset();
                    } else {
                        item->add_info_ -= remaining;
                    }
                    consumed = true;
                    ++completed;
                }
            }
        }
    }
    return completed == static_cast<int>(items.size());
}

// IDA_EA: 0x850675E
// IDA_NAME: _ZN10CInventory19tryInsertEventItemsERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CInventory::tryInsertEventItems(std::vector<std::pair<int,int> > const&)
int CInventory::tryInsertEventItems(
    const std::vector<std::pair<int, int> >& items) {
    int inserted_count = 0;
    if (items.empty()) {
        return inserted_count;
    }

    Inven_Item item;
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        const int item_id = it->first;
        const int item_count = it->second;
        if (item_id == 0 || item_id == 1) {
            ++inserted_count;
            continue;
        }

        CItem* const prototype = G_CDataManager()->find_item(item_id);
        if (!prototype) {
            return inserted_count;
        }

        item.item_id_ = item_id;
        if (prototype->is_stackable()) {
            item.add_info_ = item_count;
            prototype->vtable_->make_item(prototype, item);
            if (insertItemIntoInventory(item,
                                        static_cast<eItemAddReason>(8),
                                        false,
                                        true) == -1) {
                return inserted_count;
            }
            ++inserted_count;
        } else if (prototype->vtable_->is_avatar_item(prototype)) {
            prototype->vtable_->make_item(prototype, item);
            if (insertAvatarIntoInventory(item,
                                          static_cast<eAvatarItemAddReason>(0),
                                          false) == -1) {
                return inserted_count;
            }
            ++inserted_count;
        } else {
            for (int count = 0; count < item_count; ++count) {
                prototype->vtable_->make_item(prototype, item);
                if (insertItemIntoInventory(item,
                                            static_cast<eItemAddReason>(8),
                                            false,
                                            true) == -1) {
                    break;
                }
                ++inserted_count;
            }
        }
    }
    return inserted_count;
}

// IDA_EA: 0x8507F1A
// IDA_NAME: _ZNK10CInventory27CheckEquipmentItemConditionERKSt6vectorISt4pairIiiESaIS2_EER16stEquipConditionR10Inven_Item
// DEMANGLED: CInventory::CheckEquipmentItemCondition(std::vector<std::pair<int,int> > const&,stEquipCondition&,Inven_Item&)const
int CInventory::CheckEquipmentItemCondition(
    const std::vector<std::pair<int, int> >& items,
    stEquipCondition& condition,
    Inven_Item& material) const {
    if (items.empty()) {
        return 0;
    }

    int equipment_count = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        const int item_id = it->first;
        const int item_count = it->second;
        if (GetItemType(item_id) != 1) {
            continue;
        }
        if (item_count > 1) {
            return 1;
        }

        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(1), start, end);
        for (int slot = start; slot <= end; ++slot) {
            const Inven_Item* const found = GetInvenRef(1, slot);
            if (!IsMatchingItem(found, item_id)) {
                continue;
            }
            if (++equipment_count > 1) {
                return 18;
            }
            material.setCopy(*found);
            if (parent_->CheckItemLock(1, slot)) {
                return 213;
            }
        }
        for (int slot = 3; slot <= 8; ++slot) {
            const Inven_Item* const found = GetInvenRef(1, slot);
            if (!IsMatchingItem(found, item_id)) {
                continue;
            }
            if (++equipment_count > 1) {
                return 18;
            }
            material.setCopy(*found);
            if (parent_->CheckItemLock(1, slot)) {
                return 213;
            }
        }
    }

    if (!material.item_id_) {
        return 0;
    }
    CItem* const prototype = G_CDataManager()->find_item(material.item_id_);
    if (!prototype) {
        return 17;
    }
    const int ability_type = material.amplify_option_.getAbilityType();
    if (ability_type == -128) {
        return 19;
    }
    const CEquipItem* const equipment = static_cast<const CEquipItem*>(prototype);
    if (material.avatar_select_ != equipment->get_endurance()) {
        return 216;
    }

    condition.has_equipment_material = 1;
    condition.equipment_upgrade =
        static_cast<uint8_t>(material.GetUpgrade());
    condition.has_amplify_option = ability_type ? 1 : 0;
    condition.amplify_ability_type =
        ability_type ? static_cast<uint8_t>(ability_type) : 0;
    condition.amplify_ability_value = ability_type ?
        static_cast<uint16_t>(material.amplify_option_.getAbilityValue()) : 0;
    condition.sealed = material.seal_or_state_ == 1 ? 1 : 0;
    condition.reseal_count =
        static_cast<uint8_t>(material.GetReSealCount());
    return 0;
}

// IDA_EA: 0x85083D0
// IDA_NAME: _ZNK10CInventory27CheckEquipmentItemConditionE10INVEN_TYPEsR28stEquipConditionForChronicle
// DEMANGLED: CInventory::CheckEquipmentItemCondition(INVEN_TYPE,short,stEquipConditionForChronicle &)const
int CInventory::CheckEquipmentItemCondition(
    INVEN_TYPE inventory_type,
    int16_t slot,
    stEquipConditionForChronicle& condition) const {
    const Inven_Item target = GetInvenSlot(static_cast<int>(inventory_type),
                                           static_cast<int>(slot));
    if (GetItemType(target.item_id_) != 1) {
        return 17;
    }

    CItem* const prototype = G_CDataManager()->find_item(target.item_id_);
    if (!prototype) {
        return 17;
    }
    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(prototype);
    const int ability_type = target.amplify_option_.getAbilityType();
    if (target.amplify_option_.isIdentified() != 1) {
        return 19;
    }
    if (target.avatar_select_ != equipment->get_endurance()) {
        return 216;
    }

    condition.upgrade = static_cast<uint8_t>(target.GetUpgrade());
    if (ability_type != 0) {
        condition.has_ability = 1;
        condition.ability_type = static_cast<uint8_t>(ability_type);
        condition.ability_value = static_cast<uint16_t>(
            target.amplify_option_.getAbilityValue());
    } else {
        condition.has_ability = 0;
        condition.ability_type = 0;
        condition.ability_value = 0;
    }
    condition.seal_or_state = target.seal_or_state_ ? 1 : 0;
    condition.reseal_count = static_cast<uint8_t>(target.GetReSealCount());
    condition.extend_info = target.extend_info_;
    return 0;
}

// IDA_EA: 0x850CA0C
// IDA_NAME: _ZNK10CInventory35GetExpertJobCompoundResultVariationE20ENUM_EXPERT_JOB_TYPEiRN12STItemScript10SEXPERTJOB31stExpertCompoundResultVariationE
// DEMANGLED: CInventory::GetExpertJobCompoundResultVariation(ENUM_EXPERT_JOB_TYPE,int,STItemScript::SEXPERTJOB::stExpertCompoundResultVariation&)const
void CInventory::GetExpertJobCompoundResultVariation(
    ENUM_EXPERT_JOB_TYPE expert_job_type,
    int expert_job_level,
    STItemScript::SEXPERTJOB::stExpertCompoundResultVariation& value) const {
    if (!static_cast<int>(expert_job_type)) {
        return;
    }
    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const equipped = GetInvenRef(0, slot);
        if (!equipped || !equipped->item_id_) {
            continue;
        }
        CItem* const item = G_CDataManager()->find_item(equipped->item_id_);
        if (!item || !item->isExpertJobUsable(expert_job_type,
                                               expert_job_level)) {
            continue;
        }
        STItemScript::SEXPERTJOB::stExpertCompoundResultVariation item_value;
        item->GetExpertJobCompoundResultVariation(item_value);
        value.value_00 += item_value.value_00;
        value.value_04 += item_value.value_04;
    }
}
#include "common/config.h"
