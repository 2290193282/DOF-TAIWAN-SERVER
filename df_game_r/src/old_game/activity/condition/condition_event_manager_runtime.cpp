#include "old_game/activity/condition/condition_event_manager_runtime.h"

#include <cstring>

#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

namespace {

const int kConditionEventRepeatId = 113;

bool IsConditionEventRunning() {
    return GlobalData::s_event_manager
               ->GetRepeatEvent(kConditionEventRepeatId)
               ->IsEventing(0) == 1;
}

void SendConditionRewardByMail(CUser* user, Inven_Item& item) {
    char title[30] = {};
    char text[256] = {};
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_80", 0),
                 0x1du);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_81", 0),
                 0xffu);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
    item.reset();
}

void InsertNormalConditionReward(CUser* user,
                                 Inven_Item& item,
                                 bool creature_item) {
    const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(37), true, true);
    if (slot >= 0) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(
                                 creature_item ? 7 : 0),
                             slot);
        return;
    }
    SendConditionRewardByMail(user, item);
}

}  // namespace

// IDA_EA: 0x8334844
// IDA_NAME: _ZN22CConditionEventManagerC2Ev
// DEMANGLED: CConditionEventManager::CConditionEventManager(void)
CConditionEventManager::CConditionEventManager() {}

// IDA_EA: 0x833484A
// IDA_NAME: _ZN22CConditionEventManagerD2Ev
// DEMANGLED: CConditionEventManager::~CConditionEventManager()
CConditionEventManager::~CConditionEventManager() {}

