#include "old_game/progression/skill/skill_tree_param_runtime.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/inter_returns/inter_load_etc_break_away_reward.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x86093A8
// IDA_NAME: _ZN16stSPReturnInfo_tC2Ettjj
// DEMANGLED: stSPReturnInfo_t::stSPReturnInfo_t(
//     unsigned short,unsigned short,unsigned int,unsigned int)
stSPReturnInfo_t::stSPReturnInfo_t(uint16_t input_00,
                                   uint16_t input_02,
                                   uint32_t input_04,
                                   uint32_t input_08)
    : value_00(input_00),
      value_02(input_02),
      value_04(input_04),
      value_08(input_08) {}

// IDA_EA: 0x8609A0E
// IDA_NAME: _ZN16stSPReturnInfo_tD2Ev
// DEMANGLED: stSPReturnInfo_t::~stSPReturnInfo_t()
stSPReturnInfo_t::~stSPReturnInfo_t() {}

// IDA_EA: 0x84EEF48
// IDA_NAME: _ZN15CSkillTreeParamC2EP5CUserP12SIG_LOAD_ETCR11PacketGuard
// DEMANGLED: CSkillTreeParam::CSkillTreeParam(
//     CUser *,SIG_LOAD_ETC *,PacketGuard &)
CSkillTreeParam::CSkillTreeParam(CUser* user,
                                 SIG_LOAD_ETC* payload,
                                 PacketGuard& packet)
    : user_(user),
      payload_(payload),
      packet_(&packet),
      first_return_info_(),
      second_return_info_() {
    // The old C2 constructs the two vectors before assigning these four
    // snapshot dwords.  They intentionally begin at zero until SetSkillTree
    // captures the pre-reconciliation pools.
    first_sp_snapshot_ = 0;
    second_sp_snapshot_ = 0;
    first_sfp_snapshot_ = 0;
    second_sfp_snapshot_ = 0;
}

namespace {

void PutReturnInfo(PacketGuard& packet,
                   const std::vector<stSPReturnInfo_t>& return_info) {
    packet.put_short(static_cast<int>(return_info.size()));
    for (std::vector<stSPReturnInfo_t>::const_iterator it =
             return_info.begin();
         it != return_info.end();
         ++it) {
        packet.put_short(it->value_00);
        packet.put_short(it->value_02);
        packet.put_int(static_cast<int>(it->value_04));
        packet.put_int(static_cast<int>(it->value_08));
    }
}

}  // namespace

// IDA_EA: 0x84EEFD4
// IDA_NAME: _ZN15CSkillTreeParam24SendSkillPointReturnInfoEv
// DEMANGLED: CSkillTreeParam::SendSkillPointReturnInfo(void)
int CSkillTreeParam::SendSkillPointReturnInfo() {
    first_sp_snapshot_ =
        user_->getCurCharacSkillR()->get_remain_sp_at_index(
            ENUM_SKILL_TREE_0) -
        first_sp_snapshot_;
    first_sfp_snapshot_ =
        user_->getCurCharacSkillR()->get_remain_sfp_at_index(
            ENUM_SKILL_TREE_SFP_0) -
        first_sfp_snapshot_;

    if ((first_sp_snapshot_ > 0 || first_sfp_snapshot_ > 0) &&
        !first_return_info_.empty()) {
        packet_->clear();
        packet_->put_header(0, 122);
        packet_->put_byte(0);
        packet_->put_int(first_sfp_snapshot_);
        packet_->put_int(first_sp_snapshot_);
        PutReturnInfo(*packet_, first_return_info_);
        packet_->finalize(true);
        user_->Send(*packet_);
    }

    second_sp_snapshot_ =
        user_->getCurCharacSkillR()->get_remain_sp_at_index(
            ENUM_SKILL_TREE_1) -
        second_sp_snapshot_;
    second_sfp_snapshot_ =
        user_->getCurCharacSkillR()->get_remain_sfp_at_index(
            ENUM_SKILL_TREE_SFP_1) -
        second_sfp_snapshot_;

    const int result =
        (second_sp_snapshot_ > 0 || second_sfp_snapshot_ > 0) &&
        !second_return_info_.empty();
    if (result) {
        packet_->clear();
        packet_->put_header(0, 122);
        packet_->put_byte(1);
        packet_->put_int(second_sfp_snapshot_);
        packet_->put_int(second_sp_snapshot_);
        PutReturnInfo(*packet_, second_return_info_);
        packet_->finalize(true);
        return user_->Send(*packet_);
    }
    return result;
}

namespace {

void SendLetheSkillTreeChanged(CUser& user,
                               PacketGuard& packet,
                               uint8_t skill_tree_number) {
    packet.clear();
    packet.put_header(0, 180);
    packet.put_byte(1);
    packet.put_byte(skill_tree_number);
    packet.finalize(true);
    user.Send(packet);
}

}  // namespace

