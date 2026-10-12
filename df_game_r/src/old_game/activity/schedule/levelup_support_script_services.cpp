#include "old_game/activity/schedule/levelup_support_script_services.h"

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/arad_print_error.h"
#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/db/event/db_event_account_character_level_up_once_gift.h"
#include "old_data/db/event/db_save_event_levelup_support.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/activity/schedule/new_account_level_up_event_to_job_abi.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/os_api.h"

#include <cstring>
#include <cstdio>
#include <utility>

int LevelupSupportSciprt::clearScript() {
    for (std::map<int, std::vector<ItemInfo>* >::iterator it =
             reward_items_by_level.begin();
         it != reward_items_by_level.end(); ++it) {
        delete it->second;
    }
    reward_items_by_level.clear();
    return static_cast<int>(reinterpret_cast<intptr_t>(&reward_items_by_level));
}

int LevelupSupportSciprt::loadScript() {
    if (!loadRDARScriptFile("Event/", "tw_LevelupSupport.evt")) {
        return -1;
    }

    clearScript();
    std::string token;
    bool is_type = false;
    bool success = false;
    int level = 0;
    std::vector<ItemInfo> items;
    int result = 0;

    while (ScanType(token, true) == 1) {
        if (token != "[step]") {
            continue;
        }
        if (ScanType(token, true) != 1) {
            break;
        }
        if (token == "[level]") {
            level = ScanInt(static_cast<bool*>(0));
        }
        if (ScanType(token, true) != 1) {
            break;
        }
        if (token == "[reward]") {
            while (1) {
                const int item_index =
                    ScanTypeOrInt(is_type, token, success, true);
                if (is_type) {
                    break;
                }
                items.push_back(ItemInfo(
                    item_index, ScanInt(static_cast<bool*>(0))));
            }
        }
        if (ScanType(token, true) != 1) {
            break;
        }
        if (token != "[/step]") {
            continue;
        }
        if (reward_items_by_level.find(level) != reward_items_by_level.end()) {
            std::printf("duplicate [level] %d", level);
            result = -1;
            break;
        }
        std::vector<ItemInfo>* stored = new std::vector<ItemInfo>();
        stored->swap(items);
        reward_items_by_level.insert(std::make_pair(level, stored));
    }

    unloadRDARScriptFile();
    return result;
}

namespace ARAD {
// IDA_EA: 0x8147E44
// IDA_NAME: _ZN4ARAD9SingletonI28LevelupSupport2ndEventMangerEC2Ev
// DEMANGLED: ARAD::Singleton<LevelupSupport2ndEventManger>::Singleton(void)
template <>
Singleton<LevelupSupport2ndEventManger>::Singleton() {}

// IDA_EA: 0x81A167C
// IDA_NAME: _ZN4ARAD9SingletonI28LevelupSupport2ndEventMangerE3GetEv
// DEMANGLED: ARAD::Singleton<LevelupSupport2ndEventManger>::Get(void)
template <>
LevelupSupport2ndEventManger*
Singleton<LevelupSupport2ndEventManger>::Get() {
    static LevelupSupport2ndEventManger object;
    return &object;
}
}  // namespace ARAD

LevelupSupport2ndEventManger::LevelupSupport2ndEventManger() {}
LevelupSupport2ndEventManger::~LevelupSupport2ndEventManger() {}

// IDA_EA: 0x81477E4
// IDA_NAME: _ZN28LevelupSupport2ndEventManger4LoadEv
// DEMANGLED: LevelupSupport2ndEventManger::Load(void)
bool LevelupSupport2ndEventManger::Load() {
    cMyTrace trace("virtual bool LevelupSupport2ndEventManger::Load()", 183,
                   9, true, false);
    trace("\t- Loading Level Up Support 2nd Event script - ");
    // IDA performs the container clear before the parser sees the source.
    // This is deliberately not clearScript(): the original operation only
    // invokes std::map::clear at this point.
    script_.reward_items_by_level.clear();
    return script_.loadScript() == 0;
}

