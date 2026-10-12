#include "old_game/progression/career/premium_lethe_manager.h"
#include "common/config.h"

#include "common/manager/global_data.h"
#include "old_game/progression/skill/create_character_skill_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/db/skill/db_update_one_day_lethe_skill.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/skill/skill_changer_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <utility>
#include <vector>

const uint8_t CPremiumLetheManager::NOT_USE_LETHE_STATE;
const uint8_t CPremiumLetheManager::USE_LETHE_STATE;
const uint8_t CPremiumLetheManager::SKILL_CONFIRM_STATE;
const uint8_t CPremiumLetheManager::BACK_TO_PRE_STATE;
const uint8_t CPremiumLetheManager::INIT_LETHE_SKILL;

namespace WongWork {

int CSkillChanger::g_nStatUpItemIdx[16] = {
    8, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046,
    0, 0, 0, 0, 0, 0, 0,
};

int CSkillChanger::g_nElixerAbility[9] = {
    8, 4, 6, 5, 7, 2, 3, 8, 9,
};

// IDA_EA: 0x8234FBE
// IDA_NAME: _ZN8WongWork13CSkillChangerC2Ev
CSkillChanger::CSkillChanger() {}

// IDA_EA: 0x8234FC4
// IDA_NAME: _ZN8WongWork13CSkillChangerD2Ev
CSkillChanger::~CSkillChanger() {}

// IDA_EA: 0x8609D9C
// IDA_NAME: _ZN8WongWork13CSkillChanger11CountElixerEP5CUser
// DEMANGLED: WongWork::CSkillChanger::CountElixer(CUser *)
int CSkillChanger::CountElixer(CUser* user) {
    std::memset(elixer_count_, 0, sizeof(elixer_count_));
    std::memset(elixer_ability_, 0, sizeof(elixer_ability_));
    std::memset(need_elixer_count_, 0, sizeof(need_elixer_count_));
    std::memset(need_elixer_ability_, 0, sizeof(need_elixer_ability_));
    elixer_count_[0] = 8;
    elixer_ability_[0] = 8;
    need_elixer_count_[0] = 8;
    need_elixer_ability_[0] = 8;
    initialized_ = true;
    _GetUserItemCount(user);
    _ApplyUserData(user, ENUM_SKILL_TREE_0);
    _ApplyUserData(user, ENUM_SKILL_TREE_1);
    return static_cast<int>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x860A1E6
// IDA_NAME: _ZN8WongWork13CSkillChanger19GetCurCharacTotalSPEPK5CUserb
// DEMANGLED: WongWork::CSkillChanger::GetCurCharacTotalSP(CUser const*,bool)
int CSkillChanger::GetCurCharacTotalSP(const CUser* user,
                                       bool count_stat_items) {
    count_stat_items_ = count_stat_items;
    std::memset(elixer_count_, 0, sizeof(elixer_count_));
    std::memset(elixer_ability_, 0, sizeof(elixer_ability_));
    std::memset(need_elixer_count_, 0, sizeof(need_elixer_count_));
    std::memset(need_elixer_ability_, 0, sizeof(need_elixer_ability_));
    elixer_count_[0] = 8;
    elixer_ability_[0] = 8;
    need_elixer_count_[0] = 8;
    need_elixer_ability_[0] = 8;
    initialized_ = false;

    const int base_skill_point = _GetBaseSkillPoint(user);
    _GetUserItemCount(user);
    return base_skill_point + _GetQuestSkillPoint(user);
}

// IDA_EA: 0x869780E
// IDA_NAME: _ZN8WongWork13CSkillChanger14GetElixerCountEv
// DEMANGLED: WongWork::CSkillChanger::GetElixerCount(void)
int32_t* CSkillChanger::GetElixerCount() { return elixer_count_; }

// IDA_EA: 0x8697816
// IDA_NAME: _ZN8WongWork13CSkillChanger18GetNeedElixerCountEv
// DEMANGLED: WongWork::CSkillChanger::GetNeedElixerCount(void)
int32_t* CSkillChanger::GetNeedElixerCount() { return need_elixer_count_; }

// IDA_EA: 0x8697822
// IDA_NAME: _ZN8WongWork13CSkillChanger20GetNeedElixerAbilityEv
// DEMANGLED: WongWork::CSkillChanger::GetNeedElixerAbility(void)
int32_t* CSkillChanger::GetNeedElixerAbility() {
    return need_elixer_ability_;
}

// IDA_EA: 0x860A2EC
// IDA_NAME: _ZN8WongWork13CSkillChanger15_CountSkillBookER10Inven_Item
bool CSkillChanger::_CountSkillBook(Inven_Item& item) {
    for (int index = 1; index <= g_nStatUpItemIdx[0]; ++index) {
        if (item.getKey() == g_nStatUpItemIdx[index]) {
            elixer_count_[index] = item.get_add_info();
            elixer_ability_[index] = g_nElixerAbility[index];
        }
    }
    return false;
}

// IDA_EA: 0x860A34A
// IDA_NAME: _ZN8WongWork13CSkillChanger17_GetUserItemCountEPK5CUser
bool CSkillChanger::_GetUserItemCount(const CUser* user) {
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryConsumableSlotLast;
         ++slot) {
        Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(1, slot);
        _CountSkillBook(item);
    }
    for (int slot = 0; slot <= 151; ++slot) {
        Inven_Item item = user->getCurCharacCargoR()->get_cargo_slot(slot);
        _CountSkillBook(item);
    }
    return false;
}

// IDA_EA: 0x860A558
// IDA_NAME: _ZN8WongWork13CSkillChanger16_ResetSkillPointEP5CUser20ENUM_SKILL_TREE_KIND
bool CSkillChanger::_ResetSkillPoint(CUser* user,
                                     ENUM_SKILL_TREE_KIND kind) {
    for (int level = 1; level <= user->get_charac_level(); ++level) {
        const int remain_sp =
            user->getCurCharacSkillR()->get_remain_sp_at_index(kind);
        user->getCurCharacSkillW()->set_remain_sp_at_index(
            remain_sp + G_CDataManager()->GetSpAtLevelUp(level), kind);
    }
    return false;
}

// IDA_EA: 0x860A5D8
// IDA_NAME: _ZN8WongWork13CSkillChanger13_ResetSFPointEP5CUser20ENUM_SKILL_TREE_KIND
char CSkillChanger::_ResetSFPoint(CUser* user,
                                  ENUM_SKILL_TREE_KIND kind) {
    if (user->get_charac_level() <= 49) {
        return true;
    }
    if (kind > ENUM_SKILL_TREE_1) {
        return static_cast<char>(
            user->getCurCharacSkillW()->set_remain_sfp_at_index(
                user->get_charac_level() - 49, kind));
    }
    return static_cast<char>(kind);
}

// IDA_EA: 0x860A634
// IDA_NAME: _ZNK8WongWork13CSkillChanger18_GetBaseSkillPointEPK5CUser
// DEMANGLED: WongWork::CSkillChanger::_GetBaseSkillPoint(CUser const*)const
int CSkillChanger::_GetBaseSkillPoint(const CUser* user) const {
    int result = 0;
    for (int level = 1; user->get_charac_level() >= level; ++level) {
        result += G_CDataManager()->GetSpAtLevelUp(level);
    }
    return result;
}

// IDA_EA: 0x860A6D0
// IDA_NAME: _ZN8WongWork13CSkillChanger19_GetQuestSkillPointEPK5CUser
// DEMANGLED: WongWork::CSkillChanger::_GetQuestSkillPoint(CUser const*)
int CSkillChanger::_GetQuestSkillPoint(const CUser* user) {
    (void)user;
    return 0;
}

// IDA_EA: 0x860A950
// IDA_NAME: _ZN8WongWork13CSkillChanger14_ApplyUserDataEP5CUser20ENUM_SKILL_TREE_KIND
void CSkillChanger::_ApplyUserData(CUser* user,
                                   ENUM_SKILL_TREE_KIND kind) {
    (void)user;
    (void)kind;
}

// IDA_EA: 0x8609E90
// IDA_NAME: _ZN8WongWork13CSkillChanger15SkillInitializeEP5CUserib
CSkillChanger* CSkillChanger::SkillInitialize(
    CUser* user,
    int target_tree,
    bool count_stat_items) {
    count_stat_items_ = count_stat_items;
    std::memset(elixer_count_, 0, sizeof(elixer_count_));
    std::memset(elixer_ability_, 0, sizeof(elixer_ability_));
    std::memset(need_elixer_count_, 0, sizeof(need_elixer_count_));
    std::memset(need_elixer_ability_, 0, sizeof(need_elixer_ability_));
    elixer_count_[0] = 8;
    elixer_ability_[0] = 8;
    need_elixer_count_[0] = 8;
    need_elixer_ability_[0] = 8;
    initialized_ = false;

    if (target_tree == 0) {
        _ResetSkillPoint(user, ENUM_SKILL_TREE_0);
        _ResetSkillPoint(user, ENUM_SKILL_TREE_1);
        _ResetSFPoint(user, ENUM_SKILL_TREE_SFP_0);
        _ResetSFPoint(user, ENUM_SKILL_TREE_SFP_1);
        _GetUserItemCount(user);
        _ApplyUserData(user, ENUM_SKILL_TREE_0);
        _ApplyUserData(user, ENUM_SKILL_TREE_1);
        _ApplyUserData(user, ENUM_SKILL_TREE_SFP_0);
        _ApplyUserData(user, ENUM_SKILL_TREE_SFP_1);
        return this;
    }

    switch (target_tree) {
    case 1:
        _ResetSkillPoint(user, ENUM_SKILL_TREE_0);
        _ResetSFPoint(user, ENUM_SKILL_TREE_SFP_0);
        _GetUserItemCount(user);
        _ApplyUserData(user, ENUM_SKILL_TREE_0);
        _ApplyUserData(user, ENUM_SKILL_TREE_SFP_0);
        break;
    case 2:
        _ResetSkillPoint(user, ENUM_SKILL_TREE_1);
        _ResetSFPoint(user, ENUM_SKILL_TREE_SFP_1);
        _GetUserItemCount(user);
        _ApplyUserData(user, ENUM_SKILL_TREE_1);
        _ApplyUserData(user, ENUM_SKILL_TREE_SFP_1);
        break;
    case 3:
        _ResetSFPoint(user, ENUM_SKILL_TREE_SFP_0);
        _GetUserItemCount(user);
        _ApplyUserData(user, ENUM_SKILL_TREE_SFP_0);
        break;
    case 4:
        _ResetSFPoint(user, ENUM_SKILL_TREE_SFP_1);
        _GetUserItemCount(user);
        _ApplyUserData(user, ENUM_SKILL_TREE_SFP_1);
        break;
    default:
        break;
    }
    return this;
}

}  // namespace WongWork

