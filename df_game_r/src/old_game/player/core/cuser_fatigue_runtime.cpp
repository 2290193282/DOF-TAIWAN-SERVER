#include "old_game/player/core/cuser.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hades_public_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_gold_control_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/activity/schedule/blue_horse_event.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/activity/schedule/packet_dispatcher_facebook_like.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/os_api.h"

#include <stdint.h>

#include <cstring>
#include <string>


namespace {

const char* FatigueScriptString(const char* key) {
    const char* const value =
        g_scriptStringManager_.findString(4, key, 0);
    return value ? value : "";
}

int SendFatigueSystemMail(CUser* user,
                          int item_id,
                          int count,
                          const char* title_key,
                          const char* message_key,
                          bool pvp_server_group,
                          unsigned int expiration,
                          bool db_work) {
    char title[21] = {};
    char message[256] = {};
    std::strncpy(title, FatigueScriptString(title_key), 20u);
    std::strncpy(message, FatigueScriptString(message_key), 255u);

    Inven_Item item;
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype) {
        return 0;
    }
    item.item_id_ = item_id;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(count);

    const int server_group =
        pvp_server_group
            ? user->GetServerGroupToPvP()
            : user->GetServerGroup();
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0u,
        static_cast<unsigned int>(user->getCurCharacNo()),
        message,
        static_cast<int>(std::strlen(message)),
        expiration,
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        db_work);
}

struct __attribute__((packed)) CharacterDayUpdatePayload {
    int32_t account_id;
    uint16_t count;
    uint16_t reserved_06;
};

static_assert(sizeof(CharacterDayUpdatePayload) == 8u,
              "character-day DB payload must retain eight bytes");

}  // namespace


// This utility has no object state in the old server.  Its static member
// spelling preserves _ZN14CUserEventUtil23updateCharacterDayEventEP5CUseri.
struct CUserEventUtil {
    static void updateCharacterDayEvent(CUser* user, int count);
};

// IDA_EA: 0x8688230
// IDA_NAME: _ZN14CUserEventUtil23updateCharacterDayEventEP5CUseri
// DEMANGLED: CUserEventUtil::updateCharacterDayEvent(CUser *,int)
void CUserEventUtil::updateCharacterDayEvent(CUser* user, int count) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("user.cpp", 39669),
        true);
    (*guard) << 0x16E;
    (*guard) << user->GetUID();

    CharacterDayUpdatePayload* const payload =
        static_cast<CharacterDayUpdatePayload*>(
            guard->GetInBufferUntyped(
                static_cast<int>(sizeof(CharacterDayUpdatePayload))));
    if (!payload) {
        return;
    }
    payload->account_id = user->get_acc_id();
    payload->count = static_cast<uint16_t>(count);
    payload->reserved_06 = 0u;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8659490
// IDA_NAME: _ZN5CUser25_processCharacterDayEventEjj
// DEMANGLED: CUser::_processCharacterDayEvent(unsigned int,unsigned int)
int CUser::_processCharacterDayEvent(unsigned int used_fatigue_before,
                                     unsigned int amount) {
    const unsigned int used_fatigue_after =
        used_fatigue_before + amount;
    if (used_fatigue_before == 0u ||
        used_fatigue_after == 0u ||
        used_fatigue_before > 156u) {
        return static_cast<int>(used_fatigue_after);
    }

    int item_id = 0;
    int item_count = 0;
    const char* message_key = "";
    const int before_group = static_cast<int>(
        static_cast<long double>(used_fatigue_before) / 30.0L);
    const int after_group = static_cast<int>(
        static_cast<long double>(used_fatigue_after) / 30.0L);

    if (before_group == after_group) {
        const int before_finish = static_cast<int>(
            static_cast<long double>(used_fatigue_before) / 156.0L);
        const int after_finish = static_cast<int>(
            static_cast<long double>(used_fatigue_after) / 156.0L);
        if (before_finish == after_finish) {
            return before_finish;
        }
        item_id = 7518;
        item_count = 1;
        message_key = "game_server_msg_75";
    } else if (before_group == 0 && after_group == 1) {
        item_id = 7516;
        item_count = 2;
        message_key = "game_server_msg_71";
    } else if (before_group == 1 && after_group == 2) {
        item_id = 7516;
        item_count = 4;
        message_key = "game_server_msg_72";
    } else if (before_group == 2 && after_group == 3) {
        item_id = 7516;
        item_count = 6;
        message_key = "game_server_msg_73";
    } else if (before_group == 3 && after_group == 4) {
        item_id = 7517;
        item_count = 1;
        message_key = "game_server_msg_74";
        CUserEventUtil::updateCharacterDayEvent(this, 1);
    }

    if (item_id == 0) {
        return static_cast<int>(used_fatigue_after);
    }
    return SendFatigueSystemMail(
        this,
        item_id,
        item_count,
        "game_server_msg_70",
        message_key,
        false,
        0u,
        false);
}

