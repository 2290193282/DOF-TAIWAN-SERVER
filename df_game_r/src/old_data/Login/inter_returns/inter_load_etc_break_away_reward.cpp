#include "old_data/Login/inter_returns/inter_load_etc_break_away_reward.h"

#include "common/config.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/fatigue_battery_handle_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/town/town_script.h"
#include "old_data/db/account/db_load_break_away_reward_data.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/event/db_insert_attendance_event.h"
#include "old_data/db/event/reward_event_request.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "old_data/db/pvp_party/db_save_user_handicap.h"
#include "old_data/db/skill/db_update_one_day_lethe_skill.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/growth_creature_event.h"
#include "old_game/activity/schedule/growth_equip_event.h"
#include "old_game/activity/schedule/seria_room_ani_deco_event.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/avatar_coin.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

const char* GetBreakAwayScriptString(const char* key) {
    const char* const result = g_scriptStringManager_.findString(4, key, 0);
    return result ? result : "";
}

const char* GetBreakAwayRewardText(int reward_order) {
    switch (reward_order) {
        case 0:
            return GetBreakAwayScriptString("game_server_msg_136");
        case 1:
            return GetBreakAwayScriptString("game_server_msg_158");
        case 2:
            return GetBreakAwayScriptString("game_server_msg_142");
        case 3:
            return GetBreakAwayScriptString("game_server_msg_159");
        case 4:
            return GetBreakAwayScriptString("game_server_msg_160");
        default:
            return 0;
    }
}

}  // namespace

// IDA_EA: 0x84D94C4
// IDA_NAME: _ZN13Inter_LoadEtc18ProcessOneDayLetheEP12SIG_LOAD_ETCP5CUserRi
// DEMANGLED: Inter_LoadEtc::ProcessOneDayLethe(SIG_LOAD_ETC *,CUser *,int &)
int Inter_LoadEtc::ProcessOneDayLethe(SIG_LOAD_ETC* payload,
                                      CUser* user,
                                      int& should_init_skill) {
    user->SetOneDayLetheFlag(payload->premium_lethe.lethe_state);
    CPremiumLetheManager* const manager =
        G_CGameManager()->GetPremiumLetheManager();
    if (!manager) {
        return 0;
    }

    const uint8_t lethe_state = payload->premium_lethe.lethe_state;
    if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33)) &&
        (lethe_state == CPremiumLetheManager::USE_LETHE_STATE ||
         lethe_state == CPremiumLetheManager::SKILL_CONFIRM_STATE)) {
        if (manager->BackToPre(user,
                               payload->premium_lethe.mastered_skills,
                               ENUM_SKILL_TREE_0) != 1) {
            return 16582;
        }
        if (lethe_state != CPremiumLetheManager::SKILL_CONFIRM_STATE) {
            should_init_skill = 1;
            user->SetIsInitSkillFlag(false);
        }

        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                               "InterDispatcher.cpp", 16594),
                           true);
        *guard << 348;
        *guard << user->GetUID();
        SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* const result =
            guard->GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>();
        result->account_id = static_cast<uint32_t>(user->get_acc_id());
        result->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        std::memset(result->skill_slot, 0, sizeof(result->skill_slot));
        result->lethe_flag = CPremiumLetheManager::NOT_USE_LETHE_STATE;
        result->state = CPremiumLetheManager::NOT_USE_LETHE_STATE;
        result->skill_tree_kind = ENUM_SKILL_TREE_0;
        // This exact 0x199-byte wire span is the typed skill-slot plus
        // lethe-flag subrecord; SaveBackupSkillInfo owns the copy semantics.
        user->SaveBackupSkillInfo(
            reinterpret_cast<SIG_LOAD_PREMIUM_LETHE*>(result->skill_slot),
            ENUM_SKILL_TREE_0);
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    } else if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33)) &&
               (payload->one_day_lethe_state ==
                    CPremiumLetheManager::NOT_USE_LETHE_STATE ||
                payload->one_day_lethe_state ==
                    CPremiumLetheManager::SKILL_CONFIRM_STATE)) {
        manager->BackupSkill(user, ENUM_SKILL_TREE_0);
        user->history_log().SkillSPInfoSave(
            user, static_cast<eSkillSPInfoSaveReason>(0));
        user->history_log().SkillSave(
            user->getCurCharacSkillR()->get_mastered_skill_tree(
                ENUM_SKILL_TREE_0),
            static_cast<unsigned int>(user->get_charac_job()),
            ENUM_SKILL_TREE_0,
            static_cast<eSkillSaveReason>(1));
    }
    return 0;
}

