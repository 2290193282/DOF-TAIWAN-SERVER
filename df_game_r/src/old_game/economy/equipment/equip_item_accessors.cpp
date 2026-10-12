#include "old_game/economy/inventory/item.h"
#include "common/config.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>
#include <cstddef>
#include <cmath>
#include <vector>

// IDA_EA: 0x8513530
// IDA_NAME: _ZNK10CEquipItem16getExpBonusPointEv
// DEMANGLED: CEquipItem::getExpBonusPoint(void)const
const ExpBonusPointData* CEquipItem::getExpBonusPoint() const {
    // IDA 0x8513530 returns the in-object +0x1e8 record.  It is the
    // two-int ExpBonusPointData payload, not an untyped byte view.
    return &exp_bonus_point_;
}

// IDA_EA: 0x85137B8
// IDA_NAME: _ZNK10CEquipItem14getGrowthGradeEi
// DEMANGLED: CEquipItem::getGrowthGrade(int)const
int CEquipItem::getGrowthGrade(int charac_level) const {
    if (growth_type_) {
        return getLevelLinearAbility(
            growth_begin_level_, growth_end_level_, growth_begin_grade_,
            growth_end_grade_, charac_level);
    }
    return grade;
}

// IDA_EA: 0x851381C
// IDA_NAME: _ZNK10CEquipItem19getGrowthRepairCostEi
// DEMANGLED: CEquipItem::getGrowthRepairCost(int)const
int CEquipItem::getGrowthRepairCost(int charac_level) const {
    if (growth_type_) {
        return getLevelLinearAbility(
            growth_begin_level_, growth_end_level_, growth_begin_repair_cost_,
            growth_end_repair_cost_, charac_level);
    }
    return growth_repair_cost_;
}

// IDA_EA: 0x8514DDE
// IDA_NAME: _ZNK10CEquipItem17getUsableItemTypeEi
// DEMANGLED: CEquipItem::getUsableItemType(int)const
int CEquipItem::getUsableItemType(int item_type) const {
    // The old server's complete body is the constant zero return.  The int
    // parameter remains required by the original member symbol.
    (void)item_type;
    return 0;
}

// IDA_EA: 0x854B528
// IDA_NAME: _ZN10CEquipItem25GetEmancipateMaterialListEv
// DEMANGLED: CEquipItem::GetEmancipateMaterialList(void)
char* CEquipItem::GetEmancipateMaterialList() {
    return reinterpret_cast<char*>(&emancipate_materials_);
}

// IDA_EA: 0x854B536
// IDA_NAME: _ZN10CEquipItem23GetEmancipateResultListEv
// DEMANGLED: CEquipItem::GetEmancipateResultList(void)
char* CEquipItem::GetEmancipateResultList() {
    return reinterpret_cast<char*>(&emancipate_results_);
}

// IDA_EA: 0x8110C60
// IDA_NAME: _ZNK5CItem15getUsablePeriodEv
// DEMANGLED: CItem::getUsablePeriod(void)const
int CItem::getUsablePeriod() const {
    return usable_period_;
}

// IDA_EA: 0x817EDEC
// IDA_NAME: _ZNK5CItem14IsHiddenOptionEv
// DEMANGLED: CItem::IsHiddenOption(void)const
int CItem::IsHiddenOption() const {
    return hidden_option_;
}

// IDA_EA: 0x822C83E
// IDA_NAME: _ZNK5CItem19GetFinishPointPriceEv
// DEMANGLED: CItem::GetFinishPointPrice(void)const
int CItem::GetFinishPointPrice() const {
    return finish_point_price_;
}

// IDA_EA: 0x84E95BC
// IDA_NAME: _ZNK5CItem15check_low_levelEi
// DEMANGLED: CItem::check_low_level(int)const
bool CItem::check_low_level(int charac_level) const {
    return usable_level_ <= charac_level;
}

// IDA_EA: 0x850D728
// IDA_NAME: _ZNK5CItem14check_job_typeEc
// DEMANGLED: CItem::check_job_type(char)const
int CItem::check_job_type(char job) const {
    if (job < 0 || job >= oldsrv::config::kCharacterJobCount) {
        return 0;
    }
    return usable_job_flags_[static_cast<signed char>(job)];
}

