#include "old_game/economy/inventory/inventory.h"

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "common/manager/server_utility.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "common/manager/mt_rand.h"

// IDA_EA: 0x822C9D2
// IDA_NAME: _ZNK14CStackableItem12get_sub_typeEv
// DEMANGLED: CStackableItem::get_sub_type(void)const
int CStackableItem::get_sub_type() const {
    return sub_type_;
}

// IDA_EA: 0x822C9E0
// IDA_NAME: _ZN14CStackableItem22get_2nd_material_indexEv
// DEMANGLED: CStackableItem::get_2nd_material_index(void)
int CStackableItem::get_2nd_material_index() {
    return limit_cube_info_.value_0c;
}

// IDA_EA: 0x822C9EE
// IDA_NAME: _ZN14CStackableItem22get_2nd_material_countEv
// DEMANGLED: CStackableItem::get_2nd_material_count(void)
int CStackableItem::get_2nd_material_count() {
    return limit_cube_info_.value_10;
}

// IDA_EA: 0x822C9FC
// IDA_NAME: _ZNK14CStackableItem17getStackableLimitEv
// DEMANGLED: CStackableItem::getStackableLimit(void)const
int CStackableItem::getStackableLimit() const {
    return stackable_limit_;
}

// IDA_EA: 0x822CA0A
// IDA_NAME: _ZNK14CStackableItem19getGlobalEffectDataEv
// DEMANGLED: CStackableItem::getGlobalEffectData(void)const
const std::vector<stGlobalEffect_t>& CStackableItem::getGlobalEffectData() const {
    return global_effect_data_;
}

// IDA_EA: 0x822CA18
// IDA_NAME: _ZN14CStackableItem18IsSharedEffectItemEv
// DEMANGLED: CStackableItem::IsSharedEffectItem(void)
int CStackableItem::IsSharedEffectItem() {
    return continuous_item_state_.shared_effect_item;
}

// IDA_EA: 0x822CA28
// IDA_NAME: _ZNK14CStackableItem20getJewelTargetSocketEv
// DEMANGLED: CStackableItem::getJewelTargetSocket(void)const
int CStackableItem::getJewelTargetSocket() const {
    return jewel_target_socket_;
}

// IDA_EA: 0x822CA38
// IDA_NAME: _ZN14CStackableItem12IsEmancipateEv
// DEMANGLED: CStackableItem::IsEmancipate(void)
int CStackableItem::IsEmancipate() {
    return emancipate_;
}

// IDA_EA: 0x822CA48
// IDA_NAME: _ZNK14CStackableItem15IsDeletableItemEv
// DEMANGLED: CStackableItem::IsDeletableItem(void)const
int CStackableItem::IsDeletableItem() const {
    return deletable_item_;
}

// IDA_EA: 0x849F530
// IDA_NAME: _ZN14CStackableItem16GetMonsterCardIdEv
// DEMANGLED: CStackableItem::GetMonsterCardId(void)
int CStackableItem::GetMonsterCardId() {
    return monster_card_id_;
}

// IDA_EA: 0x84B4222
// IDA_NAME: _ZN14CStackableItem17IsReinforceTicketEv
// DEMANGLED: CStackableItem::IsReinforceTicket(void)
int CStackableItem::IsReinforceTicket() {
    return reinforce_ticket_;
}

// IDA_EA: 0x84B4232
// IDA_NAME: _ZN14CStackableItem16GetReinforceRateEv
// DEMANGLED: CStackableItem::GetReinforceRate(void)
int CStackableItem::GetReinforceRate() {
    return reinforce_rate_;
}

// IDA_EA: 0x84B4242
// IDA_NAME: _ZN14CStackableItem24IsAmplifyReinforceTicketEv
// DEMANGLED: CStackableItem::IsAmplifyReinforceTicket(void)
int CStackableItem::IsAmplifyReinforceTicket() {
    return amplify_reinforce_ticket_;
}

// IDA_EA: 0x84B4252
// IDA_NAME: _ZN14CStackableItem23GetAmplifyReinforceRateEv
// DEMANGLED: CStackableItem::GetAmplifyReinforceRate(void)
int CStackableItem::GetAmplifyReinforceRate() {
    return amplify_reinforce_rate_;
}

// IDA_EA: 0x850D780
// IDA_NAME: _ZNK14CStackableItem17getUsableItemTypeEi
// DEMANGLED: CStackableItem::getUsableItemType(int)const
bool CStackableItem::getUsableItemType(int item_type) const {
    return std::find(usable_equipment_types_.begin(),
                     usable_equipment_types_.end(),
                     item_type) != usable_equipment_types_.end();
}

