#include "old_game/activity/schedule/event_value_types.h"
#include "common/config.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/kiri_cargo_packet_dispatchers.h"
#include "old_game/activity/schedule/rainbow_snake_packet_dispatchers.h"
#include "old_game/player/core/cuser.h"

#include <stdint.h>

#include <cstring>
#include <ctime>
#include <map>
#include <new>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/mall/purchase_cash_item_bonus_runtime.h"
#include "old_data/PVF/event/in_game_event_pvf_scripts.h"
#include "old_data/PVF/event/action_mail_script.h"
#include "old_data/PVF/event/object_bring_up_rule.h"
#include "old_data/PVF/event/event_avatar_list_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "old_data/db/avatar_aura/db_update_avatar_history.h"
#include "old_data/db/account/db_insert_account_first_login_daily_flag.h"
#include "old_data/db/account/db_insert_account_first_login_flag.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/db/auction_cashshop/db_update_purchase_cash_item_bonus.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/event/db_save_pc_room_play_time_event.h"
#include "old_data/db/event/db_select_level_up_reward.h"
#include "old_data/db/event/db_update_ingame_event_history.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/statistic_proxy_abi.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_game/activity/schedule/event_packets.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "old_game/world/region/game_world.h"


namespace {

typedef char CEventEntity_Size_Must_Be_0x1c[
    (sizeof(EventClassify::CEventEntity) == 0x1c) ? 1 : -1];
typedef char CScriptBackedEventEntity_Size_Must_Be_0x20[
    (sizeof(EventClassify::CScriptBackedEventEntity) == 0x20) ? 1 : -1];
typedef char CAccountFirstLogin_Size_Must_Be_0x20[
    (sizeof(EventClassify::CAccountFirstLogin) == 0x20) ? 1 : -1];
typedef char CAttendance_Size_Must_Be_0x20[
    (sizeof(EventClassify::CAttendance) == 0x20) ? 1 : -1];
typedef char CLevelUpReward_Size_Must_Be_0x20[
    (sizeof(EventClassify::CLevelUpReward) == 0x20) ? 1 : -1];
typedef char CAddItemCountDB_Size_Must_Be_0x20[
    (sizeof(EventClassify::CAddItemCountDB) == 0x20) ? 1 : -1];
typedef char CFatigueQuantity_Size_Must_Be_0x20[
    (sizeof(EventClassify::CFatigueQuantity) == 0x20) ? 1 : -1];
}  // namespace

struct AccountFirstLoginScript : public InGameEventScript {
    AccountFirstLoginScript() __attribute__((used, noinline));

    std::string db_table_name;
    Action_SendMail send_mail;
};

struct ConditionLevelUpBridge {
    ConditionLevelUpBridge();

    unsigned char grow_type;
    unsigned char reserved_01_[3];
    std::vector<Condition_LevelUp::Reward_Item> rewards;
};

struct LevelUpRewardScript : public InGameEventScript {
    LevelUpRewardScript() __attribute__((used, noinline));

    unsigned char recv_check_item;
    unsigned char reserved_09_[3];
    std::string db_table_name;
    std::map<int, ConditionLevelUpBridge> conditions;
    Action_SendMail send_mail;
};

namespace {
typedef char AccountFirstLoginScript_Size_Must_Be_0x50[
    (sizeof(AccountFirstLoginScript) == 0x50) ? 1 : -1];
typedef char ConditionLevelUpBridge_Size_Must_Be_0x10[
    (sizeof(ConditionLevelUpBridge) == 0x10) ? 1 : -1];
typedef char LevelUpRewardScript_Size_Must_Be_0x6c[
    (sizeof(LevelUpRewardScript) == 0x6c) ? 1 : -1];

const unsigned int kActionMailMaxSystemItems = 10u;

void CopyActionMailText(const Action_SendMail& mail,
                        char* title,
                        unsigned int title_size,
                        char* text,
                        unsigned int text_size) {
    const char* title_key = toTString(mail.title_string_key);
    const char* title_src =
        g_scriptStringManager_.findString(4, title_key, 0);
    if (!title_src) {
        title_src = "";
    }
    std::strncpy(title, title_src, title_size - 1);
    title[title_size - 1] = 0;

    const char* text_key = toTString(mail.message_string_key);
    const char* text_src =
        g_scriptStringManager_.findString(4, text_key, 0);
    if (!text_src) {
        text_src = "";
    }
    std::strncpy(text, text_src, text_size - 1);
    text[text_size - 1] = 0;
}

}  // namespace

// IDA_EA: 0x8110F4C
// IDA_NAME: _ZN23InGameEventScriptLoaderC2Ev
// DEMANGLED: InGameEventScriptLoader::InGameEventScriptLoader(void)
InGameEventScriptLoader::InGameEventScriptLoader() : scripts_() {}

// IDA_EA: 0x8110F60
// IDA_NAME: _ZN23InGameEventScriptLoaderD2Ev
// DEMANGLED: InGameEventScriptLoader::~InGameEventScriptLoader()
InGameEventScriptLoader::~InGameEventScriptLoader() {}
// IDA_EA: 0x89858DA
// IDA_NAME: _ZN23AccountFirstLoginScriptC2Ev
// DEMANGLED: AccountFirstLoginScript::AccountFirstLoginScript(void)
AccountFirstLoginScript::AccountFirstLoginScript()
    : InGameEventScript(),
      db_table_name(),
      send_mail() {
    db_table_name.clear();
    send_mail.reset();
}

ConditionLevelUpBridge::ConditionLevelUpBridge()
    : grow_type(0),
      reserved_01_(),
      rewards() {}

// IDA_EA: 0x89861DA
// IDA_NAME: _ZN19LevelUpRewardScriptC2Ev
// DEMANGLED: LevelUpRewardScript::LevelUpRewardScript(void)
LevelUpRewardScript::LevelUpRewardScript()
    : InGameEventScript(),
      recv_check_item(0),
      reserved_09_(),
      db_table_name(),
      conditions(),
      send_mail() {
}

// IDA_EA: 0x897F7BF
// IDA_NAME: _Z34importAccountFirstLoginEventScriptP23AccountFirstLoginScriptPKc
// DEMANGLED: importAccountFirstLoginEventScript(AccountFirstLoginScript *,char const*)
int importAccountFirstLoginEventScript(AccountFirstLoginScript* script,
                                       const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        }
        if (tag == "[title]") {
            (void)ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            (void)ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            (void)ScanStr(&script->db_table_name);
        } else if (tag == "[send mail]") {
            (void)importActionMailScript(&script->send_mail);
        }
    }

    return 0;
}

// IDA_EA: 0x8980261
// IDA_NAME: _Z25importLevelUpRewardScriptP19LevelUpRewardScriptPKc
// DEMANGLED: importLevelUpRewardScript(LevelUpRewardScript *,char const*)
int importLevelUpRewardScript(LevelUpRewardScript* script,
                              const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    script->recv_check_item = 0;
    script->db_table_name.clear();
    script->conditions.clear();
    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        // The official 0627 importer and both 0828 LevelUpReward scripts use
        // this scalar terminal tag; there is no enclosing [event] block.
        if (tag == "[level up reward end]") {
            return 1;
        }
        if (tag == "[recv check db table]") {
            (void)ScanStr(&script->db_table_name);
            script->recv_check_item = 1;
        } else if (tag == "[send mail]") {
            (void)importActionMailScript(&script->send_mail);
        } else if (tag == "[level up]") {
            while (ScanType(tag, true) == 1) {
                int job = -1;
                if (tag == "[swordman]") {
                    job = 0;
                } else if (tag == "[fighter]") {
                    job = 1;
                } else if (tag == "[gunner]") {
                    job = 2;
                } else if (tag == "[mage]") {
                    job = 3;
                } else if (tag == "[priest]") {
                    job = 4;
                } else if (tag == "[at gunner]") {
                    job = 5;
                } else if (tag == "[thief]") {
                    job = 6;
                } else if (tag == "[knight]") {
                    job = 7;
                } else if (tag == "[demonic swordman]") {
                    job = 8;
                } else {
                    break;
                }

                ConditionLevelUpBridge condition;
                if (ScanType(tag, true) != 1) {
                    return 0;
                }
                condition.grow_type =
                    static_cast<unsigned char>(ScanInt(&success));
                while (success) {
                    Condition_LevelUp::Reward_Item reward;
                    reward.first_dword = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    reward.second_dword = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    reward.third_dword = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    (void)ScanStr(&reward.first_string);
                    (void)ScanStr(&reward.second_string);
                    condition.rewards.push_back(reward);
                }
                script->conditions.insert(std::make_pair(job, condition));
                success = true;
            }
        }
    }

    return 0;
}

namespace EventClassify {

// IDA_EA: 0x810CF32
// IDA_NAME: _ZN13EventClassify15CEventActionMng24process_action_send_mailEP5CUserR15Action_SendMailb
// DEMANGLED: EventClassify::CEventActionMng::process_action_send_mail(CUser *,Action_SendMail &,bool)
int CEventActionMng::process_action_send_mail(CUser* user,
                                              Action_SendMail& mail,
                                              bool notify) {
    if (!user) {
        return 3;
    }

    if (mail.money > 0) {
        process_gold_mail_send(user, mail);
        return 0;
    }

    Inven_Item item;
    int item_id = mail.item_id;
    int item_count = mail.item_count;
    int charac_job = 0;
    if (!mail.job_item_counts.empty()) {
        charac_job = user->get_charac_job();
        std::map<int, std::pair<int, int> >::const_iterator it =
            mail.job_item_counts.find(charac_job);
        if (it != mail.job_item_counts.end()) {
            item_id = it->second.first;
            item_count = it->second.second;
        }
    }
    if (!item_count) {
        item_count = 1;
    }

    int slot = -1;
    item.item_id_ = item_id;
    item.set_add_info(item_count);

    if (item.item_id_ == 1) {
        process_coin_send(user, mail, item);
    } else {
        CDataManager* data_manager = G_CDataManager();
        CItem* item_proto =
            data_manager ? data_manager->find_item(item_id) : 0;
        if (!item_proto) {
            return 17;
        }

        // IDA 0x810CF32 initializes the generated inventory record before
        // either direct delivery or mail fallback.  In particular this
        // establishes the category consumed by the post-action result data.
        item_proto->vtable_->make_item(item_proto, item);

        if (mail.direct_inventory) {
            bool fallback_to_mail = false;
            const bool is_avatar = item_proto->vtable_->is_avatar_item(item_proto);
            if (user->CheckInTrade()) {
                fallback_to_mail = true;
            } else if (is_avatar) {
                slot = process_avatar_send(user, item, item_proto);
                fallback_to_mail = slot == -1;
            } else {
                slot = user->getCurCharacInvenW()->insertItemIntoInventory(
                    item, static_cast<eItemAddReason>(13), true, true);
                fallback_to_mail = slot == -1;
            }

            if (!fallback_to_mail && notify) {
                PacketGuard packet;
                packet.put_header(0, 14);
                packet.put_byte(is_avatar ? 1 : 0);
                packet.put_short(1);
                if (user->getCurCharacInvenR()->MakeItemPacket(
                        static_cast<INVEN_TYPE>(is_avatar ? 2 : 1),
                        slot,
                        packet) != 1) {
                    return 0;
                }
                packet.finalize(true);
                user->Send(packet);
            }

            if (fallback_to_mail) {
                process_mail_send(user, mail, item, item_proto);
            }
        } else {
            process_mail_send(user, mail, item, item_proto);
        }
    }

    mail.result_item_values.push_back(slot);
    mail.result_item_values.push_back(item.item_id_);
    mail.result_item_values.push_back(item.get_add_info());
    if (item.item_category_ == 2u || item.item_category_ == 3u) {
        mail.result_item_values.push_back(item.avatar_select_);
    } else {
        mail.result_item_values.push_back(0);
    }

    return 0;
}

// IDA_EA: 0x810D4AE
// IDA_NAME: _ZN13EventClassify15CEventActionMng17process_coin_sendEP5CUserR15Action_SendMailR10Inven_Item
// DEMANGLED: EventClassify::CEventActionMng::process_coin_send(CUser *,Action_SendMail &,Inven_Item &)
int CEventActionMng::process_coin_send(CUser* user,
                                       Action_SendMail& mail,
                                       Inven_Item& item) {
    if (mail.direct_inventory) {
        // IDA_EA 0x810D4AE direct event-coin mutation depends on C06
        // CInventory::GetEventCoin/SetEventCoin and CUser coin-save/send ABI.
        return 17;
    }

    item.avatar_select_ = 1u;
    char title[30] = {};
    char text[256] = {};
    CopyActionMailText(mail, title, sizeof(title), text, sizeof(text));

    const int server_group = user->GetServerGroup();
    const int charac_no = user->getCurCharacNo();
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        static_cast<unsigned int>(mail.money),
        static_cast<unsigned int>(charac_no),
        text,
        static_cast<int>(std::strlen(text)),
        static_cast<unsigned int>(static_cast<int>(mail.occ_time)),
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        false);
    return 0;
}

// IDA_EA: 0x810D70C
// IDA_NAME: _ZN13EventClassify15CEventActionMng19process_avatar_sendEP5CUserR10Inven_ItemPK5CItem
// DEMANGLED: EventClassify::CEventActionMng::process_avatar_send(CUser *,Inven_Item &,CItem const*)
int CEventActionMng::process_avatar_send(CUser* user,
                                         Inven_Item& item,
                                         const CItem* item_proto) {
    char ipg_no[24] = {};
    const char* const unique_no = WongWork::CGenUniqueNo::genIPGNo(
        static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(7),
        static_cast<unsigned int>(user->getCurCharacNo()), ipg_no);
    const int slot = user->getCurCharacInvenW()->AddAvatarItem(
        item.item_id_, item_proto->getUsablePeriod(), false, 0,
        static_cast<char>(-1), unique_no,
        static_cast<eAvatarItemAddReason>(2), false, 0);
    if (slot != -1) {
        const int current_time = GlobalData::system_time()->getCurSec();
        const Inven_Item inserted_item = user->getCurCharacInvenR()
            ->GetInvenSlot(2, slot);
        item.set_add_info(user->getCurCharacInvenR()->GetAvatarItemMgrR()
                              ->GetRemainDate(inserted_item.get_add_info(),
                                              current_time));
        item.SetUpgrade(0);
    }
    return slot;
}