// IDA_EA: 0x846996A
// IDA_NAME: _ZNK5CItem17isEpicRoutingItemEv
// DEMANGLED: CItem::isEpicRoutingItem(void)const
bool CItem::isEpicRoutingItem() const {
    return epic_routing_item_ != 0;
}

// IDA_EA: 0x850D756
// IDA_NAME: _ZNK5CItem18check_item_routingEv
// DEMANGLED: CItem::check_item_routing(void)const
bool CItem::check_item_routing() const {
    return isEpicRoutingItem() || rarity_ > 0;
}


// IDA_EA: 0x8150F28
// IDA_NAME: _ZNK10CEquipItem19getAvatarTypeSelectEv
// DEMANGLED: CEquipItem::getAvatarTypeSelect(void)const
char* CEquipItem::getAvatarTypeSelect() const {
    return const_cast<char*>(reinterpret_cast<const char*>(&avatar_type_selects_));
}

// IDA_EA: 0x8513522
// IDA_NAME: _ZNK10CEquipItem15getWpBonusPointEv
// DEMANGLED: CEquipItem::getWpBonusPoint(void)const
const WpBonusPointData* CEquipItem::getWpBonusPoint() const {
    // IDA 0x8513522 returns the in-object +0x1e0 record.  Its two dwords
    // are populated by STEquipmentScript as WpBonusPointData::type/value.
    return &wp_bonus_point_;
}

// IDA_EA: 0x854B544
// IDA_NAME: _ZNK10CEquipItem20getEnchantSystemDataEv
// DEMANGLED: CEquipItem::getEnchantSystemData(void)const
char* CEquipItem::getEnchantSystemData() const {
    return const_cast<char*>(reinterpret_cast<const char*>(&enchant_system_data_));
}


// IDA_EA: 0x8514E04
// IDA_NAME: _ZNK10CEquipItem21GetGuildPowerWarPointEv
// DEMANGLED: CEquipItem::GetGuildPowerWarPoint(void)const
int CEquipItem::GetGuildPowerWarPoint() const {
    return guild_power_war_point_;
}

// IDA_EA: 0x8514D26
// IDA_NAME: _ZNK10CEquipItem11GetItemTypeEv
// DEMANGLED: CEquipItem::GetItemType(void)const
int CEquipItem::GetItemType() const {
    return item_type_;
}

// IDA_EA: 0x8514E6E
// IDA_NAME: _ZNK10CEquipItem20GetRandomOptionGradeEv
// DEMANGLED: CEquipItem::GetRandomOptionGrade(void)const
int CEquipItem::GetRandomOptionGrade() const {
    return random_option_grade_;
}

// IDA_EA: 0x832E036
// IDA_NAME: _ZNK10CEquipItem22GetUsableEquipmentTypeEv
// DEMANGLED: CEquipItem::GetUsableEquipmentType(void)const
int CEquipItem::GetUsableEquipmentType() const {
    return usable_equipment_type_;
}

// IDA_EA: 0x84E9700
// IDA_NAME: _ZNK10CEquipItem17GetUsableMaxLevelEv
// DEMANGLED: CEquipItem::GetUsableMaxLevel(void)const
int CEquipItem::GetUsableMaxLevel() const {
    return usable_max_level_;
}

// IDA_EA: 0x8514E12
// IDA_NAME: _ZNK10CEquipItem20GetUserPowerWarPointEv
// DEMANGLED: CEquipItem::GetUserPowerWarPoint(void)const
int CEquipItem::GetUserPowerWarPoint() const {
    return user_power_war_point_;
}

// IDA_EA: 0x85136C6
// IDA_NAME: _ZNK10CEquipItem22IsAvatarColorVariationEi
// DEMANGLED: CEquipItem::IsAvatarColorVariation(int)const
int CEquipItem::IsAvatarColorVariation(int index) const {
    if (static_cast<unsigned int>(index) < 2u) {
        return avatar_color_variations_[index];
    }
    return 0;
}

// IDA_EA: 0x8514D46
// IDA_NAME: _ZNK10CEquipItem12IsAvatarItemEv
// DEMANGLED: CEquipItem::IsAvatarItem(void)const
int CEquipItem::IsAvatarItem() const {
    return static_cast<uint32_t>(item_type_) <=
           oldsrv::config::kAvatarEquipmentTypeLast;
}