// IDA_EA: 0x850EB42
// IDA_NAME: _ZNK14CStackableItem24IsAvatarOptionChangeItemEv
// DEMANGLED: CStackableItem::IsAvatarOptionChangeItem(void)const
int CStackableItem::IsAvatarOptionChangeItem() const {
    return avatar_option_change_item_;
}

// IDA_EA: 0x850EB52
// IDA_NAME: _ZN14CStackableItem17check_able_To_useEii
// DEMANGLED: CStackableItem::check_able_To_use(int,int)
int CStackableItem::check_able_To_use(int charac_job, int charac_level) {
    if (check_job_type(static_cast<char>(charac_job)) != 1)
        return 1;
    return check_low_level(charac_level) == 1 ? 0 : 2;
}

// IDA_EA: 0x850EBA2
// IDA_NAME: _ZNK14CStackableItem21check_change_job_typeEcc
// DEMANGLED: CStackableItem::check_change_job_type(char,char)const
bool CStackableItem::check_change_job_type(char grow_type,
                                            char charac_job) const {
    if (check_job_type(charac_job) != 1)
        return false;
    const signed char required = static_cast<signed char>(
        arad_item_info_.values_00[1]);
    return required < 0 || required == grow_type;
}

// IDA_EA: 0x850F732
// IDA_NAME: _ZN14CStackableItem18quickslot_priorityEv
// DEMANGLED: CStackableItem::quickslot_priority(void)
bool CStackableItem::quickslot_priority() {
    return item_type_ == 0 || item_type_ == 4;
}

// IDA_EA: 0x850F75E
// IDA_NAME: _ZNK14CStackableItem14getBoosterItemERSt6vectorI28stStackableBoosterItemInfo_tSaIS1_EE
// DEMANGLED: CStackableItem::getBoosterItem(std::vector<stStackableBoosterItemInfo_t,std::allocator<stStackableBoosterItemInfo_t>> &)const
bool CStackableItem::getBoosterItem(
    std::vector<stStackableBoosterItemInfo_t>& output) const {
    const stStackableBooster_t& booster = booster_data();
    for (std::vector<stStackableBoosterElement_t>::const_iterator element =
             booster.elements.begin();
         element != booster.elements.end();
         ++element) {
        for (unsigned int count = 0;
             count < static_cast<unsigned int>(element->item_count);
             ++count) {
            const unsigned int roll =
                get_rand_int(static_cast<int>(element->total_rate));
            for (std::vector<stStackableBoosterItemInfo_t>::const_iterator item =
                     element->items.begin();
                 item != element->items.end();
                 ++item) {
                if (roll < static_cast<unsigned int>(item->accumulated_rate)) {
                    output.push_back(*item);
                    break;
                }
            }
        }
    }

    // The old ABI returns the final iterator comparison result.  At normal
    // completion that comparison is false; callers use the output vector.
    return false;
}

// IDA_EA: 0x850FA14
// IDA_NAME: _ZN14CStackableItem6verifyEv
// DEMANGLED: CStackableItem::verify(void)
bool CStackableItem::verify() {
    if (usable_period_ && getStackableLimit() != 1)
        return false;
    return (item_type_ != 15 && item_type_ != 16) ||
           !booster_data_.elements.empty();
}

// IDA_EA: 0x850FA8A
// IDA_NAME: _ZN14CStackableItem35is_included_material_for_limit_cubeEj
// DEMANGLED: CStackableItem::is_included_material_for_limit_cube(unsigned int)
int CStackableItem::is_included_material_for_limit_cube(
    unsigned int item_id) {
    const std::vector<std::pair<int, int> >& materials =
        limit_cube_info_.condition_items;
    for (std::vector<std::pair<int, int> >::const_iterator it =
             materials.begin();
         it != materials.end();
         ++it) {
        if (static_cast<unsigned int>(it->first) == item_id)
            return 1;
    }
    return 0;
}

// IDA_EA: 0x850FB32
// IDA_NAME: _ZN14CStackableItem22get_1st_material_countEj
// DEMANGLED: CStackableItem::get_1st_material_count(unsigned int)
int CStackableItem::get_1st_material_count(unsigned int item_id) {
    const std::vector<std::pair<int, int> >& materials =
        limit_cube_info_.condition_items;
    for (std::vector<std::pair<int, int> >::const_iterator it =
             materials.begin();
         it != materials.end();
         ++it) {
        if (static_cast<unsigned int>(it->first) == item_id)
            return it->second;
    }
    return -1;
}

