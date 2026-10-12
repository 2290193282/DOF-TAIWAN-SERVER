#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include <climits>
#include <cstring>
#include <stdint.h>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/history_log_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/Login/inter_returns/statistic_manager_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/activity/schedule/exp_keeping_event.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/udp_packets.h"


namespace {

int AddInt32Wrapping(int lhs, int rhs) {
    return static_cast<int32_t>(static_cast<uint32_t>(lhs) +
                                static_cast<uint32_t>(rhs));
}

int SubtractInt32Wrapping(int lhs, int rhs) {
    return static_cast<int32_t>(static_cast<uint32_t>(lhs) -
                                static_cast<uint32_t>(rhs));
}

void AddDungeonExpStatistic(Character& character, int gained_exp) {
    // CParty::OnDungeonClear @0x85B5518 reads this as the dungeon EXP
    // statistic at expanded CUser+0x8E3AC.  The physical layout proves
    // that it is Character::play_info+0x28, so the raw CUser displacement
    // remains confined to neither this path nor any caller.
    character.dungeon_exp_ += static_cast<uint32_t>(gained_exp);
}

void AddReasonOneOrTwoExpStatistic(Character& character, int gained_exp) {
    // gain_exp_sp @0x866A457 updates Character::play_info+0x34 only for
    // eExpAddReason values 1 and 2.  CParty's statistic serializer names the
    // same physical slot as its ninth per-dungeon value.
    character.reason_one_or_two_exp_ +=
        static_cast<uint32_t>(gained_exp);
}

bool IsLevelExpNotificationState(uint32_t state) {
    return state == 5u || state == 8u || state == 7u ||
           state == 10u || state == 12u;
}

}  // namespace

// IDA_EA: 0x8682BAE
// IDA_NAME: _ZN15cUserHistoryLog5SPAddEiii12eSPAddReason
// DEMANGLED: cUserHistoryLog::SPAdd(int,int,int,eSPAddReason)
int cUserHistoryLog::SPAdd(int skill_tree_kind,
                           int remaining_sp,
                           int gained_sp,
                           eSPAddReason reason) {
    return owner_->LogHistory("SP+,%d,%d,%d,%d",
                              remaining_sp,
                              gained_sp,
                              static_cast<int>(reason),
                              skill_tree_kind);
}

// IDA_EA: 0x8682BE8
// IDA_NAME: _ZN15cUserHistoryLog5SPSubEiii12eSPSubReason
int cUserHistoryLog::SPSub(int skill_tree_kind,
                           int remaining_sp,
                           int spent_sp,
                           eSPSubReason reason) {
    return owner_->LogHistory("SP-,%d,%d,%d,%d",
                              skill_tree_kind,
                              spent_sp,
                              static_cast<int>(reason),
                              remaining_sp);
}

// IDA_EA: 0x8682C22
// IDA_NAME: _ZN15cUserHistoryLog6SFPAddEiii12eSPAddReason
// DEMANGLED: cUserHistoryLog::SFPAdd(int,int,int,eSPAddReason)
int cUserHistoryLog::SFPAdd(int skill_tree_kind,
                            int remaining_sfp,
                            int gained_sfp,
                            eSPAddReason reason) {
    return owner_->LogHistory("SFP+,%d,%d,%d,%d",
                              remaining_sfp,
                              gained_sfp,
                              static_cast<int>(reason),
                              skill_tree_kind);
}

// IDA_EA: 0x8682C5C
// IDA_NAME: _ZN15cUserHistoryLog6SFPSubEiii12eSPSubReason
int cUserHistoryLog::SFPSub(int skill_tree_kind,
                            int remaining_sfp,
                            int spent_sfp,
                            eSPSubReason reason) {
    return owner_->LogHistory("SFP-,%d,%d,%d,%d",
                              skill_tree_kind,
                              spent_sfp,
                              static_cast<int>(reason),
                              remaining_sfp);
}

