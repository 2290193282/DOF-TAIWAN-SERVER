#include "old_game/progression/quest/quest_runtime.h"

#include <algorithm>
#include <vector>

#include "common/config.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"

namespace {

CUser* ParentUser(const SkillSlot* slot) {
    return *reinterpret_cast<CUser* const*>(slot);
}

ENUM_SKILL_TREE_KIND CurrentTree(const CUser* user) {
    return static_cast<ENUM_SKILL_TREE_KIND>(
        user->GetCurCharacSkillTreeIndex());
}

ENUM_SKILL_TREE_KIND CurrentSfpTree(const CUser* user) {
    const int tree = user->GetCurCharacSkillTreeIndex();
    return (tree == -1 || tree == 0) ? ENUM_SKILL_TREE_SFP_0
                                     : ENUM_SKILL_TREE_SFP_1;
}

int SkillCost(const CSkill* skill,
              int old_level,
              int new_level,
              int first_grow_type,
              int second_grow_type) {
    int cost = skill->get_need_sp_sfp_for_level(old_level, new_level);
    if (cost < 0) {
        return cost;
    }
    int fitness = skill->get_second_growtype_fitness(first_grow_type,
                                                       second_grow_type);
    if (!fitness) {
        fitness = skill->get_fitness(first_grow_type);
    }
    if (fitness > 0) {
        cost = cost * (100 - fitness) / 100;
    }
    if (skill->IsStealable(first_grow_type, second_grow_type)) {
        cost = getSkillSpendSPOnSteal(cost, skill->GetStealSPPenalty());
    }
    return cost < 0 ? 0 : cost;
}

void LogSkillLearnGate(const CUser* user,
                       const CSkill* skill,
                       int requested_level,
                       int target_level,
                       int effective_character_level,
                       int first_grow_type,
                       int second_grow_type,
                       const char* reason,
                       int prerequisite_skill,
                       int prerequisite_required,
                       int prerequisite_actual) {
    if (!user || !skill) return;
    LogManager::logFormat(
        1, "skill_buy_diag.cpp", "CanLearnSkill", 0,
        "[SKILL_BUY][GATE] skill=%d requested_level=%d target_level=%d "
        "char_level=%d effective_level=%d job=%d raw_grow=%d first_grow=%d "
        "second_grow=%d first_max=%d second_max=%d effective_max=%d "
        "prereq_skill=%d prereq_required=%d prereq_actual=%d reason=%s",
        skill->get_index(), requested_level, target_level,
        user->get_charac_level(), effective_character_level,
        user->get_charac_job(), user->getCurCharacGrowType(),
        first_grow_type, second_grow_type,
        skill->get_type_max_level(first_grow_type),
        second_grow_type > 0
            ? skill->get_second_growtype_max_level(first_grow_type,
                                                    second_grow_type)
            : 0,
        skill->getMaxSkillLevel(effective_character_level,
                                first_grow_type, second_grow_type),
        prerequisite_skill, prerequisite_required, prerequisite_actual,
        reason ? reason : "unknown");
}

bool CanLearnSkill(const SkillSlot* slot,
                   CUser* user,
                   const CSkill* skill,
                   int requested_level,
                   int target_level,
                   int charac_level,
                   int first_grow_type,
                   int second_grow_type,
                   _Mastered_skill* skill_tree) {
    if (skill->IsStealable(first_grow_type, second_grow_type)) {
        if (user->getCurCharacQuestR()->isClearQuest(0x163) != 1 &&
            user->is_clear_stealingSkillMission() != 1) {
            LogSkillLearnGate(user, skill, requested_level, target_level,
                              charac_level, first_grow_type, second_grow_type,
                              "steal_quest_missing", 0, 0, 0);
            return false;
        }
        charac_level -= skill->GetStealLevelPenalty();
    }

    const int premium_level = user->GetPremiumInfo()->getOverSkillLevel();
    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        if (skill->get_required_rank() > user->get_pvp_grade()) {
            LogSkillLearnGate(user, skill, requested_level, target_level,
                              oldsrv::config::kCharacterLevelRevision,
                              first_grow_type, second_grow_type,
                              "pvp_rank_rejected", 0, 0, 0);
            return false;
        }
        if (!skill->can_learn(oldsrv::config::kCharacterLevelRevision,
                              first_grow_type, second_grow_type,
                              target_level)) {
            LogSkillLearnGate(user, skill, requested_level, target_level,
                              oldsrv::config::kCharacterLevelRevision,
                              first_grow_type, second_grow_type,
                              "pvp_growth_or_level_rejected", 0, 0, 0);
            return false;
        }
    } else if (!skill->can_learn(charac_level + premium_level,
                                 first_grow_type,
                                 second_grow_type,
                                 target_level)) {
        LogSkillLearnGate(user, skill, requested_level, target_level,
                          charac_level + premium_level, first_grow_type,
                          second_grow_type, "growth_or_level_rejected",
                          0, 0, 0);
        return false;
    }

