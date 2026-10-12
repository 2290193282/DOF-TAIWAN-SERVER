#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "old_game/progression/skill/create_character_skill_abi.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/skill/skill_changer_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

ENUM_SKILL_TREE_KIND SkillPointTree(ENUM_SKILL_TREE_KIND skill_tree) {
    return (skill_tree == ENUM_SKILL_TREE_CURRENT ||
            skill_tree == ENUM_SKILL_TREE_0)
               ? ENUM_SKILL_TREE_SFP_0
               : ENUM_SKILL_TREE_SFP_1;
}

bool ApplyGrowthGiveSkills(CUser* user,
                           SkillSlot* skills,
                           ENUM_SKILL_TREE_KIND skill_tree,
                           unsigned int grow_stage,
                           int log_line) {
    std::vector<std::pair<int, int> > give_skills;
    CCharacList* const characters = G_CDataManager()->get_character_list();
    const int charac_job = user->get_charac_job();
    if (!characters->characters_[charac_job].get_give_skill(
            user->getCurCharFirstGrowType(),
            user->getCurCharSecondGrowType(),
            give_skills,
            grow_stage)) {
        return false;
    }
    NormalizeAutomaticGiveSkillLevels(
        user, charac_job, user->getCurCharFirstGrowType(),
        user->getCurCharSecondGrowType(), give_skills);

    for (std::vector<std::pair<int, int> >::const_iterator it =
             give_skills.begin();
         it != give_skills.end();
         ++it) {
        const int skill_index = it->first;
        const int skill_level = it->second;
        CSkill* const skill =
            G_CDataManager()->find_skill(charac_job, skill_index);
        if (!skill) {
            LogManager::logFormat(
                1,
                "user.cpp",
                "bool CUser::master_new_skills(_Mastered_skill*, ENUM_SKILL_TREE_KIND)",
                log_line,
                "G_CDataManager()->find_skill( %d, %d )",
                charac_job,
                skill_index);
            return false;
        }

        if (skills->get_skillslot_no(skill_index,
                                     skill->get_group(),
                                     skill_tree,
                                     true) < 0) {
            skills->growtype_skill(charac_job,
                                   skill_index,
                                   skill_level,
                                   skill_tree);
        }
    }
    return true;
}

}  // namespace

// IDA_EA: 0x866BCE8
// IDA_NAME: _ZN5CUser15init_skill_treeE20ENUM_SKILL_TREE_KIND
// DEMANGLED: CUser::init_skill_tree(ENUM_SKILL_TREE_KIND)
void CUser::init_skill_tree(ENUM_SKILL_TREE_KIND skill_tree) {
    if (!getCurCharacR()) {
        return;
    }

    SkillSlot* const skills = getCurCharacSkillW();
    skills->clear_all_skills(skill_tree);
    skills->set_parent(this);
    addSkillOnCreateCharacter(skills->get_mastered_skill_tree(skill_tree),
                              get_charac_job());

    std::vector<std::pair<int, int> > give_skills;
    CCharacList* const character_list = G_CDataManager()->get_character_list();
    CCharacter* const character =
        &character_list->characters_[get_charac_job()];
    if (character->get_give_skill(getCurCharFirstGrowType(),
                                  getCurCharSecondGrowType(),
                                  give_skills,
                                  0) != 1) {
        LogManager::logFormat(1,
                              "user.cpp",
                              "void CUser::init_skill_tree(ENUM_SKILL_TREE_KIND)",
                              0x5328,
                              "User %s - CUser::init_skill_tree",
                              getCurCharacName());
        return;
    }
    NormalizeAutomaticGiveSkillLevels(
        this, get_charac_job(), getCurCharFirstGrowType(),
        getCurCharSecondGrowType(), give_skills);

    for (std::vector<std::pair<int, int> >::const_iterator it =
             give_skills.begin();
         it != give_skills.end();
         ++it) {
        skills->growtype_skill(get_charac_job(),
                               it->first,
                               it->second,
                               skill_tree);
    }

    // The original has the expert-job type live in the argument register
    // immediately before the call.  Use the proved `GetExpertJobScript(int)`
    // provider rather than inventing the decompiler's apparent no-arg ABI.
    STExpertJobScript* const expert_job =
        G_CDataManager()->GetExpertJobScript(GetCurCharacExpertJobType());
    if (expert_job) {
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 expert_job->unknown_exp_pairs.begin();
             it != expert_job->unknown_exp_pairs.end();
             ++it) {
            const int expert_level =
                GetCurExpertJobLevel(GetCurCharacExpertJobExp());
            // 0x866BF76 hard-codes normal tree 1 here, irrespective of
            // `skill_tree`.
            skills->growtype_skill(get_charac_job(),
                                   it->first,
                                   expert_level,
                                   ENUM_SKILL_TREE_1);
        }
    }

    WongWork::CSkillChanger skill_changer;
    const int changer_target =
        (skill_tree == ENUM_SKILL_TREE_CURRENT ||
         skill_tree == ENUM_SKILL_TREE_0)
            ? 1
            : 2;
    skill_changer.SkillInitialize(this, changer_target, false);
}

