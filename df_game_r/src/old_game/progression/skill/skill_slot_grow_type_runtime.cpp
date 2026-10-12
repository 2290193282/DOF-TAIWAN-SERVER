#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"

namespace {

const int kGrowTypeSkillSlotCount = 204;

bool CheckGrowTypeSkillTree(SkillSlot* skill_slot,
                            ENUM_SKILL_TREE_KIND tree_kind,
                            int character_level,
                            int character_job,
                            int first_grow_type,
                            int second_grow_type,
                            int log_line) {
    _Mastered_skill* const skill_tree =
        skill_slot->get_mastered_skill_tree(tree_kind);
    for (int slot = 0; slot < kGrowTypeSkillSlotCount; ++slot) {
        _Mastered_skill& mastered_skill = skill_tree[slot];
        const unsigned char skill_index =
            static_cast<unsigned char>(mastered_skill.skill_index);
        if (!skill_index) {
            continue;
        }

        CSkill* const skill =
            G_CDataManager()->find_skill(character_job, skill_index);
        if (!skill) {
            LogManager::logFormat(
                1,
                "skill_slot.cpp",
                "void SkillSlot::debugCheckGrowTypeSkill(int, int, int, int)",
                log_line,
                "G_CDataManager()->find_skill( %d, %d )",
                character_job,
                skill_index);
            return false;
        }

        if (!skill->can_learn(character_level,
                              first_grow_type,
                              second_grow_type,
                              mastered_skill.skill_level)) {
            mastered_skill.skill_index = 0;
            mastered_skill.skill_level = 0;
        }
    }
    return true;
}

}  // namespace

// IDA_EA: 0x86053C6
// IDA_NAME: _ZN9SkillSlot23debugCheckGrowTypeSkillEiiii
// DEMANGLED: SkillSlot::debugCheckGrowTypeSkill(int,int,int,int)
void SkillSlot::debugCheckGrowTypeSkill(int character_level,
                                        int character_job,
                                        int first_grow_type,
                                        int second_grow_type) {
    if (!CheckGrowTypeSkillTree(this,
                                ENUM_SKILL_TREE_0,
                                character_level,
                                character_job,
                                first_grow_type,
                                second_grow_type,
                                0x5a4)) {
        return;
    }
    CheckGrowTypeSkillTree(this,
                           ENUM_SKILL_TREE_1,
                           character_level,
                           character_job,
                           first_grow_type,
                           second_grow_type,
                           0x5b7);
}
