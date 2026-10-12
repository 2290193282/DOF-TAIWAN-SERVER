#include "old_game/social/party/party.h"

#include <algorithm>
#include <stdint.h>

#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"

namespace {

uint32_t ScaledExp(uint32_t base_exp, float factor) {
    return static_cast<uint32_t>(static_cast<long double>(base_exp) * factor);
}

}  // namespace

// IDA_EA: 0x85ACFDE
// IDA_NAME: _ZN6CParty15CalcAvatarBonusEv
// DEMANGLED: CParty::CalcAvatarBonus(void)
long double CParty::CalcAvatarBonus() {
    bool is_avatar_equipped = false;
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        if (_checkValidUser(member_slot) == 1 &&
            members_[member_slot].user->IsEquipAvatar()) {
            is_avatar_equipped = true;
            break;
        }
    }

    if (!is_avatar_equipped) {
        return 0.0L;
    }
    return get_member_count() > 1 ? 0.050000001L : 0.02L;
}

// IDA_EA: 0x814A9C2
// IDA_NAME: _ZN6CParty14getAvatarBonusEv
// DEMANGLED: CParty::getAvatarBonus(void)
long double CParty::getAvatarBonus() {
    return CalcAvatarBonus();
}

// IDA_EA: 0x814A9E2
// IDA_NAME: _ZN6CParty16getCreatureBonusEP5CUser
// DEMANGLED: CParty::getCreatureBonus(CUser *)
long double CParty::getCreatureBonus(CUser* user) {
    return CalcCreatureBonus(user);
}

