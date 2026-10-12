#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include <cstring>
#include <list>
#include <stdint.h>

#include "core/config/env/environment.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/fatigue_battery_handle_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"
#include "old_data/db/statistics_log/db_insert_unlimit_support_log.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "old_game/activity/schedule/levelup_support_3rd.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/activity/schedule/levelup_support_script_services.h"
#include "old_game/activity/schedule/new_account_level_up_event_to_job_abi.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "infra/log/format/trace.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x84A7FF8
// IDA_NAME: _ZN21CFatigueBatteryHandleC2Ev
// DEMANGLED: CFatigueBatteryHandle::CFatigueBatteryHandle(void)
CFatigueBatteryHandle::CFatigueBatteryHandle() {}

// IDA_EA: 0x84A7FFE
// IDA_NAME: _ZN21CFatigueBatteryHandleD2Ev
// DEMANGLED: CFatigueBatteryHandle::~CFatigueBatteryHandle()
CFatigueBatteryHandle::~CFatigueBatteryHandle() {}

// IDA_EA: 0x84A8004
// IDA_NAME: _ZN21CFatigueBatteryHandle29ComputeFatigueBatteryChargingEiii
// DEMANGLED: CFatigueBatteryHandle::ComputeFatigueBatteryCharging(int,int,int)
short CFatigueBatteryHandle::ComputeFatigueBatteryCharging(
    int remaining_fatigue,
    int charac_level,
    int special_level_charac_count) {
    const stFatigueBatteryConstantTable_t& table =
        G_CDataManager()
            ->get_server_parameter_script()
            ->get_fatigue_battery_constant_table();
    const int scaled_fatigue =
        charac_level * remaining_fatigue /
        table.character_level_fatigue_battery_constance_;
    const unsigned int charging = static_cast<unsigned int>(
        static_cast<long double>(scaled_fatigue) *
        (static_cast<long double>(special_level_charac_count) *
             table.end_level_fatigue_battery_weight_ +
         1.0L));
    if (table.limit_fatigue_battery_charging_amount_ < charging) {
        return static_cast<short>(
            table.limit_fatigue_battery_charging_amount_);
    }
    return static_cast<short>(charging);
}

// IDA_EA: 0x84A8094
// IDA_NAME: _ZN21CFatigueBatteryHandle22ChargingFatigueBatteryEP5CUseri
// DEMANGLED: CFatigueBatteryHandle::ChargingFatigueBattery(CUser*,int)
short CFatigueBatteryHandle::ChargingFatigueBattery(CUser* user,
                                                     int unconnected_days) {
    const stFatigueBatteryConstantTable_t& table =
        G_CDataManager()
            ->get_server_parameter_script()
            ->get_fatigue_battery_constant_table();
    const int charac_level = user->get_charac_level();
    if (charac_level < table.limit_fatigue_battery_level_ ||
        charac_level == table.fatigue_battery_level_for_money_active_) {
        return 0;
    }

    const int special_level_charac_count =
        user->GetSpecialLevelCharacCnt(
            oldsrv::config::kFatigueBatterySpecialCharacterLevel);
    const int used_fatigue =
        static_cast<uint16_t>(user->getCurCharacUsedFatigue());
    int remaining_fatigue = DEFAULT_MAX_FATIGUE - used_fatigue;
    if (remaining_fatigue < 0) {
        remaining_fatigue = 0;
    }
    remaining_fatigue += unconnected_days * DEFAULT_MAX_FATIGUE;

    cMyTrace trace(
        "short int CFatigueBatteryHandle::ChargingFatigueBattery(CUser*, int)",
        50,
        0);
    trace("FATIGUE BATTERY : char no(%d), alreadyFatigue(%d), "
          "remainFatigue(%d), level(%d), endLevelCnt(%d), "
          "thatDayUsedFatigue(%d)",
          user->getCurCharacNo(),
          user->GetFatigueBatteryCharging(),
          remaining_fatigue,
          user->get_charac_level(),
          special_level_charac_count,
          user->getCurCharacUsedFatigue());
    return ComputeFatigueBatteryCharging(
        remaining_fatigue,
        user->get_charac_level(),
        special_level_charac_count);
}

