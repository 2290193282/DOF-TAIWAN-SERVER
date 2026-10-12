#include "old_game/progression/skill/skill_runtime.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/skill/skill_script.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <cstring>

namespace {

int ResolveGrowtypeMaximumLevel(const CSkill& skill,
                                int first_grow_type,
                                int second_grow_type) {
    if (first_grow_type < 0 || first_grow_type > 5 ||
        second_grow_type < 0 || second_grow_type > 2) {
        return -1;
    }
    if (second_grow_type > 0) {
        const int awakening_maximum =
            skill.get_second_growtype_max_level(first_grow_type,
                                                 second_grow_type);
        if (awakening_maximum > 0) return awakening_maximum;
    }
    return skill.get_type_max_level(first_grow_type);
}

}  // namespace

// IDA_EA: 0x83748DA
// IDA_NAME: _ZN23STSecondGrowTypeFitnessC2Ev
// DEMANGLED: STSecondGrowTypeFitness::STSecondGrowTypeFitness(void)
STSecondGrowTypeFitness::STSecondGrowTypeFitness() {
    clear();
}

// IDA_EA: 0x83748EE
// IDA_NAME: _ZN23STSecondGrowTypeFitness5clearEv
// DEMANGLED: STSecondGrowTypeFitness::clear(void)
void STSecondGrowTypeFitness::clear() {
    std::memset(fitness, 0, sizeof(fitness));
    std::memset(max_level, 0, sizeof(max_level));
}

// IDA_EA: 0x822B5B2
// IDA_NAME: _ZNK6CSkill14IsSpecialSkillEv
// DEMANGLED: CSkill::IsSpecialSkill(void)const
bool CSkill::IsSpecialSkill() const {
    return special_skill_ != 0;
}

// IDA_EA: 0x822B5D2
// IDA_NAME: _ZNK6CSkill20isCommandCustomizingEv
// DEMANGLED: CSkill::isCommandCustomizing(void)const
int CSkill::isCommandCustomizing() const {
    return command_customizing_;
}

// IDA_EA: 0x834FFCA
// IDA_NAME: _ZN6CSkill17add_learned_skillE10_pre_skill
// DEMANGLED: CSkill::add_learned_skill(_pre_skill)
void CSkill::add_learned_skill(_pre_skill skill) {
    learned_skills_.push_back(skill);
}

// IDA_EA: 0x834FFE8
// IDA_NAME: _ZN6CSkill11add_need_spEi
// DEMANGLED: CSkill::add_need_sp(int)
void CSkill::add_need_sp(int need_sp) {
    need_sp_.push_back(need_sp);
}

// IDA_EA: 0x8350006
// IDA_NAME: _ZN6CSkill12add_need_sfpEi
// DEMANGLED: CSkill::add_need_sfp(int)
void CSkill::add_need_sfp(int need_sfp) {
    need_sfp_.push_back(need_sfp);
}

// IDA_EA: 0x8350024
// IDA_NAME: _ZN6CSkill20set_prelearned_skillEv
// DEMANGLED: CSkill::set_prelearned_skill(void)
void CSkill::set_prelearned_skill() {
    for (std::list<_pre_skill>::iterator it = learned_skills_.begin();
         it != learned_skills_.end(); ++it) {
        CSkill* const prerequisite =
            G_CDataManager()->find_skill(job_type_, it->skill_index);
        if (prerequisite) {
            it->group = prerequisite->get_group();
        }
    }
}

// IDA_EA: 0x83500F2
// IDA_NAME: _ZNK6CSkill21get_guildSkillNeedGSPEi
// DEMANGLED: CSkill::get_guildSkillNeedGSP(int)const
int CSkill::get_guildSkillNeedGSP(int level) const {
    if (level > 0 && guild_need_gsp_.size() >=
                         static_cast<unsigned int>(level)) {
        return guild_need_gsp_.at(level - 1);
    }
    return -1;
}

// IDA_EA: 0x835013A
// IDA_NAME: _ZNK6CSkill22get_guildSkillNeedGoldEi
// DEMANGLED: CSkill::get_guildSkillNeedGold(int)const
int CSkill::get_guildSkillNeedGold(int level) const {
    if (level > 0 && guild_need_gold_.size() >=
                         static_cast<unsigned int>(level)) {
        return guild_need_gold_.at(level - 1);
    }
    return -1;
}