    for (std::list<_pre_skill>::const_iterator it = skill->pre_skills().begin();
         it != skill->pre_skills().end();
         ++it) {
        if (!slot->check_skill_mastered(skill_tree,
                                        it->skill_index,
                                        it->level,
                                        it->group)) {
            int actual_level = 0;
            for (int slot_index = 0; slot_index < 204; ++slot_index) {
                if (skill_tree[slot_index].skill_index == it->skill_index) {
                    actual_level = static_cast<unsigned char>(
                        skill_tree[slot_index].skill_level);
                    break;
                }
            }
            LogSkillLearnGate(user, skill, requested_level, target_level,
                              charac_level + premium_level, first_grow_type,
                              second_grow_type, "prerequisite_rejected",
                              it->skill_index, it->level, actual_level);
            return false;
        }
    }
    (void)requested_level;
    return true;
}

bool HasMandatorySkill(const std::vector<std::pair<int, int> >& skills,
                       int skill_index,
                       int level) {
    for (std::vector<std::pair<int, int> >::const_iterator it = skills.begin();
         it != skills.end();
         ++it) {
        if (it->first == skill_index && level < it->second) {
            return true;
        }
    }
    return false;
}

bool WouldBreakPostLearnedSkills(const SkillSlot* slot,
                                 _Mastered_skill* skill_tree,
                                 int charac_job,
                                 int skill_index,
                                 int remaining_level) {
    std::vector<std::pair<int, int> > post_skills;
    CSkillList* const list = G_CDataManager()->get_skill_list();
    if (!list) {
        return true;
    }
    list->get_post_learning_skill(charac_job, skill_index, post_skills);
    for (std::vector<std::pair<int, int> >::const_iterator it =
             post_skills.begin();
         it != post_skills.end();
         ++it) {
        for (int index = 0; index <= 203; ++index) {
            if (static_cast<unsigned char>(skill_tree[index].skill_index) ==
                    static_cast<unsigned char>(it->first) &&
                remaining_level < it->second) {
                return true;
            }
        }
    }
    return false;
}

bool IsExpertJobSkill(const CUser* user, int skill_index) {
    STExpertJobScript* const script = G_CDataManager()->GetExpertJobScript(
        user->GetCurCharacExpertJobType());
    if (!script) {
        return false;
    }
    for (std::vector<std::pair<int, int> >::const_iterator it =
             script->unknown_exp_pairs.begin();
         it != script->unknown_exp_pairs.end();
         ++it) {
        if (it->first == skill_index) {
            return true;
        }
    }
    return false;
}

}  // namespace

// IDA_EA: 0x86080A4
// IDA_NAME: _ZN9SkillSlot12insert_skillEP15_Mastered_skilliibi
int SkillSlot::insert_skill(_Mastered_skill* skill_tree,
                            int skill_index,
                            int group,
                            bool active,
                            int level) {
    if (!ParentUser(this) || !skill_tree) {
        return -1;
    }
    const int slot = get_skillslot_no(skill_tree, 0, group, active);
    if (slot >= 0) {
        skill_tree[slot].skill_index = static_cast<uint8_t>(skill_index);
        skill_tree[slot].skill_level = static_cast<int8_t>(level);
    }
    return slot;
}

