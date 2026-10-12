#include "old_data/PVF/equipment/item_upgrade.h"

#include "common/config.h"
#include <vector>

#include "old_game/progression/career/disjoint_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/log_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/mt_rand.h"

// IDA_EA: 0x822DC04
// IDA_NAME: _ZN11MSG_ENCHANTC2Ev
// DEMANGLED: MSG_ENCHANT::MSG_ENCHANT(void)
MSG_ENCHANT::MSG_ENCHANT()
    : MSG_BASE(),
      target_slot(0),
      target_item_id(0),
      material_slot(0),
      material_item_id(0),
      enchant_type(0) {}

namespace WongWork {

// IDA_EA: 0x854977A
// IDA_NAME: _ZN8WongWork12CItemUpgrade11isDoEnchantERK10Inven_ItemPc
// DEMANGLED: WongWork::CItemUpgrade::isDoEnchant(Inven_Item const&,char *)
bool CItemUpgrade::isDoEnchant(const Inven_Item& target,
                               char* rate_data) {
    const unsigned long bound = 100u;
    const int random_value = static_cast<int>(random_->randInt(bound));
    const int rate =
        (target.chronicle_random_option_.enchanted_skill_options_[0].flags_ &
         1u) != 0u
            ? static_cast<unsigned char>(rate_data[1])
            : static_cast<unsigned char>(rate_data[0]);
    return random_value <= rate;
}

// IDA_EA: 0x85497EA
// IDA_NAME: _ZN8WongWork12CItemUpgrade15_CheckConditionEPK5CUserRK10Inven_ItemPK5CItemS6_h
// DEMANGLED: WongWork::CItemUpgrade::_CheckCondition(CUser const*,Inven_Item const&,CItem const*,Inven_Item const&,unsigned char)
int CItemUpgrade::_CheckCondition(const CUser* user,
                                  const Inven_Item& target,
                                  const CItem* item,
                                  const Inven_Item& material,
                                  uint8_t enchant_type) {
    if (target.amplify_option_.isIdentified() != 1) return 174;
    if (target.item_category_ != 1u) return 19;

    const int item_type = item->vtable_->get_item_type(item);
    if (item_type == oldsrv::config::kEquipmentTypeTitle ||
        item_type < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        item_type > oldsrv::config::kOrdinaryEquipmentTypeLast) {
        return 19;
    }
    if (static_cast<int>(target.avatar_select_) !=
        static_cast<const CEquipItem*>(item)->get_endurance()) {
        return 7;
    }

    unsigned int required_slots = static_cast<unsigned int>(
        G_CDataManager()->get_disjoint()->getPredictResultItemTypeCount(
            item->get_rarity()) +
        1);
    if (target.amplify_option_.hasAbility()) ++required_slots;
    if (required_slots != 0u) {
        const int empty_slots = user->getCurCharacInvenR()->InvenEmptycount(
            static_cast<Inven_Item::ITEM_TYPE>(3));
        if (empty_slots + user->getCurCharacInvenR()->QuickEmptyCount() <
            static_cast<int>(required_slots)) {
            return 21;
        }
    }

    const CItem* const material_item =
        G_CDataManager()->find_item(static_cast<int>(material.item_id_));
    if (!material_item) return 21;
    const CStackableItem* const material_stackable =
        static_cast<const CStackableItem*>(material_item);

    const uint8_t first_enchant =
        target.chronicle_random_option_.enchanted_skill_options_[0].flags_;
    const uint8_t second_enchant =
        target.chronicle_random_option_.enchanted_skill_options_[1].flags_;
    if ((first_enchant & 1u) != 0u && (second_enchant & 1u) != 0u) return 19;
    if ((first_enchant & 1u) != 0u &&
        static_cast<uint8_t>(first_enchant >> 3) == enchant_type &&
        static_cast<uint8_t>((first_enchant >> 1) & 3u) ==
            material_stackable->third_chronicle_enchant_group()) {
        return 19;
    }

    const std::vector<STEnchantSystemData>& allowed_jobs =
        *reinterpret_cast<const std::vector<STEnchantSystemData>*>(
            static_cast<const CEquipItem*>(item)->getEnchantSystemData());
    for (std::vector<STEnchantSystemData>::const_iterator it =
             allowed_jobs.begin();
         it != allowed_jobs.end();
         ++it) {
        if (it->value_00 == user->get_charac_job() &&
            it->value_04 ==
                static_cast<uint8_t>(user->getCurCharFirstGrowType())) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8549AC8
// IDA_NAME: _ZN8WongWork12CItemUpgrade12_ProcPenaltyEP5CUserP10Inven_ItemPK5CItemtP11PacketGuard
// DEMANGLED: WongWork::CItemUpgrade::_ProcPenalty(CUser *,Inven_Item *,CItem const*,unsigned short,PacketGuard *)
int CItemUpgrade::_ProcPenalty(CUser* user,
                               Inven_Item* target,
                               const CItem* item,
                               uint16_t target_slot,
                               PacketGuard* packet) {
    const upgrade_table_t* const table = target->amplify_option_.hasAbility()
        ? GlobalData::s_itemAmplifier_->getUpgradeInfoTable()
        : CItemUpgradeTable::getUpgradeInfoTable();
    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), target_slot, 1,
            static_cast<eItemDelReason>(13), true) != 1) {
        return 17;
    }

