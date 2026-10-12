#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_CHANGER_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_CHANGER_RUNTIME_H_

#include <stdint.h>

#include "old_game/progression/skill/combo_skill.h"

class CUser;
struct Inven_Item;

namespace WongWork {

// IDA unit: 08_quest_skill_growth/WongWork_CSkillChanger.
// The four nine-dword tables and the two trailing flags are proven by
// SkillInitialize @0x8609E90; CPremiumLetheManager allocates exactly 0x94
// bytes for this object.
class CSkillChanger {
public:
    // IDA_EA: 0x8234FBE
    CSkillChanger();
    // IDA_EA: 0x8234FC4
    ~CSkillChanger();

    // IDA_EA: 0x8609D10
    // IDA_NAME: _ZN8WongWork13CSkillChanger14CheckConditionEPK5CUser
    // DEMANGLED: WongWork::CSkillChanger::CheckCondition(CUser const *)
    static bool CheckCondition(const CUser* user);

    // IDA_EA: 0x8609E90
    // IDA_NAME: _ZN8WongWork13CSkillChanger15SkillInitializeEP5CUserib
    CSkillChanger* SkillInitialize(CUser* user,
                                   int target_tree,
                                   bool count_stat_items);

    // IDA_EA: 0x8609D9C
    // IDA_NAME: _ZN8WongWork13CSkillChanger11CountElixerEP5CUser
    int CountElixer(CUser* user);

    // IDA_EA: 0x860A1E6
    // IDA_NAME: _ZN8WongWork13CSkillChanger19GetCurCharacTotalSPEPK5CUserb
    // DEMANGLED: WongWork::CSkillChanger::GetCurCharacTotalSP(
    //     CUser const*,bool)
    int GetCurCharacTotalSP(const CUser* user, bool count_stat_items);

    // IDA_EA: 0x869780E
    // IDA_NAME: _ZN8WongWork13CSkillChanger14GetElixerCountEv
    int32_t* GetElixerCount();
    // IDA_EA: 0x8697816
    // IDA_NAME: _ZN8WongWork13CSkillChanger18GetNeedElixerCountEv
    int32_t* GetNeedElixerCount();
    // IDA_EA: 0x8697822
    // IDA_NAME: _ZN8WongWork13CSkillChanger20GetNeedElixerAbilityEv
    int32_t* GetNeedElixerAbility();

private:
    // IDA_EA: 0x860A2EC
    bool _CountSkillBook(Inven_Item& item);
    // IDA_EA: 0x860A34A
    bool _GetUserItemCount(const CUser* user);
    // IDA_EA: 0x860A558
    bool _ResetSkillPoint(CUser* user, ENUM_SKILL_TREE_KIND kind);
    // IDA_EA: 0x860A5D8
    char _ResetSFPoint(CUser* user, ENUM_SKILL_TREE_KIND kind);
    // IDA_EA: 0x860A634
    int _GetBaseSkillPoint(const CUser* user) const;
    // IDA_EA: 0x860A6D0. The original body is an actual zero-return leaf.
    int _GetQuestSkillPoint(const CUser* user);
    // IDA_EA: 0x860A950.  The original function is a four-instruction
    // genuine empty body; retaining it is required by SkillInitialize's
    // exact call graph.
    void _ApplyUserData(CUser* user, ENUM_SKILL_TREE_KIND kind);

    static int g_nStatUpItemIdx[16];
    static int g_nElixerAbility[9];

    int32_t elixer_count_[9];          // +0x00
    int32_t elixer_ability_[9];        // +0x24
    int32_t need_elixer_count_[9];     // +0x48
    int32_t need_elixer_ability_[9];   // +0x6c
    bool count_stat_items_;            // +0x90
    bool initialized_;                 // +0x91
    uint8_t reserved_92[2];            // +0x92
};

static_assert(sizeof(CSkillChanger) == 0x94u,
              "CSkillChanger IDA ABI size must stay 0x94");

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_SKILL_CHANGER_RUNTIME_H_
