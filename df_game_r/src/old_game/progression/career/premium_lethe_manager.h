#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PREMIUM_LETHE_MANAGER_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PREMIUM_LETHE_MANAGER_H_

#include <stdint.h>

#include "old_game/progression/skill/combo_skill.h"

class CUser;
struct _Mastered_skill;

// IDA unit: CPremiumLetheManager.  The old constructor allocates one byte,
// proving this is an empty manager object rather than an opaque backing
// buffer.  Its reset path is owned by C's quest/skill domain.
class CPremiumLetheManager {
public:
    // IDA data: USE/SKILL_CONFIRM/BACK_TO_PRE/INIT are byte constants at
    // 0x93ABC60..0x93ABC63; NOT_USE is the separately addressed byte at
    // 0x9472DBC.  The callers use MOVZX byte ptr for every state.
    static const uint8_t NOT_USE_LETHE_STATE = 0xFF;
    static const uint8_t USE_LETHE_STATE = 1;
    static const uint8_t SKILL_CONFIRM_STATE = 2;
    static const uint8_t BACK_TO_PRE_STATE = 3;
    static const uint8_t INIT_LETHE_SKILL = 4;

    // IDA_EA: 0x85C3D08
    // IDA_NAME: _ZN20CPremiumLetheManagerC2Ev
    // DEMANGLED: CPremiumLetheManager::CPremiumLetheManager(void)
    CPremiumLetheManager();

    // IDA_EA: 0x85C3D0E
    // IDA_NAME: _ZN20CPremiumLetheManagerD2Ev
    // DEMANGLED: CPremiumLetheManager::~CPremiumLetheManager()
    ~CPremiumLetheManager();

    // IDA_EA: 0x85C3D14
    // IDA_NAME: _ZN20CPremiumLetheManager13ResetSkillReqEP5CUser
    // DEMANGLED: CPremiumLetheManager::ResetSkillReq(CUser *)
    int ResetSkillReq(CUser* user);

    // IDA_EA: 0x85C3D70
    // IDA_NAME: _ZN20CPremiumLetheManager15ConfirmSkillReqEP5CUser
    // DEMANGLED: CPremiumLetheManager::ConfirmSkillReq(CUser *)
    int ConfirmSkillReq(CUser* user);

    // IDA_EA: 0x85C3E2A
    // IDA_NAME: _ZN20CPremiumLetheManager9BackToPreEP5CUserP15_Mastered_skill20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CPremiumLetheManager::BackToPre(
    //     CUser *,_Mastered_skill *,ENUM_SKILL_TREE_KIND)
    int BackToPre(CUser* user,
                  _Mastered_skill* backup_skills,
                  ENUM_SKILL_TREE_KIND skill_tree);

    // IDA_EA: 0x85C46E8
    // IDA_NAME: _ZN20CPremiumLetheManager11BackupSkillEP5CUser20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CPremiumLetheManager::BackupSkill(CUser *,ENUM_SKILL_TREE_KIND)
    int BackupSkill(CUser* user, ENUM_SKILL_TREE_KIND skill_tree);

    // The old complete skill-reset transaction is a separately named body.
    // This concrete request path invokes it directly; it is not substituted
    // with an acknowledgement or a local success result.
    // IDA_EA: 0x85C421E
    // IDA_NAME: _ZN20CPremiumLetheManager9InitSkillEP5CUserb20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CPremiumLetheManager::InitSkill(CUser *,bool,ENUM_SKILL_TREE_KIND)
    void InitSkill(CUser* user, bool reset, ENUM_SKILL_TREE_KIND skill_tree);

    // IDA_EA: 0x85C3F30
    // IDA_NAME: _ZN20CPremiumLetheManager21UpdateBackupSkillFlagEP5CUser20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CPremiumLetheManager::UpdateBackupSkillFlag(CUser *,ENUM_SKILL_TREE_KIND)
    int UpdateBackupSkillFlag(CUser* user, ENUM_SKILL_TREE_KIND skill_tree);

    // IDA_EA: 0x85C4008
    // IDA_NAME: _ZN20CPremiumLetheManager14InitLetheSkillEP5CUser20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CPremiumLetheManager::InitLetheSkill(CUser *,ENUM_SKILL_TREE_KIND)
    int InitLetheSkill(CUser* user, ENUM_SKILL_TREE_KIND skill_tree);

private:
    // IDA_EA: 0x85C40AA
    // IDA_NAME: _ZN20CPremiumLetheManager11_resetSkillEP5CUser
    // DEMANGLED: CPremiumLetheManager::_resetSkill(CUser *)
    void _resetSkill(CUser* user);

    // IDA_EA: 0x85C4842
    // IDA_NAME: _ZN20CPremiumLetheManager7_updateEP5CUserh20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CPremiumLetheManager::_update(CUser *,unsigned char,ENUM_SKILL_TREE_KIND)
    void _update(CUser* user,
                 uint8_t state,
                 ENUM_SKILL_TREE_KIND skill_tree);
};

static_assert(sizeof(CPremiumLetheManager) == 1u,
              "CPremiumLetheManager must retain its old empty-object ABI");

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PREMIUM_LETHE_MANAGER_H_