    const unsigned int old_upgrade =
        static_cast<unsigned int>(target->GetUpgrade());
    packet->put_short(target_slot);

    DisJointResult disjoint_result;
    if (G_CDataManager()->get_disjoint()->GetResult(
            static_cast<int>(target->item_id_), disjoint_result)) {
        packet->put_byte(0);
        return 0;
    }

    target->item_id_ = 0;
    std::vector<std::pair<int, int> > result_items;
    result_items.reserve(6u);
    for (int index = 0; index <= 4; ++index) {
        if (disjoint_result.values[index + 5] != 0) {
            result_items.push_back(std::make_pair(
                disjoint_result.values[index + 5], disjoint_result.values[index]));
        }
    }
    _getAdditionalDisjointItem(old_upgrade, item, result_items, *table);
    if (target->amplify_option_.hasAbility()) {
        _getAdditionalDisjointItem(
            old_upgrade, item, result_items,
            *CItemUpgradeTable::getUpgradeInfoTable());
    }
    _getAdditionalDisjointItem(
        old_upgrade, item, result_items,
        *CItemUpgradeTable::getUpgradeInfoTable(),
        target->amplify_option_.hasAbility());

    std::vector<int> inserted_slots;
    const int inserted = user->getCurCharacInvenW()->insert_event_items(
        result_items, inserted_slots, static_cast<eMoneyAddReason>(7),
        static_cast<eItemAddReason>(19), false, "", "");
    packet->put_byte(inserted);
    for (std::vector<int>::const_iterator it = inserted_slots.begin();
         it != inserted_slots.end();) {
        const int slot = *it++;
        const unsigned int item_id = static_cast<unsigned int>(*it++);
        const int count = *it++;
        packet->put_short(slot);
        packet->put_item_idx(item_id);
        packet->put_int(count);

        CItem* const generated_item =
            G_CDataManager()->find_item(static_cast<int>(item_id));
        if (!generated_item) continue;
        Inven_Item generated;
        generated.item_id_ = static_cast<int32_t>(item_id);
        generated.set_add_info(count);
        generated_item->vtable_->make_item(generated_item, generated);
        GetInstanceCubeStatistic()->collectCubeStatistics(
            generated_item, count, user, static_cast<CUBE_STATISTIC_FIELD>(1));
        int sell_price = 0;
        G_Store()->GetSellItemPrice(generated, generated_item,
                                    static_cast<int16_t>(generated.add_info_),
                                    false, sell_price);
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(9), user,
            static_cast<unsigned int>(sell_price));
    }
    return 0;
}