// IDA_EA: 0x84D98A4
// IDA_NAME: _ZN13Inter_LoadEtc21ProcessOneDayLethe2NDEP12SIG_LOAD_ETCP5CUserRi
// DEMANGLED: Inter_LoadEtc::ProcessOneDayLethe2ND(SIG_LOAD_ETC *,CUser *,int &)
int Inter_LoadEtc::ProcessOneDayLethe2ND(SIG_LOAD_ETC* payload,
                                         CUser* user,
                                         int& should_init_skill) {
    user->SetOneDayLetheFlag2ND(payload->premium_lethe_second.lethe_state);
    CPremiumLetheManager* const manager =
        G_CGameManager()->GetPremiumLetheManager();
    if (!manager) {
        return 0;
    }

    const uint8_t lethe_state = payload->premium_lethe_second.lethe_state;
    if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33)) &&
        (lethe_state == CPremiumLetheManager::USE_LETHE_STATE ||
         lethe_state == CPremiumLetheManager::SKILL_CONFIRM_STATE)) {
        if (manager->BackToPre(user,
                               payload->premium_lethe_second.mastered_skills,
                               ENUM_SKILL_TREE_1) != 1) {
            return 16657;
        }
        if (lethe_state != CPremiumLetheManager::SKILL_CONFIRM_STATE) {
            should_init_skill = 1;
            user->SetIsInitSkillFlag2ND(false);
        }

        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                               "InterDispatcher.cpp", 16669),
                           true);
        *guard << 348;
        *guard << user->GetUID();
        SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* const result =
            guard->GetInBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>();
        result->account_id = static_cast<uint32_t>(user->get_acc_id());
        result->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        std::memset(result->skill_slot, 0, sizeof(result->skill_slot));
        result->lethe_flag = CPremiumLetheManager::NOT_USE_LETHE_STATE;
        result->state = CPremiumLetheManager::NOT_USE_LETHE_STATE;
        result->skill_tree_kind = ENUM_SKILL_TREE_1;
        user->SaveBackupSkillInfo(
            reinterpret_cast<SIG_LOAD_PREMIUM_LETHE*>(result->skill_slot),
            ENUM_SKILL_TREE_1);
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    } else if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33)) &&
               (payload->one_day_lethe_second_state ==
                    CPremiumLetheManager::NOT_USE_LETHE_STATE ||
                payload->one_day_lethe_second_state ==
                    CPremiumLetheManager::SKILL_CONFIRM_STATE)) {
        manager->BackupSkill(user, ENUM_SKILL_TREE_1);
        user->history_log().SkillSPInfoSave(
            user, static_cast<eSkillSPInfoSaveReason>(0));
        user->history_log().SkillSave(
            user->getCurCharacSkillR()->get_mastered_skill_tree(
                ENUM_SKILL_TREE_1),
            static_cast<unsigned int>(user->get_charac_job()),
            ENUM_SKILL_TREE_1,
            static_cast<eSkillSaveReason>(1));
    }
    return 0;
}

// IDA_EA: 0x84E9CB6
// IDA_NAME: _ZN13Inter_LoadEtcC2Ev
// DEMANGLED: Inter_LoadEtc::Inter_LoadEtc(void)
//
// The original body only constructs ISigDispatcher and installs this
// receiver's vptr.  There is no derived state beyond the base vptr.
Inter_LoadEtc::Inter_LoadEtc() : ISigDispatcher() {}

// IDA_EA: 0x84E9CD2 / 0x84E9D02
// IDA_NAME: _ZN13Inter_LoadEtcD2Ev / _ZN13Inter_LoadEtcD0Ev
// DEMANGLED: Inter_LoadEtc::~Inter_LoadEtc()
//
// An out-of-line virtual destructor emits the real D2 body and the compiler
// generated deleting D0 wrapper, which calls D2 then operator delete.
Inter_LoadEtc::~Inter_LoadEtc() {}