// IDA_EA: 0x810D83C
// IDA_NAME: _ZN13EventClassify15CEventActionMng22process_gold_mail_sendEP5CUserR15Action_SendMail
// DEMANGLED: EventClassify::CEventActionMng::process_gold_mail_send(CUser *,Action_SendMail &)
int CEventActionMng::process_gold_mail_send(CUser* user,
                                            Action_SendMail& mail) {
    Inven_Item item;

    char title[30] = {};
    char text[256] = {};
    CopyActionMailText(mail, title, sizeof(title), text, sizeof(text));

    const int server_group = user->GetServerGroup();
    const int charac_no = user->getCurCharacNo();
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        static_cast<unsigned int>(mail.money),
        static_cast<unsigned int>(charac_no),
        text,
        static_cast<int>(std::strlen(text)),
        static_cast<unsigned int>(static_cast<int>(mail.occ_time)),
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        false);
    return 0;
}

// IDA_EA: 0x810DA1C
// IDA_NAME: _ZN13EventClassify15CEventActionMng17process_mail_sendEP5CUserR15Action_SendMailR10Inven_ItemPK5CItem
// DEMANGLED: EventClassify::CEventActionMng::process_mail_send(CUser *,Action_SendMail &,Inven_Item &,CItem const*)
int CEventActionMng::process_mail_send(CUser* user,
                                       Action_SendMail& mail,
                                       Inven_Item& item,
                                       const CItem* item_proto) {
    if (!item_proto) {
        return 17;
    }
    if (item.isAvatarItemType()) {
        return 17;
    }

    char title[30] = {};
    char text[256] = {};
    CopyActionMailText(mail, title, sizeof(title), text, sizeof(text));

    Inven_Item mail_items[kActionMailMaxSystemItems];
    for (unsigned int index = 0; index < kActionMailMaxSystemItems; ++index) {
        mail_items[index].reset();
    }

    std::vector<std::pair<int, int> > rewards;
    rewards.push_back(std::make_pair(
        static_cast<int>(item.item_id_),
        item.get_add_info()));

    if (!WongWork::CMailBoxHelper::MakeSystemMultiMailPostal(
            rewards, mail_items, kActionMailMaxSystemItems)) {
        return 17;
    }

    const int server_group = user->GetServerGroup();
    const int charac_no = user->getCurCharacNo();
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(
        title,
        mail_items,
        static_cast<unsigned int>(rewards.size()),
        0,
        static_cast<unsigned int>(charac_no),
        text,
        static_cast<int>(std::strlen(text)),
        static_cast<unsigned int>(static_cast<int>(mail.occ_time)),
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false);
}

// IDA_EA: 0x810DC5A
// IDA_NAME: _ZN13EventClassify15CEventActionMng24getResultItemFromLotteryEjRSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: EventClassify::CEventActionMng::getResultItemFromLottery(unsigned int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CEventActionMng::getResultItemFromLottery(
    unsigned int item_id,
    std::vector<std::pair<int, int> >& reward_items) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const item = data_manager ? data_manager->find_item(item_id) : 0;
    if (!item || !item->vtable_) {
        return 17;
    }

    const int item_type = item->vtable_->get_item_type(item);
    if (item_type == 6) {
        CItemList* const item_list = data_manager->get_item_list();
        if (!item_list) {
            return 17;
        }

        Inven_Item lottery_item;
        item_list->get_lotto_item(
            item->get_index(), reinterpret_cast<char*>(&lottery_item));
        if (lottery_item.item_id_ == 0 || lottery_item.item_id_ == -1) {
            return 17;
        }
        // Inven_Item is deliberately packed for the old wire/storage ABI;
        // copy its scalar before binding it to std::make_pair's reference
        // parameter.
        const int lottery_item_id = lottery_item.item_id_;
        reward_items.push_back(std::make_pair(
            lottery_item_id, lottery_item.get_add_info()));
        return 0;
    }

    if (item_type != 16) {
        return 23;
    }

    std::vector<stStackableBoosterItemInfo_t> booster_items;
    static_cast<const CStackableItem*>(item)->getBoosterItem(booster_items);
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator it =
             booster_items.begin();
         it != booster_items.end();
         ++it) {
        if (!it->item_index) {
            return 17;
        }
        reward_items.push_back(std::make_pair(
            it->item_index, static_cast<int>(it->item_count)));
    }
    return 0;
}

// IDA_EA: 0x810AF40
// IDA_NAME: _ZN13EventClassify12CEventEntityC2Ev
// DEMANGLED: EventClassify::CEventEntity::CEventEntity(void)
CEventEntity::CEventEntity()
    : event_flag_(0),
      reserved_05_(),
      db_table_name_(),
      event_start_time_(0),
      event_end_time_(0),
      script_(0),
      event_record_type_(0) {
    db_table_name_.clear();
}

// IDA_EA: 0x810AFC4
// IDA_NAME: _ZN13EventClassify12CEventEntityD2Ev
// DEMANGLED: EventClassify::CEventEntity::~CEventEntity()
// IDA deleting destructor: 0x810AFF8.
CEventEntity::~CEventEntity() {}

// IDA_EA: 0x810B016
// IDA_NAME: _ZN13EventClassify12CEventEntity11is_eventingEv
// DEMANGLED: EventClassify::CEventEntity::is_eventing(void)
int CEventEntity::is_eventing() {
    return event_flag_;
}

// IDA_EA: 0x810B022
// IDA_NAME: _ZN13EventClassify12CEventEntity14set_event_flagEb
// DEMANGLED: EventClassify::CEventEntity::set_event_flag(bool)
CEventEntity* CEventEntity::set_event_flag(bool enabled) {
    event_flag_ = enabled ? 1 : 0;
    return this;
}

// IDA_EA: 0x810B03A
// IDA_NAME: _ZN13EventClassify12CEventEntity14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CEventEntity::set_event_data(int,InGameEventScript *)
CEventEntity* CEventEntity::set_event_data(int record_type,
                                           InGameEventScript* script) {
    event_record_type_ = record_type;
    script_ = script;
    return this;
}

// IDA_EA: 0x810B052
// IDA_NAME: _ZN13EventClassify12CEventEntity21get_event_record_typeEv
// DEMANGLED: EventClassify::CEventEntity::get_event_record_type(void)
int CEventEntity::get_event_record_type() {
    return event_record_type_;
}

// IDA_EA: 0x810B05E
// IDA_NAME: _ZN13EventClassify12CEventEntity21set_event_record_typeEi
// DEMANGLED: EventClassify::CEventEntity::set_event_record_type(int)
CEventEntity* CEventEntity::set_event_record_type(int record_type) {
    event_record_type_ = record_type;
    return this;
}

// IDA_EA: 0x810B0AC
// IDA_NAME: _ZN13EventClassify12CEventEntity14send_init_dataEP5CUserR11PacketGuardshb
// DEMANGLED: EventClassify::CEventEntity::send_init_data(CUser *,PacketGuard &,short,unsigned char,bool)
int CEventEntity::send_init_data(CUser* user,
                                 PacketGuard& guard,
                                 short event_id,
                                 unsigned char today_day,
                                 bool reset) {
    (void)user;
    (void)guard;
    (void)event_id;
    (void)today_day;
    (void)reset;
    return 0;
}

// IDA_EA: 0x810B06C
// IDA_NAME: _ZN13EventClassify12CEventEntity17get_db_table_nameEv
// DEMANGLED: EventClassify::CEventEntity::get_db_table_name(void)
const std::string& CEventEntity::get_db_table_name() {
    return db_table_name_;
}

// IDA_EA: 0x810B078
// IDA_NAME: _ZN13EventClassify12CEventEntity20set_event_start_timeEi
// DEMANGLED: EventClassify::CEventEntity::set_event_start_time(int)
CEventEntity* CEventEntity::set_event_start_time(int start_time) {
    event_start_time_ = start_time;
    return this;
}

// IDA_EA: 0x810B086
// IDA_NAME: _ZN13EventClassify12CEventEntity18set_event_end_timeEi
// DEMANGLED: EventClassify::CEventEntity::set_event_end_time(int)
CEventEntity* CEventEntity::set_event_end_time(int end_time) {
    event_end_time_ = end_time;
    return this;
}

// IDA_EA: 0x810B094
// IDA_NAME: _ZN13EventClassify12CEventEntity20get_event_start_timeEv
// DEMANGLED: EventClassify::CEventEntity::get_event_start_time(void)
int CEventEntity::get_event_start_time() {
    return event_start_time_;
}

// IDA_EA: 0x810B0A0
// IDA_NAME: _ZN13EventClassify12CEventEntity18get_event_end_timeEv
// DEMANGLED: EventClassify::CEventEntity::get_event_end_time(void)
int CEventEntity::get_event_end_time() {
    return event_end_time_;
}

CScriptBackedEventEntity::CScriptBackedEventEntity()
    : CEventEntity(),
      typed_script_(0) {}

CScriptBackedEventEntity::~CScriptBackedEventEntity() {}

CEventEntity* CScriptBackedEventEntity::set_event_data(
    int record_type,
    InGameEventScript* script) {
    CEventEntity::set_event_record_type(record_type);
    typed_script_ = script;
    script_ = script;
    return this;
}

// IDA_EA: 0x8110F88
// IDA_NAME: _ZN13EventClassify18CAccountFirstLoginC2Ev
// DEMANGLED: EventClassify::CAccountFirstLogin::CAccountFirstLogin(void)
CAccountFirstLogin::CAccountFirstLogin() : CEventEntity() {}

// IDA_EA: 0x81110BC
// IDA_NAME: _ZN13EventClassify18CAccountFirstLoginD2Ev
// DEMANGLED: EventClassify::CAccountFirstLogin::~CAccountFirstLogin()
// IDA_EA: 0x81110EC
// IDA_NAME: _ZN13EventClassify18CAccountFirstLoginD0Ev
// DEMANGLED: EventClassify::CAccountFirstLogin::~CAccountFirstLogin()
CAccountFirstLogin::~CAccountFirstLogin() {}

// IDA_EA: 0x810DEDA
// IDA_NAME: _ZN13EventClassify18CAccountFirstLogin14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CAccountFirstLogin::set_event_data(int,InGameEventScript *)
CEventEntity* CAccountFirstLogin::set_event_data(int record_type,
                                                 InGameEventScript* script) {
    set_event_record_type(record_type);
    typed_script_ = script;
    return this;
}

// IDA_EA: 0x810DEBE
// IDA_NAME: _ZN13EventClassify18CAccountFirstLogin17get_db_table_nameEv
// DEMANGLED: EventClassify::CAccountFirstLogin::get_db_table_name(void)
const std::string& CAccountFirstLogin::get_db_table_name() {
    return static_cast<AccountFirstLoginScript*>(typed_script_)->db_table_name;
}

// IDA_EA: 0x810DECC
// IDA_NAME: _ZN13EventClassify18CAccountFirstLogin13get_mail_dataEv
// DEMANGLED: EventClassify::CAccountFirstLogin::get_mail_data(void)
Action_SendMail* CAccountFirstLogin::get_mail_data() {
    return &static_cast<AccountFirstLoginScript*>(typed_script_)->send_mail;
}

// IDA_EA: 0x810DF04
// IDA_NAME: _ZN13EventClassify18CAccountFirstLogin9do_actionEiP5CUserSt6bitsetILj32EE
// DEMANGLED: EventClassify::CAccountFirstLogin::do_action(int,CUser *,std::bitset<32u>)
void CAccountFirstLogin::do_action(int event_type,
                                   CUser* user,
                                   std::bitset<32> user_confirm_flags) {
    (void)user_confirm_flags;
    if (!user) {
        return;
    }

    CEventActionMng action_manager;
    action_manager.process_action_send_mail(user, *get_mail_data(), true);

    int server_group = 0;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        server_group = user->GetServerGroup();
    } else {
        server_group = G_CEnvironment()->get_server_group();
    }

    DB_InsertAccountFirstLoginFlag::makeRequest(
        user->GetUID(),
        static_cast<unsigned int>(user->get_acc_id()),
        server_group,
        static_cast<unsigned int>(user->getCurCharacNo()),
        event_type);
}

// IDA_EA: 0x811110A
// IDA_NAME: _ZN13EventClassify23CAccountFirstLoginDailyC2Ev
// DEMANGLED: EventClassify::CAccountFirstLoginDaily::CAccountFirstLoginDaily(void)
CAccountFirstLoginDaily::CAccountFirstLoginDaily()
    : CAccountFirstLogin() {}

// IDA_EA: 0x8114C80
// IDA_NAME: _ZN13EventClassify23CAccountFirstLoginDailyD2Ev
// DEMANGLED: EventClassify::CAccountFirstLoginDaily::~CAccountFirstLoginDaily()
// IDA_EA: 0x8114CB0
// IDA_NAME: _ZN13EventClassify23CAccountFirstLoginDailyD0Ev
// DEMANGLED: EventClassify::CAccountFirstLoginDaily::~CAccountFirstLoginDaily()
CAccountFirstLoginDaily::~CAccountFirstLoginDaily() {}

// IDA_EA: 0x810DFC6
// IDA_NAME: _ZN13EventClassify23CAccountFirstLoginDaily14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CAccountFirstLoginDaily::set_event_data(int,InGameEventScript *)
CEventEntity* CAccountFirstLoginDaily::set_event_data(
    int record_type,
    InGameEventScript* script) {
    return CAccountFirstLogin::set_event_data(record_type, script);
}

// IDA_EA: 0x810DFE8
// IDA_NAME: _ZN13EventClassify23CAccountFirstLoginDaily9do_actionEiP5CUserSt6bitsetILj32EE
// DEMANGLED: EventClassify::CAccountFirstLoginDaily::do_action(int,CUser *,std::bitset<32u>)
void CAccountFirstLoginDaily::do_action(
    int event_type,
    CUser* user,
    std::bitset<32> user_confirm_flags) {
    (void)user_confirm_flags;
    if (!user) {
        return;
    }

    CEventActionMng action_manager;
    action_manager.process_action_send_mail(user, *get_mail_data(), true);
    DB_InsertAccountFirstLoginDailyFlag::makeRequest(
        user->GetUID(),
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(user->getCurCharacNo()),
        event_type);
}

// IDA_EA: 0x8110FDC
// IDA_NAME: _ZN13EventClassify11CAttendanceC2Ev
// DEMANGLED: EventClassify::CAttendance::CAttendance(void)
CAttendance::CAttendance() : CEventEntity() {}