// IDA_EA: 0x860714A
// IDA_NAME: _ZN9SkillSlot14get_give_skillEiiiRSt6vectorISt4pairIiiESaIS2_EE
int SkillSlot::get_give_skill(int charac_job,
                              int first_grow_type,
                              int second_grow_type,
                              std::vector<std::pair<int, int> >& skills) {
    CUser* const user = ParentUser(this);
    if (!user || charac_job < 0) {
        return -1;
    }
    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        skills.clear();
        return G_CDataManager()->get_pvp_skill_tree_parameter_script()
                       ->getGiveSkill(charac_job,
                                      first_grow_type,
                                      second_grow_type,
                                      skills)
                   ? 1
                   : -1;
    }

    skills = G_CDataManager()->get_initial_character_skills(charac_job);
    std::vector<std::pair<int, int> > grow_skills;
    CCharacList* const character_list = G_CDataManager()->get_character_list();
    if (!character_list ||
        character_list->characters_[charac_job].get_give_skill(
            first_grow_type,
            second_grow_type,
            grow_skills,
            second_grow_type ? 0u : 1u) != 1) {
        return -1;
    }
    NormalizeAutomaticGiveSkillLevels(user, charac_job, first_grow_type,
                                      second_grow_type,
                                      grow_skills);
    for (std::vector<std::pair<int, int> >::const_iterator it =
             grow_skills.begin();
         it != grow_skills.end();
         ++it) {
        bool merged = false;
        for (std::vector<std::pair<int, int> >::iterator target = skills.begin();
             target != skills.end();
             ++target) {
            if (target->first == it->first) {
                target->second = std::max(target->second, it->second);
                merged = true;
                break;
            }
        }
        if (!merged) {
            skills.push_back(*it);
        }
    }
    return 1;
}

// IDA_EA: 0x86037C6
// IDA_NAME: _ZN9SkillSlot22master_new_guild_skillEiiiiRK17STGuildDBInfoOnlyiRiS3_Rsi
int SkillSlot::master_new_guild_skill(int skill_index,
                                      int charac_job,
                                      int first_grow_type,
                                      int second_grow_type,
                                      const STGuildDBInfoOnly& guild_info,
                                      int charac_money,
                                      int& need_gold,
                                      int& learned_level,
                                      short& need_gsp,
                                      int guild_skill_point) {
    if (guild_info.skill_count > 15) {
        return -2;
    }
    CSkill* const skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return -1;
    }
    learned_level = 1;
    for (int index = 0; index < guild_info.skill_count; ++index) {
        if (guild_info.skills[index].skill_id == skill_index) {
            learned_level = guild_info.skills[index].level + 1;
            break;
        }
    }
    int maximum_level = second_grow_type > 0
                            ? skill->get_second_growtype_max_level(
                                  first_grow_type, second_grow_type)
                            : 0;
    if (!maximum_level) {
        maximum_level = skill->get_type_max_level(first_grow_type);
    }
    if (learned_level > maximum_level) {
        return -3;
    }
    need_gold = skill->get_guildSkillNeedGold(learned_level);
    if (need_gold < 0 || guild_info.guild_fund <
                             static_cast<unsigned int>(need_gold)) {
        return -5;
    }
    need_gsp = static_cast<short>(skill->get_guildSkillNeedGSP(learned_level));
    if (need_gsp < 0 || need_gsp > guild_skill_point) {
        return -2;
    }
    if (guild_info.guild_level < skill->get_needGuildLevel(learned_level)) {
        return -6;
    }
    CUser* const user = ParentUser(this);
    if (user) {
        CUser_history_log(user)->SkillAdd(
            user->get_charac_job(), user->GetCurCharacSkillTreeIndex(),
            skill_index, learned_level, static_cast<eSkillAddReason>(0));
        user->makeGuildSkillMessage(skill->getSkillName(), learned_level);
    }
    (void)charac_money;
    return 0;
}