// IDA_EA: 0x84A81F4
// IDA_NAME: _ZN21CFatigueBatteryHandle29ActiveFatigueBattery_EndLevelEP5CUser
// DEMANGLED:
// CFatigueBatteryHandle::ActiveFatigueBattery_EndLevel(CUser*)
short CFatigueBatteryHandle::ActiveFatigueBattery_EndLevel(CUser* user) {
    short active = static_cast<short>(user->GetFatigueBatteryCharging());
    CDataManager* const data_manager = G_CDataManager();
    (void)data_manager;

    if (static_cast<short>(user->GetFatigueBatteryCharging()) != active) {
        cMyTrace trace(__PRETTY_FUNCTION__, 119, 0);
        trace("FATIGUE BATTERY ACTIVE ERR : char no(%d), usedFatigue(%d)",
              user->getCurCharacNo(),
              active);
        active = static_cast<short>(user->GetFatigueBatteryCharging());
    }
    return active;
}

// IDA_EA: 0x84A829E
// IDA_NAME: _ZN21CFatigueBatteryHandle32ActiveFatigueBattery_NotEndLevelEP5CUser
// DEMANGLED: CFatigueBatteryHandle::ActiveFatigueBattery_NotEndLevel(CUser*)
short CFatigueBatteryHandle::ActiveFatigueBattery_NotEndLevel(CUser* user) {
    short active = 0;
    const int charging =
        static_cast<short>(user->GetFatigueBatteryCharging());
    const stFatigueBatteryConstantTable_t& table =
        G_CDataManager()
            ->get_server_parameter_script()
            ->get_fatigue_battery_constant_table();
    if (table.min_fatigue_battery_per_buff_active_ <= charging) {
        if (!table.fatigue_battery_per_grownup_buff_active_) {
            return active;
        }

        const int added_grownup_buff =
            charging * table.grownup_buff_per_fatigue_battery_active_ /
            table.fatigue_battery_per_grownup_buff_active_;
        active = static_cast<short>(charging);
        int grownup_buff = added_grownup_buff +
            static_cast<uint16_t>(user->getCurCharacFatigueGrownUpBuff());
        const uint16_t maximum_grownup_buff =
            static_cast<uint16_t>(
                table.getLimitGrownupBuffCntFatigueBattery(
                    user->get_charac_level()));
        if (maximum_grownup_buff < grownup_buff) {
            grownup_buff = maximum_grownup_buff;
        }

        Packet_Fatigue_Battery_Money_Statistic packet;
        packet.server_group_ = static_cast<uint8_t>(
            G_CEnvironment()->get_server_group());
        packet.grownup_buff_count_ = static_cast<uint16_t>(grownup_buff);
        CStatisticServerProxy* const proxy =
            GlobalData::s_statistic_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(
                    G_CEnvironment()->get_server_group()));
        proxy->SendPacket(reinterpret_cast<char*>(&packet), 0x11);
        user->setCurCharacFatigueGrownUpBuff(
            static_cast<uint16_t>(grownup_buff));
    }
    return active;
}

// IDA_EA: 0x84A83DA
// IDA_NAME: _ZN21CFatigueBatteryHandle20ActiveFatigueBatteryEP5CUser
// DEMANGLED: CFatigueBatteryHandle::ActiveFatigueBattery(CUser*)
short CFatigueBatteryHandle::ActiveFatigueBattery(CUser* user) {
    const stFatigueBatteryConstantTable_t& table =
        G_CDataManager()
            ->get_server_parameter_script()
            ->get_fatigue_battery_constant_table();
    const int charac_level = user->get_charac_level();
    if (table.limit_fatigue_battery_level_ <= charac_level &&
        charac_level < table.fatigue_battery_level_for_money_active_) {
        return ActiveFatigueBattery_NotEndLevel(user);
    }
    return 0;
}