// IDA_EA: 0x850FBE4
// IDA_NAME: _ZN14CStackableItem19get_limit_cube_itemERSt4pairIiiE
// DEMANGLED: CStackableItem::get_limit_cube_item(std::pair<int,int> &)
std::pair<int, int>* CStackableItem::get_limit_cube_item(
    std::pair<int, int>& output) {
    output.first = 0;
    output.second = 0;

    const std::vector<stLimitCubeResult>& results =
        limit_cube_info().results;
    unsigned long total_probability = 0;
    for (std::vector<stLimitCubeResult>::const_iterator result =
             results.begin();
         result != results.end();
         ++result) {
        total_probability += static_cast<unsigned long>(result->probability);
    }

    // This is the original inclusive cumulative comparison: a zero roll (or
    // zero total) selects the first row with a zero cumulative rate as well.
    const unsigned int roll =
        limit_cube_random_->randInt(total_probability);
    unsigned int cumulative_probability = 0;
    for (std::vector<stLimitCubeResult>::const_iterator result =
             results.begin();
         result != results.end();
         ++result) {
        cumulative_probability += static_cast<unsigned int>(result->probability);
        if (roll <= cumulative_probability) {
            output.first = result->item_index;
            output.second = result->item_count;
            break;
        }
    }
    return &output;
}

// IDA_EA: 0x8513444
// IDA_NAME: _ZN14CStackableItem27get_amplify_reinforce_countEi
// DEMANGLED: CStackableItem::get_amplify_reinforce_count(int)
int CStackableItem::get_amplify_reinforce_count(int random_value) {
    const std::map<int, int>& rates = amplify_reinforce_count_rates_;
    int cumulative = 0;
    int greatest_rate = 0;
    int fallback = 0;
    for (std::map<int, int>::const_iterator it = rates.begin();
         it != rates.end();
         ++it) {
        cumulative += it->second;
        if (random_value < cumulative)
            return it->first;
        if (it->second > greatest_rate) {
            greatest_rate = it->second;
            fallback = it->first;
        }
    }
    return fallback;
}

bool CStackableItem::has_valid_random_amplify_reinforce_table() const {
    if (random_amplify_reinforce_ticket_ != 1u ||
        amplify_reinforce_count_rates_.empty()) {
        return false;
    }

    int total = 0;
    for (std::map<int, int>::const_iterator current =
             amplify_reinforce_count_rates_.begin();
         current != amplify_reinforce_count_rates_.end(); ++current) {
        if (current->first < 0 || current->first >= 32 ||
            current->second <= 0 || total > 100 - current->second) {
            return false;
        }
        total += current->second;
    }
    return total == 100;
}

// IDA_EA: 0x8514A84
// IDA_NAME: _ZNK14CStackableItem11GetItemTypeEv
// DEMANGLED: CStackableItem::GetItemType(void)const
int CStackableItem::GetItemType() const {
    return item_type_;
}

// IDA_EA: 0x8514A9C
// IDA_NAME: _ZNK14CStackableItem14IsCreatureItemEv
// DEMANGLED: CStackableItem::IsCreatureItem(void)const
int CStackableItem::IsCreatureItem() const {
    return item_type_ > 8 && item_type_ <= 10;
}

// IDA_EA: 0x8514ADE
// IDA_NAME: _ZNK14CStackableItem21GetGuildPowerWarPointEv
// DEMANGLED: CStackableItem::GetGuildPowerWarPoint(void)const
int CStackableItem::GetGuildPowerWarPoint() const {
    return guild_power_war_point_;
}

// IDA_EA: 0x8514AEC
// IDA_NAME: _ZNK14CStackableItem20GetUserPowerWarPointEv
// DEMANGLED: CStackableItem::GetUserPowerWarPoint(void)const
int CStackableItem::GetUserPowerWarPoint() const {
    return user_power_war_point_;
}

// IDA_EA: 0x8514AF8
// IDA_NAME: _ZNK14CStackableItem16IsImpossibleGiftEv
// DEMANGLED: CStackableItem::IsImpossibleGift(void)const
int CStackableItem::IsImpossibleGift() const {
    return impossible_gift_;
}

// IDA_EA: 0x8514B08
// IDA_NAME: _ZNK14CStackableItem20IsRandomOptionUnsealEv
// DEMANGLED: CStackableItem::IsRandomOptionUnseal(void)const
int CStackableItem::IsRandomOptionUnseal() const {
    return random_option_unseal_;
}

