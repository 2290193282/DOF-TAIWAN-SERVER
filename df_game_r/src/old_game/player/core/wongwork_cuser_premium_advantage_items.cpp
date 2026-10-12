#include "old_game/player/core/premium_runtime.h"

#include <time.h>

#include <utility>
#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

namespace WongWork {

// IDA_EA: 0x86ADF7C
// IDA_NAME: _ZN8WongWork12CUserPremium16GetAdvantageItemEi
// DEMANGLED: WongWork::CUserPremium::GetAdvantageItem(int)
std::vector<std::pair<int, int> > CUserPremium::GetAdvantageItem(
    int premium_type) {
    // The original takes the system-time/localtime path even though the table
    // lookup below has no weekday branch.  Keep that observable lifecycle
    // behaviour rather than optimizing it away.
    time_t timer = static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    tm local_time;
    localtime_r(&timer, &local_time);

    std::vector<std::pair<int, int> > result;
    const SUserPremiumInfo& info = premium_info_[premium_type];
    if (info.premium_type != 0 && info.premium_type <= 105 &&
        g_SPremiumInfo[info.premium_type].advantage_bucket == 1) {
        const SPremiumInfo& premium = g_SPremiumInfo[info.premium_type];
        for (int item_index = 0;
             item_index <= 4 && premium.item_rewards[item_index].item_id != 0;
             ++item_index) {
            result.push_back(std::make_pair(
                static_cast<int>(premium.item_rewards[item_index].item_id),
                static_cast<int>(premium.item_rewards[item_index].item_count)));
        }
    }
    return result;
}

// IDA_EA: 0x86B086C
// IDA_NAME: _ZN8WongWork12CUserPremium19IsNeedCheckEverydayEv
// DEMANGLED: WongWork::CUserPremium::IsNeedCheckEveryday(void)
int CUserPremium::IsNeedCheckEveryday() {
    return static_cast<unsigned char>(need_check_everyday_);
}

}  // namespace WongWork
