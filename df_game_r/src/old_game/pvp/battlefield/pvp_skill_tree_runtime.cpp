#include "old_game/pvp/battlefield/pvp_skill_tree_runtime.h"

#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84BFF8E
// IDA_NAME: _Z15SetPvPSkillTreeP5CUserPK15_Mastered_skill20ENUM_SKILL_TREE_KIND
// DEMANGLED: SetPvPSkillTree(
//     CUser *,_Mastered_skill const*,ENUM_SKILL_TREE_KIND)
int SetPvPSkillTree(CUser* user,
                    const _Mastered_skill* skill_tree,
                    ENUM_SKILL_TREE_KIND skill_tree_kind) {
    int used_sp = 0;
    int used_sfp = 0;
    SkillSlot* const current_skill_slot = user->getCurCharacSkillR();
    current_skill_slot->calcUsedSP(skill_tree, used_sp, used_sfp);

    int pvp_grade = user->get_pvp_grade();
    int second_grow_type = user->getCurCharSecondGrowType();
    int first_grow_type = user->getCurCharFirstGrowType();
    int character_job = user->get_charac_job();
    PvPSkillTreeParameterScript* const pvp_skill_tree_script =
        G_CDataManager()->get_pvp_skill_tree_parameter_script();
    const int total_pvp_skill_point = pvp_skill_tree_script->getPvPSkillPoint(
        character_job,
        first_grow_type,
        second_grow_type,
        pvp_grade,
        false);

    SkillSlot* const remain_skill_slot = user->getCurCharacSkillR();
    const int remaining_sp =
        remain_skill_slot->get_remain_sp_at_index(skill_tree_kind);
    const int point_delta = total_pvp_skill_point - (remaining_sp + used_sp);

    int target_tree = 0;
    if (skill_tree_kind != ENUM_SKILL_TREE_CURRENT &&
        skill_tree_kind != ENUM_SKILL_TREE_0) {
        target_tree = 2;
    } else {
        target_tree = 1;
    }

    if (point_delta <= 0) {
        if (point_delta >= 0) {
            return user->givePvPSkillTree(0, false, target_tree);
        }

        SkillSlot* const writable_skill_slot = user->getCurCharacSkillW();
        writable_skill_slot->clear_all_skills(skill_tree_kind);
        SkillSlot* const parent_skill_slot = user->getCurCharacSkillW();
        parent_skill_slot->set_parent(user);

        pvp_grade = user->get_pvp_grade();
        second_grow_type = user->getCurCharSecondGrowType();
        first_grow_type = user->getCurCharFirstGrowType();
        character_job = user->get_charac_job();
        PvPSkillTreeParameterScript* const reset_script =
            G_CDataManager()->get_pvp_skill_tree_parameter_script();
        if (reset_script->getGivePvPSkillPoint(character_job,
                                               first_grow_type,
                                               second_grow_type,
                                               pvp_grade)) {
            SkillSlot* const reset_remain_skill_slot =
                user->getCurCharacSkillW();
            reset_remain_skill_slot->set_remain_sp_at_index(
                total_pvp_skill_point, skill_tree_kind);
            return user->givePvPSkillTree(0, false, target_tree);
        }
    } else {
        pvp_grade = user->get_pvp_grade();
        second_grow_type = user->getCurCharSecondGrowType();
        first_grow_type = user->getCurCharFirstGrowType();
        character_job = user->get_charac_job();
        PvPSkillTreeParameterScript* const grow_script =
            G_CDataManager()->get_pvp_skill_tree_parameter_script();
        if (grow_script->getGivePvPSkillPoint(character_job,
                                              first_grow_type,
                                              second_grow_type,
                                              pvp_grade)) {
            const int updated_remaining_sp = remaining_sp + point_delta;
            SkillSlot* const writable_skill_slot = user->getCurCharacSkillW();
            writable_skill_slot->set_remain_sp_at_index(
                updated_remaining_sp, skill_tree_kind);
            return user->givePvPSkillTree(0, false, target_tree);
        }
    }

    user->givePvPSkillTree(0, false, target_tree);
    return user->givePvPSkillTree(0, true, target_tree);
}
