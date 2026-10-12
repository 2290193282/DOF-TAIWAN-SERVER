#include "old_game/player/core/timer_premium_service_runtime.h"

#include <ctime>

#include "common/config.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"

namespace {

enum PremiumTimerMessage {
    kPremiumServiceOver = 4,
    kPremiumServiceUpdate = 5,
    kPremiumServiceStart = 6,
};

CUser* ResolvePremiumTimerUser(int user_uid, unsigned int account_id) {
    CGameManager* const manager = G_CGameManager();
    CUser* const user = manager ? manager->getUser(user_uid, 0) : 0;
    return user && static_cast<unsigned int>(user->get_acc_id()) == account_id
               ? user
               : 0;
}

}  // namespace

// IDA_EA: 0x863485A
// IDA_NAME: _ZN23TimerPremiumServiceOver12dispatch_sigEiij
int TimerPremiumServiceOver::dispatch_sig(int user_uid,
                                          int premium_type,
                                          unsigned int account_id) {
    (void)premium_type;
    CUser* const user = ResolvePremiumTimerUser(user_uid, account_id);
    if (!user || user->IsPremiumUser() != 1) return 0;
    WongWork::CHandlePremium::handlePremiumTimeout(user);
    return 1;
}

// IDA_EA: 0x86348D4
// IDA_NAME: _ZN25TimerPremiumServiceUpdate12dispatch_sigEiij
int TimerPremiumServiceUpdate::dispatch_sig(int user_uid,
                                            int premium_type,
                                            unsigned int account_id) {
    if (premium_type <= 0 ||
        premium_type >= oldsrv::config::kPremiumDefinitionTypeCount) return 0;
    CUser* const user = ResolvePremiumTimerUser(user_uid, account_id);
    if (!user || user->IsPremiumUser() != 1) return 0;

    WongWork::CUserPremium* const premium = user->GetPremiumInfoW();
    premium->ReCalcAdvantage();
    premium->RecalcAdditionalInfo(user);
    (void)ReconcileAutomaticGiveSkillLevels(user, user->get_state() > 2);
    if (user->get_state() > 3) user->SendFatigue();

    // IDA 0x86349B8 zeroes tm_sec/tm_min/tm_hour, obtains local midnight,
    // then adds one day before re-queuing the same message and identity tuple.
    const std::time_t now = static_cast<std::time_t>(
        GlobalData::s_systemTime_.getCurSec());
    std::tm local_time;
    localtime_r(&now, &local_time);
    local_time.tm_sec = 0;
    local_time.tm_min = 0;
    local_time.tm_hour = 0;
    const std::time_t next_midnight = std::mktime(&local_time) +
        static_cast<std::time_t>(24 * 60 * 60);
    const int delay = static_cast<int>(next_midnight - now);
    G_TimerQueue()->InsertTimer(0, user_uid, kPremiumServiceUpdate,
                                delay, static_cast<unsigned int>(premium_type),
                                static_cast<int>(account_id));
    return 1;
}

// IDA_EA: 0x8634A30
// IDA_NAME: _ZN24TimerPremiumServiceStart12dispatch_sigEiij
int TimerPremiumServiceStart::dispatch_sig(int user_uid,
                                           int premium_type,
                                           unsigned int account_id) {
    if (premium_type <= 0 ||
        premium_type >= oldsrv::config::kPremiumDefinitionTypeCount) return 0;
    CUser* const user = ResolvePremiumTimerUser(user_uid, account_id);
    if (!user) return 0;
    WongWork::CUserPremium* const premium = user->GetPremiumInfoW();
    if (premium->startPremium(premium_type, false) != 1) return 0;

    if (user->get_state() > 2) {
        const WongWork::SUserPremiumInfo* const info =
            premium->GetPremiumInfo(premium_type);
        if (!info || info->premium_type == 0) return 1;
        WongWork::CHandlePremium::handleNotifyPremiumInfo(
            user, info->premium_type, info->end_time - info->start_time);
        if (g_SPremiumInfo[info->premium_type].advantage_bucket != 2) {
            WongWork::CHandlePremium::handlePremiumEffect(
                user, static_cast<ENUM_PREMIUM_TYPE>(info->premium_type));
        }
    }
    return 1;
}