// IDA_EA: 0x84C266E
// IDA_NAME: _ZN13Inter_LoadEtc18RequestDBEventAradEP5CUser
// DEMANGLED: Inter_LoadEtc::RequestDBEventArad(CUser *)
int Inter_LoadEtc::RequestDBEventArad(CUser* user) {
    arad::SigLoadRewardEventItem payload;
    payload.set(static_cast<uint32_t>(user->get_acc_id()));

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "InterDispatcher.cpp", 4499),
                       true);
    ARAD::DISPATCHER::make_internal_stream_jpn(
        guard,
        ARAD::kInternalPacketLoadRewardEventItem,
        user->GetUID());
    guard.put_binary(&payload, sizeof(payload));
    return GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x84C2760
// IDA_NAME: _ZN13Inter_LoadEtc22_checkUserDisadvantageEP5CUser
// DEMANGLED: Inter_LoadEtc::_checkUserDisadvantage(CUser *)
char Inter_LoadEtc::_checkUserDisadvantage(CUser* user) {
    if (user->IsGameMasterMode()) {
        return 1;
    }
    if (!ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            RestrictCategory::Security, 1)) {
        return 1;
    }
    if (!user->getCurCharacR() || user->get_charac_level() <= 19 ||
        !user->getStdDropRate()) {
        return 1;
    }

    bool has_cleared_quest = false;
    UserQuest* const quests = user->getCurCharacQuestR();
    for (unsigned int quest_id = 0; quest_id <= 29999u; ++quest_id) {
        if (quests->clear_quest.isClearedQuest(quest_id)) {
            has_cleared_quest = true;
            break;
        }
    }

    int reduction = 0;
    if (!has_cleared_quest) {
        reduction += 40;
        if (!user->getCurCharacCargoR()->GetItemCount()) {
            reduction += 10;
        }
    }
    const int next_rate = std::min(
        100 - std::min(100, reduction), user->getStdDropRate());
    if (user->getStdDropRate() != next_rate) {
        DB_SaveUserHandicap::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            40001,
            101,
            next_rate,
            3,
            0);
        user->setStdDropRate(next_rate);
        return 1;
    }
    return 0;
}

// IDA_EA: 0x84C2964
// IDA_NAME: _ZN13Inter_LoadEtc12SetPvpRecordEP5CUserP12SIG_LOAD_ETC
// DEMANGLED: Inter_LoadEtc::SetPvpRecord(CUser *,SIG_LOAD_ETC *)
//
// This is an independently closed prefix of the slot-32 receiver.  It must
// remain unregistered until dispatch_sig and its remaining post-load chain are
// restored; keeping it here merely preserves the real PvP state transition for
// the eventual full receiver.
int Inter_LoadEtc::SetPvpRecord(CUser* user, SIG_LOAD_ETC* payload) {
    if (user->SetPvpRecord(payload->pvp_result) == 1) {
        user->disableSavePvP();

        const int loss_grade =
            GlobalData::s_ref_pvp_grade->getPvPExpLossGrade();
        const int loss_floor =
            GlobalData::s_ref_pvp_grade->GetPvpPoint(loss_grade);
        const PvpResultType* const current = user->getPVPResultRefR();

        if (loss_grade <= current->values[5] &&
            current->values[2] > loss_floor &&
            CheckDailyScheduleTime(4,
                                   user->getCurCharacLastPlayTick(),
                                   GlobalData::s_systemTime_.getCurSec())) {
            const int reduced_exp =
                current->values[2] -
                GlobalData::s_ref_pvp_grade->getPvPExpLossExp();
            user->getPVPResultRefW()->values[2] =
                std::max(loss_floor, reduced_exp);
        }
        return 1;
    }

    user->disableSavePvP();
    return 0;
}

// IDA_EA: 0x84C2A98
// IDA_NAME: _ZN13Inter_LoadEtc13SetRankRecordEP5CUserP12SIG_LOAD_ETC
// DEMANGLED: Inter_LoadEtc::SetRankRecord(CUser *,SIG_LOAD_ETC *)
int Inter_LoadEtc::SetRankRecord(CUser* user, SIG_LOAD_ETC* payload) {
    for (uint32_t index = 0; index < payload->battle_record_count; ++index) {
        GameResultType& record = payload->battle_records[index];
        record.raw_00 = 1;
        user->update_record(record);
    }
    return 1;
}