// IDA_EA: 0x8350182
// IDA_NAME: _ZNK6CSkill18get_needGuildLevelEi
// DEMANGLED: CSkill::get_needGuildLevel(int)const
int CSkill::get_needGuildLevel(int level) const {
    return guild_need_level_base_ + (level - 1) * guild_need_level_step_;
}

// IDA_EA: 0x83501A6
// IDA_NAME: _ZNK6CSkill22get_need_sfp_for_levelEii
// DEMANGLED: CSkill::get_need_sfp_for_level(int,int)const
int CSkill::get_need_sfp_for_level(int current_level, int target_level) const {
    if (static_cast<int>(need_sfp_.size()) < target_level ||
        current_level < 0) {
        return -1;
    }

    int accumulated = 0;
    std::list<int>::const_iterator it = need_sfp_.begin();
    for (int level = 1; level < target_level; ++level, ++it) {
        if (level > current_level && level < target_level) {
            accumulated += *it;
        }
    }
    return *it + accumulated;
}

// IDA_EA: 0x835027C
// IDA_NAME: _ZNK6CSkill25get_need_sp_sfp_for_levelEii
// DEMANGLED: CSkill::get_need_sp_sfp_for_level(int,int)const
int CSkill::get_need_sp_sfp_for_level(int current_level,
                                      int target_level) const {
    return IsSpecialSkill()
               ? get_need_sfp_for_level(current_level, target_level)
               : get_need_sp_for_level(current_level, target_level);
}

// IDA_EA: 0x83502C8
// IDA_NAME: _ZNK6CSkill21get_need_sp_for_levelEii
// DEMANGLED: CSkill::get_need_sp_for_level(int,int)const
int CSkill::get_need_sp_for_level(int current_level, int target_level) const {
    if (static_cast<int>(need_sp_.size()) < target_level ||
        current_level < 0) {
        return -1;
    }

    int accumulated = 0;
    std::list<int>::const_iterator it = need_sp_.begin();
    for (int level = 1; level < target_level; ++level, ++it) {
        if (level > current_level && level < target_level) {
            accumulated += *it;
        }
    }
    return *it + accumulated;
}

// IDA_EA: 0x835039E
// IDA_NAME: _ZNK6CSkill9can_learnEiiii
// DEMANGLED: CSkill::can_learn(int,int,int,int)const
bool CSkill::can_learn(int character_level,
                       int first_grow_type,
                       int second_grow_type,
                       int skill_level) const {
    const int max_level = ResolveGrowtypeMaximumLevel(
        *this, first_grow_type, second_grow_type);
    if (skill_level <= 0 || max_level < skill_level) return false;
    int level_by_character = character_level - level_limit_ + 1;
    if (level_by_character > max_level) level_by_character = max_level;
    return level_by_character >= skill_level &&
           level_limit_ + (skill_level - 1) * level_interval_ <=
               character_level;
}