// IDA_EA: 0x84A84EE
// IDA_NAME: _ZN38Packet_Fatigue_Battery_Money_StatisticC2Ev
// DEMANGLED: Packet_Fatigue_Battery_Money_Statistic::Packet_Fatigue_Battery_Money_Statistic(void)
Packet_Fatigue_Battery_Money_Statistic::
    Packet_Fatigue_Battery_Money_Statistic()
    : header_(0xC4Au, 0x11u),
      server_group_(0),
      reserved_0b_(0),
      grownup_buff_count_(0) {}

// IDA_EA: 0x8656636
// IDA_NAME: _ZN5CUser24GetSpecialLevelCharacCntEi
// DEMANGLED: CUser::GetSpecialLevelCharacCnt(int)
int CUser::GetSpecialLevelCharacCnt(int level) {
    int count = 0;
    for (std::vector<_Charac_info>::const_iterator character =
             characters_.begin();
         character != characters_.end(); ++character) {
        if (character->fields_.level_ == level) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x86845B2
// IDA_NAME: _ZN15cUserHistoryLog7LevelUpEi14eLevelUpReason
// DEMANGLED: cUserHistoryLog::LevelUp(int,eLevelUpReason)
int cUserHistoryLog::LevelUp(int charac_level, eLevelUpReason reason) {
    return owner_->LogHistory("Level+,%d,%d",
                              charac_level,
                              static_cast<int>(reason));
}

// IDA_EA: 0x86845DE
// IDA_NAME: _ZN15cUserHistoryLog9LevelDownEi16eLevelDownReason
// DEMANGLED: cUserHistoryLog::LevelDown(int,eLevelDownReason)
int cUserHistoryLog::LevelDown(int charac_level, eLevelDownReason reason) {
    return owner_->LogHistory("Level-,%d,%d",
                              charac_level,
                              static_cast<int>(reason));
}

// IDA_EA: 0x868460A
// IDA_NAME: _ZN15cUserHistoryLog14ChangeGrowTypeEiiiii21eChangeGrowTypeReason
// DEMANGLED: cUserHistoryLog::ChangeGrowType(int,int,int,int,int,eChangeGrowTypeReason)
int cUserHistoryLog::ChangeGrowType(int charac_job,
                                    int village,
                                    int old_grow_type,
                                    int new_grow_type,
                                    int value,
                                    eChangeGrowTypeReason reason) {
    return owner_->LogHistory("CP,%d,%d,%d,%d,%d,%d",
                              charac_job,
                              village,
                              old_grow_type,
                              new_grow_type,
                              value,
                              static_cast<int>(reason));
}

// IDA_EA: 0x86847EA
// IDA_NAME: _ZN15cUserHistoryLog8SendMailEP16MSG_MAILBOX_SENDj
// DEMANGLED: cUserHistoryLog::SendMail(MSG_MAILBOX_SEND*,unsigned int)
void cUserHistoryLog::SendMail(MSG_MAILBOX_SEND* message,
                               unsigned int value) {
    if (!message) {
        return;
    }

    owner_->LogHistory(
        "MailS,%s,%d,%d,%d,%d,%d,\"%s\",%s",
        message->receiver_name,
        message->money,
        static_cast<unsigned int>(message->inventory_type),
        static_cast<unsigned int>(message->inventory_slot),
        message->item_id,
        message->item_add_info,
        message->contents,
        NumberToString(value, 0));
}

// IDA_EA: 0x86646C8
// IDA_NAME: _ZN5CUser19_update_charac_statEb
// DEMANGLED: CUser::_update_charac_stat(bool)
_Additioal_info* CUser::_update_charac_stat(bool add_stat) {
    const int second_grow_type = getCurCharSecondGrowType();
    const int first_grow_type = getCurCharFirstGrowType();
    CDataManager* const data_manager = G_CDataManager();
    CCharacList* const character_list = data_manager->get_character_list();
    const int charac_job = get_charac_job();
    CCharacter* const character = &character_list->characters_[charac_job];
    _Additioal_info* const growtype_stat =
        character->get_growtype_stat(first_grow_type, second_grow_type);
    _Additioal_info* const current_stat = getCurCharacAddInfoW();

    if (add_stat) {
        return current_stat->add(*growtype_stat);
    }
    return current_stat->subtract(*growtype_stat);
}

// IDA_EA: 0x8664AE8
// IDA_NAME: _ZN5CUser13calc_lev_statER15_Additioal_info
// DEMANGLED: CUser::calc_lev_stat(_Additioal_info &)
bool CUser::calc_lev_stat(_Additioal_info& calculated_stat) {
    CDataManager* const data_manager = G_CDataManager();
    CCharacList* const character_list = data_manager->get_character_list();
    CCharacter* const character =
        &character_list->characters_[get_charac_job()];

    // IDA 0x8664B17 passes the first physical CCharacter member directly to
    // _Additioal_info::add.
    calculated_stat.add(character->base_status_[0]);

    const _Additioal_info* const first_stage_stat =
        character->get_growtype_stat(0, 0);
    const _Additioal_info* const second_stage_stat =
        character->get_growtype_stat(getCurCharFirstGrowType(), 0);
    const _Additioal_info* const third_stage_stat =
        character->get_growtype_stat(getCurCharFirstGrowType(),
                                     getCurCharSecondGrowType());

    bool above_level = false;
    for (int level = 1;; ++level) {
        above_level = get_charac_level() > level;
        if (!above_level) {
            break;
        }

        if (level <= 14) {
            calculated_stat.add(*first_stage_stat);
        } else if (level <= 49) {
            calculated_stat.add(*second_stage_stat);
        } else {
            calculated_stat.add(*third_stage_stat);
        }
    }
    return above_level;
}

// IDA_EA: 0x8664766
// IDA_NAME: _ZN5CUser18adjust_charac_statEv
// DEMANGLED: CUser::adjust_charac_stat(void)
int CUser::adjust_charac_stat() {
    _Additioal_info calculated_stat;
    calculated_stat.clear();
    calc_lev_stat(calculated_stat);

    _Additioal_info* const before = getCurCharacAddInfoRefR();
    cMyTrace before_trace(__PRETTY_FUNCTION__, 17617, 0);
    before_trace(
        "before adjust stat(%s(%d) : %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
        getCurCharacName(),
        getCurCharacNo(),
        before->max_hp,
        before->max_mp,
        before->physical_attack,
        before->physical_defense,
        before->magical_attack,
        before->magical_defense,
        before->move_speed,
        before->element_resist[0],
        before->element_resist[1],
        before->element_resist[2],
        before->element_resist[3]);

    _Additioal_info* const current = getCurCharacAddInfoRefW();
    std::memcpy(current, &calculated_stat, sizeof(calculated_stat));

    cMyTrace after_trace(__PRETTY_FUNCTION__, 17623, 0);
    return after_trace(
        "after adjust stat(%s(%d) : %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
        getCurCharacName(),
        getCurCharacNo(),
        current->max_hp,
        current->max_mp,
        current->physical_attack,
        current->physical_defense,
        current->magical_attack,
        current->magical_defense,
        current->move_speed,
        current->element_resist[0],
        current->element_resist[1],
        current->element_resist[2],
        current->element_resist[3]);
}

// IDA_EA: 0x8664080
// IDA_NAME: _ZN5CUser17_onChangeGrowTypeEhh21eChangeGrowTypeReason
// DEMANGLED: CUser::_onChangeGrowType(unsigned char,unsigned char,eChangeGrowTypeReason)
unsigned int CUser::_onChangeGrowType(unsigned char first_grow_type,
                                      unsigned char second_grow_type,
                                      eChangeGrowTypeReason reason) {
    CEventBase* const achievement_event =
        GlobalData::s_event_manager->GetRepeatEvent(21);
    if (achievement_event->IsEventing(0) &&
        static_cast<int>(reason) != 1) {
        DB_InsertArchieveEventLog::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            static_cast<unsigned int>(getCurCharacNo()),
            static_cast<SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE>(
                second_grow_type ? 11 : 10));
    }

    const unsigned int server_reward_open =
        G_CEnvironment()->get_server_group() == 10 &&
        static_cast<unsigned int>(GlobalData::s_systemTime_.getCurSec()) <=
            0x478E7050u;
    if (server_reward_open != 0u) {
        if (static_cast<int>(reason) == 1 || second_grow_type != 0u) {
            return server_reward_open;
        }

        char contents[256];
        std::strncpy(contents,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_13", 0),
                     0xffu);
        Inven_Item item;
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            g_scriptStringManager_.findString(4, "game_server_msg_09", 0),
            item,
            150000,
            static_cast<unsigned int>(getCurCharacNo()),
            contents,
            static_cast<int>(std::strlen(contents)),
            0,
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
            false,
            false);
    }

    const unsigned int premium_reward_open =
        isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(16)) &&
        static_cast<unsigned int>(GlobalData::s_systemTime_.getCurSec()) <=
            0x46E1BBD0u;
    if (premium_reward_open != 0u && static_cast<int>(reason) != 1 &&
        second_grow_type == 0u) {
        const Inven_Item* const equipped =
            getCurCharacInvenR()->GetInvenRef(
                0, oldsrv::config::kEquipmentTypeTitle);
        if (static_cast<unsigned int>(equipped->getKey() - 26070) <= 4u) {
            char contents[256];
            std::strncpy(contents,
                         g_scriptStringManager_.findString(
                             4, "game_server_msg_07", 0),
                         0xffu);

            Inven_Item item;
            item.item_id_ = 1085;
            item.item_category_ = 2u;
            item.set_add_info(20);
            item.ResetItemAttr();
            return static_cast<unsigned int>(
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    g_scriptStringManager_.findString(
                        4, "game_server_msg_08", 0),
                    item,
                    0,
                    static_cast<unsigned int>(getCurCharacNo()),
                    contents,
                    static_cast<int>(std::strlen(contents)),
                    0,
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                    false,
                    false));
        }
    }
    return premium_reward_open;
}