// IDA_EA: 0x8514DA0
// IDA_NAME: _ZNK10CEquipItem13IsClearAvatarEv
// DEMANGLED: CEquipItem::IsClearAvatar(void)const
int CEquipItem::IsClearAvatar() const {
    return clear_avatar_;
}

// IDA_EA: 0x8514D72
// IDA_NAME: _ZNK10CEquipItem14IsCreatureItemEv
// DEMANGLED: CEquipItem::IsCreatureItem(void)const
int CEquipItem::IsCreatureItem() const {
    return item_type_ >=
               oldsrv::config::kCreatureArtifactEquipmentTypeFirst &&
           item_type_ <=
               oldsrv::config::kCreatureArtifactEquipmentTypeLast;
}

// IDA_EA: 0x822CA70
// IDA_NAME: _ZN10CEquipItem12IsEmancipateEv
// DEMANGLED: CEquipItem::IsEmancipate(void)
int CEquipItem::IsEmancipate() {
    return emancipate_;
}

// IDA_EA: 0x8514DB0
// IDA_NAME: _ZNK10CEquipItem13IsEnchantalbeEv
// DEMANGLED: CEquipItem::IsEnchantalbe(void)const
int CEquipItem::IsEnchantalbe() const {
    return item_type_ >= oldsrv::config::kOrdinaryEquipmentTypeFirst &&
           item_type_ <= oldsrv::config::kOrdinaryEquipmentTypeLast;
}

// IDA_EA: 0x8514E2E
// IDA_NAME: _ZNK10CEquipItem19IsImpossibleAmplifyEv
// DEMANGLED: CEquipItem::IsImpossibleAmplify(void)const
int CEquipItem::IsImpossibleAmplify() const {
    return impossible_amplify_;
}

// IDA_EA: 0x8514E3E
// IDA_NAME: _ZNK10CEquipItem20IsImpossibleDisjointEv
// DEMANGLED: CEquipItem::IsImpossibleDisjoint(void)const
int CEquipItem::IsImpossibleDisjoint() const {
    return impossible_disjoint_;
}

// IDA_EA: 0x8514E4E
// IDA_NAME: _ZNK10CEquipItem16IsImpossibleGiftEv
// DEMANGLED: CEquipItem::IsImpossibleGift(void)const
int CEquipItem::IsImpossibleGift() const {
    return impossible_gift_;
}

// IDA_EA: 0x8514E1E
// IDA_NAME: _ZNK10CEquipItem19IsImpossibleUpgradeEv
// DEMANGLED: CEquipItem::IsImpossibleUpgrade(void)const
int CEquipItem::IsImpossibleUpgrade() const {
    return impossible_upgrade_;
}

// IDA_EA: 0x8514E5E
// IDA_NAME: _ZNK10CEquipItem14IsRandomOptionEv
// DEMANGLED: CEquipItem::IsRandomOption(void)const
int CEquipItem::IsRandomOption() const {
    return random_option_;
}

// IDA_EA: 0x85108FC
// IDA_NAME: _ZN10CEquipItem19check_able_To_equipEcii
// DEMANGLED: CEquipItem::check_able_To_equip(char,int,int)
int CEquipItem::check_able_To_equip(char slot,
                                   int charac_job,
                                   int charac_level) {
    CEquipItem* item = this;
    const CItem* const base_item = reinterpret_cast<const CItem*>(item);
    if (base_item->is_stackable()) {
        return 1;
    }
    if (!check_eType(slot)) {
        return 23;
    }
    if (base_item->check_job_type(static_cast<char>(charac_job)) != 1) {
        return 8;
    }
    if (base_item->check_low_level(charac_level)) {
        return 0;
    }
    return 14;
}

// Migrated to old_game/inventory/equip_item_endurance_runtime.cpp.

// IDA_EA: 0x85108C6
// IDA_NAME: _ZNK10CEquipItem11check_eTypeEc
// DEMANGLED: CEquipItem::check_eType(char)const
int CEquipItem::check_eType(char slot) const {
    return static_cast<unsigned char>(slot) <=
               oldsrv::config::kOrdinaryEquipmentTypeLast &&
           static_cast<unsigned char>(item_type_) ==
               static_cast<unsigned char>(slot);
}

// IDA_EA: 0x81803BC
// IDA_NAME: _ZNK10CEquipItem19getAvatarOptionSizeEv
// DEMANGLED: CEquipItem::getAvatarOptionSize(void)const
int CEquipItem::getAvatarOptionSize() const {
    return avatar_option_size_;
}

