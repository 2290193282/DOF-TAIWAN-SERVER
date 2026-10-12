#include "old_game/progression/quest/quest_runtime.h"

#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"

namespace {

// The original SkillSlot members recover their CUser owner from the first
// object word.  Keep that established ownership projection local instead of
// recreating any of the old CUserCharacInfo offsets at each caller.
CUser* ParentUser(const SkillSlot* slot) {
    return *reinterpret_cast<CUser* const*>(slot);
}

int GetMandatorySkillLevel(
    const std::vector<std::pair<int, int> >& mandatory_skills,
    int skill_index) {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             mandatory_skills.begin();
         it != mandatory_skills.end();
         ++it) {
        if (it->first == skill_index) {
            return it->second;
        }
    }
    return 0;
}

}  // namespace

// IDA_EA: 0x86055EC
// IDA_NAME: _ZNK9SkillSlot10calcUsedSPEPK15_Mastered_skillRiS3_
// DEMANGLED: SkillSlot::calcUsedSP(_Mastered_skill const*,int &,int &)const
void SkillSlot::calcUsedSP(const _Mastered_skill* skill_tree,
                           int& used_sp,
                           int& used_sfp) const {
    CUser* const user = ParentUser(this);

    // The original establishes these expert-job locals before calculating
    // costs.  In this binary revision its expert list pointer remains null,
    // so it never exempts a slot; preserve that observable setup without
    // inventing an unavailable ExpertJobScript ownership path.
    int expert_job_level = 0;
    int matched_expert_skill_count = 0;
    if (user->GetCurCharacExpertJobType()) {
        expert_job_level = user->GetCurExpertJobLevel(
            user->GetCurCharacExpertJobExp());
    }
    (void)expert_job_level;
    (void)matched_expert_skill_count;

    // 0x86055EC first combines initial skills with the current grow-type
    // grants, takes the maximum level per duplicate id, and replaces that
    // list with the PvP grants in a PvP skill-tree channel.  The restored
    // SkillSlot::get_give_skill is that same typed sequence.
    std::vector<std::pair<int, int> > mandatory_skills;
    if (const_cast<SkillSlot*>(this)->get_give_skill(
            user->get_charac_job(),
            user->getCurCharFirstGrowType(),
            user->getCurCharSecondGrowType(),
            mandatory_skills) != 1) {
        return;
    }

    for (int slot_index = 0; slot_index <= 203; ++slot_index) {
        const _Mastered_skill& mastered = skill_tree[slot_index];
        const int skill_index =
            static_cast<unsigned char>(mastered.skill_index);
        if (!skill_index) {
            continue;
        }

        CSkill* const skill =
            G_CDataManager()->find_skill(user->get_charac_job(), skill_index);
        if (!skill) {
            // The original stops this calculation at the first unresolved
            // slot; it does not fabricate a cost for later records.
            break;
        }

        bool stealable = false;
        int effective_character_level = user->get_charac_level();
        if (skill->IsStealable(user->getCurCharFirstGrowType(),
                               user->getCurCharSecondGrowType())) {
            stealable = true;
            effective_character_level -= skill->GetStealLevelPenalty();
        }
        effective_character_level +=
            user->GetPremiumInfo()->getOverSkillLevel();
        // This revision computes the effective level here but reaches its
        // cost branch through the mandatory-skill level only, exactly as the
        // 0x86055EC body does.
        (void)effective_character_level;

        const int mandatory_level =
            GetMandatorySkillLevel(mandatory_skills, skill_index);
        const int learned_level =
            static_cast<unsigned char>(mastered.skill_level);
        if (mandatory_level && learned_level <= mandatory_level) {
            continue;
        }

        int skill_used_sp = 0;
        int skill_used_sfp = 0;
        calcReturnSpSfpPerOneSkill(skill_used_sp,
                                   skill_used_sfp,
                                   mandatory_level,
                                   skill_tree,
                                   slot_index,
                                   skill,
                                   stealable);
        used_sp += skill_used_sp;
        used_sfp += skill_used_sfp;
    }
}
