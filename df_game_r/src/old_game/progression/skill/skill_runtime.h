#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_RUNTIME_H_

#include "old_data/PVF/equipment/datamanager/equipment_script.h"

#include <stddef.h>
#include <list>
#include <stdint.h>
#include <utility>
#include <vector>

// The three words are passed by value by CSkill::add_learned_skill and are
// consumed by CSkillList when it builds the post-learning dependency table.
struct _pre_skill {
    int skill_index;
    int level;
    int group;
};

struct STSecondGrowTypeFitness {
    STSecondGrowTypeFitness();
    void clear();

    int fitness[2];
    int max_level[2];
};

class CUser;

void NormalizeAutomaticGiveSkillLevels(
    CUser* user,
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    std::vector<std::pair<int, int> >& skills);

// Reconciles only PVF-declared fixed-level first-awakening active skills in
// both ordinary skill trees.  This changes neither SP nor SFP; it is the
// online counterpart of NormalizeAutomaticGiveSkillLevels for character-level
// and premium-contract lifecycle changes.
bool ReconcileAutomaticGiveSkillLevels(CUser* user,
                                       bool send_skill_info);

class CHackLog_InvalidUsingSkill;

class CSkill {
public:
    CSkill();
    ~CSkill();

    bool IsSpecialSkill() const;
    int isCommandCustomizing() const;
    void add_learned_skill(_pre_skill skill);
    void add_need_sp(int need_sp);
    void add_need_sfp(int need_sfp);
    void set_prelearned_skill();
    int get_guildSkillNeedGSP(int level) const;
    int get_guildSkillNeedGold(int level) const;
    int get_needGuildLevel(int level) const;
    int get_need_sfp_for_level(int current_level, int target_level) const;
    int get_need_sp_sfp_for_level(int current_level, int target_level) const;
    int get_need_sp_for_level(int current_level, int target_level) const;
    bool can_learn(int character_level,
                   int first_grow_type,
                   int second_grow_type,
                   int skill_level) const;
    int IsStealable(int first_grow_type, int second_grow_type) const;
    CSkill* set_second_growtype_fitness(int first_grow_type,
                                        int second_grow_type,
                                        int fitness);
    int get_second_growtype_fitness(int first_grow_type,
                                    int second_grow_type) const;
    CSkill* set_second_growtype_max_level(int first_grow_type,
                                          int second_grow_type,
                                          int max_level);
    int get_second_growtype_max_level(int first_grow_type,
                                      int second_grow_type) const;
    CSkill* set_fitness_second_growtype(int second_grow_type, int fitness);
    int get_fitness_second_growtype(int second_grow_type) const;
    int getMaxSkillLevel(int character_level,
                         int first_grow_type,
                         int second_grow_type) const;
    int getFixedGiveLevel(int character_level,
                          int first_grow_type,
                          int second_grow_type,
                          int minimum_level,
                          int interval_level,
                          int add_level_per_interval) const;
    void reform_ui_group_no(int& group, bool special_skill, int group_count);
    CSkill* SetStealable(bool stealable);
    CSkill* SetStealLevelPenalty(int penalty);
    CSkill* SetStealSPPenalty(int penalty);
    CSkill* set_index(int index);
    int get_index() const;
    CSkill* set_group(int group);
    int get_group() const;
    CSkill* set_job_type(int job_type);
    CSkill* set_skill_type(char skill_type);
    CSkill* set_level_limit(int level_limit);
    CSkill* set_fitness(int first_grow_type, int fitness);
    CSkill* set_type_max_level(int first_grow_type, int max_level);
    int get_type_max_level(int first_grow_type) const;
    CSkill* set_fitGrowType(int slot, int grow_type);
    char* setSkillName(const char* name);
    int GetStealLevelPenalty() const;
    int GetStealSPPenalty() const;
    bool is_active_skill() const;
    int get_fitness(int first_grow_type) const;
    const char* getSkillName() const;

