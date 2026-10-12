#include "old_game/player/core/cuser.h"

#include "common/manager/premium_public_abi.h"
#include "old_game/player/core/premium_runtime.h"

// IDA_EA: 0x85BF940
// IDA_NAME: _ZNK8WongWork12CUserPremium14GetAddExpBonusEi
// DEMANGLED: WongWork::CUserPremium::GetAddExpBonus(int)const
int WongWork::CUserPremium::GetAddExpBonus(int premium_type) const {
    const unsigned int index =
        static_cast<unsigned int>(premium_type);
    return index < 106u ? g_SPremiumInfo[index].add_exp_bonus : 0;
}

// IDA_EA: 0x830ED48
// IDA_NAME: _ZN8WongWork12CUserPremium25GetAddIndependentDropRateEij
// DEMANGLED: WongWork::CUserPremium::GetAddIndependentDropRate(
//     int,unsigned int)
int WongWork::CUserPremium::GetAddIndependentDropRate(
    int premium_type,
    unsigned int drop_slot) {
    if (drop_slot == 0 || drop_slot > 4u) {
        return 0;
    }

    const unsigned int premium_index =
        static_cast<unsigned int>(premium_type);
    if (premium_index >= 106u) {
        return 0;
    }

    // The original indexes the configured four-item vector directly after
    // the exact slot range gate above; configuration loading owns its extent.
    return static_cast<int>(
        g_SPremiumInfo[premium_index].independent_drop_items[drop_slot - 1u]);
}

// IDA_EA: 0x830EDAA
// IDA_NAME: _ZN5CUser36getAddIndependentDropRateFromPremiumE17ENUM_PREMIUM_TYPEj
// DEMANGLED: CUser::getAddIndependentDropRateFromPremium(
//     ENUM_PREMIUM_TYPE,unsigned int)
int CUser::getAddIndependentDropRateFromPremium(
    ENUM_PREMIUM_TYPE premium_type,
    unsigned int drop_slot) {
    return GetPremiumInfo()->GetAddIndependentDropRate(
        static_cast<int>(premium_type), drop_slot);
}

// IDA_EA: 0x85BFD76
// IDA_NAME: _ZN5CUser24getAddExpRateFromPremiumE17ENUM_PREMIUM_TYPE
// DEMANGLED: CUser::getAddExpRateFromPremium(ENUM_PREMIUM_TYPE)
int CUser::getAddExpRateFromPremium(
    ENUM_PREMIUM_TYPE premium_type) {
    return GetPremiumInfo()->GetAddExpBonus(
        static_cast<int>(premium_type));
}

// IDA_EA: 0x868DC64
// IDA_NAME: _ZN5CUser27get_growth_power_exp_rewardEv
// DEMANGLED: CUser::get_growth_power_exp_reward(void)
STExpReward CUser::get_growth_power_exp_reward() {
    return exp_reward_;
}
