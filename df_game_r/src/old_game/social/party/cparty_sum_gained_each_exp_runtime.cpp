#include "old_game/social/party/party.h"

#include <cstddef>

#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"

// IDA_EA: 0x85A2488
// IDA_NAME: _ZN6CParty16sumGainedEachExpEP5CUserRK19STGainedEachExpData
// DEMANGLED: CParty::sumGainedEachExp(CUser *,
//             STGainedEachExpData const&)
int CParty::sumGainedEachExp(
    CUser* user,
    const STGainedEachExpData& gained_each_exp) {
    int total_exp = 0;
    unsigned int base_exp = gained_each_exp.gained_exp;
    const float exp_scale = gained_each_exp.exp_scale;
    int fatigue_bonus_exp = 0;
    int seria_bonus_exp = 0;
    int weekend_bonus_exp = 0;
    int champion_bonus_exp = 0;

    if (static_cast<unsigned char>(
            battle_field_.isWorldMapTowerArea()) != 1u) {
        int fatigue_bonus_rate =
            static_cast<unsigned short>(
                user->getCurCharacFatigueGrownUpBuff()) != 0u
                ? 30
                : 0;
        const int acceleration_bonus_rate =
            fatigue_acceleration::GetHunterExpBonusRate(
                user,
                battle_field_.get_dungeon(),
                battle_field_.get_dungeon_index());
        if (acceleration_bonus_rate > fatigue_bonus_rate) {
            fatigue_bonus_rate = acceleration_bonus_rate;
        }
        if (fatigue_bonus_rate > 0) {
            // RECONSTRUCTED_0828: dstr 22392 supplies the 30% hunting-EXP
            // rate.  Reuse the canonical fatigue-bonus accounting slot and
            // treat the old grown-up buff as the same category, so the two
            // sources cannot multiply or stack to 60%.
            fatigue_bonus_exp = static_cast<int>(
                static_cast<long double>(base_exp) *
                static_cast<long double>(fatigue_bonus_rate) / 100.0L);
            total_exp += fatigue_bonus_exp;
            user->set_charac_fatigue_buf_bonus_exp(
                fatigue_bonus_exp);
        }

        STExpReward growth_power_reward;
        growth_power_reward = user->get_growth_power_exp_reward();
        const float growth_power_rate =
            static_cast<float>(
                static_cast<long double>(
                    growth_power_reward.gained_exp_bonus_rate) /
                100.0L);
        seria_bonus_exp = static_cast<int>(
            static_cast<long double>(base_exp) *
            static_cast<long double>(growth_power_rate));
        user->set_charac_seria_buf_bonus_exp(seria_bonus_exp);
        total_exp += seria_bonus_exp;

        CEventBase* const weekend_event =
            GlobalData::s_event_manager->GetRepeatEvent(87);
        if (weekend_event->IsEventing(0)) {
            user->setWeekendBonusExp(
                base_exp,
                G_CDataManager()->get_server_parameter_script()
                    ->get_weekend_bonus_exp());
            weekend_bonus_exp = user->getWeekendBonusExp();
            total_exp += weekend_bonus_exp;
        }

        if (user->isAffectedPremium(
                static_cast<ENUM_PREMIUM_TYPE>(79))) {
            const int premium_rate =
                user->getAddExpRateFromPremium(
                    static_cast<ENUM_PREMIUM_TYPE>(79));
            total_exp += user->setPremiumBonusExp(
                base_exp, premium_rate);
        }
    }

    const int champion_bonus_rate =
        champion_break::GetHunterExpBonusRate(user);
    if (champion_bonus_rate > 0) {
        champion_bonus_exp = static_cast<int>(
            static_cast<long double>(base_exp) * champion_bonus_rate / 100.0L);
        total_exp += champion_bonus_exp;
    }
    // NOTI37 field 15 is an incremental category, not a percentage.  Record
    // exactly the amount already included in this award so the client does not
    // double-count it when rendering DSTR80009.
    champion_break::RecordHunterExpBonus(user, champion_bonus_exp);

    float advantage_exp_rate = static_cast<float>(
        user->GetPremiumInfo()->GetAdvantageExpRate());
    advantage_exp_rate =
        static_cast<float>(CServerEvent::GetExpRate()) +
        advantage_exp_rate;
    base_exp = static_cast<unsigned int>(
        static_cast<long long>(
            static_cast<long double>(base_exp) +
            static_cast<long double>(base_exp) *
                static_cast<long double>(advantage_exp_rate) /
                100.0L));
    total_exp += static_cast<int>(base_exp);

    const int party_bonus_exp =
        static_cast<int>(base_exp) -
        static_cast<int>(
            static_cast<long double>(base_exp) /
            static_cast<long double>(exp_scale));
    user->set_charac_party_bonus_exp(party_bonus_exp);

    total_exp += user->gainExpAsUpperMember(
        static_cast<int>(base_exp));
    total_exp += user->gainGuildSkillExp(
        static_cast<int>(base_exp));
    return user->gainPowerWarRewardExp(
               static_cast<int>(base_exp)) +
           total_exp;
}