// IDA_EA: 0x854A006
// IDA_NAME: _ZN8WongWork12CItemUpgrade24ProcUpgrade_3rdChronicleEP5CUser18ENUM_EQUIPMENTTYPEhR10Inven_ItemPK5CItem
// DEMANGLED: WongWork::CItemUpgrade::ProcUpgrade_3rdChronicle(CUser *,ENUM_EQUIPMENTTYPE,unsigned char,Inven_Item &,CItem const*)
int CItemUpgrade::ProcUpgrade_3rdChronicle(
    CUser* user,
    ENUM_EQUIPMENTTYPE equipment_type,
    uint8_t enchant_type,
    Inven_Item& target,
    const CItem* material_item) {
    CStackableItem* const material = static_cast<CStackableItem*>(
        const_cast<CItem*>(material_item));
    STEnchantSystemMapKey key;
    key.value_00 = user->get_charac_job();
    key.value_04 = static_cast<uint8_t>(user->getCurCharFirstGrowType());
    key.value_08 = static_cast<int32_t>(equipment_type);
    key.job = enchant_type;

    std::map<STEnchantSystemMapKey, STEnchantSystemMapData>& map =
        material->third_chronicle_enchant_map();
    std::map<STEnchantSystemMapKey, STEnchantSystemMapData>::iterator found =
        map.find(key);
    if (found == map.end()) return 0;

    // The original takes a by-value copy of this vector before changing the
    // packed inventory flags.  Its contents are not directly emitted here,
    // but retaining the copy preserves the original lifetime/failure edge.
    std::vector<EquipmentSkillDataUp> selected_skills = found->second.skill_data;
    (void)selected_skills;

    STEnchantedSkillOption* const options =
        target.chronicle_random_option_.enchanted_skill_options_;
    STEnchantedSkillOption& output =
        (options[0].flags_ & 1u) != 0u ? options[1] : options[0];
    output.flags_ |= 1u;
    output.flags_ = static_cast<uint8_t>(
        (output.flags_ & 0xf9u) |
        ((material->third_chronicle_enchant_group() & 3u) << 1));
    output.flags_ = static_cast<uint8_t>(
        (output.flags_ & 7u) | ((enchant_type & 0x1fu) << 3));
    return 1;
}

