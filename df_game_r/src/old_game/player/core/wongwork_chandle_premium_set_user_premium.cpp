#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "common/manager/system_time.h"

#include <stdint.h>

// IDA_EA: 0x863BE94
// IDA_NAME: _ZN5CUser15GetPremiumInfoWEv
// DEMANGLED: CUser::GetPremiumInfoW(void)
WongWork::CUserPremium* CUser::GetPremiumInfoW() {
    return &user_premium_;
}

namespace WongWork {

// IDA_EA: 0x86AF7E8
// IDA_NAME: _ZN8WongWork14CHandlePremium20handleSetUserPremiumEP5CUser17ENUM_PREMIUM_TYPEllib
// DEMANGLED: WongWork::CHandlePremium::handleSetUserPremium(CUser *, ENUM_PREMIUM_TYPE, long, long, int, bool)
CUserPremium* CHandlePremium::handleSetUserPremium(CUser* user,
                                                    ENUM_PREMIUM_TYPE premium_type,
                                                    long start_time,
                                                    long end_time,
                                                    int param,
                                                    bool recalculate) {
    CUserPremium* const premium = user->GetPremiumInfoW();
    CUserPremium* result = 0;

    if (premium_type == 5) {
        // IDA retains the historical int-to-pointer return of AddPremium.
        result = reinterpret_cast<CUserPremium*>(static_cast<uintptr_t>(
            premium->AddPremium(5,
                                static_cast<int>(start_time),
                                static_cast<int>(end_time),
                                param)));
    } else if (start_time != 0 && end_time != 0) {
        const int current_second = GlobalData::system_time()->getCurSec();
        result = reinterpret_cast<CUserPremium*>(
            static_cast<uintptr_t>(current_second));
        if (current_second < end_time) {
            result = reinterpret_cast<CUserPremium*>(static_cast<uintptr_t>(
                premium->AddPremium(premium_type,
                                    static_cast<int>(start_time),
                                    static_cast<int>(end_time),
                                    param)));
        }
    } else {
        result = premium->RemovePremium(premium_type);
    }

    if (recalculate) {
        premium->ReCalcAdvantage();
        CUserPremium* const recalculated = premium->RecalcAdditionalInfo(user);
        // Premium start/removal/timeout has now committed the authoritative
        // over-skill projection.  Apply only PVF fixed-level awakening actives
        // and never alter the user's SP balance.
        (void)ReconcileAutomaticGiveSkillLevels(user, user->get_state() > 2);
        return recalculated;
    }
    return result;
}

}  // namespace WongWork