// IDA_EA: 0x83504EC
// IDA_NAME: _ZNK6CSkill11IsStealableEii
// DEMANGLED: CSkill::IsStealable(int,int)const
int CSkill::IsStealable(int first_grow_type, int second_grow_type) const {
    if (stealable_ != 1) {
        return 0;
    }
    for (int index = 0; index <= 5; ++index) {
        if (fit_grow_type_[index] == first_grow_type) {
            return 0;
        }
    }
    if (second_grow_type > 0) {
        for (int index = 0; index <= 1; ++index) {
            if (second_growtype_fitness_[index] == second_grow_type) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x835057A
// IDA_NAME: _ZN6CSkill27set_second_growtype_fitnessEiii
// DEMANGLED: CSkill::set_second_growtype_fitness(int,int,int)
CSkill* CSkill::set_second_growtype_fitness(int first_grow_type,
                                            int second_grow_type,
                                            int fitness) {
    if (static_cast<unsigned int>(first_grow_type) <= 5u &&
        second_grow_type > 0 && second_grow_type <= 2) {
        second_grow_fitness_[first_grow_type]
            .fitness[second_grow_type - 1] = fitness;
    }
    return this;
}

// IDA_EA: 0x83505BE
// IDA_NAME: _ZNK6CSkill27get_second_growtype_fitnessEii
// DEMANGLED: CSkill::get_second_growtype_fitness(int,int)const
int CSkill::get_second_growtype_fitness(int first_grow_type,
                                        int second_grow_type) const {
    if (static_cast<unsigned int>(first_grow_type) > 6u ||
        static_cast<unsigned int>(second_grow_type) > 2u) {
        return -1;
    }
    if (!second_grow_type) {
        return 0;
    }
    if (first_grow_type <= 5) {
        return second_grow_fitness_[first_grow_type]
            .fitness[second_grow_type - 1];
    }

    // IDA 0x83505BE deliberately accepts grow type 6.  In the physical
    // CSkill layout its former indexed dword expression then reaches the
    // two global second-grow fitness entries at +0x0b4/+0x0b8.  Preserve
    // that boundary behavior through the named owner rather than treating
    // CSkill as an int array.
    return second_growtype_fitness_[second_grow_type - 1];
}

// IDA_EA: 0x8350614
// IDA_NAME: _ZN6CSkill29set_second_growtype_max_levelEiii
// DEMANGLED: CSkill::set_second_growtype_max_level(int,int,int)
CSkill* CSkill::set_second_growtype_max_level(int first_grow_type,
                                              int second_grow_type,
                                              int max_level) {
    if (static_cast<unsigned int>(first_grow_type) <= 5u &&
        second_grow_type > 0 && second_grow_type <= 2) {
        second_grow_fitness_[first_grow_type]
            .max_level[second_grow_type - 1] = max_level;
    }
    return this;
}

// IDA_EA: 0x8350658
// IDA_NAME: _ZNK6CSkill29get_second_growtype_max_levelEii
// DEMANGLED: CSkill::get_second_growtype_max_level(int,int)const
int CSkill::get_second_growtype_max_level(int first_grow_type,
                                          int second_grow_type) const {
    if (static_cast<unsigned int>(first_grow_type) <= 5u &&
        second_grow_type > 0 && second_grow_type <= 2) {
        return second_grow_fitness_[first_grow_type]
            .max_level[second_grow_type - 1];
    }
    return -1;
}

// IDA_EA: 0x83506AE
// IDA_NAME: _ZN6CSkill27set_fitness_second_growtypeEii
// DEMANGLED: CSkill::set_fitness_second_growtype(int,int)
CSkill* CSkill::set_fitness_second_growtype(int second_grow_type,
                                            int fitness) {
    if (second_grow_type > 0 && second_grow_type <= 2) {
        second_growtype_fitness_[second_grow_type - 1] = fitness;
    }
    return this;
}

// IDA_EA: 0x83506D8
// IDA_NAME: _ZNK6CSkill27get_fitness_second_growtypeEi
// DEMANGLED: CSkill::get_fitness_second_growtype(int)const
int CSkill::get_fitness_second_growtype(int second_grow_type) const {
    if (second_grow_type > 0 && second_grow_type <= 2) {
        return second_growtype_fitness_[second_grow_type - 1];
    }
    return -1;
}

// IDA_EA: 0x8350700
// IDA_NAME: _ZNK6CSkill16getMaxSkillLevelEiii
// DEMANGLED: CSkill::getMaxSkillLevel(int,int,int)const
int CSkill::getMaxSkillLevel(int character_level,
                             int first_grow_type,
                             int second_grow_type) const {
    const int max_level = ResolveGrowtypeMaximumLevel(
        *this, first_grow_type, second_grow_type);
    if (!max_level) {
        return -1;
    }

    int direct_level = character_level - level_limit_ + 1;
    if (direct_level > max_level) {
        direct_level = max_level;
    }
    const int interval_level =
        (character_level - level_limit_) / level_interval_ + 1;
    if (std::min(interval_level, direct_level) < 0) {
        return 0;
    }
    return interval_level >= direct_level ? direct_level : interval_level;
}

int CSkill::getFixedGiveLevel(int character_level,
                              int first_grow_type,
                              int second_grow_type,
                              int minimum_level,
                              int interval_level,
                              int add_level_per_interval) const {
    if (minimum_level <= 0 || interval_level <= 0 ||
        add_level_per_interval <= 0) return minimum_level;
    int maximum = second_grow_type > 0
                      ? get_second_growtype_max_level(first_grow_type,
                                                       second_grow_type)
                      : 0;
    if (maximum <= 0) maximum = get_type_max_level(first_grow_type);
    if (maximum <= minimum_level || character_level < level_limit_)
        return minimum_level;
    const int intervals = (character_level - level_limit_) / interval_level;
    const int fixed_level = minimum_level +
        intervals * add_level_per_interval;
    return fixed_level < maximum ? fixed_level : maximum;
}

void NormalizeAutomaticGiveSkillLevels(
    CUser* user,
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    std::vector<std::pair<int, int> >& skills) {
    if (!user) return;
    // Reuse the original premium projection: it is non-zero only while the
    // skill contract is active and carries the allowed over-level amount.
    const int over_skill_level = user->GetPremiumInfo()->getOverSkillLevel();
    CCharacList* const characters = G_CDataManager()->get_character_list();
    if (!characters || charac_job < 0 ||
        charac_job >= oldsrv::config::kCharacterJobCount) return;
    std::vector<std::pair<int, int> > first_awakening_skills;
    if (characters->characters_[charac_job].get_give_skill_secondGrowtype(
            first_grow_type, 1, first_awakening_skills) != 1) return;
    for (std::vector<std::pair<int, int> >::iterator it = skills.begin();
         it != skills.end(); ++it) {
        bool first_awakening = false;
        for (std::vector<std::pair<int, int> >::const_iterator awakening =
                 first_awakening_skills.begin();
             awakening != first_awakening_skills.end(); ++awakening) {
            if (awakening->first == it->first) {
                first_awakening = true;
                break;
            }
        }
        if (!first_awakening) continue;
        CSkill* const skill = G_CDataManager()->find_skill(charac_job,
                                                            it->first);
        FixedLevelSkillPolicy policy = {0, 0, false};
        if (!skill || !skill->is_active_skill() ||
            !GetFixedLevelSkillPolicy(charac_job, it->first, &policy))
            continue;
        const int effective_character_level = user->get_charac_level() +
            (policy.apply_contract_skillmaster && over_skill_level > 0
                 ? over_skill_level : 0);
        it->second = skill->getFixedGiveLevel(
            effective_character_level, first_grow_type, second_grow_type,
            it->second, policy.interval_level,
            policy.add_level_per_interval);
    }
}

bool ReconcileAutomaticGiveSkillLevels(CUser* user,
                                       bool send_skill_info) {
    if (!user || !user->getCurCharacR()) {
        return false;
    }

    const int charac_job = user->get_charac_job();
    const int first_grow_type = user->getCurCharFirstGrowType();
    const int second_grow_type = user->getCurCharSecondGrowType();
    if (charac_job < 0 || charac_job >= oldsrv::config::kCharacterJobCount ||
        second_grow_type < 1) {
        return false;
    }

    CCharacList* const characters = G_CDataManager()->get_character_list();
    SkillSlot* const skill_slot = user->getCurCharacSkillW();
    if (!characters || !skill_slot) return false;

    std::vector<std::pair<int, int> > first_awakening_skills;
    if (characters->characters_[charac_job].get_give_skill_secondGrowtype(
            first_grow_type, 1, first_awakening_skills) != 1) {
        return false;
    }

    const int over_skill_level =
        user->GetPremiumInfo()->getOverSkillLevel();
    bool changed = false;
    for (std::vector<std::pair<int, int> >::const_iterator it =
             first_awakening_skills.begin();
         it != first_awakening_skills.end(); ++it) {
        CSkill* const skill = G_CDataManager()->find_skill(charac_job,
                                                            it->first);
        FixedLevelSkillPolicy policy = {0, 0, false};
        if (!skill || !skill->is_active_skill() ||
            !GetFixedLevelSkillPolicy(charac_job, it->first, &policy)) {
            continue;
        }

        const int effective_character_level = user->get_charac_level() +
            (policy.apply_contract_skillmaster && over_skill_level > 0
                 ? over_skill_level : 0);
        const int target_level = skill->getFixedGiveLevel(
            effective_character_level, first_grow_type, second_grow_type,
            it->second, policy.interval_level,
            policy.add_level_per_interval);

        const ENUM_SKILL_TREE_KIND trees[2] = {
            ENUM_SKILL_TREE_0, ENUM_SKILL_TREE_1
        };
        for (size_t tree = 0u; tree < 2u; ++tree) {
            if (skill_slot->FindSkill(it->first, trees[tree]) == target_level) {
                continue;
            }
            if (skill_slot->growtype_skill(charac_job, it->first,
                                           target_level,
                                           trees[tree]) >= 0) {
                changed = true;
            }
        }
    }

    if (changed && send_skill_info && user->get_state() >= 3)
        user->send_skill_info();
    return changed;
}

// IDA_EA: 0x83507E8
// IDA_NAME: _ZN6CSkill18reform_ui_group_noERibi
// DEMANGLED: CSkill::reform_ui_group_no(int &,bool,int)
void CSkill::reform_ui_group_no(int& group,
                                bool special_skill,
                                int group_count) {
    if (special_skill) {
        group = 3;
    } else if (group >= 0 && group <= 3) {
        group = group_count <= 2;
    } else if (group == 4) {
        group = 2;
    }
}

// IDA_EA: 0x8374944
// IDA_NAME: _ZN6CSkillC2Ev
// DEMANGLED: CSkill::CSkill(void)
CSkill::CSkill() {
    std::memset(second_growtype_fitness_, 0,
                sizeof(second_growtype_fitness_));
    special_skill_ = 0;
    key_commands_.clear();
    command_customizing_ = 1;
    std::memset(skill_name_, 0, sizeof(skill_name_));
    for (int index = 0; index <= 5; ++index) {
        fit_grow_type_[index] = -1;
    }
    usable_passiveobject_ids_.clear();
    ignore_active_status_ = 0;
    std::memset(reserved_1a9_, 0, sizeof(reserved_1a9_));
    ignored_active_status_states_.clear();
}

// IDA_EA: 0x8374B2C
// IDA_NAME: _ZN6CSkill12SetStealableEb
// DEMANGLED: CSkill::SetStealable(bool)
CSkill* CSkill::SetStealable(bool stealable) {
    stealable_ = stealable;
    return this;
}

// IDA_EA: 0x8374B48
// IDA_NAME: _ZN6CSkill20SetStealLevelPenaltyEi
// DEMANGLED: CSkill::SetStealLevelPenalty(int)
CSkill* CSkill::SetStealLevelPenalty(int penalty) {
    steal_level_penalty_ = penalty;
    return this;
}

// IDA_EA: 0x8374B5A
// IDA_NAME: _ZN6CSkill17SetStealSPPenaltyEi
// DEMANGLED: CSkill::SetStealSPPenalty(int)
CSkill* CSkill::SetStealSPPenalty(int penalty) {
    steal_sp_penalty_ = penalty;
    return this;
}

// IDA_EA: 0x8374B6C
// IDA_NAME: _ZN6CSkill9set_indexEi
// DEMANGLED: CSkill::set_index(int)
CSkill* CSkill::set_index(int index) {
    index_ = index;
    return this;
}

// IDA_EA: 0x8374B7A
// IDA_NAME: _ZNK6CSkill9get_indexEv
// DEMANGLED: CSkill::get_index(void)const
int CSkill::get_index() const {
    return index_;
}

// IDA_EA: 0x8374B84
// IDA_NAME: _ZN6CSkill9set_groupEi
// DEMANGLED: CSkill::set_group(int)
CSkill* CSkill::set_group(int group) {
    group_ = group;
    return this;
}

// IDA_EA: 0x8374B96
// IDA_NAME: _ZNK6CSkill9get_groupEv
// DEMANGLED: CSkill::get_group(void)const
int CSkill::get_group() const {
    return group_;
}

// IDA_EA: 0x8374BA4
// IDA_NAME: _ZN6CSkill12set_job_typeEi
// DEMANGLED: CSkill::set_job_type(int)
CSkill* CSkill::set_job_type(int job_type) {
    job_type_ = job_type;
    return this;
}

// IDA_EA: 0x8374BB2
// IDA_NAME: _ZN6CSkill14set_skill_typeEc
// DEMANGLED: CSkill::set_skill_type(char)
CSkill* CSkill::set_skill_type(char skill_type) {
    skill_type_ = skill_type;
    return this;
}

// IDA_EA: 0x8374BCA
// IDA_NAME: _ZN6CSkill15set_level_limitEi
// DEMANGLED: CSkill::set_level_limit(int)
CSkill* CSkill::set_level_limit(int level_limit) {
    level_limit_ = level_limit;
    return this;
}

// IDA_EA: 0x8374BDC
// IDA_NAME: _ZN6CSkill11set_fitnessEii
// DEMANGLED: CSkill::set_fitness(int,int)
CSkill* CSkill::set_fitness(int first_grow_type, int fitness) {
    fitness_[first_grow_type] = fitness;
    return this;
}

// IDA_EA: 0x8374BF2
// IDA_NAME: _ZN6CSkill18set_type_max_levelEii
// DEMANGLED: CSkill::set_type_max_level(int,int)
CSkill* CSkill::set_type_max_level(int first_grow_type, int max_level) {
    type_max_level_[first_grow_type] = max_level;
    return this;
}

// IDA_EA: 0x8374C08
// IDA_NAME: _ZNK6CSkill18get_type_max_levelEi
// DEMANGLED: CSkill::get_type_max_level(int)const
int CSkill::get_type_max_level(int first_grow_type) const {
    return type_max_level_[first_grow_type];
}

// IDA_EA: 0x8374C1A
// IDA_NAME: _ZN6CSkill15set_fitGrowTypeEii
// DEMANGLED: CSkill::set_fitGrowType(int,int)
CSkill* CSkill::set_fitGrowType(int slot, int grow_type) {
    fit_grow_type_[slot] = grow_type;
    return this;
}

// IDA_EA: 0x8374C30
// IDA_NAME: _ZN6CSkill12setSkillNameEPKc
// DEMANGLED: CSkill::setSkillName(char const*)
char* CSkill::setSkillName(const char* name) {
    return std::strncpy(skill_name_, name, sizeof(skill_name_));
}

bool CSkill::ignores_active_status_state(int state) const {
    return std::find(ignored_active_status_states_.begin(),
                     ignored_active_status_states_.end(),
                     state) != ignored_active_status_states_.end();
}

// IDA_EA: 0x8379FD8
// IDA_NAME: _ZN6CSkillD2Ev
// DEMANGLED: CSkill::~CSkill()
CSkill::~CSkill() {}

// IDA_EA: 0x860934A
// IDA_NAME: _ZNK6CSkill20GetStealLevelPenaltyEv
// DEMANGLED: CSkill::GetStealLevelPenalty(void)const
int CSkill::GetStealLevelPenalty() const {
    return steal_level_penalty_;
}

// IDA_EA: 0x8609358
// IDA_NAME: _ZNK6CSkill17GetStealSPPenaltyEv
// DEMANGLED: CSkill::GetStealSPPenalty(void)const
int CSkill::GetStealSPPenalty() const {
    return steal_sp_penalty_;
}

// IDA_EA: 0x8609366
// IDA_NAME: _ZNK6CSkill15is_active_skillEv
// DEMANGLED: CSkill::is_active_skill(void)const
bool CSkill::is_active_skill() const {
    return skill_type_ == 0;
}

// IDA_EA: 0x8609378
// IDA_NAME: _ZNK6CSkill11get_fitnessEi
// DEMANGLED: CSkill::get_fitness(int)const
int CSkill::get_fitness(int first_grow_type) const {
    return fitness_[first_grow_type];
}

// IDA_EA: 0x860938A
// IDA_NAME: _ZNK6CSkill12getSkillNameEv
// DEMANGLED: CSkill::getSkillName(void)const
const char* CSkill::getSkillName() const {
    return skill_name_;
}