// IDA_EA: 0x867B51C
// IDA_NAME: _ZN5CUser13SetSkillLevelEii
// DEMANGLED: CUser::SetSkillLevel(int,int)
int CUser::SetSkillLevel(int skill_index, int requested_level) {
    if (IsGameMasterMode() != 1 || !getCurCharacR())
        return 0;

    const int charac_job = get_charac_job();
    CSkill* const skill = G_CDataManager()->find_skill(charac_job,
                                                        skill_index);
    if (!skill)
        return 0;

    const int first_grow_type = static_cast<int>(
        static_cast<int8_t>(getCurCharFirstGrowType()));
    const int second_grow_type = static_cast<int>(getCurCharSecondGrowType());
    int maximum_level = 0;
    if (second_grow_type > 0) {
        maximum_level = skill->get_second_growtype_max_level(
            first_grow_type, second_grow_type);
        if (maximum_level == 0)
            maximum_level = skill->get_type_max_level(first_grow_type);
    } else {
        maximum_level = skill->get_type_max_level(first_grow_type);
    }

    const int skill_level = std::max(0, std::min(maximum_level,
                                                  requested_level));
    getCurCharacSkillW()->growtype_skill(
        get_charac_job(),
        skill_index,
        skill_level,
        static_cast<ENUM_SKILL_TREE_KIND>(GetCurCharacSkillTreeIndex()));
    return 1;
}