// IDA_EA: 0x86598BA
// IDA_NAME: _ZN5CUser24_processFatigueBuffEventEjj
// DEMANGLED: CUser::_processFatigueBuffEvent(unsigned int,unsigned int)
int CUser::_processFatigueBuffEvent(unsigned int used_fatigue_before,
                                    unsigned int amount) {
    const unsigned int used_fatigue_after =
        used_fatigue_before + amount;
    if (used_fatigue_before == 0u ||
        used_fatigue_after == 0u ||
        used_fatigue_before > 156u) {
        return static_cast<int>(used_fatigue_after);
    }

    const int before_group = static_cast<int>(
        static_cast<long double>(used_fatigue_before) / 50.0L);
    const int after_group = static_cast<int>(
        static_cast<long double>(used_fatigue_after) / 50.0L);
    if (before_group == after_group) {
        return before_group;
    }

    int item_count = 0;
    const char* message_key = "";
    if (before_group == 1 && after_group == 2) {
        item_count = 10;
        message_key = "game_server_msg_84";
    } else if (before_group == 2 && after_group == 3) {
        item_count = 20;
        message_key = "game_server_msg_85";
    } else {
        return before_group;
    }

    return SendFatigueSystemMail(
        this,
        7573,
        item_count,
        "game_server_msg_83",
        message_key,
        false,
        0u,
        false);
}

// IDA_EA: 0x86925C4
// IDA_NAME: _ZN5CUser20processFatigueCharacEi
// DEMANGLED: CUser::processFatigueCharac(int)
void CUser::processFatigueCharac(int amount) {
    const char job = static_cast<char>(get_charac_job());
    const int used_fatigue = static_cast<int>(
        static_cast<uint16_t>(getCurCharacUsedFatigue()));
    CDataManager* const data_manager = G_CDataManager();
    const int item_id = data_manager->get_useJobFatigueMatchGift(
        job, used_fatigue, amount);
    if (item_id == 0) {
        return;
    }

    std::string title_key;
    std::string text_key;
    data_manager->get_useJobFatigueMatchGiftMsg(
        static_cast<int>(job), title_key, text_key);

    Inven_Item item;
    CItem* const prototype = data_manager->find_item(item_id);
    if (!prototype) {
        return;
    }
    const int prototype_index = prototype->get_index();
    item.item_id_ = item_id;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(1);

    char message[256] = {};
    OS_API::snprintf(
        message,
        255u,
        FatigueScriptString(text_key.c_str()),
        static_cast<int>(
            static_cast<uint16_t>(getCurCharacUsedFatigue())));
    if (prototype_index == 0) {
        return;
    }

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        FatigueScriptString(title_key.c_str()),
        item,
        0u,
        static_cast<unsigned int>(getCurCharacNo()),
        message,
        static_cast<int>(std::strlen(message)),
        static_cast<unsigned int>(prototype->getExpirationDate()),
        static_cast<ENUM_SERVER_GROUP>(GetServerGroupToPvP()),
        false,
        true);
}