// IDA_EA: 0x8662AEA
// IDA_NAME: _ZN5CUser15_check_level_upEiRiS0_13eExpAddReasoni
// DEMANGLED:
// CUser::_check_level_up(int,int &,int &,eExpAddReason,int)
bool CUser::_check_level_up(int gained_exp,
                            int& gained_sp,
                            int& gained_sfp,
                            eExpAddReason reason,
                            int monster_id) {
    // The fifth ABI argument is forwarded by gain_exp_sp but is not read by
    // the original body.  Hades notification is gated by reason != 0.
    (void)monster_id;

    int next_level = get_charac_level() + 1;
    bool leveled_up = false;

    if (!getCurCharacR()) {
        cMyTrace trace(__PRETTY_FUNCTION__, 16411, 5);
        trace("CUser::_check_level_up m_selected is NULL");
        return false;
    }

    if (!get_charac_level()) {
        cMyTrace trace(__PRETTY_FUNCTION__, 16418, 5);
        trace("CUser::_check_level_up m_selected->m_level is 0");
        return false;
    }

    while (gained_exp > 0 &&
           next_level <= oldsrv::config::kCharacterLevelMaximum) {
        const int bottom_exp =
            G_CDataManager()->get_level_exp(get_charac_level());
        const int top_exp =
            G_CDataManager()->get_level_exp(next_level);

        const int current_exp = get_charac_exp();
        if (current_exp < bottom_exp || current_exp > top_exp) {
            const int inconsistent_exp = get_charac_exp();
            const int inconsistent_level = get_charac_level();
            const char* const charac_name = getCurCharacName();
            cMyTrace trace(__PRETTY_FUNCTION__, 16443, 5);
            trace(
                "%s is Level(%d) and Exp(%d) "
                "Inconsistency(bottom(%d), top(%d))",
                charac_name,
                inconsistent_level,
                inconsistent_exp,
                bottom_exp,
                top_exp);
            setCurCharacExp(bottom_exp);
        }

        if (SubtractInt32Wrapping(top_exp, bottom_exp) <= 0) {
            cMyTrace trace(__PRETTY_FUNCTION__, 16454, 5);
            trace("CUser::_check_level_up, LEVEL:%d TOP:%d BOTTOM:%d",
                  next_level,
                  top_exp,
                  bottom_exp);
            return false;
        }

        const int exp_before_gain = get_charac_exp();
        if (AddInt32Wrapping(gained_exp, exp_before_gain) < top_exp) {
            addCurCharacExp(gained_exp);
            if (static_cast<int>(reason) != 0) {
                getHades()->ExpUp(static_cast<unsigned int>(gained_exp));
            }
            gained_exp = 0;
            continue;
        }

        gained_sp = AddInt32Wrapping(
            gained_sp,
            G_CDataManager()->GetSpAtLevelUp(next_level));
        if (next_level > 49) {
            gained_sfp = AddInt32Wrapping(gained_sfp, 1);
        }

        const unsigned int consumed_exp = static_cast<unsigned int>(
            SubtractInt32Wrapping(top_exp, get_charac_exp()));
        ++next_level;
        incCurCharacLevel();
        gained_exp = AddInt32Wrapping(
            gained_exp,
            SubtractInt32Wrapping(get_charac_exp(), top_exp));

        if (get_charac_level() > 9 && !GetTutorialSkipable()) {
            UpdateTutorialSkipable();
        }

        setCurCharacExp(top_exp);
        _update_charac_stat(true);
        leveled_up = true;

        HistoryLog::WriteLevelUp(history_stream(), getCurCharacExp());

        if (user_premium_.GetGoldBonus(get_charac_level())) {
            const int gold_bonus =
                user_premium_.GetGoldBonus(get_charac_level());
            getCurCharacInvenW()->gain_money(
                gold_bonus,
                static_cast<eMoneyAddReason>(13),
                true,
                0);
            SendUpdateItemList(static_cast<eSendTarget>(1),
                               static_cast<ENUM_ITEMSPACE>(0),
                               0);
        }
        user_premium_.RecalcAdditionalInfo(this);

        if (static_cast<int>(reason) != 0) {
            getHades()->ExpUp(consumed_exp);
        }

        history_log().LevelUp(
            get_charac_level(),
            static_cast<eLevelUpReason>(0));

        if (get_charac_level() > 59) {
            DimensionInoutUpdate(true, true);
        }

        if (get_charac_level() == oldsrv::config::kCharacterLevelMaximum &&
            !GlobalData::s_pGMAccounts_->isGM(
                static_cast<uint32_t>(get_acc_id()))) {
            Packet_Monitor_Max_Level_BroadCast packet;
            std::strncpy(reinterpret_cast<char*>(packet.payload),
                         getCurCharacName(),
                         sizeof(packet.payload));
            GlobalData::s_monitor_proxy_mgr
                ->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()))
                ->SendPacket(reinterpret_cast<char*>(&packet),
                             sizeof(packet));
        }

        const int dungeon_index = 0x2AFF;
        if (get_charac_level() ==
            G_GameWorld()->getDungeonMinimumRequiredLevel(dungeon_index)) {
            const bool dungeon_open =
                G_CDataManager()
                    ->get_server_parameter_script()
                    ->isDungeonOpen(dungeon_index) != 0;
            G_GameWorld()->send_user_dungeon_inout_message(
                this,
                dungeon_index,
                dungeon_open);
        }

        makeGuildLevelUpMessage(get_charac_level());
        _onLevelUp();
    }

    return leveled_up;
}