// IDA_EA: 0x866AFA2
// IDA_NAME: _ZN5CUser16master_new_skillER14stBuySkillInfob
// DEMANGLED: CUser::master_new_skill(stBuySkillInfo &,bool)
void CUser::master_new_skill(stBuySkillInfo& skill_info, bool legacy_flag) {
    (void)legacy_flag;

    const unsigned int requested_skill = skill_info.skill_index;
    int result = -1;
    int learned_guild_skill_level = 0;

    if (requested_skill <= 0xc7u || requested_skill > 0xd0u) {
        const int current_tree = GetCurCharacSkillTreeIndex();
        if ((current_tree != -1 && current_tree != 0)
                ? skill_info.skill_tree_index != 1u
                : skill_info.skill_tree_index == 1u) {
            return;
        }

        result = getCurCharacSkillW()->master_new_skill(
            static_cast<int>(requested_skill),
            get_charac_level(),
            get_charac_job(),
            getCurCharFirstGrowType(),
            getCurCharSecondGrowType(),
            static_cast<int>(skill_info.skill_level_delta));
    } else {
        if (!IsGuildMaster() && !IsSubGuildMaster()) {
            return;
        }

        STGuildDBInfoOnly* const guild = GetGuildDBInfo();
        int gold_cost = 0;
        short guild_skill_point = 0;
        result = getCurCharacSkillR()->master_new_guild_skill(
            static_cast<int>(requested_skill),
            get_charac_job(),
            getCurCharFirstGrowType(),
            getCurCharSecondGrowType(),
            *guild,
            getCurCharacMoney(),
            gold_cost,
            learned_guild_skill_level,
            guild_skill_point,
            static_cast<int>(guild->raw_42));

        if (result >= 0) {
            CGuildServerProxy* const proxy =
                GlobalData::s_guild_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
            proxy->SendBuyGuildSkill(
                static_cast<unsigned int>(get_charac_no(-1)),
                static_cast<unsigned int>(get_charac_guildkey()),
                static_cast<int>(requested_skill),
                learned_guild_skill_level,
                guild_skill_point,
                static_cast<unsigned int>(gold_cost));
        }
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_BUY_SKILL);
    if (result >= 0) {
        const int current_tree = GetCurCharacSkillTreeIndex();
        SkillSlot* const skills = getCurCharacSkillR();
        packet.put_byte(1);
        packet.put_byte(current_tree);
        packet.put_short(skills->get_remain_sp_at_index(
            static_cast<ENUM_SKILL_TREE_KIND>(current_tree)));
        packet.put_short(skills->get_remain_sfp_at_index(
            current_tree == -1 || current_tree == 0
                ? ENUM_SKILL_TREE_SFP_0
                : ENUM_SKILL_TREE_SFP_1));

        if (requested_skill <= 0xc7u || requested_skill > 0xd0u) {
            packet.put_byte(result);
            packet.put_byte(requested_skill);
            packet.put_byte(skills->get_skill_level(result));
        } else {
            packet.put_byte(0xff);
            packet.put_byte(requested_skill);
            packet.put_byte(learned_guild_skill_level);
        }

        HistoryLog::WriteLearnSkill(
            history_stream(),
            static_cast<int>(requested_skill),
            skills->get_skill_level(result));
    } else {
        packet.put_byte(0);
        switch (result) {
            case -6: packet.put_byte(92); break;
            case -5: packet.put_byte(91); break;
            case -4: packet.put_byte(1); break;
            case -3: packet.put_byte(18); break;
            case -2: packet.put_byte(22); break;
            case -1: packet.put_byte(21); break;
            default: break;
        }
    }
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x866B53E
// IDA_NAME: _ZN5CUser17master_new_skillsEP15_Mastered_skill20ENUM_SKILL_TREE_KIND
// DEMANGLED: CUser::master_new_skills(_Mastered_skill *,ENUM_SKILL_TREE_KIND)
int CUser::master_new_skills(_Mastered_skill* skill_tree,
                             ENUM_SKILL_TREE_KIND skill_tree_kind) {
    int used_sp = 0;
    int used_sfp = 0;
    getCurCharacSkillR()->calcUsedSP(skill_tree, used_sp, used_sfp);

    const int remain_sp =
        getCurCharacSkillR()->get_remain_sp_at_index(skill_tree_kind);
    getCurCharacSkillW()->set_remain_sp_at_index(
        remain_sp >= used_sp ? remain_sp - used_sp : 0,
        skill_tree_kind);

    const ENUM_SKILL_TREE_KIND sfp_tree = SkillPointTree(skill_tree_kind);
    const int remain_sfp =
        getCurCharacSkillR()->get_remain_sfp_at_index(sfp_tree);
    getCurCharacSkillW()->set_remain_sfp_at_index(
        remain_sfp >= used_sfp ? remain_sfp - used_sfp : 0,
        sfp_tree);

    history_log().SFPSub(
        sfp_tree,
        getCurCharacSkillR()->get_remain_sfp_at_index(sfp_tree),
        used_sfp,
        static_cast<eSPSubReason>(1));
    history_log().SPSub(
        skill_tree_kind,
        getCurCharacSkillR()->get_remain_sp_at_index(skill_tree_kind),
        used_sp,
        static_cast<eSPSubReason>(1));

    getCurCharacSkillW()->set_skill_slot_at_index(
        reinterpret_cast<char*>(skill_tree),
        0x198,
        skill_tree_kind);

    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        return 1;
    }

    const int second_grow_type = getCurCharSecondGrowType();
    if (second_grow_type) {
        return ApplyGrowthGiveSkills(
                   this,
                   getCurCharacSkillW(),
                   skill_tree_kind,
                   0u,
                   0x52BD)
                   ? 1
                   : 0;
    }

    if (!getCurCharFirstGrowType()) {
        return 1;
    }
    return ApplyGrowthGiveSkills(this,
                                 getCurCharacSkillW(),
                                 skill_tree_kind,
                                 1u,
                                 0x52E0)
               ? 1
               : 0;
}