// IDA_EA: 0x8665400
// IDA_NAME: _ZN5CUser16givePvPSkillTreeEibi
// DEMANGLED: CUser::givePvPSkillTree(int,bool,int)
bool CUser::givePvPSkillTree(int pvp_grade,
                             bool use_pvp_skill_tree,
                             int target_tree) {
    PvPSkillTreeParameterScript* const script =
        G_CDataManager()->get_pvp_skill_tree_parameter_script();

    for (int second_grow_type = 0;
         second_grow_type <= getCurCharSecondGrowType();
         ++second_grow_type) {
        for (int grade = pvp_grade; grade <= get_pvp_grade(); ++grade) {
            std::vector<std::pair<int, int> >* skills = 0;
            if (!script->getPvPSkillTree(
                    get_charac_job(),
                    getCurCharFirstGrowType(),
                    second_grow_type,
                    grade,
                    use_pvp_skill_tree,
                    &skills)) {
                continue;
            }

            for (std::vector<std::pair<int, int> >::const_iterator iter =
                     skills->begin();
                 iter != skills->end();
                 ++iter) {
                switch (target_tree) {
                case 1:
                    if (!use_pvp_skill_tree ||
                        getCurCharacSkillR()->FindSkill(
                            iter->first, ENUM_SKILL_TREE_0) < iter->second) {
                        getCurCharacSkillW()->growtype_skill(
                            get_charac_job(),
                            iter->first,
                            iter->second,
                            ENUM_SKILL_TREE_0);
                    }
                    break;
                case 2:
                    if (!use_pvp_skill_tree ||
                        getCurCharacSkillR()->FindSkill(
                            iter->first, ENUM_SKILL_TREE_1) < iter->second) {
                        getCurCharacSkillW()->growtype_skill(
                            get_charac_job(),
                            iter->first,
                            iter->second,
                            ENUM_SKILL_TREE_1);
                    }
                    break;
                case 3:
                    if (!use_pvp_skill_tree ||
                        getCurCharacSkillR()->FindSkill(
                            iter->first, ENUM_SKILL_TREE_0) < iter->second) {
                        getCurCharacSkillW()->growtype_skill(
                            get_charac_job(),
                            iter->first,
                            iter->second,
                            ENUM_SKILL_TREE_0);
                    }
                    if (!use_pvp_skill_tree ||
                        getCurCharacSkillR()->FindSkill(
                            iter->first, ENUM_SKILL_TREE_1) < iter->second) {
                        getCurCharacSkillW()->growtype_skill(
                            get_charac_job(),
                            iter->first,
                            iter->second,
                            ENUM_SKILL_TREE_1);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    return false;
}

// IDA_EA: 0x85C3F30
// IDA_NAME: _ZN20CPremiumLetheManager21UpdateBackupSkillFlagEP5CUser20ENUM_SKILL_TREE_KIND
// DEMANGLED: CPremiumLetheManager::UpdateBackupSkillFlag(CUser *,ENUM_SKILL_TREE_KIND)
int CPremiumLetheManager::UpdateBackupSkillFlag(
    CUser* user,
    ENUM_SKILL_TREE_KIND skill_tree) {
    const int lethe_flag =
        skill_tree == ENUM_SKILL_TREE_CURRENT || skill_tree == ENUM_SKILL_TREE_0
            ? user->GetOneDayLetheFlag()
            : user->GetOneDayLetheFlag2ND();
    if (lethe_flag != NOT_USE_LETHE_STATE) {
        return 0;
    }

    _update(user, NOT_USE_LETHE_STATE, skill_tree);
    cMyTrace trace(
        "bool CPremiumLetheManager::UpdateBackupSkillFlag(CUser*, ENUM_SKILL_TREE_KIND)",
        105,
        0);
    trace("ONE_DAY_LETHE : SUCCESS_INIT_FOR_LEGACY_LETHE, char(%s)",
          user->getCurCharacName());
    return 1;
}

// IDA_EA: 0x85C4008
// IDA_NAME: _ZN20CPremiumLetheManager14InitLetheSkillEP5CUser20ENUM_SKILL_TREE_KIND
// DEMANGLED: CPremiumLetheManager::InitLetheSkill(CUser *,ENUM_SKILL_TREE_KIND)
int CPremiumLetheManager::InitLetheSkill(
    CUser* user,
    ENUM_SKILL_TREE_KIND skill_tree) {
    if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33))) {
        return 0;
    }

    _update(user, INIT_LETHE_SKILL, skill_tree);
    cMyTrace trace(
        "bool CPremiumLetheManager::InitLetheSkill(CUser*, ENUM_SKILL_TREE_KIND)",
        117,
        0);
    trace("ONE_DAY_LETHE : SUCCESS_INIT_FOR_1ST_GROW_CHAR, char(%s)",
          user->getCurCharacName());
    return 1;
}

// IDA_EA: 0x85C3D70
// IDA_NAME: _ZN20CPremiumLetheManager15ConfirmSkillReqEP5CUser
// DEMANGLED: CPremiumLetheManager::ConfirmSkillReq(CUser *)
int CPremiumLetheManager::ConfirmSkillReq(CUser* user) {
    if (!user) {
        return 0;
    }
    if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33))) {
        return 0;
    }

    cMyTrace trace("bool CPremiumLetheManager::ConfirmSkillReq(CUser*)", 53,
                   0);
    trace("ONE_DAY_LETHE : CONFIRM_SKILL, char(%s)",
          user->getCurCharacName());

    _update(user,
            SKILL_CONFIRM_STATE,
            static_cast<ENUM_SKILL_TREE_KIND>(
                user->GetCurCharacSkillTreeIndex()));
    return 1;
}

