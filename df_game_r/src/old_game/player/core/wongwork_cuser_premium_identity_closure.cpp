#include "old_game/player/core/premium_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

namespace WongWork {

// IDA_EA: 0x8328100
// IDA_NAME: _ZNK8WongWork12CUserPremium23getCeraShopDiscountRateEv
// DEMANGLED: WongWork::CUserPremium::getCeraShopDiscountRate(void)const
int CUserPremium::getCeraShopDiscountRate() const {
    return static_cast<int>(cera_shop_discount_rate_);
}

// IDA_EA: 0x86ADD9C
// IDA_NAME: _ZNK8WongWork12CUserPremium19CheckPremiumTimeoutEv
// DEMANGLED: WongWork::CUserPremium::CheckPremiumTimeout(void)const
SUserPremiumInfo CUserPremium::CheckPremiumTimeout() const {
    const int current_second = GlobalData::system_time()->getCurSec();
    SUserPremiumInfo result;
    result.premium_type = 0;

    for (int premium_type = 1; premium_type <= 105; ++premium_type) {
        const SUserPremiumInfo& info = premium_info_[premium_type];
        if (info.premium_type != 0 &&
            g_SPremiumInfo[info.premium_type].uses_time_window != 0 &&
            info.state != 3 && info.end_time <= current_second) {
            return info;
        }
    }
    return result;
}

// IDA_EA: 0x86ADF52
// IDA_NAME: _ZNK8WongWork12CUserPremium14GetPremiumInfoEi
// DEMANGLED: WongWork::CUserPremium::GetPremiumInfo(int)const
const SUserPremiumInfo* CUserPremium::GetPremiumInfo(int premium_type) const {
    if (premium_type <= 105) {
        return &premium_info_[premium_type];
    }
    return premium_info_;
}

// IDA_EA: 0x86B084C
// IDA_NAME: _ZN8WongWork12CUserPremium15setPremiumStateE17ENUM_PREMIUM_TYPENS_18ENUM_PREMIUM_STATEE
// DEMANGLED: WongWork::CUserPremium::setPremiumState(ENUM_PREMIUM_TYPE,WongWork::ENUM_PREMIUM_STATE)
int CUserPremium::setPremiumState(ENUM_PREMIUM_TYPE premium_type,
                                  ENUM_PREMIUM_STATE state) {
    const int state_value = static_cast<int>(state);
    premium_info_[static_cast<int>(premium_type)].state = state_value;
    return state_value;
}

}  // namespace WongWork