// IDA_EA: 0x84C2B82
// IDA_NAME: _ZN13Inter_LoadEtc10SendResultEP5CUser
// DEMANGLED: Inter_LoadEtc::SendResult(CUser *)
int Inter_LoadEtc::SendResult(CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SELECT_CHARACTER);
    // CLIENT_IDA_EVIDENCE: handler sub_55E2F0 reads this official body
    // through the final u16/u16/u8 tail at 0x55E98F/0x55E9B7/0x55EB96.
    packet.put_byte(1);
    packet.put_int(user->getRegTime());
    packet.put_int(user->getCurCharacCreateTime());
    packet.put_short(user->get_unique_id());
    packet.put_short(user->getCurCharacTotalFatigue());
    packet.put_short(user->getCurCharacTotalMaxFatigue());
    packet.put_short(user->getCurCharacUsedFatigue());

    const std::vector<WongWork::SUserPremiumInfo> premium_infos =
        user->GetPremiumInfo()->GetPremiumInfoList(9999);
    int premium_count_index = packet.get_index();
    int premium_count = 0;
    packet.put_byte(0);
    const int current_sec = GlobalData::s_systemTime_.getCurSec();
    for (std::vector<WongWork::SUserPremiumInfo>::const_iterator it =
             premium_infos.begin();
         it != premium_infos.end();
         ++it) {
        const WongWork::SUserPremiumInfo& premium = *it;
        if (premium.state == 2) {
            continue;
        }

        const bool hide_pvp_skill_tree_premium =
            G_GameWorld()->IsPvPSkilTreeChannel() &&
            user->get_pvp_grade() <= 9 && premium.premium_type == 33;
        if (hide_pvp_skill_tree_premium) {
            continue;
        }

        packet.put_byte(premium.premium_type);
        packet.put_int(premium.end_time - current_sec);
        ++premium_count;
    }
    packet.put_byte(premium_count_index, premium_count);
    packet.put_int(user->GetCera());

    if (!user->getCurCharacR()) {
        cMyTrace trace("bool Inter_LoadEtc::SendResult(CUser*)", 4899, 5);
        trace("bool Inter_LoadEtc::SendResult()");
        return 0;
    }

    user->verifyDoingQuest();
    UserQuest* const quest = user->getCurCharacQuestR();
    for (int index = 0; index <= 19; ++index) {
        const uint32_t quest_id = quest->play_quest[index];
        packet.put_short(quest_id ? static_cast<int>(quest_id) : 0xFFFF);
        packet.put_int(static_cast<int>(quest->play_quest_trigger[index]));
    }
    for (int index = 0; index <= 4; ++index) {
        int32_t notify_value = 0;
        std::memcpy(&notify_value,
                    quest->quest_notify + index * sizeof(notify_value),
                    sizeof(notify_value));
        packet.put_int(notify_value);
    }

    packet.put_byte(user->getCurCharacVill());
    packet.put_byte(user->GetTutorialSkipable());

    int tutorial_count_index = packet.get_index();
    int tutorial_count = 0;
    packet.put_byte(0);
    for (unsigned int index = 0; index <= 127u; ++index) {
        if (user->getCurCharacEscaladeTutorialFlag(index)) {
            packet.put_byte(static_cast<int>(index));
            ++tutorial_count;
        }
    }
    packet.put_byte(tutorial_count_index, tutorial_count);
    packet.put_short(user->GetFatigueBatteryCharging());
    packet.put_short(user->getCurCharacFatigueGrownUpBuff());
    packet.put_byte(user->isTradePunishOverThirtyDays() != 0);
    packet.finalize(true);
    user->Send(packet);
    AvatarCoin::SendSyncPacket(user);
    return 1;
}