// IDA_EA: 0x85C3E2A
// IDA_NAME: _ZN20CPremiumLetheManager9BackToPreEP5CUserP15_Mastered_skill20ENUM_SKILL_TREE_KIND
// DEMANGLED: CPremiumLetheManager::BackToPre(
//     CUser *,_Mastered_skill *,ENUM_SKILL_TREE_KIND)
int CPremiumLetheManager::BackToPre(CUser* user,
                                    _Mastered_skill* backup_skills,
                                    ENUM_SKILL_TREE_KIND skill_tree) {
    if (!user->getCurCharacR()) {
        return 0;
    }

    InitSkill(user, false, skill_tree);
    if (user->master_new_skills(backup_skills, skill_tree) != 1) {
        return 0;
    }

    if (skill_tree != ENUM_SKILL_TREE_CURRENT &&
        skill_tree != ENUM_SKILL_TREE_0) {
        user->SetOneDayLetheFlag2ND(NOT_USE_LETHE_STATE);
    } else {
        user->SetOneDayLetheFlag(NOT_USE_LETHE_STATE);
    }

    cMyTrace trace(
        "bool CPremiumLetheManager::BackToPre(CUser*, _Mastered_skill*, "
        "ENUM_SKILL_TREE_KIND)",
        79,
        0);
    trace("ONE_DAY_LETHE : SUCCESS_BACK_TO_PRE, char(%s)",
          user->getCurCharacName());
    return 1;
}