// IDA_EA: 0x8114B96
// IDA_NAME: _ZN13EventClassify11CAttendanceD2Ev
// DEMANGLED: EventClassify::CAttendance::~CAttendance()
CAttendance::~CAttendance() {}

// IDA_EA: 0x8114BC6
// IDA_NAME: _ZN13EventClassify11CAttendanceD0Ev
// DEMANGLED: EventClassify::CAttendance::~CAttendance()
// The C++ ABI emits this deleting destructor from the virtual destructor
// definition above; it calls D2 and then operator delete, matching the old
// server body.

// IDA_EA: 0x810EADC
// IDA_NAME: _ZN13EventClassify11CAttendance14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CAttendance::set_event_data(int,InGameEventScript *)
CEventEntity* CAttendance::set_event_data(int record_type,
                                          InGameEventScript* script) {
    set_event_record_type(record_type);
    typed_script_ = script;
    return this;
}

// IDA_EA: 0x810E7CC
// IDA_NAME: _ZN13EventClassify11CAttendance17get_db_table_nameEv
// DEMANGLED: EventClassify::CAttendance::get_db_table_name(void)
const std::string& CAttendance::get_db_table_name() {
    return static_cast<AttendanceScript*>(typed_script_)->db_table_name;
}

// IDA_EA: 0x810E75C
// IDA_NAME: _ZN13EventClassify11CAttendance11is_eventingEv
// DEMANGLED: EventClassify::CAttendance::is_eventing(void)
int CAttendance::is_eventing() {
    if (!CEventEntity::is_eventing()) {
        return 0;
    }
    CEventBase* const repeat_event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(86)
            : 0;
    return repeat_event && repeat_event->IsEventing(0) ? 1 : 0;
}

// IDA_EA: 0x810E7DA
// IDA_NAME: _ZN13EventClassify11CAttendance13get_mail_dataEv
// DEMANGLED: EventClassify::CAttendance::get_mail_data(void)
Action_SendMail* CAttendance::get_mail_data() {
    AttendanceScript* const attendance =
        static_cast<AttendanceScript*>(typed_script_);
    return attendance ? &attendance->mail_data : 0;
}

// IDA_EA: 0x810E7E8
// IDA_NAME: _ZN13EventClassify11CAttendance21get_fatigue_conditionEv
// DEMANGLED: EventClassify::CAttendance::get_fatigue_condition(void)
int CAttendance::get_fatigue_condition() {
    const AttendanceScript* const attendance =
        static_cast<const AttendanceScript*>(typed_script_);
    return attendance ? attendance->fatigue_condition : 0;
}

// IDA_EA: 0x810E7F6
// IDA_NAME: _ZN13EventClassify11CAttendance23get_final_day_conditionEv
// DEMANGLED: EventClassify::CAttendance::get_final_day_condition(void)
int CAttendance::get_final_day_condition() {
    const AttendanceScript* const attendance =
        static_cast<const AttendanceScript*>(typed_script_);
    return attendance ? attendance->final_day_condition : 0;
}

// IDA_EA: 0x810E804
// IDA_NAME: _ZN13EventClassify11CAttendance14send_init_dataEP5CUserR11PacketGuardshb
// DEMANGLED: EventClassify::CAttendance::send_init_data(CUser *,PacketGuard &,short,unsigned char,bool)
int CAttendance::send_init_data(CUser* user,
                                PacketGuard& guard,
                                short event_id,
                                unsigned char today_day,
                                bool reset) {
    (void)reset;
    guard.put_short(event_id);
    const int fatigue_count =
        user->get_server_fatigue_count(today_day) +
        user->get_fatigue_count_db(today_day);
    guard.put_short(fatigue_count);
    return 1;
}

// IDA_EA: 0x810E878
// IDA_NAME: _ZN13EventClassify11CAttendance23send_attend_reward_itemEP5CUseri
// DEMANGLED: EventClassify::CAttendance::send_attend_reward_item(CUser *,int)
int CAttendance::send_attend_reward_item(CUser* user, int pass_day) {
    AttendanceScript* const attendance =
        static_cast<AttendanceScript*>(typed_script_);
    if (!attendance) {
        return 0;
    }
    if (attendance->reward_items_by_day.empty()) {
        return 1;
    }
    if (pass_day < 0) {
        return 0;
    }
    if (static_cast<std::size_t>(pass_day) >=
        attendance->reward_items_by_day.size()) {
        return 1;
    }

    const std::pair<int, int>& reward =
        attendance->reward_items_by_day[pass_day];
    attendance->mail_data.item_id = reward.first;
    attendance->mail_data.item_count = reward.second;
    attendance->mail_data.message_string_key = attendance->attend_mail_text;
    CEventActionMng action_manager;
    return action_manager.process_action_send_mail(
        user, attendance->mail_data, true);
}

// IDA_EA: 0x810E9C0
// IDA_NAME: _ZN13EventClassify11CAttendance28send_attend_final_rward_itemEP5CUserii
// DEMANGLED: EventClassify::CAttendance::send_attend_final_rward_item(CUser *,int,int)
int CAttendance::send_attend_final_rward_item(CUser* user,
                                              int attendance_bits,
                                              int final_day_condition) {
    AttendanceScript* const attendance =
        static_cast<AttendanceScript*>(typed_script_);
    if (!attendance) {
        return 0;
    }

    int attended_days = 0;
    const unsigned int bits =
        static_cast<unsigned int>(attendance_bits);
    for (int bit = 0; bit <= 31; ++bit) {
        if ((bits & (1u << bit)) != 0) {
            ++attended_days;
        }
    }
    if (final_day_condition != attended_days) {
        return final_day_condition;
    }

    attendance->mail_data.item_id = attendance->final_reward_item.first;
    attendance->mail_data.item_count = attendance->final_reward_item.second;
    attendance->mail_data.message_string_key = attendance->final_mail_text;
    CEventActionMng action_manager;
    return action_manager.process_action_send_mail(
        user, attendance->mail_data, true);
}

// IDA_EA: 0x8110FA4
// IDA_NAME: _ZN13EventClassify15CAddItemCountDBC2Ev
// DEMANGLED: EventClassify::CAddItemCountDB::CAddItemCountDB(void)
CAddItemCountDB::CAddItemCountDB() : CEventEntity() {}

// IDA_EA: 0x8114C32
// IDA_NAME: _ZN13EventClassify15CAddItemCountDBD2Ev
// DEMANGLED: EventClassify::CAddItemCountDB::~CAddItemCountDB()
CAddItemCountDB::~CAddItemCountDB() {}

// IDA_EA: 0x8114C62
// IDA_NAME: _ZN13EventClassify15CAddItemCountDBD0Ev
// DEMANGLED: EventClassify::CAddItemCountDB::~CAddItemCountDB()
// The C++ ABI emits this deleting destructor from the virtual destructor
// definition above; it calls D2 and then operator delete, matching the old
// server body.

// IDA_EA: 0x810E064
// IDA_NAME: _ZN13EventClassify15CAddItemCountDB14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CAddItemCountDB::set_event_data(int,InGameEventScript *)
CEventEntity* CAddItemCountDB::set_event_data(int record_type,
                                              InGameEventScript* script) {
    set_event_record_type(record_type);
    typed_script_ = script;
    return this;
}

// IDA_EA: 0x810E15A
// IDA_NAME: _ZN13EventClassify15CAddItemCountDB17get_db_table_nameEv
// DEMANGLED: EventClassify::CAddItemCountDB::get_db_table_name(void)
const std::string& CAddItemCountDB::get_db_table_name() {
    return static_cast<AddItemCountDBScript*>(typed_script_)->value_20;
}