// IDA_EA: 0x84C311A
// IDA_NAME: _ZN13Inter_LoadEtc10SetFatigueEP5CUser
// DEMANGLED: Inter_LoadEtc::SetFatigue(CUser *)
int Inter_LoadEtc::SetFatigue(CUser* user) {
    if (!user->getCurCharacR()) {
        return 0;
    }

    const long current_tick = GlobalData::s_systemTime_.getCurSec();
    const long last_play_tick = user->getCurCharacLastPlayTick();
    if (CheckDailyScheduleTime(G_CEnvironment()->get_fatigue_time(),
                               last_play_tick,
                               current_tick)) {
        int unconnected_days = 0;
        if (last_play_tick) {
            unconnected_days = ComputeDaysScheduleTime(
                G_CEnvironment()->get_fatigue_time(),
                last_play_tick,
                GlobalData::s_systemTime_.getCurSec());
        }

        const short charging =
            GlobalData::s_fatigueBatteryHandle_->ChargingFatigueBattery(
                user,
                unconnected_days);
        if (charging > 0) {
            user->IncFatigueBatteryCharging(charging);
            const int current_charging = user->GetFatigueBatteryCharging();
            const long trace_tick = GlobalData::s_systemTime_.getCurSec();
            cMyTrace trace("bool Inter_LoadEtc::SetFatigue(CUser*)", 5018,
                           0);
            trace("FATIGUE BATTERY : charno(%d) days(%d), "
                  "lastplaytick(%d), curTick(%d), charging(%d), "
                  "cur charging(%d)",
                  user->getCurCharacNo(),
                  unconnected_days,
                  last_play_tick,
                  trace_tick,
                  charging,
                  current_charging);
        }

        if (static_cast<short>(user->GetFatigueBatteryCharging()) > 0) {
            const short active =
                GlobalData::s_fatigueBatteryHandle_->ActiveFatigueBattery(
                    user);
            if (active) {
                user->DecFatigueBatteryCharging(active);
                const int current_charging =
                    user->GetFatigueBatteryCharging();
                cMyTrace trace("bool Inter_LoadEtc::SetFatigue(CUser*)",
                               5028,
                               0);
                trace("FATIGUE BATTERY ACTIVE: charno(%d) , "
                      "descFatigueAmount(%d), cur charging(%d)",
                      user->getCurCharacNo(),
                      active,
                      current_charging);
            }
        }

        user->RecoverFatigue(0);
        user->resetCurCharacMemberBonusFatigue();
        user->ResetDailyQuest();
        user->ResetTrainingQuest();
    }
    return 1;
}

// IDA_EA: 0x84E83C2
// IDA_NAME: _ZN13Inter_LoadEtc26processGrowthCreatureEventER5CUser
// DEMANGLED: Inter_LoadEtc::processGrowthCreatureEvent(CUser &)
void Inter_LoadEtc::processGrowthCreatureEvent(CUser& user) {
    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(103);
    if (base_event->IsEventing(0)) {
        GiveGrowCreatureEvent* const event =
            static_cast<GiveGrowCreatureEvent*>(
                GlobalData::s_event_manager->GetRepeatEvent(103));
        if (event) {
            event->SendGrowthCreatureEventInfo(user);
        }
    }
}

// IDA_EA: 0x84E834A
// IDA_NAME: _ZN13Inter_LoadEtc24processGrowthWeaponEventER5CUser
// DEMANGLED: Inter_LoadEtc::processGrowthWeaponEvent(CUser &)
void Inter_LoadEtc::processGrowthWeaponEvent(CUser& user) {
    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(100);
    if (base_event->IsEventing(0)) {
        GrowthEquipEvent* const event = static_cast<GrowthEquipEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(100));
        if (event) {
            event->GiftGrowthWeaponBox(user);
            event->SendGrowthWeaponEventInfo(user);
        }
    }
}