// IDA_EA: 0x85C46E8
// IDA_NAME: _ZN20CPremiumLetheManager11BackupSkillEP5CUser20ENUM_SKILL_TREE_KIND
// DEMANGLED: CPremiumLetheManager::BackupSkill(CUser *,ENUM_SKILL_TREE_KIND)
int CPremiumLetheManager::BackupSkill(CUser* user,
                                      ENUM_SKILL_TREE_KIND skill_tree) {
    if (!user->getCurCharacR()) {
        return 0;
    }

    const bool use_second_tree =
        skill_tree != ENUM_SKILL_TREE_CURRENT && skill_tree != ENUM_SKILL_TREE_0;
    if (use_second_tree) {
        const uint8_t lethe_flag =
            static_cast<uint8_t>(user->GetOneDayLetheFlag2ND());
        if (lethe_flag != NOT_USE_LETHE_STATE &&
            lethe_flag != SKILL_CONFIRM_STATE) {
            return 0;
        }
    } else {
        const uint8_t lethe_flag =
            static_cast<uint8_t>(user->GetOneDayLetheFlag());
        if (lethe_flag != NOT_USE_LETHE_STATE &&
            lethe_flag != SKILL_CONFIRM_STATE) {
            return 0;
        }
    }

    if (use_second_tree) {
        user->SetOneDayLetheFlag2ND(USE_LETHE_STATE);
        _update(user, USE_LETHE_STATE, ENUM_SKILL_TREE_1);
    } else {
        user->SetOneDayLetheFlag(USE_LETHE_STATE);
        _update(user, USE_LETHE_STATE, ENUM_SKILL_TREE_0);
    }
    return 1;
}

