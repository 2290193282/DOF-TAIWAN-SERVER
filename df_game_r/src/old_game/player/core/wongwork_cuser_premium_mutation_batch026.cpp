#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <stdint.h>
#include <string.h>

namespace WongWork {

// IDA_EA: 0x86ADAD2
// IDA_NAME: _ZN8WongWork12CUserPremium10AddPremiumEiiii
// DEMANGLED: WongWork::CUserPremium::AddPremium(int,int,int,int)
int CUserPremium::AddPremium(int premium_type,
                             int start_time,
                             int end_time,
                             int param) {
    SUserPremiumInfo& info = premium_info_[premium_type];
    if (info.premium_type != 0) {
        RemovePremium(premium_type);
    }
    info.premium_type = premium_type;
    info.start_time = start_time;
    info.end_time = end_time;
    info.param = param;
    if (g_SPremiumInfo[premium_type].apply_weekdays[0] != -1) {
        need_check_everyday_ = true;
    }
    return startPremium(premium_type, true);
}

// IDA_EA: 0x86ADBA6
// IDA_NAME: _ZN8WongWork12CUserPremium13RemovePremiumEi
// DEMANGLED: WongWork::CUserPremium::RemovePremium(int)
CUserPremium* CUserPremium::RemovePremium(int premium_type) {
    SUserPremiumInfo& info = premium_info_[premium_type];
    if (info.state == 2) {
        return reinterpret_cast<CUserPremium*>(static_cast<uintptr_t>(2u));
    }

    info.premium_type = 0;
    if (g_SPremiumInfo[premium_type].advantage_bucket == 1) {
        advantage_premium_count_ =
            std::max(0, advantage_premium_count_ - 1);
    } else {
        normal_premium_count_ = std::max(0, normal_premium_count_ - 1);
    }
    return this;
}

// IDA_EA: 0x86ADC6A
// IDA_NAME: _ZN8WongWork12CUserPremium12startPremiumEib
// DEMANGLED: WongWork::CUserPremium::startPremium(int,bool)
int CUserPremium::startPremium(int premium_type, bool force_start) {
    SUserPremiumInfo& info = premium_info_[premium_type];
    if (!force_start && info.state != 2) {
        return 0;
    }

    const int current_second = GlobalData::system_time()->getCurSec();
    const SPremiumInfo& premium = g_SPremiumInfo[premium_type];
    if (premium.uses_time_window != 0) {
        if (info.start_time > current_second) {
            info.state = 2;
            return 0;
        }
        if (info.end_time <= current_second) {
            return 0;
        }
    }

    if (premium.advantage_bucket == 1) {
        ++advantage_premium_count_;
    } else {
        ++normal_premium_count_;
    }
    info.state = 1;
    return 1;
}

// IDA_EA: 0x86AE10E
// IDA_NAME: _ZN8WongWork12CUserPremium15ReCalcAdvantageEv
// DEMANGLED: WongWork::CUserPremium::ReCalcAdvantage(void)
bool CUserPremium::ReCalcAdvantage() {
    advantage_fatigue_rate_ = 0;
    advantage_coin_ = 0;
    advantage_exp_rate_ = 0;
    unlimited_fatigue_ = false;
    gold_bonus_ = false;
    memset(over_equipable_level_, 0, sizeof(over_equipable_level_));
    advantage_additional_info_.clear();
    calculated_additional_info_.clear();
    return_item_rates_.clear();
    over_skill_level_ = 0;
    cera_shop_discount_rate_ = 0;

    const time_t now =
        static_cast<time_t>(GlobalData::system_time()->getCurSec());
    tm local_time;
    localtime_r(&now, &local_time);

    for (int slot = 1; slot <= 105; ++slot) {
        const SUserPremiumInfo& info = premium_info_[slot];
        const int premium_type = info.premium_type;
        const SPremiumInfo& premium = g_SPremiumInfo[premium_type];
        if (premium_type == 0 ||
            premium.advantage_bucket != 1 ||
            info.state == 2 ||
            _CheckApply(premium_type, local_time) != 1) {
            continue;
        }

        if (premium.unlimited_fatigue) {
            unlimited_fatigue_ = true;
        }
        over_skill_level_ = std::max(
            over_skill_level_,
            premium.over_skill_level);
        cera_shop_discount_rate_ = std::max(
            cera_shop_discount_rate_,
            premium.cera_shop_discount_rate);
        advantage_fatigue_rate_ = static_cast<uint16_t>(
            advantage_fatigue_rate_ +
            premium.fatigue_rate);
        advantage_exp_rate_ += premium.exp_bonus;
        advantage_coin_ += premium.coin_bonus;
        if (premium.gold_bonus) {
            gold_bonus_ = true;
        }

        const SPremiumInfo& slot_premium = g_SPremiumInfo[slot];
        const int over_count = slot_premium.over_equipable_count;
        for (int i = 0; i < over_count; ++i) {
            const int vector_index = slot_premium.over_equipable[i].list_index;
            const SPremiumOverEquipableInfo& values =
                g_vecPremiumOverEquipableInfo[vector_index];
            const int value_count =
                static_cast<int>(values.equipment_type_count);
            for (int j = 0; j < value_count; ++j) {
                const int equipment_type =
                    static_cast<int>(values.equipment_types[j]);
                over_equipable_level_[equipment_type] = std::max(
                    over_equipable_level_[equipment_type],
                    slot_premium.over_equipable[i].rate);
            }
        }

        const int return_count = slot_premium.return_item_count;
        for (int i = 0; i < return_count; ++i) {
            const int vector_index = slot_premium.return_items[i].list_index;
            const SPremiumReturnItemInfo& values =
                g_vecPremiumReturnItemInfo[vector_index];
            const int value_count = static_cast<int>(values.item_count);
            for (int j = 0; j < value_count; ++j) {
                const int item_id = static_cast<int>(values.item_ids[j]);
                return_item_rates_[item_id] += slot_premium.return_items[i].rate;
            }
        }

        advantage_additional_info_ += g_SPremiumInfo[premium_type].additional_info;
    }
    return false;
}

// IDA_EA: 0x86AE834
// IDA_NAME: _ZN8WongWork12CUserPremium11_CheckApplyEi2tm
// DEMANGLED: WongWork::CUserPremium::_CheckApply(int,tm)
int CUserPremium::_CheckApply(int premium_type, tm local_time) {
    const int* const weekdays = g_SPremiumInfo[premium_type].apply_weekdays;
    if (weekdays[0] == -1) {
        return 1;
    }

    int applies = 0;
    for (int index = 0; index <= 6 && weekdays[index] != -1; ++index) {
        if (weekdays[index] == local_time.tm_wday) {
            applies = 1;
        }
    }
    return applies;
}

// IDA_EA: 0x86AE8C6
// IDA_NAME: _ZN8WongWork12CUserPremium20RecalcAdditionalInfoEPK5CUser
// DEMANGLED: WongWork::CUserPremium::RecalcAdditionalInfo(CUser const*)
CUserPremium* CUserPremium::RecalcAdditionalInfo(const CUser* user) {
    if (!user->getCurCharacR() || GetAdvPremiumCount() == 0) {
        return reinterpret_cast<CUserPremium*>(static_cast<uintptr_t>(1u));
    }

    const _Additioal_info* const current = user->getCurCharacAddInfoR();
    calculated_additional_info_.max_hp =
        current->max_hp * advantage_additional_info_.max_hp / 100u;
    calculated_additional_info_.max_mp =
        current->max_mp * advantage_additional_info_.max_mp / 100u;
    calculated_additional_info_.physical_attack = static_cast<uint16_t>(
        static_cast<int16_t>(current->physical_attack) *
        static_cast<int16_t>(advantage_additional_info_.physical_attack) /
        100);
    calculated_additional_info_.physical_defense = static_cast<uint16_t>(
        static_cast<int16_t>(current->physical_defense) *
        static_cast<int16_t>(advantage_additional_info_.physical_defense) /
        100);
    calculated_additional_info_.magical_attack = static_cast<uint16_t>(
        static_cast<int16_t>(current->magical_attack) *
        static_cast<int16_t>(advantage_additional_info_.magical_attack) /
        100);
    calculated_additional_info_.magical_defense = static_cast<uint16_t>(
        static_cast<uint32_t>(current->magical_defense) *
        advantage_additional_info_.magical_defense / 100u);
    for (int index = 0; index < 4; ++index) {
        calculated_additional_info_.element_resist[index] =
            static_cast<uint16_t>(
                static_cast<int16_t>(current->element_resist[index]) *
                static_cast<int16_t>(
                    advantage_additional_info_.element_resist[index]) /
                100);
    }
    calculated_additional_info_.inventory_weight =
        static_cast<int32_t>(
            static_cast<uint32_t>(current->inventory_weight) *
            static_cast<uint32_t>(
                advantage_additional_info_.inventory_weight) /
            100u);
    calculated_additional_info_.hp_regen_speed = static_cast<int16_t>(
        current->hp_regen_speed *
        advantage_additional_info_.hp_regen_speed / 100);
    calculated_additional_info_.mp_regen_speed = static_cast<int16_t>(
        current->mp_regen_speed *
        advantage_additional_info_.mp_regen_speed / 100);
    calculated_additional_info_.move_speed =
        current->move_speed * advantage_additional_info_.move_speed / 100u;
    calculated_additional_info_.attack_speed = static_cast<uint16_t>(
        static_cast<int16_t>(current->attack_speed) *
        static_cast<int16_t>(advantage_additional_info_.attack_speed) / 100);
    calculated_additional_info_.cast_speed = static_cast<uint16_t>(
        static_cast<int16_t>(current->cast_speed) *
        static_cast<int16_t>(advantage_additional_info_.cast_speed) / 100);
    calculated_additional_info_.hit_recovery = static_cast<int16_t>(
        current->hit_recovery * advantage_additional_info_.hit_recovery /
        100);
    calculated_additional_info_.jump_power = static_cast<int16_t>(
        current->jump_power * advantage_additional_info_.jump_power / 100);
    return this;
}

}  // namespace WongWork