// IDA_EA: 0x854A1BA
// IDA_NAME: _ZN8WongWork12CItemUpgrade11ProcUpgradeEP5CUserP11MSG_ENCHANTP11PacketGuard
// DEMANGLED: WongWork::CItemUpgrade::ProcUpgrade(CUser *,MSG_ENCHANT *,PacketGuard *)
int CItemUpgrade::ProcUpgrade(CUser* user,
                              MSG_ENCHANT* message,
                              PacketGuard* packet) {
    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item target = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), message->target_slot);
    if (target.isEmpty() ||
        target.item_id_ != static_cast<int32_t>(message->target_item_id)) {
        return 4;
    }
    Inven_Item material = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), message->material_slot);
    if (material.isEmpty()) return 22;

    int start = 0;
    int end = 0;
    inventory->StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(0), start, end);
    if (message->target_slot >= static_cast<uint16_t>(start) &&
        message->target_slot <= static_cast<uint16_t>(end)) {
        return 19;
    }

    CItem* const target_item =
        G_CDataManager()->find_item(static_cast<int>(target.item_id_));
    if (!target_item) return 4;
    CItem* const material_item =
        G_CDataManager()->find_item(static_cast<int>(material.item_id_));
    if (!material_item) return 4;
    if (!material_item->is_stackable()) return 19;

    const int condition = _CheckCondition(
        user, target, target_item, material, message->enchant_type);
    if (condition != 0) {
        LogManager::logFormat(1, "ItemUpgrade.cpp", __FUNCTION__, __LINE__,
            "[CHRONICLE_ENCHANT][CONDITION_REJECT] char=%d target=%u slot=%u "
            "material=%u skill=%u job=%d grow=%d type=%d durability=%u "
            "flags=%u,%u error=%d",
            user->getCurCharacNo(), message->target_item_id,
            static_cast<unsigned int>(message->target_slot), message->material_item_id,
            static_cast<unsigned int>(message->enchant_type), user->get_charac_job(),
            static_cast<int>(user->getCurCharFirstGrowType()),
            target_item->vtable_->get_item_type(target_item),
            static_cast<unsigned int>(target.avatar_select_),
            static_cast<unsigned int>(target.chronicle_random_option_.enchanted_skill_options_[0].flags_),
            static_cast<unsigned int>(target.chronicle_random_option_.enchanted_skill_options_[1].flags_),
            condition);
        return condition;
    }

    // Validate the selected skill before consuming the breath or rolling a
    // destructive failure.  Build the successful result on a private copy.
    Inven_Item enchanted_target = target;
    if (ProcUpgrade_3rdChronicle(
            user, static_cast<ENUM_EQUIPMENTTYPE>(
                target_item->vtable_->get_item_type(target_item)),
            message->enchant_type, enchanted_target, material_item) != 1) {
        LogManager::logFormat(1, "ItemUpgrade.cpp", __FUNCTION__, __LINE__,
            "[CHRONICLE_ENCHANT][SKILL_NOT_FOUND] char=%d target=%u material=%u "
            "job=%d grow=%d type=%d skill=%u",
            user->getCurCharacNo(), message->target_item_id, message->material_item_id,
            user->get_charac_job(), static_cast<int>(user->getCurCharFirstGrowType()),
            target_item->vtable_->get_item_type(target_item),
            static_cast<unsigned int>(message->enchant_type));
        return 19;
    }

    CStackableItem* const material_stackable =
        static_cast<CStackableItem*>(material_item);
    const bool enchanted =
        isDoEnchant(target, material_stackable->third_chronicle_enchant_rates());
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                               message->material_slot,
                               1,
                               static_cast<eItemDelReason>(12),
                               true) != 1) {
        return 17;
    }

    packet->put_byte(1);
    packet->put_short(message->material_slot);
    packet->put_short(static_cast<uint16_t>(
        inventory->GetInvenSlot(static_cast<INVEN_TYPE>(1), message->material_slot)
            .get_add_info()));
    packet->put_byte(enchanted ? 1 : 0);

    if (enchanted) {
        target = enchanted_target;
        // DNF.exe 0828 CMD367 receiver @0x577890 reads these fields even
        // on success: target slot (u16), disjoint result count (u8).
        packet->put_short(message->target_slot);
        packet->put_byte(0);
    } else {
        int sell_price = 0;
        G_Store()->GetSellItemPrice(
            target, target_item, static_cast<int16_t>(target.add_info_),
            false, sell_price);
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(15), user,
            static_cast<unsigned int>(sell_price));
        const int penalty = _ProcPenalty(
            user, &target, target_item, message->target_slot, packet);
        if (penalty != 0) return penalty;
    }

    if (target.item_id_ != 0) {
        inventory->update_item(static_cast<INVEN_TYPE>(1), message->target_slot,
                               target);
    }
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0), message->target_slot);

    int enchant_count = 0;
    if ((target.chronicle_random_option_.enchanted_skill_options_[0].flags_ &
         1u) != 0u) {
        ++enchant_count;
        if ((target.chronicle_random_option_.enchanted_skill_options_[1].flags_ &
             1u) != 0u) {
            ++enchant_count;
        }
    }
    user->history_log().Enchant3rdChronicleItem(
        enchanted, target_item->get_index(), material.item_id_,
        message->enchant_type, enchant_count);
    return 0;
}

// IDA_EA: 0x854A916
// IDA_NAME: _ZN8WongWork12CItemUpgrade27find3rdChronicleEnchantDataE21STEnchantSystemMapKeyi
// DEMANGLED: WongWork::CItemUpgrade::find3rdChronicleEnchantData(STEnchantSystemMapKey,int)
std::vector<EquipmentSkillDataUp>* CItemUpgrade::find3rdChronicleEnchantData(
    STEnchantSystemMapKey key,
    int item_id) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item) return 0;
    CStackableItem* const material = static_cast<CStackableItem*>(item);
    std::map<STEnchantSystemMapKey, STEnchantSystemMapData>& map =
        material->third_chronicle_enchant_map();
    std::map<STEnchantSystemMapKey, STEnchantSystemMapData>::iterator found =
        map.find(key);
    return found == map.end() ? 0 : &found->second.skill_data;
}

}  // namespace WongWork
