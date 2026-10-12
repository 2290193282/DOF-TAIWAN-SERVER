#ifndef OLDSRV_GLOBAL_PVP_SKILL_TREE_RUNTIME_H_
#define OLDSRV_GLOBAL_PVP_SKILL_TREE_RUNTIME_H_

#include "old_game/progression/skill/combo_skill.h"

class CUser;
struct _Mastered_skill;

// IDA_EA: 0x84BFF8E
// IDA_NAME: _Z15SetPvPSkillTreeP5CUserPK15_Mastered_skill20ENUM_SKILL_TREE_KIND
// DEMANGLED: SetPvPSkillTree(
//     CUser *,_Mastered_skill const*,ENUM_SKILL_TREE_KIND)
int SetPvPSkillTree(CUser* user,
                    const _Mastered_skill* skill_tree,
                    ENUM_SKILL_TREE_KIND skill_tree_kind);

#endif  // OLDSRV_GLOBAL_PVP_SKILL_TREE_RUNTIME_H_
