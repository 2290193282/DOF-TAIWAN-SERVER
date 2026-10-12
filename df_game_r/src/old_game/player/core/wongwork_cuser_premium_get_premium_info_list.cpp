#include "old_game/player/core/premium_runtime.h"

namespace WongWork {

// IDA_EA: 0x86ADE9C
// IDA_NAME: _ZNK8WongWork12CUserPremium18GetPremiumInfoListEi
// DEMANGLED: WongWork::CUserPremium::GetPremiumInfoList(int)const
std::vector<SUserPremiumInfo> CUserPremium::GetPremiumInfoList(
    int premium_type) const {
    std::vector<SUserPremiumInfo> result;

    // Slot zero is the original sentinel; runtime premium types occupy 1..105.
    for (int index = 1; index <= 105; ++index) {
        const SUserPremiumInfo& info = premium_info_[index];
        if (info.premium_type != 0 &&
            (premium_type == 9999 || info.premium_type == premium_type)) {
            result.push_back(info);
        }
    }
    return result;
}

}  // namespace WongWork