// IDA_EA: 0x85AD278
// IDA_NAME: _ZN6CParty22getClearRewardBonusExpEP5CUserRjRK27stClearRewardExpParameter_tR23stClearRewardBonusExp_t
// DEMANGLED: CParty::getClearRewardBonusExp(CUser *,unsigned int &,stClearRewardExpParameter_t const&,stClearRewardBonusExp_t &)
unsigned int CParty::getClearRewardBonusExp(
    CUser* user,
    unsigned int& base_exp,
    const stClearRewardExpParameter_t& parameter,
    stClearRewardBonusExp_t& bonus_exp) {
    const unsigned int original_base_exp = base_exp;
    uint32_t burning_fatigue_exp = 0;

    if (parameter.death_penalty_rate != 0.0f) {
        base_exp = ScaledExp(base_exp, 1.0f - parameter.death_penalty_rate);
    }

    bonus_exp.rank_bonus_exp = ScaledExp(base_exp, parameter.rank_bonus_rate);
    bonus_exp.avatar_bonus_exp = ScaledExp(base_exp, parameter.avatar_bonus_rate);
    if (parameter.avatar_bonus_rate != 0.0f) {
        bonus_exp.avatar_bonus_exp =
            std::max<uint32_t>(bonus_exp.avatar_bonus_exp, 1u);
    }

    bonus_exp.creature_bonus_exp =
        ScaledExp(base_exp, parameter.creature_bonus_rate);
    if (parameter.creature_bonus_rate != 0.0f) {
        bonus_exp.creature_bonus_exp =
            std::max<uint32_t>(bonus_exp.creature_bonus_exp, 1u);
    }

    bonus_exp.period_exp_item_bonus_exp =
        ScaledExp(base_exp, parameter.period_exp_item_rate);
    bonus_exp.same_pc_room_party_bonus_exp =
        ScaledExp(base_exp, parameter.same_pc_room_party_rate);
    bonus_exp.return_user_party_bonus_exp =
        ScaledExp(base_exp, parameter.return_user_party_rate);
    bonus_exp.new_character_party_bonus_exp =
        ScaledExp(base_exp, parameter.new_character_party_rate);

    if (parameter.pc_room_burning_rate == 0.0f) {
        if (parameter.double_exp_event_rate != 0.0f) {
            bonus_exp.event_bonus_exp =
                ScaledExp(base_exp, parameter.double_exp_event_rate);
            bonus_exp.event_bonus_type = 1;
        }
    } else {
        bonus_exp.event_bonus_exp =
            ScaledExp(base_exp, parameter.pc_room_burning_rate);
        bonus_exp.event_bonus_type = 2;
    }

    if (parameter.global_effect_rate != 0.0f) {
        burning_fatigue_exp = ScaledExp(base_exp, parameter.global_effect_rate);
        const uint32_t reward_kind = bonus_exp.event_bonus_type;
        if (reward_kind < 1 ||
            (reward_kind > 2 && reward_kind != 3) ||
            ((reward_kind == 1 || reward_kind == 2) &&
             bonus_exp.event_bonus_exp < burning_fatigue_exp)) {
            bonus_exp.event_bonus_exp = burning_fatigue_exp;
            bonus_exp.event_bonus_type = 4;
        }
    }

    bonus_exp.reserved_bonus_exp =
        ScaledExp(base_exp, parameter.reserved_bonus_rate);
    if (parameter.burning_fatigue_bonus_percent) {
        bonus_exp.burning_fatigue_bonus_exp =
            base_exp * parameter.burning_fatigue_bonus_percent / 100u;
        bonus_exp.event_bonus_type = 1;
    }
    bonus_exp.channel_bonus_exp =
        ScaledExp(base_exp, parameter.channel_bonus_rate);
    bonus_exp.server_event_bonus_exp =
        ScaledExp(base_exp, parameter.server_event_rate);
    bonus_exp.premium_advantage_bonus_exp =
        ScaledExp(base_exp, parameter.premium_advantage_rate);
    bonus_exp.character_manage_bonus_exp =
        ScaledExp(base_exp, parameter.character_manage_rate);
    bonus_exp.growth_power_bonus_exp =
        ScaledExp(base_exp, parameter.growth_power_rate);
    bonus_exp.premium_exp_bonus_exp =
        ScaledExp(base_exp, parameter.premium_exp_rate);

    uint32_t total_exp = bonus_exp.burning_fatigue_bonus_exp +
                         bonus_exp.server_event_bonus_exp +
                         bonus_exp.premium_advantage_bonus_exp +
                         bonus_exp.reserved_bonus_exp +
                         bonus_exp.event_bonus_exp +
                         bonus_exp.avatar_bonus_exp +
                         bonus_exp.rank_bonus_exp + base_exp +
                         bonus_exp.channel_bonus_exp;
    total_exp += bonus_exp.creature_bonus_exp;
    bonus_exp.upper_member_bonus_exp = user->gainExpAsUpperMember(base_exp);
    total_exp += bonus_exp.upper_member_bonus_exp;
    total_exp += user->gainGuildSkillExp(base_exp);
    total_exp += user->gainPowerWarRewardExp(static_cast<int>(base_exp));
    total_exp += bonus_exp.period_exp_item_bonus_exp +
                 bonus_exp.same_pc_room_party_bonus_exp +
                 bonus_exp.return_user_party_bonus_exp +
                 bonus_exp.new_character_party_bonus_exp +
                 bonus_exp.character_manage_bonus_exp +
                 bonus_exp.growth_power_bonus_exp +
                 bonus_exp.premium_exp_bonus_exp;

    if (total_exp > 0xF4240u) {
        LogManager::logFormat(
            1, "party.cpp",
            "unsigned int CParty::getClearRewardBonusExp(CUser*, unsigned int&, const stClearRewardExpParameter_t&, stClearRewardBonusExp_t&)",
            0x28B1, "BLOOD_LOG : EXP(%u), ExpPerUser(%u)\n", total_exp,
            original_base_exp);
        LogManager::logFormat(
            1, "party.cpp",
            "unsigned int CParty::getClearRewardBonusExp(CUser*, unsigned int&, const stClearRewardExpParameter_t&, stClearRewardBonusExp_t&)",
            0x28C2,
            "BLOOD_LOG : BONUS EXP PARAMETER : %f, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
            parameter.death_penalty_rate, bonus_exp.rank_bonus_exp,
            bonus_exp.avatar_bonus_exp, bonus_exp.creature_bonus_exp,
            bonus_exp.period_exp_item_bonus_exp,
            bonus_exp.same_pc_room_party_bonus_exp, bonus_exp.event_bonus_exp,
            burning_fatigue_exp, bonus_exp.reserved_bonus_exp,
            bonus_exp.burning_fatigue_bonus_exp, bonus_exp.channel_bonus_exp,
            bonus_exp.server_event_bonus_exp,
            bonus_exp.premium_advantage_bonus_exp,
            bonus_exp.upper_member_bonus_exp);
    }
    return total_exp;
}