// IDA_EA: 0x85C4842
// IDA_NAME: _ZN20CPremiumLetheManager7_updateEP5CUserh20ENUM_SKILL_TREE_KIND
// DEMANGLED: CPremiumLetheManager::_update(CUser *,unsigned char,ENUM_SKILL_TREE_KIND)
void CPremiumLetheManager::_update(CUser* user,
                                   uint8_t state,
                                   ENUM_SKILL_TREE_KIND skill_tree) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("PremiumLetheManager.cpp", 293),
        true);
    *guard << 348;
    *guard << user->GetUID();

    SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* const payload =
        guard->GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>();
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());

    // The original code tests INIT_LETHE_SKILL itself at 0x85C4912 rather
    // than comparing state against it.  Its nonzero value therefore keeps
    // the live skill-tree snapshot on every path.
    if (state == USE_LETHE_STATE || state == SKILL_CONFIRM_STATE ||
        INIT_LETHE_SKILL) {
        user->getCurCharacSkillR()->get_skill_slot_at_index(
            payload->skill_slot,
            static_cast<int>(sizeof(payload->skill_slot)),
            skill_tree);
    } else {
        std::memset(payload->skill_slot, 0, sizeof(payload->skill_slot));
    }

    payload->state = state;
    payload->lethe_flag = state;
    payload->skill_tree_kind = static_cast<uint8_t>(skill_tree);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x85C421E