    // These two read-only views are used exclusively by SkillSlot's original
    // purchase/refund legality routines.  They retain CSkill as the physical
    // owner of its pre-skill list and PvP rank gate.
    const std::list<_pre_skill>& pre_skills() const { return learned_skills_; }
    int get_required_rank() const { return required_rank_; }
    const std::vector<int>& usable_passiveobject_ids() const {
        return usable_passiveobject_ids_;
    }
    bool ignores_active_status() const { return ignore_active_status_ != 0; }
    bool ignores_active_status_state(int state) const;

private:
    // CDataManager::Verify @0x8360D94 tests level_interval_ at +0xC8.
    friend class CDataManager;
    // IDA 0x864FB3A reads the guild-exp multiplier from level_values_[0].
    // The consuming original CUser member is granted typed access instead of
    // reproducing the old +0x108 byte expression.
    friend class CUser;
    // CSkillList::init @0x8350C26 is the owning construction path for these
    // fields.  It needs typed access to rebuild the original CSkill object
    // from STSkillScript without reviving the old byte-offset expressions.
    friend class CSkillList;
    // CMD41's original legality check reads the six canonical first-grow
    // ownership entries at CSkill + 0x0cc.  Keep that access inside the
    // named validator rather than recreating it with a raw offset.
    friend class CHackLog_InvalidUsingSkill;
    friend struct CSkillLayoutChecks;

    int index_;                                      // 0x000
    int job_type_;                                   // 0x004
    std::list<int> need_sp_;                         // 0x008
    std::list<int> need_sfp_;                        // 0x010
    std::list<_pre_skill> learned_skills_;           // 0x018
    char skill_type_;                                // 0x020
    unsigned char reserved_021_[3];
    int fitness_[6];                                 // 0x024
    int type_max_level_[6];                          // 0x03C
    STSecondGrowTypeFitness second_grow_fitness_[6]; // 0x054
    int second_growtype_fitness_[2];                 // 0x0B4
    int group_;                                      // 0x0BC
    int reserved_0c0_;
    int level_limit_;                                // 0x0C4
    int level_interval_;                             // 0x0C8
    int fit_grow_type_[6];                           // 0x0CC
    std::vector<int> guild_need_gsp_;                // 0x0E4
    std::vector<int> guild_need_gold_;               // 0x0F0
    int guild_need_level_base_;                      // 0x0FC
    int guild_need_level_step_;                      // 0x100
    int maximum_level_;                             // 0x104
    std::vector<std::vector<int> > level_values_;    // 0x108
    unsigned char stealable_;                        // 0x114
    unsigned char reserved_115_[3];
    int steal_sp_penalty_;                           // 0x118
    int steal_level_penalty_;                        // 0x11C
    unsigned char special_skill_;                    // 0x120
    unsigned char reserved_121_[3];
    std::vector<STKeyCommandUnit> key_commands_;     // 0x124
    unsigned char command_customizing_;              // 0x130
    char skill_name_[100];                           // 0x131
    unsigned char reserved_195_[3];
    int required_rank_;                             // 0x198
    // 0828 PVF prototype extensions.  These are immutable skill-definition
    // data copied by CSkillList::init; they are not character DB state.
    std::vector<int> usable_passiveobject_ids_;
    unsigned char ignore_active_status_;
    unsigned char reserved_1a9_[3];
    std::vector<int> ignored_active_status_states_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
struct CSkillLayoutChecks {
    static_assert(offsetof(CSkill, second_grow_fitness_) == 0x054u,
                  "CSkill per-grow second fitness must stay at +0x054");
    static_assert(offsetof(CSkill, second_growtype_fitness_) == 0x0b4u,
                  "CSkill global second-grow fitness must stay at +0x0b4");
};

static_assert(sizeof(_pre_skill) == 0x0Cu,
              "_pre_skill old ABI size must stay 0x0C");
static_assert(sizeof(STSecondGrowTypeFitness) == 0x10u,
              "STSecondGrowTypeFitness old ABI size must stay 0x10");
static_assert(sizeof(CSkill) == 0x1B8u,
              "CSkill 0828 PVF prototype size must stay 0x1B8");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_RUNTIME_H_