// IDA_EA: 0x8147864
// IDA_NAME: _ZN28LevelupSupport2ndEventManger7DestroyEv
// DEMANGLED: LevelupSupport2ndEventManger::Destroy(void)
int LevelupSupport2ndEventManger::Destroy() {
    return script_.clearScript(), 1;
}

// IDA_EA: 0x8147880
// IDA_NAME: _ZN28LevelupSupport2ndEventManger7ProcessEP5CUser
// DEMANGLED: LevelupSupport2ndEventManger::Process(CUser*)
void LevelupSupport2ndEventManger::Process(CUser* user) {
    if (!user || script_.job_type != user->get_charac_job()) {
        return;
    }

    const std::map<int, std::vector<LevelupSupport2ndScript::ItemInfo>* >::
        const_iterator found =
            script_.reward_items_by_level.find(user->get_charac_level());
    if (found == script_.reward_items_by_level.end() || !found->second) {
        return;
    }

    const char* const title = g_scriptStringManager_.findString(
        4, script_.mail_title.c_str(), 0);
    const char* const contents = g_scriptStringManager_.findString(
        4, script_.mail_content.c_str(), 0);
    const std::vector<LevelupSupport2ndScript::ItemInfo>& rewards =
        *found->second;
    for (std::vector<LevelupSupport2ndScript::ItemInfo>::const_iterator
             reward = rewards.begin();
         reward != rewards.end();
         ++reward) {
        CItem* const prototype =
            G_CDataManager()->find_item(reward->index);
        if (!prototype) {
            continue;
        }

        Inven_Item item;
        item.item_id_ = prototype->get_index();
        prototype->vtable_->make_item(prototype, item);
        item.set_add_info(reward->count);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            contents,
            static_cast<int>(std::strlen(contents)),
            15,
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
            false,
            false);
    }
}

LevelupSupportEventManger::LevelupSupportEventManger()
    : script_(), step_indices_(), default_event_info_() {}

LevelupSupportEventManger::~LevelupSupportEventManger() {
    Destroy();
}

// IDA_EA: 0x8146E3C
// IDA_NAME: _ZN25LevelupSupportEventManger4LoadEv
// DEMANGLED: LevelupSupportEventManger::Load(void)
bool LevelupSupportEventManger::Load() {
    cMyTrace trace("virtual bool LevelupSupportEventManger::Load()", 24, 9,
                   true, false);
    trace("\t- Loading Levelup Support Event script - ");
    // IDA clears the map before attempting to open the script, including the
    // failed-open path.  Do not defer this to loadScript().
    script_.reward_items_by_level.clear();
    if (script_.loadScript() != 0) {
        return false;
    }
    step_indices_.clear();
    default_event_info_.clear();
    int state_index = 0;
    for (std::map<int, std::vector<LevelupSupportSciprt::ItemInfo>* >::const_iterator
             it = script_.reward_items_by_level.begin();
         it != script_.reward_items_by_level.end(); ++it) {
        step_indices_.insert(std::make_pair(it->first, state_index++));
        default_event_info_.push_back(EventInfo(
            static_cast<uint16_t>(it->first), static_cast<uint16_t>(0)));
    }
    return true;
}

// IDA_EA: 0x8146FDA
// IDA_NAME: _ZN25LevelupSupportEventManger7DestroyEv
// DEMANGLED: LevelupSupportEventManger::Destroy(void)
int LevelupSupportEventManger::Destroy() {
    script_.clearScript();
    step_indices_.clear();
    default_event_info_.clear();
    return 1;
}

// IDA_EA: 0x814701E
// IDA_NAME:
// _ZN25LevelupSupportEventManger15SendCharacStateEP5CUseri
// DEMANGLED: LevelupSupportEventManger::SendCharacState(CUser*,int)
void LevelupSupportEventManger::SendCharacState(CUser* user,
                                                int charac_level) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_LEVELUP_SUPPORT_INFO);
    packet.put_byte(
        static_cast<int>(user->levelup_support_events_.size()));
    for (std::vector<EventInfo>::const_iterator event =
             user->levelup_support_events_.begin();
         event != user->levelup_support_events_.end();
         ++event) {
        packet.put_int(event->level);
        packet.put_byte(event->state);
    }
    packet.put_byte(charac_level);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8147164
