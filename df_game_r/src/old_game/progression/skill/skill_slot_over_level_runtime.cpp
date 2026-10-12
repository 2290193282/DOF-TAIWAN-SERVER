#include "old_game/progression/quest/quest_runtime.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/progression/skill/skill_tree_param_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"

namespace {

// SkillSlot's first word is the owning CUser pointer.  This is the existing
// owner projection used by restored SkillSlot members; it avoids reviving the
// old callers' scattered slot-buffer offsets.
CUser* ParentUser(const SkillSlot* slot) {
    return *reinterpret_cast<CUser* const*>(slot);
}

bool IsFirstSkillTree(ENUM_SKILL_TREE_KIND kind) {
    return kind == ENUM_SKILL_TREE_CURRENT || kind == ENUM_SKILL_TREE_0;
}

}  // namespace

// IDA_EA: 0x860658E
// IDA_NAME: _ZNK9SkillSlot26calcReturnSpSfpPerOneSkillERiS0_iPK15_Mastered_skilliPK6CSkillb
// DEMANGLED: SkillSlot::calcReturnSpSfpPerOneSkill(
//     int &,int &,int,_Mastered_skill const*,int,CSkill const*,bool)const
char SkillSlot::calcReturnSpSfpPerOneSkill(
    int& returned_sp,
    int& returned_sfp,
    int maximum_level,
    const _Mastered_skill* skill_tree,
    int slot_index,
    const CSkill* skill,
    bool stealable) const {
    int spent = 0;
    const int learned_level =
        static_cast<unsigned char>(skill_tree[slot_index].skill_level);

    for (int target_level = maximum_level + 1;
         learned_level >= target_level;
         ++target_level) {
        const CUser* const user = ParentUser(this);
        int fitness = skill->get_second_growtype_fitness(
            user->getCurCharFirstGrowType(),
            user->getCurCharSecondGrowType());
        if (!fitness) {
            fitness = skill->get_fitness(user->getCurCharFirstGrowType());
        }

        spent = skill->get_need_sp_sfp_for_level(target_level - 1,
                                                   target_level);
        if (spent < 0) {
            break;
        }
        if (fitness > 0) {
            spent = spent * (100 - fitness) / 100;
        }
        if (stealable) {
            spent = getSkillSpendSPOnSteal(spent,
                                            skill->GetStealSPPenalty());
        }
        if (spent < 0) {
            break;
        }
        if (skill->IsSpecialSkill()) {
            returned_sfp += spent;
        } else {
            returned_sp += spent;
        }
    }
    return static_cast<char>(spent);
}