// IDA_EA: 0x866A3FE
// IDA_NAME: _ZN5CUser11gain_exp_spEiRiS0_13eExpAddReasonib
// DEMANGLED:
// CUser::gain_exp_sp(int,int &,int &,eExpAddReason,int,bool)
bool CUser::gain_exp_sp(int gained_exp,
                        int& gained_sp,
                        int& gained_sfp,
                        eExpAddReason reason,
                        int monster_id,
                        bool preserve_fatigue_bonus_exp) {
    if (exp_keeping::RejectUnrelatedExperienceMutation(this)) {
        gained_sp = 0;
        gained_sfp = 0;
        return false;
    }
    if (G_CEnvironment()->get_server_type() == 2 &&
        get_charac_level() > 49) {
        gained_exp = 0;
    }
    if (gained_exp < 0) {
        gained_exp = 0;
    }

    if (static_cast<int>(reason) == 1 ||
        static_cast<int>(reason) == 2) {
        AddReasonOneOrTwoExpStatistic(character_, gained_exp);
    }
    AddDungeonExpStatistic(character_, gained_exp);
    incPlayExpAdd(gained_exp);

    bool leveled_up = false;
    if (get_charac_level() <=
        oldsrv::config::kCharacterLevelBeforeMaximum) {
        leveled_up = _check_level_up(
            gained_exp,
            gained_sp,
            gained_sfp,
            reason,
            monster_id);

        const int previous_sp =
            getCurCharacSkillR()->get_remain_sp_at_index(
                ENUM_SKILL_TREE_0);
        gain_sp(gained_sp);

        const int previous_sfp =
            getCurCharacSkillR()->get_remain_sfp_at_index(
                ENUM_SKILL_TREE_SFP_0);
        gain_sfp(gained_sfp);

        if (leveled_up) {
            // Reconcile only after the ordinary level-up SP/SFP grants have
            // committed, so the skill-info packet contains the final point
            // balances as well as the PVF-derived awakening level.
            (void)ReconcileAutomaticGiveSkillLevels(this, true);
        }

        history_log_sp(previous_sp,
                       gained_sp,
                       static_cast<eSPAddReason>(0));
        history_log_sfp(previous_sfp,
                        gained_sfp,
                        static_cast<eSPAddReason>(0));
    }

    PacketGuard packet;
    if (leveled_up) {
        CalcurateUserMaxLevel();
        if (GetCurCharacMaxEquipLevel() &&
            get_charac_level() == GetCurCharacMaxEquipLevel()) {
            SetCurCharacMaxEquipLevel(0);
        }

        getCurCharacQuestW()->ResetUrgentQuestWaitingList();
        getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&packet));
        character_view_.enableSaveCharacView();
        Send(packet);

        if (get_charac_level() == oldsrv::config::kCharacterLevelMaximum) {
            resetCharacFatigueGrownUpBuff();
        }

        _UpdateUserInfo4Guild();

        GlobalData::s_guild_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()))
            ->SendCharLevelGrowType(
                static_cast<unsigned int>(get_acc_id()),
                static_cast<unsigned int>(get_charac_no(-1)),
                static_cast<short>(get_charac_level()),
                static_cast<short>(
                    static_cast<signed char>(getCurCharacGrowType())));

        GlobalData::s_monitor_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()))
            ->SendCharLevelGrowType(
                static_cast<unsigned int>(get_acc_id()),
                static_cast<unsigned int>(get_charac_no(-1)),
                static_cast<short>(get_charac_level()),
                static_cast<short>(
                    static_cast<signed char>(getCurCharacGrowType())));

        if (VerifyPresentAvengerTitle() != 1) {
            const unsigned int charac_no =
                static_cast<unsigned int>(getCurCharacNo());
            const char* const account_id =
                NumberToString(
                    static_cast<unsigned int>(get_acc_id()),
                    0);
            cMyTrace trace(__PRETTY_FUNCTION__, 20550, 0);
            trace(
                "CUser::VerifyPresentAvengerTitle() "
                "m_id(%s), charac_no(%u)",
                account_id,
                charac_no);
        }

        AddCurCharacMercenaryInfo();
    }

    if (IsLevelExpNotificationState(state_)) {
        if (!preserve_fatigue_bonus_exp) {
            SendNotiPacket(static_cast<eSendTarget>(1),
                           static_cast<ENUM_NOTIPACKET>(37),
                           0);
            set_charac_fatigue_buf_bonus_exp(0);
        } else if (leveled_up) {
            SendNotiPacket(static_cast<eSendTarget>(1),
                           static_cast<ENUM_NOTIPACKET>(37),
                           0);
        }

        if (leveled_up) {
            decide_growth_power_reward_system();
        }
    }

    if (leveled_up) {
        GlobalData::s_statisticMgr->level_dungeon_play_.IncreaseLevelDungeonPlay(
            get_charac_level(),
            GetCurCharacDungeonPlayCount());
        ResetCurCharacDungeonPlayCount();
        expert_job::CExpertJob::IncreaseExpertJobExp(
            this,
            GetCurCharacExpertJobExp());
        ReCalcChattingEmoticon();
        SendChattingEmoticon();
    }

    if (leveled_up &&
        get_charac_level() == oldsrv::config::kCharacterLevelMaximum) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this,
            static_cast<APSystem::_ActionGroupIndex>(0x21),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    }

    return leveled_up;
}