// IDA_NAME: _ZN25LevelupSupportEventManger12CheckNewStepEi
// DEMANGLED: LevelupSupportEventManger::CheckNewStep(int)
bool LevelupSupportEventManger::CheckNewStep(int charac_level) {
    return script_.reward_items_by_level.find(charac_level) !=
           script_.reward_items_by_level.end();
}

// IDA_EA: 0x81471C6
// IDA_NAME:
// _ZN25LevelupSupportEventManger15UpdateEventInfoEP5CUser
// DEMANGLED: LevelupSupportEventManger::UpdateEventInfo(CUser*)
void LevelupSupportEventManger::UpdateEventInfo(CUser* user) {
    const int charac_level = user->get_charac_level();
    if (!CheckNewStep(charac_level) &&
        !GlobalData::s_GM_Manager->GetCurrentGmMode(
            static_cast<unsigned int>(user->get_acc_id()))) {
        return;
    }

    const int user_index = user->GetUID();
    const int charac_no = user->getCurCharacNo();
    const unsigned int event_count =
        static_cast<unsigned int>(default_event_info_.size());
    for (unsigned int index = 0; index < event_count; ++index) {
        EventInfo& event = user->levelup_support_events_[index];
        if (event.level <= charac_level && event.state != 2) {
            event.state = 1;
            DB_SaveEventLevelupSupport::makeRequest(
                user_index, charac_no, event.level, 1);
        }
    }
    SendCharacState(user, CheckNewStep(charac_level) ? charac_level : 0);
}

// IDA_EA: 0x8147012
// IDA_NAME: _ZNK25LevelupSupportEventManger19GetDefaultEventInfoEv
// DEMANGLED: LevelupSupportEventManger::GetDefaultEventInfo(void)const
const std::vector<LevelupSupportEventManger::EventInfo>&
LevelupSupportEventManger::GetDefaultEventInfo() const {
    return default_event_info_;
}

// IDA_EA: 0x8147362
// IDA_NAME: _ZN25LevelupSupportEventManger12GetStepIndexEi
// DEMANGLED: LevelupSupportEventManger::GetStepIndex(int)
int LevelupSupportEventManger::GetStepIndex(int requested_level) {
    const std::map<int, int>::const_iterator found =
        step_indices_.find(requested_level);
    return found != step_indices_.end() ? found->second : -1;
}

// IDA_EA: 0x81473CC
// IDA_NAME: _ZN25LevelupSupportEventManger10InsertItemEP5CUseriR10ENUM_ERROR
// DEMANGLED: LevelupSupportEventManger::InsertItem(CUser *,int,ENUM_ERROR &)
void LevelupSupportEventManger::InsertItem(CUser* user,
                                           int step_index,
                                           ENUM_ERROR& error) {
    user->levelup_support_events_[step_index].state = 2;
    const int requested_level =
        user->levelup_support_events_[step_index].level;
    std::vector<LevelupSupportSciprt::ItemInfo>* const rewards =
        GetItemInfoVector(requested_level);
    if (!rewards) {
        error = static_cast<ENUM_ERROR>(21);
        return;
    }

    for (std::vector<LevelupSupportSciprt::ItemInfo>::const_iterator reward =
             rewards->begin();
         reward != rewards->end();
         ++reward) {
        CItem* const prototype = G_CDataManager()->find_item(reward->item_index);
        if (!prototype) {
            error = static_cast<ENUM_ERROR>(21);
            ARAD::PRINT_ERROR::find_item(
                const_cast<char*>(" LevelupSupportEventManger::InsertItem"),
                user,
                reward->item_index);
            return;
        }

        Inven_Item item;
        item.item_id_ = prototype->get_index();
        prototype->vtable_->make_item(prototype, item);
        item.set_add_info(reward->item_count);
        const int inserted_slot = user->getCurCharacInvenW()->
            insertItemIntoInventory(item, static_cast<eItemAddReason>(13),
                                    true, true);
        if (inserted_slot < 0) {
            const char* const title = g_scriptStringManager_.findString(
                4, "game_server_msg_241", 0);
            const char* const contents = g_scriptStringManager_.findString(
                4, "game_server_msg_242", 0);
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                item,
                0,
                static_cast<unsigned int>(user->getCurCharacNo()),
                contents,
                static_cast<int>(std::strlen(contents)),
                15,
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
                false,
                false);
        } else {
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 inserted_slot);
        }
    }

    DB_SaveEventLevelupSupport::makeRequest(
        user->GetUID(),
        user->getCurCharacNo(),
        user->levelup_support_events_[step_index].level,
        2);
    SendCharacState(user, 0);
}

