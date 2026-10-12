#include "old_game/player/core/local_china_user.h"

#include <stdint.h>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/reliable_person.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"


// IDA_EA: 0x812CA88
// IDA_NAME: _ZN16CLocalChina_User16SendFatigueEventEv
// DEMANGLED: CLocalChina_User::SendFatigueEvent(void)
bool CLocalChina_User::SendFatigueEvent() {
    CMaxFatigueFactorEvent* repeat_event =
        static_cast<CMaxFatigueFactorEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(2));

    G_GameWorld()->ProcessAllUsers([repeat_event](CUser* user) {
        if (user->get_state() <= 2) {
            return;
        }

        if (repeat_event) {
            _Charac_info* const current_charac = user->getCurCharacW();
            if (!current_charac) {
                return;
            }

            if (repeat_event->IsEventing(0)) {
                const int max_fatigue_factor =
                    repeat_event->GetMaxFatigueFactor();
                current_charac->fields_.maximum_fatigue_ =
                    static_cast<uint16_t>(DEFAULT_MAX_FATIGUE *
                                          max_fatigue_factor / 100);
            } else {
                current_charac->fields_.maximum_fatigue_ =
                    static_cast<uint16_t>(DEFAULT_MAX_FATIGUE);
            }
        }

        if (user->IsHavePremiumAdvantage()) {
            if (repeat_event && repeat_event->IsEventing(0)) {
                const uint16_t extra_max_fatigue = static_cast<uint16_t>(
                    DEFAULT_MAX_FATIGUE *
                    repeat_event->GetMaxFatigueFactor() / 100 -
                    DEFAULT_MAX_FATIGUE);
                WongWork::CUserPremium* premium = user->GetPremiumInfo();
                const int base_premium_fatigue =
                    DEFAULT_MAX_FATIGUE *
                    static_cast<uint16_t>(
                        premium->GetAdvantageFatigueRate()) /
                    100;
                WongWork::CUserPremium* premium_again =
                    user->GetPremiumInfo();
                const uint16_t rate = static_cast<uint16_t>(
                    premium_again->GetAdvantageFatigueRate());
                user->setCurCharacMaxPremiumFatigue(
                    static_cast<uint16_t>(
                        base_premium_fatigue +
                        rate * extra_max_fatigue / 100));
            } else {
                const uint16_t current_max_fatigue =
                    static_cast<uint16_t>(
                        user->getCurCharacMaxFatigue());
                WongWork::CUserPremium* premium = user->GetPremiumInfo();
                const uint16_t rate = static_cast<uint16_t>(
                    premium->GetAdvantageFatigueRate());
                user->setCurCharacMaxPremiumFatigue(
                    static_cast<uint16_t>(
                        rate * current_max_fatigue / 100));
            }
        }

        user->SendFatigue();
    });
    return false;
}