// IDA_EA: 0x8334850
// IDA_NAME: _ZN22CConditionEventManager27RequestConditionEventRewardEP5CUserscRSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
// DEMANGLED: CConditionEventManager::RequestConditionEventReward(CUser *,short,char,std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> &)
int CConditionEventManager::RequestConditionEventReward(
    CUser* user,
    short event_index,
    char reward_step,
    std::map<int, int>& rewards) {
    if (!IsConditionEventRunning()) {
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    stConditionEventInfo* const event_info =
        data_manager->get_condition_event_info();
    if (event_index != event_info->first_dword) {
        return 1;
    }
    if (reward_step <= 0 || reward_step > user->GetCurConditionEventStep() ||
        user->GetCurConditionEventRewardStep() != reward_step - 1) {
        return 19;
    }

    std::vector<stConditionEventRewardInfo> reward_info;
    GetConditionRewardInfo(0, reward_info, event_index, reward_step);
    ConditionRewardSameItemCombine(reward_info, rewards);

    for (std::map<int, int>::const_iterator it = rewards.begin();
         it != rewards.end();
         ++it) {
        const int item_id = it->first;
        const int item_count = it->second;
        if (item_id == 1 && item_count >= 0) {
            const int current_coin = user->getCurCharacInvenR()->GetEventCoin();
            user->getCurCharacInvenW()->SetEventCoin(current_coin + item_count);
            user->history_log().EventCoinAdd(
                user->getCurCharacInvenR()->GetEventCoin(),
                item_count,
                static_cast<eCoinAddReason>(2));
            user->SaveMoneyCoin();
            user->SendCoin();
            continue;
        }

        CItem* const item_data = data_manager->find_item(item_id);
        if (!item_data) {
            return 17;
        }

        Inven_Item item;
        item.item_id_ = item_id;
        data_manager->get_item_list()->create_item(
            ENUM_ITEM_CREATE_TYPE_DEFAULT, item, item_count);
        if (!item.item_id_ || !item.get_add_info()) {
            continue;
        }

        int special_type = 0;
        int special_value = 0;
        bool unique = false;
        if (!GlobalData::s_pCeraShop->IsSpecialItem(
                item.item_id_, special_type, special_value, unique)) {
            InsertNormalConditionReward(user, item, item.IsCreatureItemType());
            continue;
        }

        const int item_type = item_data->vtable_->get_item_type(item_data);
        if (item_type == 16 || item.item_id_ == 1) {
            if (item_type == 16) {
                InsertNormalConditionReward(user, item, false);
            }
            continue;
        }

        int processing_result = 0;
        const int restrict_code =
            data_manager->get_item_list()->GetRestrictCode(item.item_id_);
        const int current_seconds = GlobalData::s_systemTime_.getCurSec();
        std::vector<Inven_Item> output_items;
        WongWork::CCeraShop::stCeraShopItemParam_t item_param;
        if (GlobalData::s_pCeraShop->ProcessSpecialItem(
                user,
                item.item_id_,
                restrict_code,
                processing_result,
                current_seconds,
                true,
                output_items,
                item_param,
                0,
                0,
                false,
                1)) {
            if (processing_result) {
                return 17;
            }
            if (restrict_code && !user->IsRestrictedGoods(restrict_code)) {
                user->SetRestrictedGoods(restrict_code);
                GlobalData::s_pCeraShop->SaveFeaturedIdx(
                    user, restrict_code, current_seconds, 0);
                user->SendRestrictedGoods(true, restrict_code);
            }
        }
    }

    user->IncCurConditionEventRewardStep();
    return 0;
}

// IDA_EA: 0x8335264
// IDA_NAME: _ZN22CConditionEventManager30ConditionRewardSameItemCombineERSt6vectorI26stConditionEventRewardInfoSaIS1_EERSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
// DEMANGLED: CConditionEventManager::ConditionRewardSameItemCombine(std::vector<stConditionEventRewardInfo,std::allocator<stConditionEventRewardInfo>> &,std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> &)
bool CConditionEventManager::ConditionRewardSameItemCombine(
    std::vector<stConditionEventRewardInfo>& source,
    std::map<int, int>& rewards) {
    for (std::vector<stConditionEventRewardInfo>::const_iterator it =
             source.begin();
         it != source.end();
         ++it) {
        std::map<int, int>::iterator existing = rewards.find(it->first_dword);
        if (existing == rewards.end()) {
            rewards.insert(std::make_pair(it->first_dword, it->second_dword));
        } else {
            existing->second += it->second_dword;
        }
    }
    // The generated IDA body returns the final iterator comparison; it is
    // false after reaching end, including for an initially empty source.
    return false;
}

// IDA_EA: 0x83353A0
// IDA_NAME: _ZN22CConditionEventManager22GetConditionRewardInfoEcRSt6vectorI26stConditionEventRewardInfoSaIS1_EEsc
// DEMANGLED: CConditionEventManager::GetConditionRewardInfo(char,std::vector<stConditionEventRewardInfo,std::allocator<stConditionEventRewardInfo>> &,short,char)
char CConditionEventManager::GetConditionRewardInfo(
    char quick_party,
    std::vector<stConditionEventRewardInfo>& rewards,
    short event_index,
    char reward_step) {
    char result = static_cast<char>(!IsConditionEventRunning());
    if (result) {
        return result;
    }

    stConditionEventInfo* const event_info =
        G_CDataManager()->get_condition_event_info();
    result = static_cast<char>(event_index);
    if (event_index != event_info->first_dword) {
        return result;
    }

    rewards.clear();
    const std::multimap<int, stConditionEventRewardInfo>& table =
        quick_party ? event_info->quick_party_rewards
                    : event_info->condition_rewards;
    const std::pair<std::multimap<int, stConditionEventRewardInfo>::const_iterator,
                    std::multimap<int, stConditionEventRewardInfo>::const_iterator>
        range = table.equal_range(reward_step);
    for (std::multimap<int, stConditionEventRewardInfo>::const_iterator it =
             range.first;
         it != range.second;
         ++it) {
        rewards.push_back(it->second);
    }
    // The original loop's terminal iterator comparison is the return value.
    return 0;
}

// IDA_EA: 0x83354F0
// IDA_NAME: _ZN22CConditionEventManager16GetMaxRewardStepEs
// DEMANGLED: CConditionEventManager::GetMaxRewardStep(short)
int CConditionEventManager::GetMaxRewardStep(short event_index) {
    if (!IsConditionEventRunning()) {
        return 0;
    }
    const stConditionEventInfo* const event_info =
        G_CDataManager()->get_condition_event_info();
    return event_index == event_info->first_dword
               ? static_cast<int>(event_info->condition_matches.size())
               : 0;
}

// IDA_EA: 0x8335566
// IDA_NAME: _ZN22CConditionEventManager18ProcessCheckStepUpEP5CUsers
// DEMANGLED: CConditionEventManager::ProcessCheckStepUp(CUser *,short)
int CConditionEventManager::ProcessCheckStepUp(CUser* user, short event_index) {
    if (!IsConditionEventRunning()) {
        return 1;
    }
    if (event_index == G_CDataManager()->get_condition_event_info()->first_dword &&
        event_index == 2) {
        _processProperDungeonClearEvent(user, event_index);
    }
    return event_index;
}

// IDA_EA: 0x83355E6
// IDA_NAME: _ZN22CConditionEventManager31_processProperDungeonClearEventEP5CUsers
// DEMANGLED: CConditionEventManager::_processProperDungeonClearEvent(CUser *,short)
void CConditionEventManager::_processProperDungeonClearEvent(
    CUser* user,
    short event_index) {
    (void)event_index;
    const std::pair<int, int> required =
        _getRequiredParameter(
            static_cast<short>(user->GetCurConditionEventStep()));
    if (required.first != -1 &&
        required.first <= user->GetProperDungeonClearCount()) {
        user->IncCurConditionEventStep();
        user->SendConditionEventInfo();
    }
    user->SendProperDungeonClearCount();
}

// IDA_EA: 0x83356A4
// IDA_NAME: _ZN22CConditionEventManager21_getRequiredParameterEs
// DEMANGLED: CConditionEventManager::_getRequiredParameter(short)
std::pair<int, int> CConditionEventManager::_getRequiredParameter(
    short reward_step) {
    const stConditionEventInfo* const event_info =
        G_CDataManager()->get_condition_event_info();
    const std::map<int, std::pair<int, int> >::const_iterator found =
        event_info->condition_matches.find(reward_step);
    return found == event_info->condition_matches.end()
               ? std::make_pair(-1, -1)
               : found->second;
}

// IDA_EA: 0x8335754
// IDA_NAME: _ZN22CConditionEventManager16GetCurEventIndexEv
// DEMANGLED: CConditionEventManager::GetCurEventIndex(void)
int CConditionEventManager::GetCurEventIndex() {
    return IsConditionEventRunning()
               ? G_CDataManager()->get_condition_event_info()->first_dword
               : 0;
}

// IDA_EA: 0x833579E
// IDA_NAME: _ZN22CConditionEventManager14IsFreeGoldCardEP5CUserc
// DEMANGLED: CConditionEventManager::IsFreeGoldCard(CUser *,char)
int CConditionEventManager::IsFreeGoldCard(CUser* user, char reward_step) {
    if (CheckErrorCardForStep(user, reward_step) != 1) {
        return 0;
    }
    std::vector<stConditionEventRewardInfo> rewards;
    GetConditionRewardInfo(0,
                           rewards,
                           static_cast<short>(GetCurEventIndex()),
                           reward_step);
    return IsFreeGoldCardProcess(rewards);
}

// IDA_EA: 0x833586C
// IDA_NAME: _ZN22CConditionEventManager21IsFreeGoldCardProcessERSt6vectorI26stConditionEventRewardInfoSaIS1_EE
// DEMANGLED: CConditionEventManager::IsFreeGoldCardProcess(std::vector<stConditionEventRewardInfo,std::allocator<stConditionEventRewardInfo>> &)
int CConditionEventManager::IsFreeGoldCardProcess(
    std::vector<stConditionEventRewardInfo>& rewards) {
    for (std::vector<stConditionEventRewardInfo>::const_iterator it =
             rewards.begin();
         it != rewards.end();
         ++it) {
        if (it->first_flag) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x83358BC
// IDA_NAME: _ZN22CConditionEventManager16IsDoubleGoldCardEP5CUserc
// DEMANGLED: CConditionEventManager::IsDoubleGoldCard(CUser *,char)
int CConditionEventManager::IsDoubleGoldCard(CUser* user, char reward_step) {
    if (CheckErrorCardForStep(user, reward_step) != 1) {
        return 0;
    }

    const short event_index = static_cast<short>(GetCurEventIndex());
    std::vector<stConditionEventRewardInfo> rewards;
    GetConditionRewardInfo(0, rewards, event_index, reward_step);
    int result = IsDoubleGoldCardProcess(rewards);
    if (result != 1 && user->GetParty() &&
        user->GetParty()->is_quick_party()) {
        std::vector<stConditionEventRewardInfo> quick_party_rewards;
        GetConditionRewardInfo(
            1, quick_party_rewards, event_index, reward_step);
        result = IsDoubleGoldCardProcess(quick_party_rewards);
    }
    return result;
}

// IDA_EA: 0x8335A3A
// IDA_NAME: _ZN22CConditionEventManager23IsDoubleGoldCardProcessERSt6vectorI26stConditionEventRewardInfoSaIS1_EE
// DEMANGLED: CConditionEventManager::IsDoubleGoldCardProcess(std::vector<stConditionEventRewardInfo,std::allocator<stConditionEventRewardInfo>> &)
int CConditionEventManager::IsDoubleGoldCardProcess(
    std::vector<stConditionEventRewardInfo>& rewards) {
    for (std::vector<stConditionEventRewardInfo>::const_iterator it =
             rewards.begin();
         it != rewards.end();
         ++it) {
        if (it->second_flag) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8335A8A
// IDA_NAME: _ZN22CConditionEventManager21CheckErrorCardForStepEP5CUserc
// DEMANGLED: CConditionEventManager::CheckErrorCardForStep(CUser *,char)
int CConditionEventManager::CheckErrorCardForStep(CUser* user,
                                                   char reward_step) {
    if (!user || user->IsProperDungeonUser() != 1 || reward_step < 0) {
        return 0;
    }
    const std::pair<int, int> required = _getRequiredParameter(reward_step);
    return required.first != -1 &&
           required.first >= user->GetProperDungeonClearCount();
}