// IDA_EA: 0x8147724
// IDA_NAME: _ZN25LevelupSupportEventManger17GetItemInfoVectorEi
// DEMANGLED: LevelupSupportEventManger::GetItemInfoVector(int)
std::vector<LevelupSupportSciprt::ItemInfo>*
LevelupSupportEventManger::GetItemInfoVector(int requested_level) {
    const std::map<int, std::vector<LevelupSupportSciprt::ItemInfo>* >::
        const_iterator found = script_.reward_items_by_level.find(requested_level);
    return found != script_.reward_items_by_level.end() ? found->second : 0;
}

namespace ARAD {
// IDA_EA: 0x8147B56
// IDA_NAME: _ZN4ARAD9SingletonI25LevelupSupportEventMangerEC2Ev
// DEMANGLED: ARAD::Singleton<LevelupSupportEventManger>::Singleton(void)
template <>
Singleton<LevelupSupportEventManger>::Singleton() {}

// IDA_EA: 0x8147D68
// IDA_NAME: _ZN4ARAD9SingletonI25LevelupSupportEventMangerE3GetEv
// DEMANGLED: ARAD::Singleton<LevelupSupportEventManger>::Get(void)
template <>
LevelupSupportEventManger* Singleton<LevelupSupportEventManger>::Get() {
    static LevelupSupportEventManger object;
    return &object;
}
}  // namespace ARAD

// IDA_EA: 0x81B82A4
// IDA_NAME: _ZN27NewAccountLevelUpEventToJob19processLevelUpToJobER5CUserb
// DEMANGLED: NewAccountLevelUpEventToJob::processLevelUpToJob(CUser &,bool)
bool NewAccountLevelUpEventToJob::processLevelUpToJob(
    CUser& user,
    bool growup) {
    const LevelUpRewardItemTableKey key(
        user.get_charac_level(),
        user.get_charac_job(),
        user.getCurCharFirstGrowType());
    std::vector<LevelUpRewardItemData> rewards;
    stLevelUpRewardItemScript* const script =
        G_CDataManager()->
            get_new_account_level_up_equip_reward_script();
    if (growup) {
        if (script->get_data_when_growup(key, rewards) != 1) {
            return false;
        }
    } else {
        LevelUpRewardItemData reward;
        if (script->get_data(key, reward) != 1) {
            return false;
        }
        rewards.push_back(reward);
    }

    CharacLevelUpGiftAccountOnce* const gift_state =
        user.getCharacLevelUpGift_AccountOnce();
    const int charac_job = user.get_charac_job();
    const int charac_level = user.get_charac_level();
    const std::pair<std::multimap<int, int>::const_iterator,
                    std::multimap<int, int>::const_iterator> range =
        gift_state->gifts_.equal_range(charac_job);
    for (std::multimap<int, int>::const_iterator gift = range.first;
         gift != range.second;
         ++gift) {
        if (gift->second == charac_level) {
            return false;
        }
    }

    gift_state->gifts_.insert(std::make_pair(charac_job, charac_level));
    DB_EventAccountCharacterLevelUpOnceGift::makeRequest(
        static_cast<unsigned int>(user.get_acc_id()),
        static_cast<short>(user.GetServerGroupToPvP()),
        user.getCurCharacNo(),
        static_cast<short>(charac_job),
        static_cast<short>(charac_level));

    char contents[256] = {};
    OS_API::snprintf(
        contents,
        0xffu,
        g_scriptStringManager_.findString(
            4, "game_server_msg_286", 0),
        charac_level);
    for (std::vector<LevelUpRewardItemData>::const_iterator reward =
             rewards.begin();
         reward != rewards.end();
         ++reward) {
        Inven_Item item;
        item.item_id_ = reward->item_id;
        G_CDataManager()->get_item_list()->create_item(
            ENUM_ITEM_CREATE_TYPE_DEFAULT, item, reward->count);
        if (WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                g_scriptStringManager_.findString(
                    4, "game_server_msg_285", 0),
                item,
                0,
                static_cast<unsigned int>(user.getCurCharacNo()),
                contents,
                static_cast<int>(std::strlen(contents)),
                0,
                static_cast<ENUM_SERVER_GROUP>(
                    user.GetServerGroupToPvP()),
                false,
                false)) {
            LogManager::logFormat(
                1,
                "NewAccountLevelUpEventToJob.cpp",
                "bool NewAccountLevelUpEventToJob::"
                "processLevelUpToJob(CUser&, bool)",
                127,
                "NewAccountLevelUpEventToJob Event Start!");
        }
    }
    return true;
}