// IDA_EA: 0x8603A74
// IDA_NAME: _ZN9SkillSlot16master_new_skillEiiiiii
int SkillSlot::master_new_skill(int skill_index,
                                int charac_level,
                                int charac_job,
                                int first_grow_type,
                                int second_grow_type,
                                int requested_level) {
    CUser* const user = ParentUser(this);
    CSkill* const skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return -1;
    }
    if (!user) {
        return -4;
    }

    const ENUM_SKILL_TREE_KIND tree = CurrentTree(user);
    _Mastered_skill* const slots = get_mastered_skill_tree(tree);
    const int slot = get_skillslot_no(skill_index, skill->get_group(), tree, true);
    const int old_level = slot < 0 ? 0 : slots[slot].skill_level;
    const int target_level = old_level + requested_level;
    if (!CanLearnSkill(this, user, skill, requested_level, target_level,
                       charac_level, first_grow_type, second_grow_type, slots)) {
        return -3;
    }

    const int cost = SkillCost(skill, old_level, target_level,
                               first_grow_type, second_grow_type);
    if (cost < 0) {
        return -4;
    }
    ENUM_SKILL_TREE_KIND point_tree = tree;
    if (skill->IsSpecialSkill()) {
        point_tree = CurrentSfpTree(user);
    }
    int* const remaining = get_cur_remain_sp(point_tree);
    if (!remaining) {
        return -1;
    }
    if (*remaining < cost) {
        return -2;
    }
    *remaining -= cost;
    const int result = slot < 0
                           ? insert_skill(skill_index, skill->get_group(),
                                          skill->is_active_skill(), tree,
                                          requested_level)
                           : slot;
    if (slot >= 0) {
        slots[slot].skill_level = static_cast<int8_t>(target_level);
    }
    if (result < 0) {
        return result;
    }
    CUser_history_log(user)->SkillAdd(user->get_charac_job(),
                                      user->GetCurCharacSkillTreeIndex(),
                                      skill_index, target_level,
                                      static_cast<eSkillAddReason>(0));
    if (skill->IsSpecialSkill()) {
        CUser_history_log(user)->SFPSub(static_cast<int>(point_tree),
                                        *remaining, cost,
                                        static_cast<eSPSubReason>(0));
    } else {
        CUser_history_log(user)->SPSub(static_cast<int>(tree), *remaining,
                                       cost, static_cast<eSPSubReason>(0));
    }
    return result;
}

// IDA_EA: 0x8607460
// IDA_NAME: _ZN9SkillSlot21try_master_new_skillsEiiiiii
int SkillSlot::try_master_new_skills(int skill_index,
                                     int charac_level,
                                     int charac_job,
                                     int first_grow_type,
                                     int second_grow_type,
                                     int requested_level) {
    CUser* const user = ParentUser(this);
    CSkill* const skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return -1;
    }
    if (!user) {
        return -4;
    }
    _Mastered_skill* const slots = get_temp_skill_slot();
    if (!slots) {
        return -4;
    }
    const int slot = get_skillslot_no(slots, skill_index, skill->get_group(), true);
    const int old_level = slot < 0 ? 0 : slots[slot].skill_level;
    const int target_level = old_level + requested_level;
    if (!CanLearnSkill(this, user, skill, requested_level, target_level,
                       charac_level, first_grow_type, second_grow_type, slots)) {
        return -3;
    }
    const int cost = SkillCost(skill, old_level, target_level,
                               first_grow_type, second_grow_type);
    if (cost < 0) {
        return -4;
    }
    int* const remaining = skill->IsSpecialSkill()
                               ? get_temp_remain_sfp()
                               : get_temp_remain_sp();
    if (!remaining) {
        return -1;
    }
    if (*remaining < cost) {
        return -2;
    }
    *remaining -= cost;
    if (slot >= 0) {
        slots[slot].skill_level = static_cast<int8_t>(target_level);
        return slot;
    }
    return insert_skill(slots, skill_index, skill->get_group(),
                        skill->is_active_skill(), requested_level);
}