// IDA_EA: 0x84E866C
// IDA_NAME: _ZN13Inter_LoadEtc19verifyCharacVillageEP5CUser
// DEMANGLED: Inter_LoadEtc::verifyCharacVillage(CUser *)
int Inter_LoadEtc::verifyCharacVillage(CUser* user) {
    int result = G_GameWorld()->IsPVPChannel();
    if (!result) {
        int current_village =
            static_cast<char>(user->getCurCharacVill());
        if (current_village == 1 && user->getCurCharacVillPrev() == 0 &&
            user->get_charac_level() == 1) {
            CDataManager* const data_manager = G_CDataManager();
            CCharacList* const character_list =
                data_manager ? data_manager->get_character_list() : 0;
            const int character_job = user->get_charac_job();
            if (character_list && character_job >= 0 &&
                character_job < oldsrv::config::kCharacterJobCount) {
                const int start_town =
                    character_list->characters_[character_job].get_start_town();
                if (start_town > 0 && start_town != current_village &&
                    G_GameWorld()->GetVillage(start_town)) {
                    user->setCurCharacVill(static_cast<char>(start_town));
                    current_village = start_town;
                }
            }
        }
        Village* const village = G_GameWorld()->GetVillage(current_village);
        if (!village) {
            return user->setCurCharacVill(1);
        }

        const TownPermissionRequirement* const permission =
            GetTownPermissionRequirement(current_village,
                                         user->get_charac_job());
        bool needs_fallback = user->get_charac_level() <
            (permission ? permission->required_level
                        : village->get_required_level());
        result = 0;
        if (permission) {
            if (!permission->required_quest_ids.empty()) {
                UserQuest* const quest = user->getCurCharacQuestR();
                if (!quest) {
                    result = 1;
                    needs_fallback = true;
                } else {
                    for (std::vector<int32_t>::const_iterator required_quest =
                             permission->required_quest_ids.begin();
                         required_quest !=
                             permission->required_quest_ids.end();
                         ++required_quest) {
                        if (static_cast<unsigned char>(quest->isClearQuest(
                                static_cast<unsigned int>(*required_quest))) !=
                            1) {
                            result = 1;
                            needs_fallback = true;
                            break;
                        }
                    }
                }
            }
        } else {
            result = village->get_required_quest_id();
            if (result) {
                UserQuest* const quest = user->getCurCharacQuestR();
                result = !quest ||
                         static_cast<unsigned char>(quest->isClearQuest(
                             static_cast<unsigned int>(result))) != 1;
                if (result) {
                    needs_fallback = true;
                }
            }
        }

        if (needs_fallback) {
            switch (current_village) {
                case 2:
                case 4:
                    return user->setCurCharacVill(1);
                case 3:
                case 5:
                case 11:
                case 17:
                    return user->setCurCharacVill(2);
                case 6:
                case 9:
                    return user->setCurCharacVill(3);
                default:
                    return result;
            }
        }
    }
    return result;
}

// IDA_EA: 0x84E807E
// IDA_NAME: _ZN13Inter_LoadEtc22processAttendAnceEventEP20SIG_ATTENDANCE_EVENTR5CUser
// DEMANGLED: Inter_LoadEtc::processAttendAnceEvent(SIG_ATTENDANCE_EVENT *,CUser &)
AttendanceEvent* Inter_LoadEtc::processAttendAnceEvent(
    SIG_ATTENDANCE_EVENT* payload,
    CUser& user) {
    // The original return is an opaque legacy status (0/1 or the input
    // payload cast to AttendanceEvent*); dispatch_sig deliberately ignores
    // it.  Preserve it rather than giving this side-effect helper a fake
    // success convention.
    AttendanceEvent* result = reinterpret_cast<AttendanceEvent*>(
        static_cast<uintptr_t>(payload->can_attend ^ 1u));
    if (payload->can_attend != 1) {
        return result;
    }

    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(96);
    result = reinterpret_cast<AttendanceEvent*>(
        static_cast<uintptr_t>(base_event->IsEventing(0) ^ 1));
    if (result) {
        return result;
    }

    AttendanceEvent* const event = static_cast<AttendanceEvent*>(
        GlobalData::s_event_manager->GetRepeatEvent(96));
    if (!event) {
        return 0;
    }

    const int reward_item_id = event->getRewordItem();
    CItem* const prototype = G_CDataManager()->find_item(reward_item_id);
    if (!prototype) {
        return 0;
    }

    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(1);

    char text[256] = {};
    char title[21] = {};
    std::strncpy(text,
                 g_scriptStringManager_.findChangedString(
                     4, "game_server_msg_276", 0),
                 255u);
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_275", 0),
                 20u);

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user.getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user.GetServerGroup()),
        false,
        false);
    DB_InsertAttendanceEvent::makeRequest(
        user.GetUID(),
        static_cast<unsigned int>(user.get_acc_id()),
        static_cast<unsigned int>(user.getCurCharacNo()));
    payload->can_attend = 0;
    return reinterpret_cast<AttendanceEvent*>(payload);
}