// IDA_EA: 0x81672BE
// IDA_NAME:
// _ZN16HeroMissionEvent14processMissionEP5CUserN20HeroMissionCondition9MissionNo1TEj
// DEMANGLED:
// HeroMissionEvent::processMission(
//     CUser*,HeroMissionCondition::MissionNo::T,unsigned int)
void HeroMissionEvent::processMission(
    CUser* user,
    HeroMissionCondition::MissionNo::T mission_no,
    unsigned int param) {
    if (!IsEventing(0)) {
        return;
    }

    if (!user) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::processMission(CUser*, "
            "HeroMissionCondition::MissionNo::T, unsigned int)",
            696,
            "[HeroMission] User is null.");
        return;
    }

    HeroMissionValue* value = 0;
    if (!findMissionValue(user, mission_no, value)) {
        return;
    }

    std::map<HeroMissionCondition::MissionNo::T,
             BaseHeroMissionCondition*>::iterator condition =
        conditions_.find(mission_no);
    if (condition == conditions_.end()) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::processMission(CUser*, "
            "HeroMissionCondition::MissionNo::T, unsigned int)",
            706,
            "[HeroMission] Invalid mission. (charac:%u, mission:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(mission_no));
        user->DisConnSig(static_cast<DISCONN_SIG>(62), true, 707);
        return;
    }

    const uint16_t reward_count = value->reward_count;
    condition->second->processMission(user, *value, param);
    if (mission_no == HeroMissionCondition::MissionNo::LEVEL_UP &&
        reward_count == 0 && value->reward_count != 0) {
        processMission(user, HeroMissionCondition::MissionNo::TOTAL, 1);
    }
}

// IDA_EA: 0x8168010
// IDA_NAME:
// _ZN16HeroMissionEvent16findMissionValueEP5CUserN20HeroMissionCondition9MissionNo1TERP16HeroMissionValue
// DEMANGLED:
// HeroMissionEvent::findMissionValue(
//     CUser*,HeroMissionCondition::MissionNo::T,HeroMissionValue*&)
bool HeroMissionEvent::findMissionValue(
    CUser* user,
    HeroMissionCondition::MissionNo::T mission_no,
    HeroMissionValue*& value) {
    if (IsEventing(0) != 1 ||
        mission_no <= 0 ||
        mission_no > HeroMissionCondition::MissionNo::TOTAL) {
        return false;
    }

    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no(-1));
    std::map<unsigned int, std::vector<HeroMissionValue> >::iterator found =
        values_by_charac_.find(charac_no);
    if (found == values_by_charac_.end()) {
        return false;
    }

    for (std::vector<HeroMissionValue>::iterator current =
             found->second.begin();
         current != found->second.end();
         ++current) {
        if (current->mission_no == mission_no) {
            value = &*current;
            return true;
        }
    }
    return false;
}