// IDA_EA: 0x8514B18
// IDA_NAME: _ZN14CStackableItem26set_parent_booster_item_idEm
// DEMANGLED: CStackableItem::set_parent_booster_item_id(unsigned long)
CStackableItem* CStackableItem::set_parent_booster_item_id(
    unsigned long item_id) {
    parent_booster_item_id_ = item_id;
    return this;
}

// IDA_EA: 0x8514B2A
// IDA_NAME: _ZNK14CStackableItem26get_parent_booster_item_idEv
// DEMANGLED: CStackableItem::get_parent_booster_item_id(void)const
int CStackableItem::get_parent_booster_item_id() const {
    return static_cast<int>(parent_booster_item_id_);
}

// IDA_EA: 0x8514B38
// IDA_NAME: _ZNK14CStackableItem18getTradeLimitCountEv
// DEMANGLED: CStackableItem::getTradeLimitCount(void)const
int CStackableItem::getTradeLimitCount() const {
    return trade_limit_count_;
}

// IDA_EA: 0x854B4E8
// IDA_NAME: _ZN14CStackableItem13GetEmancipateEv
// DEMANGLED: CStackableItem::GetEmancipate(void)
int CStackableItem::GetEmancipate() {
    return continuous_item_state_.emancipation_value;
}

// IDA_EA: 0x854B4F8
// IDA_NAME: _ZN14CStackableItem17GetReinforceVauleEv
// DEMANGLED: CStackableItem::GetReinforceVaule(void)
int CStackableItem::GetReinforceVaule() {
    return reinforce_value_;
}

// IDA_EA: 0x854B508
// IDA_NAME: _ZN14CStackableItem24GetAmplifyReinforceValueEv
// DEMANGLED: CStackableItem::GetAmplifyReinforceValue(void)
int CStackableItem::GetAmplifyReinforceValue() {
    return amplify_reinforce_value_;
}

// IDA_EA: 0x854B518
// IDA_NAME: _ZN14CStackableItem34is_random_amplify_reinforce_ticketEv
// DEMANGLED: CStackableItem::is_random_amplify_reinforce_ticket(void)
int CStackableItem::is_random_amplify_reinforce_ticket() {
    return random_amplify_reinforce_ticket_;
}

// IDA_EA: 0x86946D2
// IDA_NAME: _ZN14CStackableItem17GetCoolTimeOptionEv
// DEMANGLED: CStackableItem::GetCoolTimeOption(void)
CStackableCoolTimeOption CStackableItem::GetCoolTimeOption() {
    CStackableCoolTimeOption result = {
        static_cast<uint16_t>(
            continuous_item_state_.cool_time_maintenance.option_type |
            (static_cast<uint16_t>(continuous_item_state_
                                       .cool_time_maintenance.option_value)
             << 8)),
        continuous_item_state_.cool_time_maintenance.enabled};
    return result;
}

// IDA_EA: 0x86946F4
// IDA_NAME: _ZN14CStackableItem19GetEffectItemOptionEv
// DEMANGLED: CStackableItem::GetEffectItemOption(void)
CStackableEffectItemOption CStackableItem::GetEffectItemOption() {
    CStackableEffectItemOption result = {
        static_cast<uint16_t>(
            continuous_item_state_.effect_maintenance.option_type |
            (static_cast<uint16_t>(continuous_item_state_
                                       .effect_maintenance.option_value)
             << 8)),
        continuous_item_state_.effect_maintenance.enabled};
    return result;
}

// IDA_EA: 0x8694716
// IDA_NAME: _ZN14CStackableItem17GetStatChangeInfoEv
// DEMANGLED: CStackableItem::GetStatChangeInfo(void)
CStackableStatChangeInfo CStackableItem::GetStatChangeInfo() {
    return continuous_item_state_.stat_change;
}

// IDA_EA: 0x8694736
// IDA_NAME: _ZN14CStackableItem11IsExpAffectEv
// DEMANGLED: CStackableItem::IsExpAffect(void)
int CStackableItem::IsExpAffect() {
    return continuous_item_state_.exp_affect_enabled;
}

// IDA_EA: 0x8694746
// IDA_NAME: _ZN14CStackableItem17GetExpAffectRatioEv
// DEMANGLED: CStackableItem::GetExpAffectRatio(void)
long double CStackableItem::GetExpAffectRatio() {
    return continuous_item_state_.exp_affect_ratio;
}

// IDA_EA: 0x869475E
// IDA_NAME: _ZNK14CStackableItem26IsUse2ndUpgradeTableRecipeEv
// DEMANGLED: CStackableItem::IsUse2ndUpgradeTableRecipe(void)const
int CStackableItem::IsUse2ndUpgradeTableRecipe() const {
    return script_flag_d0_;
}
