#include "old_game/progression/skill/skill_changer_runtime.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8609D10
// IDA_NAME: _ZN8WongWork13CSkillChanger14CheckConditionEPK5CUser
// DEMANGLED: WongWork::CSkillChanger::CheckCondition(CUser const *)
bool WongWork::CSkillChanger::CheckCondition(const CUser* user) {
    const int level = user->get_charac_level();
    const int current_level_exp = G_CDataManager()->get_level_exp(level);
    const int next_level_exp = G_CDataManager()->get_level_exp(level + 1);
    const int character_exp = user->get_charac_exp();
    return character_exp >= current_level_exp && character_exp <= next_level_exp;
}