// IDA_EA: 0x867925A
// IDA_NAME: _ZN5CUser20processGrowTypeEventEhh21eChangeGrowTypeReason
// DEMANGLED: CUser::processGrowTypeEvent(unsigned char,unsigned char,eChangeGrowTypeReason)
void CUser::processGrowTypeEvent(unsigned char first_grow_type,
                                 unsigned char second_grow_type,
                                 eChangeGrowTypeReason reason) {
    if (static_cast<int>(reason) == 1) {
        return;
    }

    _onChangeGrowType(first_grow_type, second_grow_type, reason);
    G_CDataManager()->get_event_script_mng()->process_level_up_reward(this,
                                                                        true);
    if (second_grow_type == 0u) {
        processLevelUpEventReward(true);
    }
}

// IDA_EA: 0x86689EE
// IDA_NAME: _ZN5CUser21_postQuestTriggerItemEPK5Quest
// DEMANGLED: CUser::_postQuestTriggerItem(Quest const*)
char CUser::_postQuestTriggerItem(const Quest* quest) {
    if (quest->mail_enclose.empty()) {
        Inven_Item item;
        return static_cast<char>(
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                quest->mail_title.c_str(),
                item,
                0,
                static_cast<unsigned int>(getCurCharacNo()),
                quest->mail_contents.c_str(),
                static_cast<int>(quest->mail_contents.size()),
                0,
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                false,
                false));
    }

    for (std::vector<int>::const_iterator current =
             quest->mail_enclose.begin();
         current != quest->mail_enclose.end();) {
        const int item_id = *current++;
        const int count = *current;
        Inven_Item item;
        if (item_id == 0 && count > 0) {
            // IDA 0x8668B20..0x8668BD8 takes the money-only path with the
            // freshly constructed Inven_Item unchanged.  The count is the
            // mail's money argument, not an item add-info value.
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                quest->mail_title.c_str(),
                item,
                static_cast<unsigned int>(count),
                static_cast<unsigned int>(getCurCharacNo()),
                quest->mail_contents.c_str(),
                static_cast<int>(quest->mail_contents.size()),
                0,
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                false,
                false);
        } else {
            CItem* const prototype = G_CDataManager()->find_item(item_id);
            if (prototype) {
                prototype->vtable_->make_item(prototype, item);
                item.item_id_ = item_id;
                item.set_add_info(count);
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    quest->mail_title.c_str(),
                    item,
                    0,
                    static_cast<unsigned int>(getCurCharacNo()),
                    quest->mail_contents.c_str(),
                    static_cast<int>(quest->mail_contents.size()),
                    0,
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                    false,
                    false);
            }
        }
        ++current;
    }
    return 0;
}