// IDA_EA: 0x86069BA
// IDA_NAME: _ZN9SkillSlot12refund_skillEiiiiiiRi
// DEMANGLED: SkillSlot::refund_skill(int,int,int,int,int,int,int &)
int SkillSlot::refund_skill(int skill_index,
                            int charac_level,
                            int charac_job,
                            int first_grow_type,
                            int second_grow_type,
                            int requested_level,
                            int& reason) {
    CUser* const user = ParentUser(this);
    reason = 19;
    if (!user) {
        return -4;
    }
    CSkill* const skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return -1;
    }
    if (skill->get_group() == 4) {
        reason = 7;
        return -5;
    }
    if (G_GameWorld()->IsPvPSkilTreeChannel() && user->get_pvp_grade() <= 9) {
        reason = 7;
        return -3;
    }
    const ENUM_SKILL_TREE_KIND tree = CurrentTree(user);
    _Mastered_skill* const slots = get_mastered_skill_tree(tree);
    const int slot = get_skillslot_no(skill_index, skill->get_group(), tree, true);
    if (slot < 0) {
        return -1;
    }
    const int remaining_level = slots[slot].skill_level - requested_level;
    if (remaining_level < 0) {
        return -1;
    }
    if (IsExpertJobSkill(user, skill_index)) {
        reason = 7;
        return -5;
    }
    std::vector<std::pair<int, int> > give_skills;
    if (get_give_skill(charac_job, first_grow_type, second_grow_type,
                       give_skills) < 0) {
        return -1;
    }
    if (HasMandatorySkill(give_skills, skill_index, remaining_level)) {
        reason = 7;
        return -1;
    }
    if (WouldBreakPostLearnedSkills(this, slots, charac_job, skill_index,
                                    remaining_level)) {
        reason = 20;
        return -1;
    }
    const int refund = SkillCost(skill, remaining_level, slots[slot].skill_level,
                                 first_grow_type, second_grow_type);
    slots[slot].skill_level = static_cast<int8_t>(remaining_level);
    if (!slots[slot].skill_level) {
        slots[slot].skill_index = 0;
    }
    const ENUM_SKILL_TREE_KIND point_tree = skill->IsSpecialSkill()
                                                ? CurrentSfpTree(user)
                                                : tree;
    int* const remaining = get_cur_remain_sp(point_tree);
    if (!remaining) {
        return -1;
    }
    *remaining += refund;
    CUser_history_log(user)->SkillDel(user->get_charac_job(),
                                      user->GetCurCharacSkillTreeIndex(),
                                      skill_index, requested_level,
                                      static_cast<eSkillDelReason>(1));
    if (skill->IsSpecialSkill()) {
        CUser_history_log(user)->SFPAdd(static_cast<int>(point_tree),
                                        *remaining, refund,
                                        static_cast<eSPAddReason>(2));
    } else {
        CUser_history_log(user)->SPAdd(static_cast<int>(tree), *remaining,
                                       refund, static_cast<eSPAddReason>(2));
    }
    (void)charac_level;
    reason = 0;
    return slot;
}

// IDA_EA: 0x86078F4
// IDA_NAME: _ZN9SkillSlot16try_refund_skillEiiiiiiRi
int SkillSlot::try_refund_skill(int skill_index,
                                int charac_level,
                                int charac_job,
                                int first_grow_type,
                                int second_grow_type,
                                int requested_level,
                                int& reason) {
    CUser* const user = ParentUser(this);
    reason = 19;
    if (!user) {
        return -4;
    }
    CSkill* const skill = G_CDataManager()->find_skill(charac_job, skill_index);
    if (!skill) {
        return -1;
    }
    if (skill->get_group() == 4) {
        reason = 7;
        return -5;
    }
    if (G_GameWorld()->IsPvPSkilTreeChannel() && user->get_pvp_grade() <= 9) {
        reason = 7;
        return -3;
    }
    _Mastered_skill* const slots = get_temp_skill_slot();
    if (!slots) {
        return -4;
    }
    const int slot = get_skillslot_no(slots, skill_index, skill->get_group(), true);
    if (slot < 0) {
        return -1;
    }
    const int remaining_level = slots[slot].skill_level - requested_level;
    if (remaining_level < 0) {
        return -1;
    }
    if (IsExpertJobSkill(user, skill_index)) {
        reason = 7;
        return -5;
    }
    std::vector<std::pair<int, int> > give_skills;
    if (get_give_skill(charac_job, first_grow_type, second_grow_type,
                       give_skills) < 0) {
        return -1;
    }
    if (HasMandatorySkill(give_skills, skill_index, remaining_level)) {
        reason = 7;
        return -1;
    }
    if (WouldBreakPostLearnedSkills(this, slots, charac_job, skill_index,
                                    remaining_level)) {
        reason = 20;
        return -1;
    }
    const int refund = SkillCost(skill, remaining_level, slots[slot].skill_level,
                                 first_grow_type, second_grow_type);
    slots[slot].skill_level = static_cast<int8_t>(remaining_level);
    if (!slots[slot].skill_level) {
        slots[slot].skill_index = 0;
    }
    int* const remaining = skill->IsSpecialSkill()
                               ? get_temp_remain_sfp()
                               : get_temp_remain_sp();
    if (!remaining) {
        return -1;
    }
    *remaining += refund;
    (void)charac_level;
    reason = 0;
    return slot;
}