int CEquipItem::getAvatarOptionKeyByOrdinal(int ordinal) const {
    if (ordinal < 0) {
        return -1;
    }
    for (size_t key = 0u; key < avatar_ability_variations_.size(); ++key) {
        if (avatar_ability_variations_[key].ability_index < 0) {
            continue;
        }
        if (ordinal == 0) {
            return static_cast<int>(key);
        }
        --ordinal;
    }
    return -1;
}

// IDA_EA: 0x8150F36
// IDA_NAME: _ZNK10CEquipItem15getAvatarSocketEiR20stAvatarEmblemInfo_t
// DEMANGLED: CEquipItem::getAvatarSocket(int,stAvatarEmblemInfo_t &)const
char CEquipItem::getAvatarSocket(int avatar_select,
                                stAvatarEmblemInfo_t& emblem) const {
    const std::vector<uint16_t>& sockets = avatar_socket_types_;
    int copied = 0;
    for (std::vector<uint16_t>::const_iterator it = sockets.begin();
         it != sockets.end();
         ++it) {
        emblem.sockets[copied].socket_type = *it;
        ++copied;
    }

    const std::vector<stAvatarTypeSelect_t>& selections = avatar_type_selects_;
    // -1 is the protocol sentinel for "use the first selectable avatar type
    // that defines sockets".  Do not cast that sentinel to unsigned before
    // the dedicated scan below; doing so turns it into UINT_MAX and rejects
    // every selection-only skin/avatar before its socket definition is read.
    unsigned int result =
        avatar_select < -1 ||
        (avatar_select != -1 &&
         selections.size() <= static_cast<unsigned int>(avatar_select));
    if (static_cast<unsigned char>(result) != 0u) {
        return static_cast<char>(result);
    }
    result = selections.empty() ? 1u : 0u;
    if (static_cast<unsigned char>(result) != 0u) {
        return static_cast<char>(result);
    }

    int selection_index = avatar_select;
    if (avatar_select == -1) {
        std::vector<stAvatarTypeSelect_t>::const_iterator it =
            selections.begin();
        const std::vector<stAvatarTypeSelect_t>::const_iterator end =
            selections.end();
        while (it != end) {
            ++selection_index;
            if (it->socket_count > 0) {
                break;
            }
            ++it;
        }
        result = it == end ? 1u : 0u;
        if (static_cast<unsigned char>(result) != 0u) {
            return static_cast<char>(result);
        }
    }

    const stAvatarTypeSelect_t& selected = selections.at(selection_index);
    const int selected_count = selected.socket_count;
    result = static_cast<unsigned int>(copied + selected_count);
    if (result <= 4u) {
        for (int index = 0;; ++index) {
            result = selected_count > index ? 1u : 0u;
            if (selected_count <= index) {
                break;
            }
            emblem.sockets[copied++].socket_type =
                selected.socket_types[index];
        }
    }
    return static_cast<char>(result);
}

// IDA_EA: 0x822CA80
// IDA_NAME: _ZNK10CEquipItem22getUpgradeCostDiscountEv
// DEMANGLED: CEquipItem::getUpgradeCostDiscount(void)const
float CEquipItem::getUpgradeCostDiscount() const {
    return upgrade_cost_discount_;
}

// IDA_EA: 0x854B552
// IDA_NAME: _ZNK10CEquipItem22getUpgradeProbIncreaseEv
// DEMANGLED: CEquipItem::getUpgradeProbIncrease(void)const
int CEquipItem::getUpgradeProbIncrease() const {
    return upgrade_probability_increase_;
}

// IDA_EA: 0x8510C0C
// IDA_NAME: _ZN10CEquipItem28get_aura_avatar_option_valueEv
// DEMANGLED: CEquipItem::get_aura_avatar_option_value(void)
int CEquipItem::get_aura_avatar_option_value() {
    return aura_avatar_option_value_;
}

// IDA_EA: 0x833EEE8
// IDA_NAME: _ZNK10CEquipItem26get_creature_minimum_levelEv
// DEMANGLED: CEquipItem::get_creature_minimum_level(void)const
int CEquipItem::get_creature_minimum_level() const {
    return creature_minimum_level_;
}

