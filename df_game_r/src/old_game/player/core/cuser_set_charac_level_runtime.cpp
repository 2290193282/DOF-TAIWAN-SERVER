#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <cstring>

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/udp_packets.h"

namespace {

// SetCharacLevel @0x867A95C writes the two ordinary skill trees and the two
// SFP trees independently.  Keep its signed 32-bit arithmetic and explicit
// clamps, rather than routing through gain_sp/gain_sfp (which have different
// saturation behavior for this GM operation).
int ClampSkillPoint(int value) {
    return std::max(0, std::min(0x3fffffff, value));
}

}  // namespace

// IDA_EA: 0x867A95C
// IDA_NAME: _ZN5CUser14SetCharacLevelEi
// DEMANGLED: CUser::SetCharacLevel(int)
int CUser::SetCharacLevel(int requested_level) {
    if (!getCurCharacR()) {
        return 0;
    }

    // 0x867A985: target = max(1, min(200, requested_level)).
    const int target_level = std::max(1, std::min(200, requested_level));
    const int current_level = get_charac_level();
    const int sp_reference_level =
        current_level >= target_level ? target_level + 1 : target_level;
    const int gained_sp = G_CDataManager()->GetSpAtLevelUp(sp_reference_level) *
                          (target_level - current_level);

    setCurCharacExp(G_CDataManager()->get_level_exp(target_level));

    SkillSlot* const skill_read = getCurCharacSkillR();
    getCurCharacSkillW()->set_remain_sp_at_index(
        ClampSkillPoint(skill_read->get_remain_sp_at_index(ENUM_SKILL_TREE_0) +
                        gained_sp),
        ENUM_SKILL_TREE_0);

    SkillSlot* const second_skill_read = getCurCharacSkillR();
    getCurCharacSkillW()->set_remain_sp_at_index(
        ClampSkillPoint(
            second_skill_read->get_remain_sp_at_index(ENUM_SKILL_TREE_1) +
            gained_sp),
        ENUM_SKILL_TREE_1);

    if (target_level <= 49) {
        getCurCharacSkillW()->set_remain_sfp_at_index(
            0, ENUM_SKILL_TREE_SFP_0);
        getCurCharacSkillW()->set_remain_sfp_at_index(
            0, ENUM_SKILL_TREE_SFP_1);
    } else {
        int first_sfp = 0;
        int second_sfp = 0;
        // This condition intentionally observes the pre-transition level.
        // At 50 it reseeds both SFP trees from level 49; above it preserves
        // each tree's existing independent point value.
        if (get_charac_level() <= 50) {
            first_sfp = target_level - 49;
            second_sfp = target_level - 49;
        } else {
            first_sfp = getCurCharacSkillR()->get_remain_sfp_at_index(
                            ENUM_SKILL_TREE_SFP_0) +
                        target_level - get_charac_level();
            second_sfp = getCurCharacSkillR()->get_remain_sfp_at_index(
                             ENUM_SKILL_TREE_SFP_1) +
                         target_level - get_charac_level();
        }
        getCurCharacSkillW()->set_remain_sfp_at_index(
            first_sfp, ENUM_SKILL_TREE_SFP_0);
        getCurCharacSkillW()->set_remain_sfp_at_index(
            second_sfp, ENUM_SKILL_TREE_SFP_1);
    }

    // The original obtains one growth-stat record before the loop, then
    // mutates the character a level at a time so history, premium side
    // effects, and status accumulation remain in their old order.
    while (get_charac_level() != target_level) {
        if (get_charac_level() >= target_level) {
            _update_charac_stat(false);
            decCurCharacLevel();
            history_log().LevelDown(
                get_charac_level(), static_cast<eLevelDownReason>(0));
        } else {
            _update_charac_stat(true);
            incCurCharacLevel();
            history_log().LevelUp(
                get_charac_level(), static_cast<eLevelUpReason>(1));

            const int gold_bonus =
                user_premium_.GetGoldBonus(get_charac_level());
            if (gold_bonus) {
                getCurCharacInvenW()->gain_money(
                    gold_bonus, static_cast<eMoneyAddReason>(13), true, 0);
                SendUpdateItemList(static_cast<eSendTarget>(1),
                                   static_cast<ENUM_ITEMSPACE>(0),
                                   0);
            }
        }
        user_premium_.RecalcAdditionalInfo(this);
    }

    // 0x867ACED redundantly commits the final level after the transition
    // loop.  Keep that store: it is part of the old write protocol.
    setCurCharacLevel(target_level);

    STExpertJobEtcScript* const expert_job_etc =
        G_CDataManager()->GetExpertJobEtcScript();
    if (GetCurCharacExpertJobType() &&
        expert_job_etc->isExpertJobLimitLevel(target_level)) {
        expert_job::CExpertJob::IncreaseExpertJobExp(
            this, GetCurCharacExpertJobExp());
    }

    makeGuildLevelUpMessage(target_level);

    if (target_level == oldsrv::config::kCharacterLevelMaximum &&
        !GlobalData::s_pGMAccounts_->isGM(
            static_cast<unsigned int>(get_acc_id()))) {
        Packet_Monitor_Max_Level_BroadCast packet;
        std::strncpy(reinterpret_cast<char*>(packet.payload),
                     getCurCharacName(),
                     sizeof(packet.payload));
        GlobalData::s_monitor_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    }

    const int dungeon_index = 0x2aff;
    if (get_charac_level() ==
        G_GameWorld()->getDungeonMinimumRequiredLevel(dungeon_index)) {
        const bool is_open = G_CDataManager()
                                 ->get_server_parameter_script()
                                 ->isDungeonOpen(dungeon_index) != 0;
        G_GameWorld()->send_user_dungeon_inout_message(
            this, dungeon_index, is_open);
    }

    if (get_charac_level() > 59) {
        DimensionInoutUpdate(true, true);
    }
    // GM level changes can cross several PVF fixed-level intervals in either
    // direction.  Keep the two ordinary trees exact without touching SP/SFP.
    (void)ReconcileAutomaticGiveSkillLevels(this, true);
    character_view_.enableSaveCharacView();
    return 1;
}
