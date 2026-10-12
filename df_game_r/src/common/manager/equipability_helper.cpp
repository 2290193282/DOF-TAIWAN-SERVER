#include "old_game/economy/inventory/item.h"

#include <algorithm>
#include <stdint.h>

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x85003BE
// IDA_NAME: _Z11IsEquipableP5CUserPK5CItemi
// DEMANGLED: IsEquipable(CUser *,CItem const*,int)
int IsEquipable(CUser* user, const CItem* item, int slot) {
    const CItemVTable* const item_vtable = item->vtable_;
    if (item_vtable->is_creature_item(item) != true &&
        item_vtable->get_item_type(item) != slot) {
        return 17;
    }

    const int foot_control_rate_limit = item->getFootControlRateLimit();
    if (foot_control_rate_limit >= 0) {
        const unsigned int foot_control_rate =
            static_cast<unsigned int>(
                user->getCurCharacHelpAbuseComputedRatio()) /
            0x2710u;
        if (foot_control_rate >
            static_cast<unsigned int>(foot_control_rate_limit)) {
            return 17;
        }
    }

    const int equipment_type = item_vtable->get_item_type(item);
    int over_equipable_level = user->GetPremiumInfo()->GetOverEquipableLevel(
        static_cast<ENUM_EQUIPMENTTYPE>(equipment_type));
    const int level_gap = user->GetCurCharacMaxEquipLevel() -
                          user->get_charac_level();
    over_equipable_level = std::max(over_equipable_level, level_gap);

    if (item->check_job_type(user->get_charac_job()) != 1) {
        return 17;
    }

    const int charac_level = user->get_charac_level();
    const CEquipItem* const equipment_item =
        static_cast<const CEquipItem*>(item);
    if (!item->check_low_level(over_equipable_level + charac_level) ||
        (item->GetAttachType() == 5 &&
         equipment_item->GetUsableMaxLevel() < charac_level)) {
        if (G_GameWorld()->IsCharacterLevelRevisionChannel()) {
            return 7;
        }
        return 14;
    }

    const int need_skill = item->get_need_skill();
    if (need_skill != -1) {
        CSkill* const skill = G_CDataManager()->find_skill(
            user->get_charac_job(), need_skill);
        if (!skill) {
            return 17;
        }

        const int skill_tree_index = user->GetCurCharacSkillTreeIndex();
        if (user->getCurCharacSkillR()->get_skillslot_no(
                need_skill,
                skill->get_group(),
                static_cast<ENUM_SKILL_TREE_KIND>(skill_tree_index),
                true) < 0) {
            return 17;
        }
    }

    const int expert_job_level =
        user->GetCurExpertJobLevel(user->GetCurCharacExpertJobExp());
    if (item->isExpertJobUsable(
            static_cast<ENUM_EXPERT_JOB_TYPE>(
                user->GetCurCharacExpertJobType()),
            expert_job_level)) {
        return 0;
    }
    return 17;
}