// IDA_EA: 0x833EEBE
// IDA_NAME: _ZNK10CEquipItem20get_creature_speciesEv
// DEMANGLED: CEquipItem::get_creature_species(void)const
int CEquipItem::get_creature_species() const {
    return creature_species_;
}

// IDA_EA: 0x833EEA6
// IDA_NAME: _ZNK10CEquipItem21get_experience_amountEv
// DEMANGLED: CEquipItem::get_experience_amount(void)const
float CEquipItem::get_experience_amount() const {
    return experience_amount_;
}

// IDA_EA: 0x833EEDA
// IDA_NAME: _ZNK10CEquipItem17get_hatch_item_idEv
// DEMANGLED: CEquipItem::get_hatch_item_id(void)const
int CEquipItem::get_hatch_item_id() const {
    return hatch_item_id_;
}

// IDA_EA: 0x8514E92
// IDA_NAME: _ZNK10CEquipItem26get_parent_booster_item_idEv
// DEMANGLED: CEquipItem::get_parent_booster_item_id(void)const
int CEquipItem::get_parent_booster_item_id() const {
    return static_cast<int>(parent_booster_item_id_);
}

// IDA_EA: 0x833EECC
// IDA_NAME: _ZNK10CEquipItem12get_sub_typeEv
// DEMANGLED: CEquipItem::get_sub_type(void)const
int CEquipItem::get_sub_type() const {
    return sub_type_;
}

// IDA_EA: 0x85136B6
// IDA_NAME: _ZNK10CEquipItem22isAmplifyPollutionItemEv
// DEMANGLED: CEquipItem::isAmplifyPollutionItem(void)const
int CEquipItem::isAmplifyPollutionItem() const {
    return amplify_pollution_item_;
}

// IDA_EA: 0x8514DE8
// IDA_NAME: _ZNK10CEquipItem18isEnableAvatarFuncE23ENUM_AVATAR_FUNC_FILTER
// DEMANGLED: CEquipItem::isEnableAvatarFunc(ENUM_AVATAR_FUNC_FILTER)const
bool CEquipItem::isEnableAvatarFunc(ENUM_AVATAR_FUNC_FILTER filter) const {
    return (filter & avatar_function_mask_) == filter;
}

// IDA_EA: 0x85137A8
// IDA_NAME: _ZNK10CEquipItem12isGrowthTypeEv
// DEMANGLED: CEquipItem::isGrowthType(void)const
int CEquipItem::isGrowthType() const {
    return growth_type_;
}

// IDA_EA: 0x8510BDC
// IDA_NAME: _ZN10CEquipItem21is_aura_avatar_optionEc
// DEMANGLED: CEquipItem::is_aura_avatar_option(char)
int CEquipItem::is_aura_avatar_option(char option) {
    return aura_avatar_option_type_ ==
           static_cast<unsigned char>(option);
}

// IDA_EA: 0x8514D34
// IDA_NAME: _ZN10CEquipItem19setAvatarOptionSizeEi
// DEMANGLED: CEquipItem::setAvatarOptionSize(int)
CEquipItem* CEquipItem::setAvatarOptionSize(int size) {
    avatar_option_size_ = size;
    return this;
}

// IDA_EA: 0x8514E80
// IDA_NAME: _ZN10CEquipItem26set_parent_booster_item_idEm
// DEMANGLED: CEquipItem::set_parent_booster_item_id(unsigned long)
CEquipItem* CEquipItem::set_parent_booster_item_id(unsigned long item_id) {
    parent_booster_item_id_ = item_id;
    return this;
}

// IDA_EA: 0x898C79D
// IDA_NAME: _Z21getLevelLinearAbilityiiiii
// DEMANGLED: getLevelLinearAbility(int,int,int,int,int)
int getLevelLinearAbility(int begin_level,
                          int end_level,
                          int begin_value,
                          int end_value,
                          int charac_level) {
    double ratio = static_cast<long double>(charac_level - begin_level) /
                   static_cast<long double>(end_level - begin_level);
    if (ratio < 0.0) {
        ratio = 0.0;
    }
    if (ratio > 1.0) {
        ratio = 1.0;
    }
    const int result = static_cast<int>(
        static_cast<long double>(begin_value) +
        (std::floor(ratio * 1000000.0 + 0.5) / 1000000.0) *
            static_cast<double>(end_value - begin_value));
    return result < 0 ? 0 : result;
}