// IDA_EA: 0x866A9A0
// IDA_NAME: _ZN5CUser7gain_spEi
// DEMANGLED: CUser::gain_sp(int)
int CUser::gain_sp(int gained_sp) {
    SkillSlot* skill_slot = getCurCharacSkillR();
    const int first_sp =
        skill_slot->get_remain_sp_at_index(ENUM_SKILL_TREE_0);
    if (INT_MAX - gained_sp >= first_sp) {
        getCurCharacSkillW()->set_remain_sp_at_index(
            first_sp + gained_sp, ENUM_SKILL_TREE_0);
    } else {
        getCurCharacSkillW()->set_remain_sp_at_index(
            INT_MAX, ENUM_SKILL_TREE_0);
    }

    skill_slot = getCurCharacSkillR();
    const int second_sp =
        skill_slot->get_remain_sp_at_index(ENUM_SKILL_TREE_1);
    if (INT_MAX - gained_sp >= second_sp) {
        return getCurCharacSkillW()->set_remain_sp_at_index(
            second_sp + gained_sp, ENUM_SKILL_TREE_1);
    }
    return getCurCharacSkillW()->set_remain_sp_at_index(
        INT_MAX, ENUM_SKILL_TREE_1);
}

// IDA_EA: 0x866AAD2
// IDA_NAME: _ZN5CUser8gain_sfpEi
// DEMANGLED: CUser::gain_sfp(int)
void CUser::gain_sfp(int gained_sfp) {
    if (!gained_sfp) {
        return;
    }

    SkillSlot* skill_slot = getCurCharacSkillR();
    const int first_sfp =
        skill_slot->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_0);
    if (INT_MAX - gained_sfp >= first_sfp) {
        getCurCharacSkillW()->set_remain_sfp_at_index(
            first_sfp + gained_sfp, ENUM_SKILL_TREE_SFP_0);
    } else {
        getCurCharacSkillW()->set_remain_sfp_at_index(
            INT_MAX, ENUM_SKILL_TREE_SFP_0);
    }

    skill_slot = getCurCharacSkillR();
    const int second_sfp =
        skill_slot->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_1);
    if (INT_MAX - gained_sfp >= second_sfp) {
        getCurCharacSkillW()->set_remain_sfp_at_index(
            second_sfp + gained_sfp, ENUM_SKILL_TREE_SFP_1);
    } else {
        getCurCharacSkillW()->set_remain_sfp_at_index(
            INT_MAX, ENUM_SKILL_TREE_SFP_1);
    }
}

// IDA_EA: 0x866AC0E
// IDA_NAME: _ZN5CUser14history_log_spEii12eSPAddReason
// DEMANGLED: CUser::history_log_sp(int,int,eSPAddReason)
char CUser::history_log_sp(int previous_sp,
                           int gained_sp,
                           eSPAddReason reason) {
    const int remaining_sp =
        getCurCharacSkillR()->get_remain_sp_at_index(ENUM_SKILL_TREE_0);
    if (previous_sp != -1 && remaining_sp == previous_sp) {
        return 0;
    }
    return static_cast<char>(
        history_log_.SPAdd(0, remaining_sp, gained_sp, reason));
}

// IDA_EA: 0x866ACD0
// IDA_NAME: _ZN5CUser15history_log_sfpEii12eSPAddReason
// DEMANGLED: CUser::history_log_sfp(int,int,eSPAddReason)
char CUser::history_log_sfp(int previous_sfp,
                            int gained_sfp,
                            eSPAddReason reason) {
    const int remaining_sfp =
        getCurCharacSkillR()->get_remain_sfp_at_index(
            ENUM_SKILL_TREE_SFP_0);
    if (previous_sfp != -1 && remaining_sfp == previous_sfp) {
        return 0;
    }
    return static_cast<char>(
        history_log_.SFPAdd(2, remaining_sfp, gained_sfp, reason));
}