// IDA_NAME: _ZN20CPremiumLetheManager9InitSkillEP5CUserb20ENUM_SKILL_TREE_KIND
// DEMANGLED: CPremiumLetheManager::InitSkill(CUser *,bool,ENUM_SKILL_TREE_KIND)
void CPremiumLetheManager::InitSkill(
    CUser* user,
    bool reset,
    ENUM_SKILL_TREE_KIND skill_tree) {
    SkillSlot* const skill_slot = user->getCurCharacSkillW();
    skill_slot->clear_all_skills(skill_tree);
    skill_slot->set_parent(user);

    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        const int target_tree =
            (skill_tree != ENUM_SKILL_TREE_CURRENT &&
             skill_tree != ENUM_SKILL_TREE_0) ? 2 : 1;
        user->givePvPSkillTree(0, true, target_tree);

        const int pvp_skill_point =
            G_CDataManager()->get_pvp_skill_tree_parameter_script()
                ->getPvPSkillPoint(
                    user->get_charac_job(),
                    user->getCurCharFirstGrowType(),
                    user->getCurCharSecondGrowType(),
                    user->get_pvp_grade(),
                    false);
        skill_slot->set_remain_sp_at_index(pvp_skill_point, skill_tree);

        if (reset) {
            user->send_skill_info();
            cMyTrace trace(
                "void CPremiumLetheManager::InitSkill(CUser*, bool, ENUM_SKILL_TREE_KIND)",
                177,
                0);
            trace("ONE_DAY_LETHE : SUCCESS_INIT_SKILL and SEND_SKILL_INFO, char(%s)",
                  user->getCurCharacName());
        }
        return;
    }

    addSkillOnCreateCharacter(
        skill_slot->get_mastered_skill_tree(skill_tree),
        user->get_charac_job());

    std::vector<std::pair<int, int> > give_skills;
    CCharacList* const character_list =
        G_CDataManager()->get_character_list();
    CCharacter* const character =
        &character_list->characters_[user->get_charac_job()];
    if (character->get_give_skill(
            user->getCurCharFirstGrowType(),
            user->getCurCharSecondGrowType(),
            give_skills,
            0) != 1) {
        return;
    }
    NormalizeAutomaticGiveSkillLevels(
        user, user->get_charac_job(), user->getCurCharFirstGrowType(),
        user->getCurCharSecondGrowType(), give_skills);

    for (std::vector<std::pair<int, int> >::const_iterator iter =
             give_skills.begin();
         iter != give_skills.end();
         ++iter) {
        skill_slot->growtype_skill(
            user->get_charac_job(),
            iter->first,
            iter->second,
            skill_tree);
    }

    STExpertJobScript* const expert_job_script =
        G_CDataManager()->GetExpertJobScript(
            user->GetCurCharacExpertJobType());
    if (expert_job_script) {
        for (std::vector<std::pair<int, int> >::const_iterator iter =
                 expert_job_script->unknown_exp_pairs.begin();
             iter != expert_job_script->unknown_exp_pairs.end();
             ++iter) {
            skill_slot->growtype_skill(
                user->get_charac_job(),
                iter->first,
                user->GetCurExpertJobLevel(
                    user->GetCurCharacExpertJobExp()),
                skill_tree);
        }
    }

    WongWork::CSkillChanger skill_changer;
    const int target_tree =
        (skill_tree != ENUM_SKILL_TREE_CURRENT &&
         skill_tree != ENUM_SKILL_TREE_0) ? 2 : 1;
    skill_changer.SkillInitialize(user, target_tree, false);

    if (reset) {
        user->send_skill_info();
        cMyTrace trace(
            "void CPremiumLetheManager::InitSkill(CUser*, bool, ENUM_SKILL_TREE_KIND)",
            247,
            0);
        trace("ONE_DAY_LETHE : SUCCESS_INIT_SKILL and SEND_SKILL_INFO, char(%s)",
              user->getCurCharacName());
    }
}