// IDA_EA: 0x8605D2A
// IDA_NAME: _ZN9SkillSlot19checkOverLevelSkillERSt6vectorI16stSPReturnInfo_tSaIS1_EE20ENUM_SKILL_TREE_KINDi
// DEMANGLED: SkillSlot::checkOverLevelSkill(
//     std::vector<stSPReturnInfo_t,std::allocator<stSPReturnInfo_t> > &,
//     ENUM_SKILL_TREE_KIND,int)
int SkillSlot::checkOverLevelSkill(
    std::vector<stSPReturnInfo_t>& return_info,
    ENUM_SKILL_TREE_KIND kind,
    int charac_level) {
    CUser* const user = ParentUser(this);
    if (!user || !user->getCurCharacR()) {
        return 0;
    }

    _Mastered_skill* const skill_tree = get_mastered_skill_tree(kind);
    int* const sfp_pool = IsFirstSkillTree(kind)
                              ? get_cur_remain_sp(ENUM_SKILL_TREE_SFP_0)
                              : kind == ENUM_SKILL_TREE_1
                                    ? get_cur_remain_sp(ENUM_SKILL_TREE_SFP_1)
                                    : 0;
    int* const sp_pool = get_cur_remain_sp(kind);
    if (!skill_tree || !sp_pool || !sfp_pool) {
        return 0;
    }

    const int expert_job_type = user->GetCurCharacExpertJobType();
    STExpertJobScript* const expert_job_script =
        G_CDataManager()->GetExpertJobScript(expert_job_type);
    int expert_job_level = 0;
    int matched_expert_skill_count = 0;
    if (expert_job_type) {
        expert_job_level = user->GetCurExpertJobLevel(
            user->GetCurCharacExpertJobExp());
    }

    int changed = 0;
    for (int slot_index = 0; slot_index <= 203; ++slot_index) {
        _Mastered_skill& mastered = skill_tree[slot_index];
        const int skill_index =
            static_cast<unsigned char>(mastered.skill_index);
        if (!skill_index) {
            continue;
        }

        CSkill* const skill =
            G_CDataManager()->find_skill(user->get_charac_job(), skill_index);
        if (!skill) {
            return 0;
        }

        bool stealable = false;
        int effective_level = charac_level;
        if (skill->IsStealable(user->getCurCharFirstGrowType(),
                               user->getCurCharSecondGrowType())) {
            stealable = true;
            effective_level -= skill->GetStealLevelPenalty();
        }
        effective_level += user->GetPremiumInfo()->getOverSkillLevel();

        int maximum_level = skill->getMaxSkillLevel(
            effective_level,
            user->getCurCharFirstGrowType(),
            user->getCurCharSecondGrowType());
        if (maximum_level == -1) {
            mastered.skill_index = 0;
            mastered.skill_level = 0;
            continue;
        }

        bool expert_skill = false;
        if (expert_job_script) {
            for (std::vector<std::pair<int, int> >::const_iterator iter =
                     expert_job_script->unknown_exp_pairs.begin();
                 iter != expert_job_script->unknown_exp_pairs.end();
                 ++iter) {
                if (iter->first == skill_index) {
                    if (static_cast<unsigned char>(mastered.skill_level) ==
                        expert_job_level) {
                        ++matched_expert_skill_count;
                    }
                    expert_skill = true;
                    break;
                }
            }
        }

        if (!expert_skill) {
            const std::list<_pre_skill>& prerequisites = skill->pre_skills();
            for (std::list<_pre_skill>::const_iterator iter =
                     prerequisites.begin();
                 iter != prerequisites.end();
                 ++iter) {
                if (!check_skill_mastered(iter->skill_index,
                                          iter->level,
                                          iter->group,
                                          kind)) {
                    maximum_level = 0;
                    break;
                }
            }
        }

        const int learned_level =
            static_cast<unsigned char>(mastered.skill_level);
        if (learned_level <= maximum_level) {
            continue;
        }

        int mandatory_level = 0;
        std::vector<std::pair<int, int> > give_skills;
        if (get_give_skill(user->get_charac_job(),
                           user->getCurCharFirstGrowType(),
                           user->getCurCharSecondGrowType(),
                           give_skills) == 1) {
            for (std::vector<std::pair<int, int> >::const_iterator iter =
                     give_skills.begin();
                 iter != give_skills.end();
                 ++iter) {
                if (iter->first == skill_index) {
                    mandatory_level = iter->second;
                    break;
                }
            }
        }
        maximum_level = std::max(maximum_level, mandatory_level);
        if (learned_level <= maximum_level) {
            continue;
        }

        int returned_sp = 0;
        int returned_sfp = 0;
        calcReturnSpSfpPerOneSkill(returned_sp,
                                   returned_sfp,
                                   maximum_level,
                                   skill_tree,
                                   slot_index,
                                   skill,
                                   stealable);
        return_info.push_back(stSPReturnInfo_t(
            static_cast<uint16_t>(skill_index),
            static_cast<uint16_t>(learned_level - maximum_level),
            static_cast<uint32_t>(returned_sp),
            static_cast<uint32_t>(returned_sfp)));
        *sfp_pool += returned_sfp;
        *sp_pool += returned_sp;
        mastered.skill_level = static_cast<int8_t>(maximum_level);
        if (!mastered.skill_level) {
            mastered.skill_index = 0;
        }
        changed = 1;
    }

    if (expert_job_script &&
        expert_job_script->unknown_exp_pairs.size() !=
            static_cast<size_t>(matched_expert_skill_count)) {
        for (std::vector<std::pair<int, int> >::const_iterator iter =
                 expert_job_script->unknown_exp_pairs.begin();
             iter != expert_job_script->unknown_exp_pairs.end();
             ++iter) {
            growtype_skill(user->get_charac_job(),
                           iter->first,
                           expert_job_level,
                           kind);
            changed = 1;
        }
    }
    return changed;
}