// IDA_EA: 0x8663CC0
// IDA_NAME: _ZN5CUser25processLevelUpEventRewardEb
// DEMANGLED: CUser::processLevelUpEventReward(bool)
int CUser::processLevelUpEventReward(bool growup) {
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(93);
    const int result = event->IsEventing(0) ^ 1;
    if (result) {
        return result;
    }

    const LevelUpRewardItemTableKey key(
        get_charac_level(),
        get_charac_job(),
        getCurCharFirstGrowType());
    std::vector<LevelUpRewardItemData> rewards;
    // The original GCC 4.9 i386 vector D2 chain returns its this pointer in
    // EAX (0x81B8C14 -> 0x81B933A -> 0x81B9312).  All active-event exits
    // return that exact value rather than a logical success flag.
    const int vector_d2_result =
        static_cast<int>(reinterpret_cast<intptr_t>(&rewards));
    stLevelUpRewardItemScript* const script =
        G_CDataManager()->get_level_up_reward_item_script();
    if (growup) {
        if (script->get_data_when_growup(key, rewards) != 1) {
            return vector_d2_result;
        }
    } else {
        LevelUpRewardItemData reward;
        if (script->get_data(key, reward) != 1) {
            return vector_d2_result;
        }
        rewards.push_back(reward);
    }

    char contents[256] = {};
    std::strncpy(
        contents,
        g_scriptStringManager_.findString(
            4, "game_server_msg_188", 0),
        sizeof(contents) - 1);

    for (std::vector<LevelUpRewardItemData>::const_iterator reward =
             rewards.begin();
         reward != rewards.end();
         ++reward) {
        Inven_Item item;
        item.item_id_ = reward->item_id;
        G_CDataManager()->get_item_list()->create_item(
            ENUM_ITEM_CREATE_TYPE_DEFAULT, item, reward->count);

        const char* const title =
            g_scriptStringManager_.findString(
                4, "game_server_msg_187", 0);
        if (!WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                item,
                0,
                static_cast<unsigned int>(getCurCharacNo()),
                contents,
                static_cast<int>(std::strlen(contents)),
                0,
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                false,
                false)) {
            MSG_MAILBOX_SEND message = {};
            std::strncpy(message.receiver_name, title, 0x14u);
            message.item_id = item.getKey();
            message.item_add_info = item.get_add_info();
            history_log().SendMail(&message, 0);
        }
    }
    return vector_d2_result;
}