// IDA_EA: 0x810E08E
// IDA_NAME: _ZN13EventClassify15CAddItemCountDB13is_event_itemEi
// DEMANGLED: EventClassify::CAddItemCountDB::is_event_item(int)
int CAddItemCountDB::is_event_item(int item_id) {
    const AddItemCountDBScript* const add_item_script =
        static_cast<const AddItemCountDBScript*>(typed_script_);
    if (!add_item_script) {
        return 0;
    }

    for (std::vector<int>::const_iterator it =
             add_item_script->values_08.begin();
         it != add_item_script->values_08.end(); ++it) {
        if (*it == item_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x810E0F4
// IDA_NAME: _ZN13EventClassify15CAddItemCountDB14is_event_questEi
// DEMANGLED: EventClassify::CAddItemCountDB::is_event_quest(int)
int CAddItemCountDB::is_event_quest(int quest_id) {
    const AddItemCountDBScript* const add_item_script =
        static_cast<const AddItemCountDBScript*>(typed_script_);
    if (!add_item_script) {
        return 0;
    }

    for (std::vector<int>::const_iterator it =
             add_item_script->values_14.begin();
         it != add_item_script->values_14.end(); ++it) {
        if (*it == quest_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8110FC0
// IDA_NAME: _ZN13EventClassify14CLevelUpRewardC2Ev
// DEMANGLED: EventClassify::CLevelUpReward::CLevelUpReward(void)
CLevelUpReward::CLevelUpReward() : CEventEntity() {}

// IDA_EA: 0x8114BE4
// IDA_NAME: _ZN13EventClassify14CLevelUpRewardD2Ev
// DEMANGLED: EventClassify::CLevelUpReward::~CLevelUpReward()
CLevelUpReward::~CLevelUpReward() {}

// IDA_EA: 0x8114C14
// IDA_NAME: _ZN13EventClassify14CLevelUpRewardD0Ev
// DEMANGLED: EventClassify::CLevelUpReward::~CLevelUpReward()
// The C++ ABI emits this deleting destructor from the virtual destructor
// definition above; it calls D2 and then operator delete, matching the old
// server body.

// IDA_EA: 0x810E168
// IDA_NAME: _ZN13EventClassify14CLevelUpReward14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CLevelUpReward::set_event_data(int,InGameEventScript *)
CEventEntity* CLevelUpReward::set_event_data(int record_type,
                                             InGameEventScript* script) {
    set_event_record_type(record_type);
    typed_script_ = script;
    return this;
}

// IDA_EA: 0x810E2D6
// IDA_NAME: _ZN13EventClassify14CLevelUpReward17get_db_table_nameEv
// DEMANGLED: EventClassify::CLevelUpReward::get_db_table_name(void)
const std::string& CLevelUpReward::get_db_table_name() {
    return static_cast<LevelUpRewardScript*>(typed_script_)->db_table_name;
}

// IDA_EA: 0x810E192
// IDA_NAME: _ZN13EventClassify14CLevelUpReward16check_recv_levelEiiib
// DEMANGLED: EventClassify::CLevelUpReward::check_recv_level(int,int,int,bool)
int CLevelUpReward::check_recv_level(int charac_level,
                                     int charac_job,
                                     int first_grow_type,
                                     bool include_lower_levels) {
    const LevelUpRewardScript* const level_script =
        static_cast<const LevelUpRewardScript*>(typed_script_);
    if (!level_script) {
        return 0;
    }

    std::map<int, ConditionLevelUpBridge>::const_iterator condition =
        level_script->conditions.find(charac_job);
    if (condition == level_script->conditions.end()) {
        return 0;
    }
    if (condition->second.grow_type != 0xffu &&
        static_cast<signed char>(condition->second.grow_type) !=
            first_grow_type) {
        return 0;
    }

    for (std::vector<Condition_LevelUp::Reward_Item>::const_iterator reward =
             condition->second.rewards.begin();
         reward != condition->second.rewards.end(); ++reward) {
        if (include_lower_levels
                ? reward->first_dword <= charac_level
                : reward->first_dword == charac_level) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x810E2E4
// IDA_NAME: _ZN13EventClassify14CLevelUpReward13get_mail_dataEv
// DEMANGLED: EventClassify::CLevelUpReward::get_mail_data(void)
Action_SendMail* CLevelUpReward::get_mail_data() {
    LevelUpRewardScript* const level_script =
        static_cast<LevelUpRewardScript*>(typed_script_);
    return level_script ? &level_script->send_mail : 0;
}

// IDA_EA: 0x810E2F2
// IDA_NAME: _ZN13EventClassify14CLevelUpReward18is_recv_check_itemEv
// DEMANGLED: EventClassify::CLevelUpReward::is_recv_check_item(void)
int CLevelUpReward::is_recv_check_item() {
    const LevelUpRewardScript* const level_script =
        static_cast<const LevelUpRewardScript*>(typed_script_);
    return level_script ? level_script->recv_check_item : 0;
}

// IDA_EA: 0x810E302
// IDA_NAME: _ZN13EventClassify14CLevelUpReward15set_reward_itemEiii
// DEMANGLED: EventClassify::CLevelUpReward::set_reward_item(int,int,int)
int CLevelUpReward::set_reward_item(int charac_level,
                                    int charac_job,
                                    int first_grow_type) {
    LevelUpRewardScript* const level_script =
        static_cast<LevelUpRewardScript*>(typed_script_);
    if (!level_script) {
        return 0;
    }

    std::map<int, ConditionLevelUpBridge>::iterator condition =
        level_script->conditions.find(charac_job);
    if (condition == level_script->conditions.end()) {
        return 0;
    }
    if (condition->second.grow_type != 0xffu &&
        static_cast<signed char>(condition->second.grow_type) !=
            first_grow_type) {
        return 0;
    }

    for (std::vector<Condition_LevelUp::Reward_Item>::const_iterator reward =
             condition->second.rewards.begin();
         reward != condition->second.rewards.end(); ++reward) {
        if (reward->first_dword != charac_level) {
            continue;
        }
        level_script->send_mail.reset();
        level_script->send_mail.set_item(reward->second_dword,
                                         reward->third_dword);
        level_script->send_mail.title_string_key = reward->first_string;
        level_script->send_mail.message_string_key = reward->second_string;
        return 1;
    }
    return 0;
}

// IDA_EA: 0x810E4EA
// IDA_NAME: _ZN13EventClassify14CLevelUpReward9do_actionEP5CUser
// DEMANGLED: EventClassify::CLevelUpReward::do_action(CUser *)
int CLevelUpReward::do_action(CUser* user) {
    LevelUpRewardScript* const level_script =
        static_cast<LevelUpRewardScript*>(typed_script_);
    if (!level_script) {
        return 3;
    }
    CEventActionMng action_manager;
    return action_manager.process_action_send_mail(
        user, level_script->send_mail, true);
}

// IDA_EA: 0x810E51A
// IDA_NAME: _ZN13EventClassify14CLevelUpReward27send_reward_from_recv_levelEP5CUserii
// DEMANGLED: EventClassify::CLevelUpReward::send_reward_from_recv_level(CUser *,int,int)
int CLevelUpReward::send_reward_from_recv_level(CUser* user,
                                                 int current_level,
                                                 int received_level) {
    LevelUpRewardScript* const level_script =
        static_cast<LevelUpRewardScript*>(typed_script_);
    if (!user || !level_script) {
        return 0;
    }

    const int charac_job = user->get_charac_job();
    const int first_grow_type = user->getCurCharFirstGrowType();
    std::map<int, ConditionLevelUpBridge>::iterator condition =
        level_script->conditions.find(charac_job);
    if (condition == level_script->conditions.end()) {
        return 1;
    }
    if (condition->second.grow_type != 0xffu &&
        static_cast<signed char>(condition->second.grow_type) !=
            first_grow_type) {
        return 1;
    }

    for (std::vector<Condition_LevelUp::Reward_Item>::const_iterator reward =
             condition->second.rewards.begin();
         reward != condition->second.rewards.end(); ++reward) {
        if (reward->first_dword <= received_level ||
            reward->first_dword > current_level) {
            continue;
        }
        level_script->send_mail.reset();
        level_script->send_mail.set_item(reward->second_dword,
                                         reward->third_dword);
        level_script->send_mail.title_string_key = reward->first_string;
        level_script->send_mail.message_string_key = reward->second_string;
        do_action(user);
    }
    return 1;
}

// IDA_EA: 0x8110FF8
// IDA_NAME: _ZN13EventClassify25CExchangeRandomItemRewardC2Ev
// DEMANGLED: EventClassify::CExchangeRandomItemReward::CExchangeRandomItemReward(void)
CExchangeRandomItemReward::CExchangeRandomItemReward()
    : CScriptBackedEventEntity() {}
CExchangeRandomItemReward::~CExchangeRandomItemReward() {}

// IDA_EA: 0x810EB06
// IDA_NAME: _ZN13EventClassify25CExchangeRandomItemReward14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CExchangeRandomItemReward::set_event_data(int,InGameEventScript *)
CEventEntity* CExchangeRandomItemReward::set_event_data(
    int record_type,
    InGameEventScript* script) {
    CEventEntity::set_event_record_type(record_type);
    typed_script_ = script;
    return this;
}

// IDA_EA: 0x810EB30
// IDA_NAME: _ZN13EventClassify25CExchangeRandomItemReward24verifyMaterialItemByRuleEP5CUserRSt6vectorIiSaIiEERS3_ISt4pairIiiESaIS8_EE
// DEMANGLED: EventClassify::CExchangeRandomItemReward::verifyMaterialItemByRule(CUser *,std::vector<int,std::allocator<int>> &,std::vector&<std::pair<int,int>,std::allocator<std::pair>>)
int CExchangeRandomItemReward::verifyMaterialItemByRule(
    CUser* user,
    const std::vector<int>& item_slots,
    std::vector<std::pair<int, int> >& used_material_items) {
    ExchangeRandomItemRewardScript* const script =
        static_cast<ExchangeRandomItemRewardScript*>(typed_script_);
    if (!user || !script) {
        return 22;
    }

    const CInventory* const inventory = user->getCurCharacInvenR();
    if (script->enabled) {
        for (std::vector<std::pair<int, int> >::const_iterator rule =
                 script->exchange_items.begin();
             rule != script->exchange_items.end();
             ++rule) {
            for (std::vector<int>::const_iterator slot = item_slots.begin();
                 slot != item_slots.end();
                 ++slot) {
                const Inven_Item item = inventory->GetInvenSlot(1, *slot);
                if (item.item_id_ == rule->first &&
                    item.get_add_info() >= rule->second) {
                    used_material_items.push_back(
                        std::make_pair(*slot, rule->second));
                }
            }
        }
        return used_material_items.size() == script->exchange_items.size()
                   ? 0
                   : 22;
    }

    // The disabled branch deliberately accepts the first viable rule/slot
    // pair only.  This early exit mirrors the 0x810EB30 nested-loop flow.
    for (std::vector<std::pair<int, int> >::const_iterator rule =
             script->exchange_items.begin();
         rule != script->exchange_items.end();
         ++rule) {
        for (std::vector<int>::const_iterator slot = item_slots.begin();
             slot != item_slots.end();
             ++slot) {
            const Inven_Item item = inventory->GetInvenSlot(1, *slot);
            if (item.item_id_ == rule->first &&
                item.get_add_info() >= rule->second) {
                used_material_items.push_back(
                    std::make_pair(*slot, rule->second));
                break;
            }
        }
        if (!used_material_items.empty()) {
            break;
        }
    }
    return used_material_items.empty() ? 22 : 0;
}

// IDA_EA: 0x810EE40
// IDA_NAME: _ZN13EventClassify25CExchangeRandomItemReward13getRewardItemERSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: EventClassify::CExchangeRandomItemReward::getRewardItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CExchangeRandomItemReward::getRewardItem(
    std::vector<std::pair<int, int> >& reward_items) {
    ExchangeRandomItemRewardScript* const script =
        static_cast<ExchangeRandomItemRewardScript*>(typed_script_);
    if (!script) {
        return 17;
    }

    CEventActionMng action_manager;
    for (std::vector<int>::const_iterator reward = script->reward_items.begin();
         reward != script->reward_items.end();
         ++reward) {
        const int result = action_manager.getResultItemFromLottery(
            static_cast<unsigned int>(*reward), reward_items);
        if (result) {
            return result;
        }
    }
    return 0;
}

// IDA_EA: 0x810EEDE
// IDA_NAME: _ZN13EventClassify25CExchangeRandomItemReward22deleteUsedMaterialItemEP5CUserRSt6vectorISt4pairIiiESaIS5_EES8_
// DEMANGLED: EventClassify::CExchangeRandomItemReward::deleteUsedMaterialItem(CUser *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CExchangeRandomItemReward::deleteUsedMaterialItem(
    CUser* user,
    const std::vector<std::pair<int, int> >& used_material_slots,
    std::vector<std::pair<int, int> >& used_material_items) {
    if (!user) {
        return 8;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    std::vector<std::pair<int, int> >::const_iterator current =
        used_material_slots.begin();
    for (; current != used_material_slots.end(); ++current) {
        const Inven_Item item = inventory->GetInvenSlot(1, current->first);
        if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                   current->first,
                                   current->second,
                                   static_cast<eItemDelReason>(3),
                                   true) != 1) {
            break;
        }
        used_material_items.push_back(
            std::make_pair(item.item_id_, current->second));
    }

    if (current == used_material_slots.end()) {
        return 0;
    }

    // Retail compensates every already-deleted material if a later deletion
    // fails.  Keep that transactional behavior instead of silently losing an
    // earlier stack on a partially successful exchange.
    CDataManager* const data_manager = G_CDataManager();
    CItemList* const item_list = data_manager ? data_manager->get_item_list() : 0;
    if (item_list) {
        for (std::vector<std::pair<int, int> >::const_iterator restore =
                 used_material_items.begin();
             restore != used_material_items.end();
             ++restore) {
            Inven_Item restored_item;
            restored_item.item_id_ = restore->first;
            restored_item.set_add_info(restore->second);
            item_list->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, restored_item, restore->second);
            (void)inventory->insertItemIntoInventory(
                restored_item, static_cast<eItemAddReason>(13), true, true);
        }
    }
    return 8;
}

// IDA_EA: 0x810F26E
// IDA_NAME: _ZN13EventClassify25CExchangeRandomItemReward20sendRandomItemRewardEP5CUserRSt6vectorISt4pairIiiESaIS5_EERS3_IiSaIiEERh
// DEMANGLED: EventClassify::CExchangeRandomItemReward::sendRandomItemReward(CUser *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,std::vector&<int,std::allocator<int>>,unsigned char &)
int CExchangeRandomItemReward::sendRandomItemReward(
    CUser* user,
    const std::vector<std::pair<int, int> >& rewards,
    std::vector<int>& mail_result_items,
    uint8_t& reward_count) {
    ExchangeRandomItemRewardScript* const script =
        static_cast<ExchangeRandomItemRewardScript*>(typed_script_);
    if (!script) {
        return 17;
    }

    CEventActionMng action_manager;
    for (std::vector<std::pair<int, int> >::const_iterator reward =
             rewards.begin();
         reward != rewards.end();
         ++reward) {
        script->send_mail.set_item(reward->first, reward->second);
        const int result = action_manager.process_action_send_mail(
            user, script->send_mail, true);
        if (result) {
            return result;
        }
        ++reward_count;
    }
    mail_result_items = script->send_mail.result_item_values;
    script->send_mail.result_item_values.clear();
    return 0;
}

// IDA_EA: 0x8111014
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomRewardC2Ev
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::CAvatarDisjointRandomReward(void)
CAvatarDisjointRandomReward::CAvatarDisjointRandomReward()
    : CScriptBackedEventEntity() {}
CAvatarDisjointRandomReward::~CAvatarDisjointRandomReward() {}

// IDA_EA: 0x810F37A
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::set_event_data(int,InGameEventScript *)
CEventEntity* CAvatarDisjointRandomReward::set_event_data(
    int record_type,
    InGameEventScript* script) {
    return CScriptBackedEventEntity::set_event_data(record_type, script);
}

// IDA_EA: 0x810F3A4
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward24checkAvatarDisjointErrorEP5CUsertR10Inven_ItemPK5CItem
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::checkAvatarDisjointError(CUser *,unsigned short,Inven_Item &,CItem const*)
int CAvatarDisjointRandomReward::checkAvatarDisjointError(
    CUser* user,
    uint16_t avatar_slot,
    const Inven_Item& inventory_item,
    const CItem* item_proto) {
    if (!user || !item_proto || !item_proto->vtable_) {
        return 17;
    }

    const int socket_error = user->isEnableAvatarSocketAction();
    if (socket_error) {
        return socket_error;
    }
    if (user->CheckItemLock(2, static_cast<int>(avatar_slot))) {
        return 213;
    }
    if (!item_proto->get_grade() || item_proto->get_grade() > 3) {
        return 19;
    }

    // The original virtual dispatch is CItem vtable +0x24 (slot 9), the
    // isEnableAvatarFunc(AVATAR_FUNC_FILTER_DISJOINT) override.  The generic
    // public CItem vtable deliberately keeps slots 6..11 opaque, so give
    // this proven slot its exact local call signature here.
    typedef int (*AvatarFunctionSlot)(const CItem*, int);
    const AvatarFunctionSlot is_enable_avatar_func =
        reinterpret_cast<AvatarFunctionSlot>(
            item_proto->vtable_->slots_6_through_9[3]);
    if (!is_enable_avatar_func ||
        is_enable_avatar_func(item_proto, 2) != 1) {
        return 23;
    }

    const CInventory* const inventory = user->getCurCharacInvenR();
    const WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrR();
    if (!avatar_manager || avatar_manager->GetExpireDate(
                               inventory_item.get_add_info())) {
        return 23;
    }

    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager ||
        data_manager->get_event_avatar_list_script()->isfindEventAvatar(
            inventory_item.item_id_)) {
        return 23;
    }
    return inventory_item.upgrade_separate_.IsTradeRestriction() ? 23 : 0;
}

// IDA_EA: 0x810F4E2
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward16deleteAvatarItemEP5CUsertR10Inven_ItemPK5CItem
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::deleteAvatarItem(CUser *,unsigned short,Inven_Item &,CItem const*)
int CAvatarDisjointRandomReward::deleteAvatarItem(
    CUser* user,
    uint16_t avatar_slot,
    const Inven_Item& inventory_item,
    const CItem* item_proto) {
    if (!user || !item_proto) {
        return 22;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const int avatar_key = inventory_item.get_add_info();
    WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrW();
    if (!avatar_manager ||
        inventory->delete_item(static_cast<INVEN_TYPE>(2),
                               static_cast<int>(avatar_slot),
                               1,
                               static_cast<eItemDelReason>(6),
                               true) != 1) {
        return 22;
    }

    char ipg_agency_no[0x18] = {};
    const char* const source_agency_no =
        avatar_manager->GetIPGAgencyNo(avatar_key);
    if (source_agency_no) {
        std::strncpy(ipg_agency_no, source_agency_no,
                     sizeof(ipg_agency_no) - 1);
    }
    if (avatar_manager->UnRegistItem(avatar_key) != 1) {
        return 22;
    }

    inventory->SendAvatarEvent(54,
                               avatar_key,
                               5,
                               static_cast<int>(avatar_slot) +
                                   oldsrv::config::kAvatarInventorySlotBase,
                               0);
    DB_UpdateAvatarHistory::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()),
        avatar_key,
        ipg_agency_no);
    (void)SendDisjointAvatarStatistic(item_proto);
    return 0;
}

// IDA_EA: 0x810F5F4
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward27SendDisjointAvatarStatisticEPK5CItem
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::SendDisjointAvatarStatistic(CItem const*)
int CAvatarDisjointRandomReward::SendDisjointAvatarStatistic(
    const CItem* item_proto) {
    if (!item_proto || !item_proto->vtable_ ||
        !GlobalData::s_statistic_proxy_mgr) {
        return 0;
    }

    Packet_Avater_Disjoint_Statistic packet;
    packet.action_0a = 1;
    packet.first_grade_bucket_0e = item_proto->get_rarity() == 2
                                        ? 2
                                        : item_proto->get_grade() - 1;
    packet.first_item_type_12 = item_proto->vtable_->get_item_type(item_proto);
    packet.first_succeeded_16 = 1;

    CStatisticServerProxy* const proxy =
        GlobalData::s_statistic_proxy_mgr->GetServerProxy(SERVER_GROUP_DEFAULT);
    return proxy ? proxy->SendPacket(
                       reinterpret_cast<char*>(&packet), sizeof(packet))
                 : 0;
}

// IDA_EA: 0x810F694
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward13getRewardItemEPK5CItemRSt6vectorISt4pairIiiESaIS6_EE
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::getRewardItem(CItem const*,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CAvatarDisjointRandomReward::getRewardItem(
    const CItem* item_proto,
    std::vector<std::pair<int, int> >& reward_items) {
    AvatarDisjointRandomRewardScript* const script =
        static_cast<AvatarDisjointRandomRewardScript*>(typed_script_);
    if (!item_proto || !script) {
        return 21;
    }

    const std::map<char, unsigned int>::const_iterator reward =
        script->random_reward_items.find(
            static_cast<char>(item_proto->get_grade()));
    if (reward == script->random_reward_items.end()) {
        return 21;
    }

    CEventActionMng action_manager;
    return action_manager.getResultItemFromLottery(
        reward->second, reward_items);
}

// IDA_EA: 0x810F742
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward25SendCreateEmblemStatisticERSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::SendCreateEmblemStatistic(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CAvatarDisjointRandomReward::SendCreateEmblemStatistic(
    const std::vector<std::pair<int, int> >& reward_items) {
    if (reward_items.size() > 5u || !GlobalData::s_statistic_proxy_mgr) {
        return 0;
    }

    Packet_Emblem_Create_Statistic packet;
    int emitted = 0;
    CDataManager* const data_manager = G_CDataManager();
    for (std::vector<std::pair<int, int> >::const_iterator reward =
             reward_items.begin();
         reward != reward_items.end();
         ++reward) {
        CItem* const item = data_manager ? data_manager->find_item(reward->first)
                                         : 0;
        if (!item || !item->vtable_) {
            return 0;
        }
        if (item->vtable_->get_item_type(item) == 20) {
            packet.emblem_grades_0e[emitted] = item->get_grade();
            packet.emblem_counts_22[emitted] = reward->second;
            if (++emitted > 5) {
                break;
            }
        }
    }
    if (!emitted) {
        return 0;
    }

    packet.emblem_count_0a = emitted;
    CStatisticServerProxy* const proxy =
        GlobalData::s_statistic_proxy_mgr->GetServerProxy(SERVER_GROUP_DEFAULT);
    return proxy ? proxy->SendPacket(
                       reinterpret_cast<char*>(&packet), sizeof(packet))
                 : 0;
}

// IDA_EA: 0x810F88C
// IDA_NAME: _ZN13EventClassify27CAvatarDisjointRandomReward20sendRandomItemRewardEP5CUserRSt6vectorISt4pairIiiESaIS5_EE
// DEMANGLED: EventClassify::CAvatarDisjointRandomReward::sendRandomItemReward(CUser *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CAvatarDisjointRandomReward::sendRandomItemReward(
    CUser* user,
    const std::vector<std::pair<int, int> >& rewards) {
    AvatarDisjointRandomRewardScript* const script =
        static_cast<AvatarDisjointRandomRewardScript*>(typed_script_);
    if (!script) {
        return 17;
    }

    CEventActionMng action_manager;
    for (std::vector<std::pair<int, int> >::const_iterator reward =
             rewards.begin();
         reward != rewards.end();
         ++reward) {
        script->send_mail.set_item(reward->first, reward->second);
        const int result = action_manager.process_action_send_mail(
            user, script->send_mail, false);
        if (result) {
            return result;
        }
    }
    return 0;
}

// IDA_EA: 0x8111030
// IDA_NAME: _ZN13EventClassify16CFatigueQuantityC2Ev
// DEMANGLED: EventClassify::CFatigueQuantity::CFatigueQuantity(void)
CFatigueQuantity::CFatigueQuantity() : CEventEntity() {}

// IDA_EA: 0x8114AAC
// IDA_NAME: _ZN13EventClassify16CFatigueQuantityD2Ev
// DEMANGLED: EventClassify::CFatigueQuantity::~CFatigueQuantity()
CFatigueQuantity::~CFatigueQuantity() {}

// IDA_EA: 0x8114ADC
// IDA_NAME: _ZN13EventClassify16CFatigueQuantityD0Ev
// DEMANGLED: EventClassify::CFatigueQuantity::~CFatigueQuantity()
// The C++ ABI emits this deleting destructor from the virtual destructor
// definition above; it calls D2 and then operator delete, matching the old
// server body.

// IDA_EA: 0x810F95E
// IDA_NAME: _ZN13EventClassify16CFatigueQuantity14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CFatigueQuantity::set_event_data(int,InGameEventScript *)
CEventEntity* CFatigueQuantity::set_event_data(int record_type,
                                               InGameEventScript* script) {
    set_event_record_type(record_type);
    typed_script_ = script;
    return this;
}

// IDA_EA: 0x810F988
// IDA_NAME: _ZN13EventClassify16CFatigueQuantity17get_db_table_nameEv
// DEMANGLED: EventClassify::CFatigueQuantity::get_db_table_name(void)
const std::string& CFatigueQuantity::get_db_table_name() {
    return static_cast<FatigueQuantityScript*>(typed_script_)->value_08;
}

// IDA_EA: 0x8110EDC
// IDA_NAME: _ZNK13EventClassify16CFatigueQuantity13get_user_typeEv
// DEMANGLED: EventClassify::CFatigueQuantity::get_user_type(void)const
int CFatigueQuantity::get_user_type() const {
    const FatigueQuantityScript* fatigue =
        static_cast<const FatigueQuantityScript*>(typed_script_);
    return fatigue ? fatigue->flag_0c : 0;
}

// IDA_EA: 0x811104C
// IDA_NAME: _ZN13EventClassify22CPurchaseCashItemBonusC2Ev
// DEMANGLED: EventClassify::CPurchaseCashItemBonus::CPurchaseCashItemBonus(void)
CPurchaseCashItemBonus::CPurchaseCashItemBonus()
    : CScriptBackedEventEntity() {}
CPurchaseCashItemBonus::~CPurchaseCashItemBonus() {}

// IDA_EA: 0x810F996
// IDA_NAME: _ZN13EventClassify22CPurchaseCashItemBonus14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CPurchaseCashItemBonus::set_event_data(int,InGameEventScript *)
CEventEntity* CPurchaseCashItemBonus::set_event_data(
    int record_type,
    InGameEventScript* script) {
    CScriptBackedEventEntity::set_event_data(record_type, script);
    PurchaseCashItemBonusScript* bonus =
        static_cast<PurchaseCashItemBonusScript*>(script);
    db_table_name_ = bonus ? bonus->db_table_name : "";
    return this;
}

// IDA_EA: 0x810F9C0
// IDA_NAME: _ZN13EventClassify22CPurchaseCashItemBonus17get_db_table_nameEv
// DEMANGLED: EventClassify::CPurchaseCashItemBonus::get_db_table_name(void)
const std::string& CPurchaseCashItemBonus::get_db_table_name() {
    return db_table_name_;
}

// IDA_EA: 0x810FAE0
// IDA_NAME: _ZN13EventClassify22CPurchaseCashItemBonus21verifyGiveBonusRewardERN27PurchaseCashItemBonusScript17PurchaseBonusRuleEi
// DEMANGLED: EventClassify::CPurchaseCashItemBonus::verifyGiveBonusReward(PurchaseCashItemBonusScript::PurchaseBonusRule &,int)
bool CPurchaseCashItemBonus::verifyGiveBonusReward(
    PurchaseCashItemBonusScript::PurchaseBonusRule& rule,
    int purchase_count) {
    return rule.purchase_start_count <= purchase_count &&
           (rule.purchase_end_count >= purchase_count ||
            rule.purchase_end_count == 0) &&
           (purchase_count - rule.purchase_start_count) %
                   rule.purchase_multiple_number ==
               0;
}

// IDA_EA: 0x810F9CE
// IDA_NAME: _ZN13EventClassify22CPurchaseCashItemBonus22updateTargetGoodsCountEP5CUserjP22stCeraShopIPGRequest_t
// DEMANGLED: EventClassify::CPurchaseCashItemBonus::updateTargetGoodsCount(CUser *,unsigned int,stCeraShopIPGRequest_t *)
int CPurchaseCashItemBonus::updateTargetGoodsCount(CUser* user,
                                                   unsigned int charac_no,
                                                   stCeraShopIPGRequest_t* ipg_request) {
    if (!user) {
        cMyTrace trace(
            "void EventClassify::CPurchaseCashItemBonus::updateTargetGoodsCount(CUser*, unsigned int, stCeraShopIPGRequest_t*)",
            1847,
            0);
        return trace("Purchase Error Not Found User(charac_no:%d)",
                     charac_no);
    }

    PurchaseCashItemBonusScript* bonus =
        static_cast<PurchaseCashItemBonusScript*>(typed_script_);
    if (!bonus || !ipg_request) {
        return 0;
    }

    const unsigned int item_index = ipg_request->item_index;
    if (bonus->item_indexes.find(item_index) == bonus->item_indexes.end()) {
        return 0;
    }

    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    DB_UpdatePurchaseCashItemBonus::makeRequest(user->GetUID(),
                                                account_id);
    cMyTrace trace(
        "void EventClassify::CPurchaseCashItemBonus::updateTargetGoodsCount(CUser*, unsigned int, stCeraShopIPGRequest_t*)",
        1858,
        0);
    return trace("CPurchaseCashItemBonus::updateTargetGoodsCount(%d)",
                 account_id);
}

// IDA_EA: 0x810FB34
// IDA_NAME: _ZN13EventClassify22CPurchaseCashItemBonus23sendPurchaseBonusRewardEP5CUseri
// DEMANGLED: EventClassify::CPurchaseCashItemBonus::sendPurchaseBonusReward(CUser *,int)
int CPurchaseCashItemBonus::sendPurchaseBonusReward(CUser* user,
                                                    int purchase_count) {
    if (!user) {
        LogManager::logFormat(
            1,
            "EventClassify.cpp",
            "bool EventClassify::CPurchaseCashItemBonus::sendPurchaseBonusReward(CUser*, int)",
            0x755,
            "CPurchaseCashItemBonus::sendPurchaseBonusReward(pUser(NULL))");
        return 0;
    }

    PurchaseCashItemBonusScript* bonus =
        static_cast<PurchaseCashItemBonusScript*>(typed_script_);

    int send_result = 0;
    unsigned char result = 1;
    for (std::vector<std::pair<
             PurchaseCashItemBonusScript::PurchaseBonusRule,
             Action_SendMail> >::iterator it = bonus->bonus_rules.begin();
         it != bonus->bonus_rules.end();
         ++it) {
        if (!verifyGiveBonusReward(it->first, purchase_count)) {
            continue;
        }

        CEventActionMng action_manager;
        send_result =
            action_manager.process_action_send_mail(user, it->second, true);
        if (send_result) {
            LogManager::logFormat(
                1,
                "EventClassify.cpp",
                "bool EventClassify::CPurchaseCashItemBonus::sendPurchaseBonusReward(CUser*, int)",
                0x763,
                "CPurchaseCashItemBonus::sendPurchaseBonusReward(m_id:%d, charac_no:%d, reward:%d)",
                user->get_acc_id(),
                user->getCurCharacNo(),
                it->second.item_id);
            result = 0;
        }
    }

    return result;
}

// IDA_EA: 0x8111068
// IDA_NAME: _ZN13EventClassify14CObjectBringUpC2Ev
// DEMANGLED: EventClassify::CObjectBringUp::CObjectBringUp(void)
CObjectBringUp::CObjectBringUp() : CScriptBackedEventEntity() {}
CObjectBringUp::~CObjectBringUp() {}

// IDA_EA: 0x810FCB2
// IDA_NAME: _ZN13EventClassify14CObjectBringUp14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CObjectBringUp::set_event_data(int,InGameEventScript *)
CEventEntity* CObjectBringUp::set_event_data(int record_type,
                                             InGameEventScript* script) {
    CScriptBackedEventEntity::set_event_data(record_type, script);
    ObjectBringUpScript* bring_up = static_cast<ObjectBringUpScript*>(script);
    db_table_name_ = bring_up ? bring_up->value_08 : "";
    return this;
}

// IDA_EA: 0x810FCDC
// IDA_NAME: _ZN13EventClassify14CObjectBringUp17get_db_table_nameEv
// DEMANGLED: EventClassify::CObjectBringUp::get_db_table_name(void)
const std::string& CObjectBringUp::get_db_table_name() {
    return db_table_name_;
}

// IDA_EA: 0x810FCEA
// IDA_NAME: _ZN13EventClassify14CObjectBringUp24verifyMaterialItemByRuleEP5CUserj
// DEMANGLED: EventClassify::CObjectBringUp::verifyMaterialItemByRule(CUser *,unsigned int)
bool CObjectBringUp::verifyMaterialItemByRule(CUser* user,
                                              unsigned int item_id) {
    ObjectBringUpScript* const script =
        static_cast<ObjectBringUpScript*>(typed_script_);
    if (script->flag_0c) {
        const unsigned int material_total = static_cast<unsigned char>(
            user->getCurUseBringUpMaterialTotal());
        const std::size_t item_index = script->values_10.size() > material_total
                                           ? material_total
                                           : script->values_10.size() - 1u;
        return static_cast<unsigned int>(script->values_10[item_index]) ==
               item_id;
    }

    for (std::vector<int>::const_iterator value = script->values_10.begin();
         value != script->values_10.end(); ++value) {
        if (static_cast<unsigned int>(*value) == item_id) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x810FE1A
// IDA_NAME: _ZN13EventClassify14CObjectBringUp23verifyGiveBringUpRewardERN19ObjectBringUpScript17ObjectBringUpRuleEi
// DEMANGLED: EventClassify::CObjectBringUp::verifyGiveBringUpReward(ObjectBringUpScript::ObjectBringUpRule &,int)
bool CObjectBringUp::verifyGiveBringUpReward(
    ObjectBringUpScript::ObjectBringUpRule& rule,
    int material_total) {
    if (rule.value_08 == 0) {
        return rule.value_00 == material_total;
    }
    return rule.value_00 <= material_total &&
           (rule.value_04 >= material_total || rule.value_04 == 0) &&
           (material_total - rule.value_00) % rule.value_08 == 0;
}

// IDA_EA: 0x810FE90
// IDA_NAME: _ZN13EventClassify14CObjectBringUp17sendBringUpRewardEP5CUserRSt6vectorIiSaIiEERh
// DEMANGLED: EventClassify::CObjectBringUp::sendBringUpReward(CUser *,std::vector<int,std::allocator<int> > &,unsigned char &)
int CObjectBringUp::sendBringUpReward(CUser* user,
                                      std::vector<int>& reward_values,
                                      unsigned char& reward_count) {
    if (!user) {
        LogManager::logFormat(
            1,
            "EventClassify.cpp",
            "bool EventClassify::CObjectBringUp::sendBringUpReward(CUser*, IntVector&, unsigned char&)",
            0x7AD,
            "CObjectBringUp::sendBringUpReward(pUser(NULL))");
        return 0;
    }

    ObjectBringUpScript* const script =
        static_cast<ObjectBringUpScript*>(typed_script_);
    const int reward_item_id = script->bring_up_rules_28[1].second.item_id;

    for (std::vector<std::pair<ObjectBringUpScript::ObjectBringUpRule,
                               Action_SendMail> >::iterator rule =
             script->bring_up_rules_28.begin();
         rule != script->bring_up_rules_28.end(); ++rule) {
        const int material_total = static_cast<unsigned char>(
            user->getCurUseBringUpMaterialTotal());
        if (!verifyGiveBringUpReward(rule->first, material_total)) {
            continue;
        }

        CEventActionMng action_manager;
        if (reward_item_id == rule->second.item_id) {
            if (static_cast<unsigned char>(user->getBringUpRewardFlag())) {
                continue;
            }

            if (action_manager.process_action_send_mail(user,
                                                        rule->second,
                                                        false)) {
                LogManager::logFormat(
                    1,
                    "EventClassify.cpp",
                    "bool EventClassify::CObjectBringUp::sendBringUpReward(CUser*, IntVector&, unsigned char&)",
                    0x7C3,
                    "CObjectBringUp::sendBringUpReward(m_id:%d, charac_no:%d, reward:%d)",
                    user->get_acc_id(),
                    user->getCurCharacNo(),
                    rule->second.item_id);
                return 0;
            }

            reward_values.insert(reward_values.end(),
                                 rule->second.result_item_values.begin(),
                                 rule->second.result_item_values.end());
            rule->second.result_item_values.clear();
            ++reward_count;
            user->setBringUpRewardFlag(true);

            CStreamGuard guard(
                GlobalData::stream_pool()->Acquire("EventClassify.cpp", 1996),
                true);
            *guard << 714;
            *guard << -1;
            SIG_OBJECT_BRING_UP* const payload =
                guard.GetInBuffer<SIG_OBJECT_BRING_UP>();
            payload->m_id = static_cast<uint32_t>(user->get_acc_id());
            payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
            payload->server_no = static_cast<uint32_t>(
                static_cast<unsigned char>(G_CEnvironment()->get_server_group()));
            payload->use_material_count = static_cast<uint8_t>(
                user->getCurUseBringUpMaterialCount());
            payload->give_reward_flag = static_cast<uint8_t>(
                user->getBringUpRewardFlag());
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
            user->resetCurUseBringUpMaterialCount();
            continue;
        }

        if (action_manager.process_action_send_mail(user, rule->second, false)) {
            LogManager::logFormat(
                1,
                "EventClassify.cpp",
                "bool EventClassify::CObjectBringUp::sendBringUpReward(CUser*, IntVector&, unsigned char&)",
                0x7E2,
                "CObjectBringUp::sendBringUpReward(m_id:%d, charac_no:%d, reward:%d)",
                user->get_acc_id(),
                user->getCurCharacNo(),
                rule->second.item_id);
            return 0;
        }

        reward_values.insert(reward_values.end(),
                             rule->second.result_item_values.begin(),
                             rule->second.result_item_values.end());
        rule->second.result_item_values.clear();
        ++reward_count;
    }
    return 1;
}

// IDA_EA: 0x8110386
// IDA_NAME: _ZN13EventClassify14CObjectBringUp24sendEventItemFromFatigueEP5CUserj
// DEMANGLED: EventClassify::CObjectBringUp::sendEventItemFromFatigue(CUser *,unsigned int)
int CObjectBringUp::sendEventItemFromFatigue(CUser* user,
                                             unsigned int amount) {
    ObjectBringUpScript* const script =
        static_cast<ObjectBringUpScript*>(typed_script_);
    if (!script || script->mail_rules_1c.empty()) {
        return 1;
    }

    const unsigned int current = static_cast<unsigned int>(
        static_cast<uint16_t>(user->getCurCharacUsedFatigue()));
    const unsigned int previous = current - amount;

    for (std::vector<std::pair<std::vector<unsigned int>,
                               Action_SendMail> >::iterator rule =
             script->mail_rules_1c.begin();
         rule != script->mail_rules_1c.end(); ++rule) {
        for (std::vector<unsigned int>::const_iterator threshold =
                 rule->first.begin();
             threshold != rule->first.end(); ++threshold) {
            if (*threshold <= previous || *threshold > current) {
                continue;
            }

            CEventActionMng action_manager;
            if (action_manager.process_action_send_mail(
                    user, rule->second, true)) {
                LogManager::logFormat(
                    1,
                    "EventClassify.cpp",
                    "void EventClassify::CObjectBringUp::sendEventItemFromFatigue(CUser*, unsigned int)",
                    0x803,
                    "CObjectBringUp::sendEventItemFromFatigue(m_id:%d, charac_no:%d, reward:%d)",
                    user->get_acc_id(),
                    user->getCurCharacNo(),
                    rule->second.item_id);
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8111084
// IDA_NAME: _ZN13EventClassify15CPcRoomPlayTimeC2Ev
// DEMANGLED: EventClassify::CPcRoomPlayTime::CPcRoomPlayTime(void)
CPcRoomPlayTime::CPcRoomPlayTime() : CScriptBackedEventEntity() {}
CPcRoomPlayTime::~CPcRoomPlayTime() {}

// IDA_EA: 0x8110814
// IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime11is_eventingEv
// DEMANGLED: EventClassify::CPcRoomPlayTime::is_eventing(void)
int CPcRoomPlayTime::is_eventing() {
    if (!CEventEntity::is_eventing()) {
        return 0;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(92)
            : 0;
    return repeat_event && repeat_event->IsEventing(0) ? 1 : 0;
}

// IDA_EA: 0x8110872
// IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CPcRoomPlayTime::set_event_data(int,InGameEventScript *)
CEventEntity* CPcRoomPlayTime::set_event_data(int record_type,
                                              InGameEventScript* script) {
    CScriptBackedEventEntity::set_event_data(record_type, script);
    PcBangPlayTimeScript* pc_room = static_cast<PcBangPlayTimeScript*>(script);
    db_table_name_ = pc_room ? pc_room->value_08 : "";
    return this;
}

// IDA_EA: 0x8110864
// IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime17get_db_table_nameEv
// DEMANGLED: EventClassify::CPcRoomPlayTime::get_db_table_name(void)
const std::string& CPcRoomPlayTime::get_db_table_name() {
    return db_table_name_;
}

// IDA_EA: 0x8110A38
// IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime25verifyUserPcRoomPlayEventEP5CUser
// DEMANGLED: EventClassify::CPcRoomPlayTime::verifyUserPcRoomPlayEvent(CUser *)
int CPcRoomPlayTime::verifyUserPcRoomPlayEvent(CUser* user) {
    if (is_eventing() != 1) {
        return 178;
    }
    if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12))) {
        return 7;
    }

    const PcBangPlayTimeScript* const pc_room =
        static_cast<const PcBangPlayTimeScript*>(typed_script_);
    if (pc_room &&
        user->getPcRoomPlayTimeRewardCnt() < pc_room->flag_10) {
        return 0;
    }
    return 18;
}

// IDA_EA: 0x811089C
// IDA_NAME: _ZN13EventClassify15CPcRoomPlayTime26processEventPcRoomPlayTimeEP5CUser
// DEMANGLED: EventClassify::CPcRoomPlayTime::processEventPcRoomPlayTime(CUser *)
int CPcRoomPlayTime::processEventPcRoomPlayTime(CUser* user) {
    user->collectPcRoomPlayTime();

    const PcBangPlayTimeScript* const pc_room =
        static_cast<const PcBangPlayTimeScript*>(typed_script_);
    if (user->getTotalPcRoomPlayTime() <
        static_cast<unsigned int>(pc_room->value_0c)) {
        return 19;
    }

    user->incPcRoomPlayTimeRewardCnt();
    const unsigned char reward_count = user->getPcRoomPlayTimeRewardCnt();
    const unsigned int total_play_time = user->getTotalPcRoomPlayTime();
    const int charac_no = user->getCurCharacNo();
    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());

    cMyTrace trace(
        "int EventClassify::CPcRoomPlayTime::processEventPcRoomPlayTime(CUser*)",
        2282,
        0);
    trace("<PCROOM_PLAY_TIME_EVENT> send_reward m_id(%d), charac_no(%d), total_time(%d), reward_cnt(%d)",
          account_id,
          charac_no,
          total_play_time,
          static_cast<unsigned int>(reward_count));

    user->resetTotalPcRoomPlayTime();
    DB_SavePcRoomPlayTimeEvent::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        user->getTotalPcRoomPlayTime(),
        user->getPcRoomPlayTimeRewardCnt());

    Packet_PcRoomPlayTimeReward packet;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        packet.byte_0a = static_cast<uint8_t>(user->GetServerGroup());
    } else {
        packet.byte_0a = G_CEnvironment()->get_accept_handshake_flag_888();
    }
    packet.dword_0b = static_cast<unsigned int>(user->get_acc_id());
    packet.dword_0f = static_cast<unsigned int>(user->getCurCharacNo());

    CMonitorServerProxy* const server_proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    server_proxy->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    return 0;
}

// IDA_EA: 0x81110A0
// IDA_NAME: _ZN13EventClassify20CUsedFatigueGiveItemC2Ev
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::CUsedFatigueGiveItem(void)
CUsedFatigueGiveItem::CUsedFatigueGiveItem()
    : CScriptBackedEventEntity() {}
CUsedFatigueGiveItem::~CUsedFatigueGiveItem() {}

// IDA_EA: 0x811056E
// IDA_NAME: _ZN13EventClassify20CUsedFatigueGiveItem11is_eventingEv
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::is_eventing(void)
int CUsedFatigueGiveItem::is_eventing() {
    return CEventEntity::is_eventing();
}

// IDA_EA: 0x81105BE
// IDA_NAME: _ZN13EventClassify20CUsedFatigueGiveItem14set_event_dataEiP17InGameEventScript
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::set_event_data(int,InGameEventScript *)
CEventEntity* CUsedFatigueGiveItem::set_event_data(int record_type,
                                                   InGameEventScript* script) {
    CScriptBackedEventEntity::set_event_data(record_type, script);
    UsedFatigueGiveItemScript* fatigue =
        static_cast<UsedFatigueGiveItemScript*>(script);
    db_table_name_ = fatigue ? fatigue->value_08 : "";
    return this;
}

// IDA_EA: 0x81105E8
// IDA_NAME: _ZN13EventClassify20CUsedFatigueGiveItem17get_db_table_nameEv
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::get_db_table_name(void)
const std::string& CUsedFatigueGiveItem::get_db_table_name() {
    return db_table_name_;
}

// IDA_EA: 0x81107EA
// IDA_NAME: _ZNK13EventClassify20CUsedFatigueGiveItem16getIsCharacEventERb
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::getIsCharacEvent(bool &)const
int CUsedFatigueGiveItem::getIsCharacEvent(bool& is_charac_event) const {
    const UsedFatigueGiveItemScript* fatigue =
        static_cast<const UsedFatigueGiveItemScript*>(typed_script_);
    if (!fatigue) {
        return 0;
    }
    is_charac_event = fatigue->flag_14 != 0;
    return 1;
}

// IDA_EA: 0x8110EEC
// IDA_NAME: _ZNK13EventClassify20CUsedFatigueGiveItem13get_user_typeEv
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::get_user_type(void)const
int CUsedFatigueGiveItem::get_user_type() const {
    const UsedFatigueGiveItemScript* fatigue =
        static_cast<const UsedFatigueGiveItemScript*>(typed_script_);
    return fatigue ? fatigue->flag_0c : 0;
}

// IDA_EA: 0x8110756
// IDA_NAME: _ZN13EventClassify20CUsedFatigueGiveItem15verifyApplyUserEP5CUser
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::verifyApplyUser(CUser *)
bool CUsedFatigueGiveItem::verifyApplyUser(CUser* user) {
    const UsedFatigueGiveItemScript* const fatigue =
        static_cast<const UsedFatigueGiveItemScript*>(typed_script_);
    if (!fatigue || !user) {
        return false;
    }
    if (fatigue->value_18 >= 0 &&
        fatigue->value_18 != user->get_charac_job()) {
        return false;
    }
    const int user_type = get_user_type();
    if (user_type == 0) {
        return true;
    }
    return user_type == 1 &&
           user->isAffectedPremium(
               static_cast<ENUM_PREMIUM_TYPE>(12)) != 0;
}

// IDA_EA: 0x81105F6
// IDA_NAME: _ZN13EventClassify20CUsedFatigueGiveItem24sendEventItemFromFatigueEP5CUserj
// DEMANGLED: EventClassify::CUsedFatigueGiveItem::sendEventItemFromFatigue(CUser *,unsigned int)
bool CUsedFatigueGiveItem::sendEventItemFromFatigue(
    CUser* user,
    unsigned int amount) {
    UsedFatigueGiveItemScript* const fatigue =
        static_cast<UsedFatigueGiveItemScript*>(typed_script_);
    if (!fatigue || !user || fatigue->value_10 == 0) {
        return false;
    }

    const unsigned int current =
        user->GetCurCharacUsedGiftFatigueQuantity();
    if (current == 0u) {
        return false;
    }
    unsigned int threshold = current - amount;
    if (fatigue->value_1c >= 0 &&
        static_cast<unsigned int>(fatigue->value_1c) <= threshold) {
        return false;
    }

    while (threshold < current) {
        if (fatigue->value_1c >= 0 &&
            static_cast<unsigned int>(fatigue->value_1c) <= threshold) {
            break;
        }
        ++threshold;
        if (threshold %
                static_cast<unsigned int>(fatigue->value_10) !=
            0u) {
            continue;
        }

        CEventActionMng action_manager;
        if (action_manager.process_action_send_mail(
                user, fatigue->send_mail_20, true)) {
            LogManager::logFormat(
                1,
                "EventClassify.cpp",
                "void EventClassify::CUsedFatigueGiveItem::sendEventItemFromFatigue(CUser*, unsigned int)",
                0x83F,
                "CUsedFatigueGiveItem::sendEventItemFromFatigue(m_id:%d, charac_no:%d, reward:%d)",
                user->get_acc_id(),
                user->getCurCharacNo(),
                fatigue->send_mail_20.item_id);
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x810B136
// IDA_NAME: _ZN13EventClassify15CEventScriptMngC2Ev
// DEMANGLED: EventClassify::CEventScriptMng::CEventScriptMng(void)
CEventScriptMng::CEventScriptMng()
    : info_list_(),
      script_loader_(),
      event_entities_(),
      today_time_(0),
      today_day_(0),
      reserved_4d_(),
      event_nums_() {
    event_entities_.clear();
    info_list_.events.clear();
    script_loader_.scripts_.clear();
}

// IDA_EA: 0x810B218
// IDA_NAME: _ZN13EventClassify15CEventScriptMngD2Ev
// DEMANGLED: EventClassify::CEventScriptMng::~CEventScriptMng()
CEventScriptMng::~CEventScriptMng() {
    for (InGameEventScriptLoader::ScriptMap::iterator it =
             script_loader_.scripts_.begin();
         it != script_loader_.scripts_.end();
         ++it) {
        delete it->second;
        it->second = 0;
    }

    std::set<CEventEntity*> deleted_entities;
    for (EntityMap::iterator it = event_entities_.begin();
         it != event_entities_.end();
         ++it) {
        if (it->second && deleted_entities.insert(it->second).second) {
            delete it->second;
        }
        it->second = 0;
    }
}

// IDA_EA: 0x810B0CC
// IDA_NAME: _ZN13EventClassify15CEventScriptMng31get_convert_to_user_confirmflagEi
// DEMANGLED: EventClassify::CEventScriptMng::get_convert_to_user_confirmflag(int)
int CEventScriptMng::get_convert_to_user_confirmflag(int event_type) {
    switch (event_type) {
        case 401:
        case 145:
            return 0;
        case 402:
        case 146:
            return 1;
        case 412:
        case 156:
            return 2;
        default:
            return -1;
    }
}

// IDA_EA: 0x810B110
// IDA_NAME: _ZN13EventClassify15CEventScriptMng17get_db_table_nameEi
// DEMANGLED: EventClassify::CEventScriptMng::get_db_table_name(int)
const std::string* CEventScriptMng::get_db_table_name(int event_type) {
    CEventEntity* entity = get_event_entity(event_type);
    if (!entity) {
        return 0;
    }
    return &entity->get_db_table_name();
}

// IDA_EA: 0x810B402
// IDA_NAME: _ZN13EventClassify15CEventScriptMng19create_event_entityEib
// DEMANGLED: EventClassify::CEventScriptMng::create_event_entity(int,bool)
CEventEntity* CEventScriptMng::create_event_entity(int event_id,
                                                   bool base_entity) {
    if (base_entity) {
        return new CEventEntity();
    }

    switch (event_id) {
        case 86:
            return new CAttendance();
        case 92:
            return new CPcRoomPlayTime();
        case 101:
            return new CUsedFatigueGiveItem();
        case 401:
        case 402:
            return new CAccountFirstLogin();
        case 403:
            return new CAddItemCountDB();
        case 404:
        case 413:
            return new CLevelUpReward();
        case 406:
            return new CExchangeRandomItemReward();
        case 407:
            return new CAvatarDisjointRandomReward();
        case 409:
            return new CFatigueQuantity();
        case 410:
            return new CPurchaseCashItemBonus();
        case 411:
            return new CObjectBringUp();
        case 412:
            return new CAccountFirstLoginDaily();
        default:
            return 0;
    }
}

// IDA_EA: 0x810B96A
// IDA_NAME: _ZN13EventClassify15CEventScriptMng16get_event_entityEi
// DEMANGLED: EventClassify::CEventScriptMng::get_event_entity(int)
CEventEntity* CEventScriptMng::get_event_entity(int event_type) {
    EntityMap::iterator it = event_entities_.find(event_type);
    if (it == event_entities_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x810B9D4
// IDA_NAME: _ZNK13EventClassify15CEventScriptMng15is_event_entityEi
// DEMANGLED: EventClassify::CEventScriptMng::is_event_entity(int)const
bool CEventScriptMng::is_event_entity(int event_type) const {
    return event_entities_.find(event_type) != event_entities_.end();
}

// IDA_EA: 0x810BA36
// IDA_NAME: _ZN13EventClassify15CEventScriptMng11is_eventingEi
// DEMANGLED: EventClassify::CEventScriptMng::is_eventing(int)
int CEventScriptMng::is_eventing(int event_type) {
    if (event_type > 0x19c) {
        return 0;
    }
    CEventEntity* entity = get_event_entity(static_cast<int>(event_type));
    return entity ? entity->is_eventing() : 0;
}

// IDA_EA: 0x810BDE0
// IDA_NAME: _ZN13EventClassify15CEventScriptMng27process_account_first_loginEiP5CUserSt6bitsetILj32EE
// DEMANGLED: EventClassify::CEventScriptMng::process_account_first_login(int,CUser *,std::bitset<32u>)
void CEventScriptMng::process_account_first_login(
    int event_type,
    CUser* user,
    std::bitset<32> user_confirm_flags) {
    if (!user) {
        return;
    }
    if (is_eventing(event_type) != 1) {
        return;
    }

    const int flag_index = get_convert_to_user_confirmflag(event_type);
    if (user_confirm_flags[static_cast<size_t>(flag_index)]) {
        return;
    }

    CEventEntity* const event_entity = get_event_entity(event_type);
    if (event_entity) {
        static_cast<CAccountFirstLogin*>(event_entity)->do_action(
            event_type, user, user_confirm_flags);
    }
}

// IDA_EA: 0x810BEA8
// IDA_NAME: _ZN13EventClassify15CEventScriptMng25process_add_item_count_dbEP5CUserii
// DEMANGLED: EventClassify::CEventScriptMng::process_add_item_count_db(CUser *,int,int)
void CEventScriptMng::process_add_item_count_db(CUser* user,
                                                int item_id,
                                                int count) {
    if (!user) {
        return;
    }
    CAddItemCountDB* const event =
        static_cast<CAddItemCountDB*>(get_event_entity(403));
    if (event && event->is_event_item(item_id)) {
        user->IncCurCharacEventAddItemCount(
            static_cast<uint16_t>(count));
    }
}

// IDA_EA: 0x810BF00
// IDA_NAME: _ZN13EventClassify15CEventScriptMng26process_add_quest_count_dbEP5CUseri
// DEMANGLED: EventClassify::CEventScriptMng::process_add_quest_count_db(CUser *,int)
void CEventScriptMng::process_add_quest_count_db(CUser* user,
                                                 int quest_id) {
    if (!user) {
        return;
    }
    CAddItemCountDB* const event =
        static_cast<CAddItemCountDB*>(get_event_entity(403));
    if (event && event->is_event_quest(quest_id)) {
        user->IncCurCharacEventAddItemCount(1u);
    }
}

// IDA_EA: 0x810BF56
// IDA_NAME: _ZN13EventClassify15CEventScriptMng23process_level_up_rewardEP5CUserb
// DEMANGLED: EventClassify::CEventScriptMng::process_level_up_reward(CUser *,bool)
void CEventScriptMng::process_level_up_reward(
    CUser* user,
    bool include_lower_levels) {
    if (!user || is_eventing(404) != 1) {
        return;
    }
    CLevelUpReward* const event =
        static_cast<CLevelUpReward*>(get_event_entity(404));
    if (!event) {
        return;
    }

    const int charac_level = user->get_charac_level();
    const int charac_job = user->get_charac_job();
    const int first_grow_type = user->getCurCharFirstGrowType();
    if (event->is_recv_check_item()) {
        if (event->check_recv_level(charac_level,
                                    charac_job,
                                    first_grow_type,
                                    include_lower_levels)) {
            DB_SelectLevelUpReward::makeRequest(
                user->GetUID(),
                static_cast<unsigned int>(user->getCurCharacNo()),
                charac_level);
        }
        return;
    }

    if (event->set_reward_item(charac_level,
                               charac_job,
                               first_grow_type)) {
        event->do_action(user);
    }
}

// IDA_EA: 0x810C0A6
// IDA_NAME: _ZN13EventClassify15CEventScriptMng34process_level_up_reward_recv_levelEP5CUserii
// DEMANGLED: EventClassify::CEventScriptMng::process_level_up_reward_recv_level(CUser *,int,int)
void CEventScriptMng::process_level_up_reward_recv_level(
    CUser* user,
    int current_level,
    int received_level) {
    if (!user || is_eventing(404) != 1) {
        return;
    }
    CLevelUpReward* const event =
        static_cast<CLevelUpReward*>(get_event_entity(404));
    if (event) {
        event->send_reward_from_recv_level(
            user, current_level, received_level);
    }
}

// IDA_EA: 0x810C110
// IDA_NAME: _ZN13EventClassify15CEventScriptMng25process_add_fatigue_countEP5CUseri
// DEMANGLED: EventClassify::CEventScriptMng::process_add_fatigue_count(CUser *,int)
void CEventScriptMng::process_add_fatigue_count(CUser* user,
                                                int count) {
    if (user && is_eventing(86) == 1) {
        user->set_server_fatigue_count(static_cast<short>(count));
    }
}

// IDA_EA: 0x810C152
// IDA_NAME: _ZN13EventClassify15CEventScriptMng16inc_user_fatigueEP5CUserj
// DEMANGLED: EventClassify::CEventScriptMng::inc_user_fatigue(CUser *,unsigned int)
bool CEventScriptMng::inc_user_fatigue(CUser* user,
                                       unsigned int amount) {
    if (!user || is_eventing(409) != 1) {
        return false;
    }
    CFatigueQuantity* const event =
        static_cast<CFatigueQuantity*>(get_event_entity(409));
    if (!event) {
        return false;
    }
    const int user_type = event->get_user_type();
    if (user_type != 0 &&
        (user_type != 1 || user->GetPCRoomNo() == 0)) {
        return false;
    }
    user->IncCurCharacUsedFatigueQuantity(amount);
    return true;
}

// IDA_EA: 0x810C1EA
// IDA_NAME: _ZN13EventClassify15CEventScriptMng22send_gift_fatigue_itemEP5CUserj
// DEMANGLED: EventClassify::CEventScriptMng::send_gift_fatigue_item(CUser *,unsigned int)
bool CEventScriptMng::send_gift_fatigue_item(CUser* user,
                                             unsigned int amount) {
    if (is_eventing(101) != 1) {
        return false;
    }
    CUsedFatigueGiveItem* const event =
        static_cast<CUsedFatigueGiveItem*>(get_event_entity(101));
    if (!event || !event->verifyApplyUser(user)) {
        return false;
    }
    user->IncCurCharacUsedGiftFatigueQuantity(amount);
    return event->sendEventItemFromFatigue(user, amount);
}

// IDA_EA: 0x810C33A
// IDA_NAME: _ZN13EventClassify15CEventScriptMng26send_bring_up_item_fatigueEP5CUseri
// DEMANGLED: EventClassify::CEventScriptMng::send_bring_up_item_fatigue(CUser *,int)
int CEventScriptMng::send_bring_up_item_fatigue(CUser* user,
                                                int amount) {
    if (is_eventing(411) != 1) {
        return 0;
    }
    CObjectBringUp* const event =
        static_cast<CObjectBringUp*>(get_event_entity(411));
    if (!event) {
        return 0;
    }
    event->sendEventItemFromFatigue(
        user, static_cast<unsigned int>(amount));
    return 1;
}

// IDA_EA: 0x810C3A4
// IDA_NAME: _ZN13EventClassify15CEventScriptMng27send_pc_room_play_init_timeEP5CUser
// DEMANGLED: EventClassify::CEventScriptMng::send_pc_room_play_init_time(CUser *)
bool CEventScriptMng::send_pc_room_play_init_time(CUser* user) {
    if (!user || is_eventing(92) != 1) {
        return false;
    }

    CPcRoomPlayTime* const event =
        static_cast<CPcRoomPlayTime*>(get_event_entity(92));
    if (!event || event->verifyUserPcRoomPlayEvent(user) != 0) {
        return false;
    }

    user->sendNotiPcRoomPlayTime();
    return true;
}

// IDA_EA: 0x810C40E
// IDA_NAME: _ZN13EventClassify15CEventScriptMng20send_event_init_dataEP5CUserb
// DEMANGLED: EventClassify::CEventScriptMng::send_event_init_data(CUser *,bool)
void CEventScriptMng::send_event_init_data(CUser* user, bool reset) {
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 374);
    packet.put_int(today_time_);
    int count_index = packet.get_index();
    packet.put_byte(0);
    unsigned char event_count = 0;

    for (EntityMap::iterator it = event_entities_.begin();
         it != event_entities_.end(); ++it) {
        if (it->second && is_eventing(it->first) &&
            it->second->send_init_data(
                user,
                packet,
                static_cast<short>(it->first),
                today_day_,
                reset)) {
            ++event_count;
        }
    }

    packet.put_byte(count_index, event_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x810C82E
// IDA_NAME: _ZN13EventClassify15CEventScriptMng29process_update_ingame_historyEP5CUser
// DEMANGLED: EventClassify::CEventScriptMng::process_update_ingame_history(CUser *)
void CEventScriptMng::process_update_ingame_history(CUser* user) {
    if (!user) {
        return;
    }
    const std::map<int, bool>* const updates =
        user->get_ingame_event_history_update();
    const std::map<int, std::pair<int, int> >* const history =
        user->get_ingame_event_history();
    if (!updates || !history) {
        return;
    }

    for (std::map<int, bool>::const_iterator update = updates->begin();
         update != updates->end(); ++update) {
        if (!update->second) {
            continue;
        }
        std::map<int, std::pair<int, int> >::const_iterator value =
            history->find(update->first);
        if (value == history->end()) {
            continue;
        }
        DB_UpdateInGameEventHistory::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            update->first,
            value->second.first,
            value->second.second);
    }
}

// IDA_EA: 0x810CB2C
// IDA_NAME: _ZN13EventClassify15CEventScriptMng27send_event_init_record_dataEP5CUser
// DEMANGLED: EventClassify::CEventScriptMng::send_event_init_record_data(CUser *)
void CEventScriptMng::send_event_init_record_data(CUser* user) {
    if (!user) {
        return;
    }
    const std::map<int, std::pair<int, int> >* const history =
        user->get_ingame_event_history();
    if (!history) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 375);
    int generic_record_count = 0;
    for (std::map<int, std::pair<int, int> >::const_iterator value =
             history->begin();
         value != history->end(); ++value) {
        if (value->first != RainbowSnake::kInGameEventHistoryType &&
            value->first != KiriCargo::kInGameEventHistoryType) {
            ++generic_record_count;
        }
    }
    packet.put_byte(generic_record_count);
    for (std::map<int, std::pair<int, int> >::const_iterator value =
             history->begin();
         value != history->end(); ++value) {
        // Type 15 is consumed only through CMD580's {u8 stage,u16 points}
        // response. Feeding it to generic NOTI375 loses history_2 and aliases
        // an unrelated client event record.
        if (value->first == RainbowSnake::kInGameEventHistoryType ||
            value->first == KiriCargo::kInGameEventHistoryType) {
            continue;
        }
        packet.put_short(value->first);
        packet.put_int(value->second.first);
    }
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace EventClassify

// IDA_EA: 0x8982A4A
// IDA_NAME: _ZN23InGameEventScriptLoader23importInGameEventScriptEiRKSs
// DEMANGLED: InGameEventScriptLoader::importInGameEventScript(int,std::string const&)
InGameEventScript* InGameEventScriptLoader::importInGameEventScript(
    int event_id,
    const std::string& filename) {
    InGameEventScript* script = 0;
    int loaded = 1;
    switch (event_id) {
        case 86: {
            AttendanceScript* attendance = new AttendanceScript();
            script = attendance;
            loaded = importAttendanceScript(attendance, filename.c_str());
            break;
        }
        case 92: {
            PcBangPlayTimeScript* pc_room = new PcBangPlayTimeScript();
            script = pc_room;
            loaded = importPcBangPlayTimeScript(pc_room, filename.c_str());
            break;
        }
        case 101: {
            UsedFatigueGiveItemScript* fatigue =
                new UsedFatigueGiveItemScript();
            script = fatigue;
            loaded = importUsedFatigueGiveItemEventScript(fatigue,
                                                          filename.c_str());
            break;
        }
        case 401:
        case 402:
        case 412: {
            AccountFirstLoginScript* login = new AccountFirstLoginScript();
            script = login;
            loaded = importAccountFirstLoginEventScript(login,
                                                        filename.c_str());
            break;
        }
        case 403: {
            AddItemCountDBScript* add_item = new AddItemCountDBScript();
            script = add_item;
            loaded = importAddItemCountDBEventScript(add_item,
                                                     filename.c_str());
            break;
        }
        case 404:
        case 413: {
            LevelUpRewardScript* level_up = new LevelUpRewardScript();
            script = level_up;
            loaded = importLevelUpRewardScript(level_up, filename.c_str());
            break;
        }
        case 406: {
            ExchangeRandomItemRewardScript* exchange =
                new ExchangeRandomItemRewardScript();
            script = exchange;
            loaded = importExchangeRandomItemRewardScript(exchange,
                                                          filename.c_str());
            break;
        }
        case 407: {
            AvatarDisjointRandomRewardScript* avatar =
                new AvatarDisjointRandomRewardScript();
            script = avatar;
            loaded = importAvatarDisjointRandomRewardScript(
                avatar, filename.c_str());
            break;
        }
        case 409: {
            FatigueQuantityScript* fatigue = new FatigueQuantityScript();
            script = fatigue;
            loaded = importFatigueQuantityEventScript(fatigue,
                                                      filename.c_str());
            break;
        }
        case 410: {
            PurchaseCashItemBonusScript* bonus =
                new PurchaseCashItemBonusScript();
            script = bonus;
            loaded = importPurchaseCashItemBonusEventScript(bonus,
                                                            filename.c_str());
            break;
        }
        case 411: {
            ObjectBringUpScript* bring_up = new ObjectBringUpScript();
            script = bring_up;
            loaded = importObjectBringUpEventScript(bring_up,
                                                    filename.c_str());
            break;
        }
        default:
            break;
    }

    if (loaded != 1) {
        delete script;
        return 0;
    }
    return script;
}

// IDA_EA: 0x8982F10
// IDA_NAME: _ZN23InGameEventScriptLoader17insertEventScriptERKSsP17InGameEventScript
// DEMANGLED: InGameEventScriptLoader::insertEventScript(std::string const&,InGameEventScript *)
InGameEventScript* InGameEventScriptLoader::insertEventScript(
    const std::string& filename,
    InGameEventScript* script) {
    if (script) {
        scripts_[filename] = script;
    }
    return script;
}

// IDA_EA: 0x8982F38
// IDA_NAME: _ZN23InGameEventScriptLoader14getEventScriptEiRKSs
// DEMANGLED: InGameEventScriptLoader::getEventScript(int,std::string const&)
InGameEventScript* InGameEventScriptLoader::getEventScript(
    int event_id,
    const std::string& filename) {
    InGameEventScriptLoader::ScriptMap::iterator it = scripts_.find(filename);
    if (it != scripts_.end()) {
        return it->second;
    }
    return insertEventScript(filename,
                             importInGameEventScript(event_id, filename));
}

namespace EventClassify {

// IDA_EA: 0x810BA7A
// IDA_NAME: _ZN13EventClassify15CEventScriptMng14initScriptlistER19InGameEventInfoList
// DEMANGLED: EventClassify::CEventScriptMng::initScriptlist(InGameEventInfoList &)
int CEventScriptMng::initScriptlist(InGameEventInfoList& info_list) {
    if (&info_list != &info_list_) {
        info_list_ = info_list;
    }

    for (std::map<int, InGameEventInfo>::iterator it =
             info_list_.events.begin();
         it != info_list_.events.end();
         ++it) {
        const int record_type = it->first;
        InGameEventInfo& info = it->second;
        const int event_id = static_cast<int>(info.first_dword);
        if (!info.fifth_dword) {
            continue;
        }

        InGameEventScript* script = script_loader_.getEventScript(event_id, info.event_name);
        if (!script) {
            return 0;
        }

        CEventEntity* entity = create_event_entity(event_id, false);
        if (!entity) {
            return 0;
        }
        event_entities_[event_id] = entity;
        if (record_type != event_id) {
            event_entities_[record_type] = entity;
        }
        entity->set_event_flag(info.fifth_dword != 0);
        entity->set_event_data(record_type, script);
        entity->set_event_start_time(static_cast<int>(info.third_dword));
        entity->set_event_end_time(static_cast<int>(info.fourth_dword));
    }

    init_evnet_entity();
    return 1;
}

// IDA_EA: 0x810BD14
// IDA_NAME: _ZN13EventClassify15CEventScriptMng17init_evnet_entityEv
// DEMANGLED: EventClassify::CEventScriptMng::init_evnet_entity(void)
bool CEventScriptMng::init_evnet_entity() {
    init_event_nums();
    for (std::vector<int>::const_iterator it = event_nums_.begin();
         it != event_nums_.end();
         ++it) {
        if (!get_event_entity(*it)) {
            event_entities_[*it] = create_event_entity(*it, true);
        }
    }
    return false;
}

// IDA_EA: 0x810C5D2
// IDA_NAME: _ZN13EventClassify15CEventScriptMng18update_time_valuesEi
// DEMANGLED: EventClassify::CEventScriptMng::update_time_values(int)
CEventScriptMng* CEventScriptMng::update_time_values(int reset_hour) {
    time_t now = GlobalData::system_time()
        ? GlobalData::system_time()->getCurSec()
        : time(0);
    time_t yesterday = now - 86400;

    tm today_tm;
    tm yesterday_tm;
    localtime_r(&now, &today_tm);
    localtime_r(&yesterday, &yesterday_tm);

    tm* selected = &today_tm;
    if (today_tm.tm_hour >= 0 && today_tm.tm_hour < reset_hour) {
        selected = &yesterday_tm;
    }

    today_day_ = static_cast<unsigned char>(selected->tm_mday);
    today_time_ = static_cast<int>(today_day_) +
        10000 * selected->tm_year +
        100 * (selected->tm_mon + 1) +
        19000000;
    return this;
}

// IDA_EA: 0x810C6CA
// IDA_NAME: _ZN13EventClassify15CEventScriptMng18get_today_time_dayEv
// DEMANGLED: EventClassify::CEventScriptMng::get_today_time_day(void)
int CEventScriptMng::get_today_time_day() {
    return today_day_;
}

// IDA_EA: 0x810C6D6
// IDA_NAME: _ZN13EventClassify15CEventScriptMng14get_today_timeEv
// DEMANGLED: EventClassify::CEventScriptMng::get_today_time(void)
int CEventScriptMng::get_today_time() {
    return today_time_;
}

// IDA_EA: 0x810C6E2
// IDA_NAME: _ZN13EventClassify15CEventScriptMng33get_ingame_event_record_type_listERSt6vectorIiSaIiEE
// DEMANGLED: EventClassify::CEventScriptMng::get_ingame_event_record_type_list(std::vector<int,std::allocator<int> > &)
int CEventScriptMng::get_ingame_event_record_type_list(
    std::vector<int>& record_types) {
    record_types.clear();
    for (std::map<int, InGameEventInfo>::const_iterator it =
             info_list_.events.begin();
         it != info_list_.events.end();
         ++it) {
        record_types.push_back(it->first);
    }
    return 0;
}

// IDA_EA: 0x810C75C
// IDA_NAME: _ZN13EventClassify15CEventScriptMng35is_current_ingame_event_record_typeEi
// DEMANGLED: EventClassify::CEventScriptMng::is_current_ingame_event_record_type(int)
bool CEventScriptMng::is_current_ingame_event_record_type(int record_type) {
    // Rainbow Snake is a dedicated 0828 packet family rather than an
    // InGameEventInfo/NOTI375 script entity. Its account row still travels
    // through the shared login payload and must survive the live-event filter.
    return record_type == RainbowSnake::kInGameEventHistoryType ||
           record_type == KiriCargo::kInGameEventHistoryType ||
           info_list_.events.find(record_type) != info_list_.events.end();
}

// IDA_EA: 0x810C7B8
// IDA_NAME: _ZN13EventClassify15CEventScriptMng15get_record_typeEi
// DEMANGLED: EventClassify::CEventScriptMng::get_record_type(int)
int CEventScriptMng::get_record_type(int event_type) {
    CEventEntity* entity = get_event_entity(event_type);
    return entity ? entity->get_event_record_type() : 0;
}

// IDA_EA: 0x810C974
// IDA_NAME: _ZN13EventClassify15CEventScriptMng12get_pass_dayEii
// DEMANGLED: EventClassify::CEventScriptMng::get_pass_day(int,int)
int CEventScriptMng::get_pass_day(int start_day, int current_day) {
    if (start_day <= 0 && current_day <= 0) {
        return -1;
    }

    tm start_tm;
    tm current_tm;
    std::memset(&start_tm, 0, sizeof(start_tm));
    std::memset(&current_tm, 0, sizeof(current_tm));
    start_tm.tm_year = start_day / 10000 - 1900;
    start_tm.tm_mon = start_day % 10000 / 100 - 1;
    start_tm.tm_mday = start_day % 100;
    current_tm.tm_year = current_day / 10000 - 1900;
    current_tm.tm_mon = current_day % 10000 / 100 - 1;
    current_tm.tm_mday = current_day % 100;

    const time_t start_time = mktime(&start_tm);
    const time_t current_time = mktime(&current_tm);
    if (current_time < start_time) {
        return -1;
    }
    return static_cast<int>((current_time - start_time) / 86400);
}

namespace {

struct ResetEventDailyUserState {
    void operator()(CUser* user) const {
        if (user && user->get_state() > 2) {
            user->RecoverFatigue(0);
            user->SendFatigue();
            G_CDataManager()->get_event_script_mng()->send_event_init_data(
                user, true);
        }
    }
};

}  // namespace

// IDA_EA: 0x810CC70
// IDA_NAME: _ZN13EventClassify15CEventScriptMng23test_process_change_dayEi
// DEMANGLED: EventClassify::CEventScriptMng::test_process_change_day(int)
void CEventScriptMng::test_process_change_day(int today) {
    if (today > 20109999) {
        today_time_ = today;
        today_day_ = static_cast<unsigned char>(today % 100);
        G_GameWorld()->ProcessAllUsers(ResetEventDailyUserState());
    }
}

// IDA_EA: 0x810C270
// IDA_NAME: _ZN13EventClassify15CEventScriptMng33update_purchase_target_item_countEP5CUserjP22stCeraShopIPGRequest_t
// DEMANGLED: EventClassify::CEventScriptMng::update_purchase_target_item_count(CUser *,unsigned int,stCeraShopIPGRequest_t *)
int CEventScriptMng::update_purchase_target_item_count(
    CUser* user,
    unsigned int charac_no,
    stCeraShopIPGRequest_t* ipg_request) {
    int result = is_eventing(0x19a) ^ 1;
    if ((result & 0xff) == 0) {
        CPurchaseCashItemBonus* entity =
            static_cast<CPurchaseCashItemBonus*>(get_event_entity(0x19a));
        if (entity) {
            return entity->updateTargetGoodsCount(
                user, charac_no, ipg_request);
        }
    }
    return result;
}

// IDA_EA: 0x810C2D4
// IDA_NAME: _ZN13EventClassify15CEventScriptMng24send_purchase_bonus_mailEP5CUserj
// DEMANGLED: EventClassify::CEventScriptMng::send_purchase_bonus_mail(CUser *,unsigned int)
int CEventScriptMng::send_purchase_bonus_mail(CUser* user,
                                              unsigned int purchase_count) {
    if (is_eventing(0x19a) != 1) {
        return 0;
    }
    CPurchaseCashItemBonus* entity =
        static_cast<CPurchaseCashItemBonus*>(get_event_entity(0x19a));
    if (!entity) {
        return 0;
    }
    return entity->sendPurchaseBonusReward(
        user, static_cast<int>(purchase_count));
}

// IDA_EA: 0x810CDB0
// IDA_NAME: _ZN13EventClassify15CEventScriptMng15init_event_numsEv
// DEMANGLED: EventClassify::CEventScriptMng::init_event_nums(void)
int CEventScriptMng::init_event_nums() {
    event_nums_.clear();
    event_nums_.push_back(86);
    event_nums_.push_back(401);
    event_nums_.push_back(402);
    event_nums_.push_back(403);
    event_nums_.push_back(404);
    event_nums_.push_back(406);
    event_nums_.push_back(407);
    event_nums_.push_back(409);
    event_nums_.push_back(410);
    event_nums_.push_back(411);
    event_nums_.push_back(92);
    event_nums_.push_back(101);
    event_nums_.push_back(412);
    return 0;
}

}  // namespace EventClassify