// IDA_EA: 0x8655C60
// IDA_NAME: _ZN5CUser9FatigueUpEi
// DEMANGLED: CUser::FatigueUp(int)
int CUser::FatigueUp(int fatigue) {
    if (fatigue == 0) {
        fatigue = 1;
    }

    if (getCurCharacR()) {
        const int break_away_luck =
            getUserBreakAwayRewardLuckPoint();
        if (break_away_luck > 0) {
            IncCurCharacLuckPoint(break_away_luck);
            setUserBreakAwayRewardLuckPoint(0);
            setUsedBreakAwayLuckPoint(true);
        }
    }

    getHades()->UseFatigue(static_cast<unsigned int>(fatigue));
    static_cast<Secu_GoldControl*>(
        GetGoldControl())->UseFatigue(fatigue);

    if (get_state() == 5 && getCurCharacR()) {
        IncCurCharacAssaultFatigueCount();
        if (GetCurCharacAssaultFatigueCount() > 1) {
            SubCurCharacChaosPoint(1);
            SetCurCharacAssaultFatigueCount(0);

            character_.dungeon_bonus_exp_ +=
                static_cast<uint32_t>(fatigue);
        }
    }

    creature_exp_fatigue_ = fatigue;

    const unsigned int used_fatigue_before =
        static_cast<unsigned int>(
            static_cast<uint16_t>(getCurCharacUsedFatigue()));
    incCurCharacUsedFatigue(static_cast<uint16_t>(fatigue));
    const uint16_t current_used_fatigue =
        static_cast<uint16_t>(getCurCharacUsedFatigue());
    facebook_like::OnFatigueChanged(this, current_used_fatigue);
    if (fatigue > 0) {
        freely_give_item::OnFatigueUsed(
            this, static_cast<uint32_t>(fatigue));
        blue_horse_event::OnFatigueUsed(
            this, static_cast<uint32_t>(fatigue));
    }

    if (current_used_fatigue % 50u == 0u) {
        getHackAnalyzer()->addServerHackCnt(
            this,
            static_cast<WongWork::ENUM_HACKTYPE>(0x263),
            1u,
            0u,
            0u);
    }

    if (current_used_fatigue == 156u) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this,
            static_cast<APSystem::_ActionGroupIndex>(0x16),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    }
    if (current_used_fatigue == 50u) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this,
            static_cast<APSystem::_ActionGroupIndex>(0x28),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    } else if (current_used_fatigue == 100u) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            this,
            static_cast<APSystem::_ActionGroupIndex>(0x2A),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    }

    EventClassify::CEventScriptMng* const event_scripts =
        G_CDataManager()->get_event_script_mng();
    event_scripts->inc_user_fatigue(
        this, static_cast<unsigned int>(fatigue));
    event_scripts->send_gift_fatigue_item(
        this, static_cast<unsigned int>(fatigue));
    if (event_scripts->is_eventing(411)) {
        event_scripts->send_bring_up_item_fatigue(this, fatigue);
    }

    if (isJoinedSchoolPointEvent()) {
        calcCurCharacSchoolPoint(
            static_cast<int>(used_fatigue_before));
    }

    ARAD::Singleton<HeroMissionEvent>::Get()->processMission(
        this,
        HeroMissionCondition::MissionNo::USE_FATIGUE,
        static_cast<unsigned int>(fatigue));

    CEventBase* repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(41);
    if (repeat_event && repeat_event->IsEventing(0)) {
        CCharacterDayEvent* const character_day =
            static_cast<CCharacterDayEvent*>(repeat_event);
        if (character_day->GetEventJob() == get_charac_job()) {
            _processCharacterDayEvent(
                used_fatigue_before,
                static_cast<unsigned int>(fatigue));
        }
    }

    repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(43);
    if (repeat_event && repeat_event->IsEventing(0)) {
        _processFatigueBuffEvent(
            used_fatigue_before,
            static_cast<unsigned int>(fatigue));
    }

    ARAD::Singleton<AvatarRechargeServer>::Get()->CalcDurability(
        this, fatigue);

    account_used_fatigue_ = static_cast<uint16_t>(
        account_used_fatigue_ + fatigue);

    const int charac_level = get_charac_level();
    if (charac_level != 0) {
        const int guild_weight =
            G_CDataManager()
                ->GetGuildParameterScriptForBuild()
                ->getGuildExpWeight(charac_level);
        add_guild_temp_exp((fatigue * guild_weight) / 100);
    }

    decCurCharacFatigueGrownUpBuff(
        static_cast<unsigned int>(fatigue));
    if (GetGuildLevel() + 1 > 16) {
        reset_guild_temp_exp();
    }
    processFatigueCharac(fatigue);

    if (user_premium_.isAffectedUnlimitFatigue()) {
        return getCurCharacTotalFatigue();
    }

    repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(1);
    if (repeat_event && repeat_event->IsEventing(0)) {
        IncCurCharacLuckPoint(
            G_CDataManager()
                ->GetPremiumFatigueLuckPointRateForBuild() *
            fatigue);
        return getCurCharacTotalFatigue();
    }

    repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(10);
    if (repeat_event && repeat_event->IsEventing(this)) {
        return getCurCharacTotalFatigue();
    }

    if (static_cast<uint16_t>(getCurCharacFatigue()) ==
        0xffffu) {
        return static_cast<uint16_t>(getCurCharacFatigue());
    }

    if (get_charac_memberkey() && is_connect_upper_member()) {
        add_member_pay_tex_fatigue_to_upper(fatigue);
    }
    event_scripts->process_add_fatigue_count(this, fatigue);

    uint16_t refill_fatigue = 0u;
    repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(15);
    if (repeat_event && repeat_event->IsEventing(0)) {
        refill_fatigue =
            static_cast<uint16_t>(
                static_cast<CBurningFatigueEvent*>(repeat_event)
                    ->getRefillFatigue(
                        static_cast<uint16_t>(
                            getCurCharacUsedFatigue()),
                        static_cast<uint16_t>(fatigue)));
    }

    if (IsHavePremiumAdvantage()) {
        const int premium_before =
            static_cast<uint16_t>(
                getCurCharacPremiumFatigue());
        const int premium_max =
            static_cast<uint16_t>(
                getCurCharacMaxPremiumFatigue());
        if (premium_before + fatigue <= premium_max) {
            setCurCharacPremiumFatigue(
                static_cast<uint16_t>(
                    premium_before + fatigue));
            if (refill_fatigue != 0u) {
                gainCurCharacFatigue(refill_fatigue);
                SendFatigue();
            }
            IncCurCharacLuckPoint(
                G_CDataManager()
                    ->GetPremiumFatigueLuckPointRateForBuild() *
                fatigue);
            return getCurCharacTotalFatigue();
        }
        if (premium_before < premium_max) {
            setCurCharacPremiumFatigue(
                static_cast<uint16_t>(premium_max));
            fatigue =
                premium_before + fatigue - premium_max;
        }
    }

    const int fatigue_before =
        static_cast<uint16_t>(getCurCharacFatigue());
    int fatigue_after = fatigue_before + fatigue;
    const int fatigue_max =
        static_cast<uint16_t>(getCurCharacMaxFatigue());
    if (fatigue_after > fatigue_max) {
        fatigue_after = fatigue_max;
    }
    setCurCharacFatigue(fatigue_after);
    IncCurCharacLuckPoint(
        G_CDataManager()
            ->GetNormalFatigueLuckPointRateForBuild() *
        (fatigue_after - fatigue_before));

    if (refill_fatigue != 0u) {
        gainCurCharacFatigue(refill_fatigue);
        SendFatigue();
    }
    return static_cast<uint16_t>(getCurCharacFatigue());
}