// IDA_EA: 0x84EF4A6
// IDA_NAME: _ZN15CSkillTreeParam12SetSkillTreeEP13Inter_LoadEtc
// DEMANGLED: CSkillTreeParam::SetSkillTree(Inter_LoadEtc *)
int CSkillTreeParam::SetSkillTree(Inter_LoadEtc* dispatcher) {
    // 0x84EF4A6 takes both persisted premium-Lethe trees as the backup source
    // before either Lethe state machine may alter the live slots.
    user_->SaveBackupSkillInfo(&payload_->premium_lethe,
                               ENUM_SKILL_TREE_0);
    user_->SaveBackupSkillInfo(&payload_->premium_lethe_second,
                               ENUM_SKILL_TREE_1);

    int first_lethe_should_initialize = 0;
    int second_lethe_should_initialize = 0;
    if (!G_GameWorld()->IsPvPSkilTreeChannel() || user_->get_pvp_grade() > 9) {
        dispatcher->ProcessOneDayLethe(
            payload_, user_, first_lethe_should_initialize);
        dispatcher->ProcessOneDayLethe2ND(
            payload_, user_, second_lethe_should_initialize);
    }

    if (user_->GetCurCharacSkillTreeIndex() ==
        ENUM_SKILL_TREE_CURRENT) {
        if (first_lethe_should_initialize == 1) {
            SendLetheSkillTreeChanged(*user_, *packet_, 1);
        }
    } else {
        if (first_lethe_should_initialize == 1) {
            SendLetheSkillTreeChanged(*user_, *packet_, 1);
        }

        // This clear is intentionally unconditional for a selected secondary
        // skill tree: it is present in the old packet sequence even when the
        // second Lethe state does not produce a notification.
        packet_->clear();
        if (second_lethe_should_initialize == 1) {
            packet_->put_header(0, 180);
            packet_->put_byte(1);
            packet_->put_byte(2);
            packet_->finalize(true);
            user_->Send(*packet_);
        }
    }

    first_sp_snapshot_ = user_->getCurCharacSkillR()->get_remain_sp_at_index(
        ENUM_SKILL_TREE_0);
    second_sp_snapshot_ = user_->getCurCharacSkillR()->get_remain_sp_at_index(
        ENUM_SKILL_TREE_1);
    first_sfp_snapshot_ =
        user_->getCurCharacSkillR()->get_remain_sfp_at_index(
            ENUM_SKILL_TREE_SFP_0);
    second_sfp_snapshot_ =
        user_->getCurCharacSkillR()->get_remain_sfp_at_index(
            ENUM_SKILL_TREE_SFP_1);

    int character_level = user_->get_charac_level();
    if (G_GameWorld()->IsCharacterLevelRevisionChannel()) {
        character_level = oldsrv::config::kCharacterLevelRevision;
    }

    if (user_->getCurCharacSkillR()->checkOverLevelSkill(
            first_return_info_, ENUM_SKILL_TREE_0, character_level)) {
        user_->enableSaveSkill();
    }
    if (user_->getCurCharacSkillR()->checkOverLevelSkill(
            second_return_info_, ENUM_SKILL_TREE_1, character_level)) {
        user_->enableSaveSkill();
    }

    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        return 0;
    }

    // The post-reconciliation loop is deliberately over the base character
    // list only.  Grow-type grants were handled by the Lethe/skill routines;
    // adding them here would make each login synthesize extra skill slots.
    const int charac_job = user_->get_charac_job();
    const std::vector<std::pair<int, int> > initial_skills(
        G_CDataManager()->get_initial_character_skills(charac_job));
    for (std::vector<std::pair<int, int> >::const_iterator it =
             initial_skills.begin();
         it != initial_skills.end();
         ++it) {
        CSkill* const skill =
            G_CDataManager()->find_skill(charac_job, it->first);
        if (!skill) {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "int CSkillTreeParam::SetSkillTree(Inter_LoadEtc*)",
                0x9FF,
                "G_CDataManager()->find_skill( %d, %d )",
                charac_job,
                it->first);
            return 2560;
        }

        const int skill_group = skill->get_group();
        if (user_->getCurCharacSkillR()->get_skillslot_no(
                it->first,
                skill_group,
                ENUM_SKILL_TREE_0,
                true) < 0) {
            user_->getCurCharacSkillW()->growtype_skill(
                charac_job,
                it->first,
                it->second,
                ENUM_SKILL_TREE_0);
        }
        if (user_->getCurCharacSkillR()->get_skillslot_no(
                it->first,
                skill_group,
                ENUM_SKILL_TREE_1,
                true) < 0) {
            user_->getCurCharacSkillW()->growtype_skill(
                charac_job,
                it->first,
                it->second,
                ENUM_SKILL_TREE_1);
        }
    }

    // The old decompiler lost this one explicit argument because the caller
    // obtained the type immediately before it.  The canonical provider is
    // `GetExpertJobScript(int)`; do not manufacture a no-argument wrapper.
    STExpertJobScript* const expert_job_script =
        G_CDataManager()->GetExpertJobScript(
            user_->GetCurCharacExpertJobType());
    if (expert_job_script) {
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 expert_job_script->unknown_exp_pairs.begin();
             it != expert_job_script->unknown_exp_pairs.end();
             ++it) {
            const int expert_level = user_->GetCurExpertJobLevel(
                user_->GetCurCharacExpertJobExp());
            if (user_->getCurCharacSkillR()->check_skill_mastered(
                    it->first,
                    expert_level,
                    2,
                    ENUM_SKILL_TREE_0) != 1) {
                user_->getCurCharacSkillW()->growtype_skill(
                    charac_job,
                    it->first,
                    expert_level,
                    ENUM_SKILL_TREE_0);
            }
            if (user_->getCurCharacSkillR()->check_skill_mastered(
                    it->first,
                    expert_level,
                    2,
                    ENUM_SKILL_TREE_1) != 1) {
                user_->getCurCharacSkillW()->growtype_skill(
                    charac_job,
                    it->first,
                    expert_level,
                    ENUM_SKILL_TREE_1);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x84EFC92
// IDA_NAME: _ZN15CSkillTreeParamD2Ev
// DEMANGLED: CSkillTreeParam::~CSkillTreeParam()
CSkillTreeParam::~CSkillTreeParam() {}
