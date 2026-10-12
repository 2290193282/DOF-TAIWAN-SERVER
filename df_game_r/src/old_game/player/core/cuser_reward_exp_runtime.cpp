#include "old_game/player/core/cuser.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/progression/skill/skill_runtime.h"

// IDA_EA: 0x864FA08
// IDA_NAME: _ZN5CUser20gainExpAsUpperMemberEi
// DEMANGLED: CUser::gainExpAsUpperMember(int)
int CUser::gainExpAsUpperMember(int base_exp) {
    if (!get_charac_memberkey()) {
        return 0;
    }

    if (!is_connect_upper_member()) {
        set_charac_member_bonus_exp(0);
        return 0;
    }

    const float rate =
        static_cast<float>(static_cast<uint8_t>(get_uppermember_exp_level())) *
        0.0099999998f;
    const int bonus_exp = static_cast<int>(static_cast<float>(base_exp) * rate);
    set_charac_member_bonus_exp(bonus_exp > 0 ? bonus_exp : 0);
    return bonus_exp;
}

// IDA_EA: 0x864FB3A
// IDA_NAME: _ZN5CUser17gainGuildSkillExpEi
// DEMANGLED: CUser::gainGuildSkillExp(int)
int CUser::gainGuildSkillExp(int base_exp) {
    if (!get_charac_guildkey()) {
        return 0;
    }
    if (!getGuildMemberGrade()) {
        return 0;
    }

    const int guild_skill_level = IsGuildSkillLearn(201);
    if (!guild_skill_level) {
        return 0;
    }

    CSkill* const guild_skill =
        G_CDataManager()->find_skill(get_charac_job(), 201);
    const int exp_rate = guild_skill->level_values_[0].at(
        static_cast<size_t>(guild_skill_level - 1));
    return exp_rate * base_exp / 100;
}

// IDA_EA: 0x866AD92
// IDA_NAME: _ZN5CUser8lost_extEiRib
// DEMANGLED: CUser::lost_ext(int,int &,bool)
int CUser::lost_ext(int percentage,
                    int& lost_experience,
                    bool force_level_down) {
    unsigned char level_dropped = 0;
    const int current_level = get_charac_level();
    const int current_level_exp =
        G_CDataManager()->get_level_exp(current_level);
    const int next_level_exp =
        G_CDataManager()->get_level_exp(get_charac_level() + 1);
    const int calculated_loss =
        percentage * (next_level_exp - current_level_exp) / 100;

    if (force_level_down) {
        lost_experience = calculated_loss;
        subtractCurCharacExp(calculated_loss);
        if (get_charac_exp() <
            G_CDataManager()->get_level_exp(get_charac_level())) {
            decCurCharacLevel();
            _update_charac_stat(false);
            level_dropped = 1;
        }
    } else if (get_charac_exp() - calculated_loss > current_level_exp) {
        lost_experience = calculated_loss;
        subtractCurCharacExp(calculated_loss);
    } else {
        lost_experience = get_charac_exp() - current_level_exp;
        setCurCharacExp(current_level_exp);
    }

    SendNotiPacket(static_cast<eSendTarget>(1),
                   static_cast<ENUM_NOTIPACKET>(37),
                   0);
    return level_dropped;
}