// IDA_EA: 0x869115A
// IDA_NAME: _ZN5CUser19processLevelUpEventEv
// DEMANGLED: CUser::processLevelUpEvent(void)
void CUser::processLevelUpEvent() {
    CEventBase* event =
        GlobalData::s_event_manager->GetRepeatEvent(104);
    if (event->IsEventing(0)) {
        NewAccountLevelUpEventToJob* const new_account_event =
            static_cast<NewAccountLevelUpEventToJob*>(event);
        if (new_account_event) {
            new_account_event->processLevelUpToJob(*this, false);
        }
    }

    event = GlobalData::s_event_manager->GetRepeatEvent(158);
    if (event->IsEventing(0)) {
        ARAD::Singleton<LevelupSupportEventManger>::Get()->
            UpdateEventInfo(this);
    }

    event = GlobalData::s_event_manager->GetRepeatEvent(163);
    if (event->IsEventing(0)) {
        ARAD::Singleton<LevelupSupport2ndEventManger>::Get()->
            Process(this);
    }

    ARAD::Singleton<HeroMissionEvent>::Get()->processMission(
        this, HeroMissionCondition::MissionNo::LEVEL_UP, 0);
}

// IDA_EA: 0x866311A
// IDA_NAME: _ZN5CUser10_onLevelUpEv
// DEMANGLED: CUser::_onLevelUp(void)
void CUser::_onLevelUp() {
    if (get_charac_level() == 15 &&
        getCurCharacCreateTime() > 1203541199 &&
        GlobalData::s_systemTime_.getCurSec() <= 1205074800) {
        DB_InsertUnlimitSupportLog::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            static_cast<unsigned int>(getCurCharacNo()),
            getCurCharacName());
    }

    _processLevelReward(
        static_cast<unsigned int>(get_charac_level()),
        static_cast<unsigned int>(getCurCharacNo()),
        static_cast<char>(get_charac_job()));

    CEventBase* const achievement_event =
        GlobalData::s_event_manager->GetRepeatEvent(21);
    if (achievement_event->IsEventing(0)) {
        if (get_charac_level() == 18) {
            DB_InsertArchieveEventLog::makeRequest(
                static_cast<unsigned int>(get_acc_id()),
                static_cast<unsigned int>(getCurCharacNo()),
                static_cast<
                    SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE>(1));
        }
        if (get_charac_level() == 48) {
            DB_InsertArchieveEventLog::makeRequest(
                static_cast<unsigned int>(get_acc_id()),
                static_cast<unsigned int>(getCurCharacNo()),
                static_cast<
                    SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE>(2));
        }
    }

    if (G_CEnvironment()->get_server_group() == 10 &&
        static_cast<unsigned int>(
            GlobalData::s_systemTime_.getCurSec()) <= 0x478e7050u &&
        (get_charac_level() == 30 || get_charac_level() == 40)) {
        Inven_Item item;
        char contents[256] = {};
        if (get_charac_level() == 30) {
            std::strncpy(
                contents,
                g_scriptStringManager_.findString(
                    4, "game_server_msg_11", 0),
                0xffu);
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                g_scriptStringManager_.findString(
                    4, "game_server_msg_09", 0),
                item,
                500000,
                static_cast<unsigned int>(getCurCharacNo()),
                contents,
                static_cast<int>(std::strlen(contents)),
                0,
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                false,
                false);
        } else {
            std::strncpy(
                contents,
                g_scriptStringManager_.findString(
                    4, "game_server_msg_12", 0),
                0xffu);
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                g_scriptStringManager_.findString(
                    4, "game_server_msg_09", 0),
                item,
                1000000,
                static_cast<unsigned int>(getCurCharacNo()),
                contents,
                static_cast<int>(std::strlen(contents)),
                0,
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                false,
                false);
        }
    }

    std::list<int> mail_quests;
    stSelectQuestParam select_param(this);
    if (getCurCharacQuestR()->get_mail_quest_info(
            mail_quests, select_param)) {
        for (std::list<int>::const_iterator current =
                 mail_quests.begin();
             current != mail_quests.end();
             ++current) {
            Quest* const quest = G_CDataManager()->find_quest(*current);
            if (quest && !quest->mail_time &&
                get_charac_level() == quest->minimum_level) {
                _postQuestTriggerItem(quest);
            }
        }
    }

    if (isAffectedPremium(
            static_cast<ENUM_PREMIUM_TYPE>(16)) &&
        static_cast<unsigned int>(
            GlobalData::s_systemTime_.getCurSec()) <= 0x46e1bbd0u) {
        const Inven_Item* const equipped =
            getCurCharacInvenR()->GetInvenRef(
                0, oldsrv::config::kEquipmentTypeTitle);
        if (static_cast<unsigned int>(
                equipped->getKey() - 26070) <= 4u) {
            char contents[256] = {};
            if (get_charac_level() == 30) {
                std::strncpy(
                    contents,
                    g_scriptStringManager_.findString(
                        4, "game_server_msg_05", 0),
                    0xffu);
                Inven_Item item;
                item.item_id_ = 15;
                item.item_category_ = 2u;
                item.set_add_info(5);
                item.ResetItemAttr();
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    g_scriptStringManager_.findString(
                        4, "game_server_msg_08", 0),
                    item,
                    0,
                    static_cast<unsigned int>(getCurCharacNo()),
                    contents,
                    static_cast<int>(std::strlen(contents)),
                    0,
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                    false,
                    false);
            } else if (get_charac_level() == 40) {
                std::strncpy(
                    contents,
                    g_scriptStringManager_.findString(
                        4, "game_server_msg_06", 0),
                    0xffu);
                Inven_Item item;
                item.item_id_ = 36;
                item.item_category_ = 2u;
                item.set_add_info(5);
                item.ResetItemAttr();
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    g_scriptStringManager_.findString(
                        4, "game_server_msg_08", 0),
                    item,
                    0,
                    static_cast<unsigned int>(getCurCharacNo()),
                    contents,
                    static_cast<int>(std::strlen(contents)),
                    0,
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
                    false,
                    false);
            }
        }
    }

    processNPCGiftOnLevelUp();
    processLevelUpEventReward(false);
    processLevelUpEvent();
    decide_levelup_event::OnLevelUp(this);
    levelup_support_3rd::OnLevelUp(this);

    if (get_charac_level() == oldsrv::config::kCharacterLevelMaximum) {
        const short active =
            GlobalData::s_fatigueBatteryHandle_->
                ActiveFatigueBattery_EndLevel(this);
        if (active) {
            DecFatigueBatteryCharging(active);
            const int charging = GetFatigueBatteryCharging();
            cMyTrace trace("void CUser::_onLevelUp()", 17040, 0);
            trace(
                "FATIGUE BATTERY ACTIVE END_LEVEL: charno(%d) , "
                "descFatigueAmount(%d), cur charging(%d)",
                getCurCharacNo(),
                static_cast<int>(active),
                charging);
        }
    }

    if (get_charac_level() == 20) {
        CItem* const prototype = G_CDataManager()->find_item(4183);
        if (prototype) {
            Inven_Item item;
            int slot = getCurCharacInvenR()->GetInvenData(4183, item);
            if (slot >= 0) {
                if (item.get_add_info() <= 2) {
                    const int previous = item.get_add_info();
                    history_log().ItemAdd(
                        static_cast<INVEN_TYPE>(1),
                        3 - previous,
                        previous,
                        item,
                        static_cast<eItemAddReason>(13));
                    item.set_add_info(3);
                    getCurCharacInvenW()->update_item(
                        static_cast<INVEN_TYPE>(1), slot, item);
                }
            } else {
                prototype->vtable_->make_item(prototype, item);
                item.set_add_info(3);
                item.item_id_ = 4183;
                slot = getCurCharacInvenW()->insertItemIntoInventory(
                    item,
                    static_cast<eItemAddReason>(13),
                    true,
                    false);
            }
            if (slot >= 0) {
                SendUpdateItemList(
                    static_cast<eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    slot);
            }
        }
    }

    G_CDataManager()->get_event_script_mng()->
        process_level_up_reward(this, false);
}