// IDA_EA: 0x84E84F6
// IDA_NAME: _ZN13Inter_LoadEtc30processCheckInvalidExpPerLevelER5CUser
// DEMANGLED: Inter_LoadEtc::processCheckInvalidExpPerLevel(CUser &)
int Inter_LoadEtc::processCheckInvalidExpPerLevel(CUser& user) {
    const int level = user.get_charac_level();
    const int level_exp = G_CDataManager()->get_level_exp(level);
    const int next_level_exp = G_CDataManager()->get_level_exp(level + 1);
    const int character_exp = user.get_charac_exp();

    if (character_exp < level_exp || character_exp > next_level_exp) {
        if (!user.isTradePunishType(1)) {
            user.SetTradePunishType(1);
            DB_SaveUserHandicap::makeRequest(
                static_cast<unsigned int>(user.get_acc_id()), 66, 11, 0, 3,
                0);
            cMyTrace trace(
                "void Inter_LoadEtc::processCheckInvalidExpPerLevel(CUser&)",
                27773,
                5);
            return trace(
                "[InvalidExp] Invalid character exp per level. "
                "(mid:%u, charac:%u, level:%d, exp:%d,%d,%d)",
                static_cast<unsigned int>(user.get_acc_id()),
                static_cast<unsigned int>(user.get_charac_no(-1)),
                level,
                character_exp,
                level_exp,
                next_level_exp);
        }
    }
    return 0;
}

// IDA_EA: 0x84D8E68
// IDA_NAME: _ZN13Inter_LoadEtc19giveBreakAwayRewardEP5CUser
// DEMANGLED: Inter_LoadEtc::giveBreakAwayReward(CUser *)
char Inter_LoadEtc::giveBreakAwayReward(CUser* user) {
    if (user->getUserBreakAwayRewardLuckPoint() <= 0) {
        user->setBreakAwayRewardData(false);
    }

    const int item_id = user->getBreakAwayRewardItemId();
    const int currency_count = user->getBreakAwayRewardAItemCnt();
    if (item_id == 0 && currency_count == 0) {
        return 0;
    }

    const char* const reward_text =
        GetBreakAwayRewardText(user->getBreakAwayRewardItemOrder());
    if (!reward_text) {
        return 0;
    }

    char title[21] = {};
    char text[256] = {};
    std::strncpy(title, GetBreakAwayScriptString("game_server_msg_135"), 20u);
    std::strncpy(text, reward_text, 255u);

    Inven_Item item;
    unsigned int mail_currency_count = 0;
    if (item_id != 0) {
        CItem* const prototype = G_CDataManager()->find_item(item_id);
        if (!prototype) {
            return 0;
        }
        item.item_id_ = item_id;
        item.item_category_ =
            static_cast<uint8_t>(prototype->vtable_->get_item_type(prototype));
        prototype->vtable_->make_item(prototype, item);
        item.set_add_info(currency_count);
    } else {
        mail_currency_count = static_cast<unsigned int>(currency_count);
        GetInstanceCirculationStatistic()->AddCirculationStatistic(
            static_cast<CIRCULATION_STATISTIC_FIELD>(16),
            user,
            mail_currency_count);
    }

    const int mail_result = WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        mail_currency_count,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        7u,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
    if (mail_result != 0) {
        return static_cast<char>(mail_result);
    }

    DB_LoadBreakAwayRewardData::makeRequest(user->GetUID(),
                                            user->get_acc_id(),
                                            user->getCurCharacNo(),
                                            *user->getBreakAwayRewards());
    user->setBreakAwayRewardItemId(0);
    user->setBreakAwayRewardAItemCnt(0);
    return 0;
}

// IDA_EA: 0x84E8428
// IDA_NAME: _ZN13Inter_LoadEtc28processSeriaRoomAniDecoEventE17seriaRoom_AniDecoR5CUser
// DEMANGLED: Inter_LoadEtc::processSeriaRoomAniDecoEvent(seriaRoom_AniDeco,CUser &)
char Inter_LoadEtc::processSeriaRoomAniDecoEvent(seriaRoom_AniDeco decoration,
                                                 CUser& user) {
    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(105);
    char result = static_cast<char>(base_event->IsEventing(0));
    if (result == 0) {
        return result;
    }

    user.setSeriaRoomAniDecoInfo(decoration);
    SeriaRoomAniDecoEvent* const event = static_cast<SeriaRoomAniDecoEvent*>(
        GlobalData::s_event_manager->GetRepeatEvent(105));
    if (!event) {
        return result;
    }

    event->SendEventInfo(user);
    if (CheckDailyScheduleTime(G_CEnvironment()->get_fatigue_time(),
                               user.getCurCharacLastPlayTick(),
                               GlobalData::s_systemTime_.getCurSec())) {
        result = static_cast<char>(event->SendAniDecoItem(user));
    }
    return result;
}
