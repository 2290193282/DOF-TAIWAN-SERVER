#include "old_game/player/core/premium_runtime.h"

namespace WongWork {

// IDA_EA: 0x86AE7A8
// IDA_NAME: _ZNK8WongWork12CUserPremium17GetReturnItemRateEi
// DEMANGLED: WongWork::CUserPremium::GetReturnItemRate(int)const
int CUserPremium::GetReturnItemRate(int item_key) const {
    const std::map<int, int>::const_iterator found =
        return_item_rates_.find(item_key);
    return found == return_item_rates_.end() ? 0 : found->second;
}

}  // namespace WongWork
