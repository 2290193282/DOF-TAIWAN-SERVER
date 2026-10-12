#include "old_game/player/core/cbooster_gage_runtime.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_xml_runtime.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x80DC6D6
// IDA_NAME: _ZN12CBoosterGage11make_postarEP5CUserRK10Inven_ItemPKci
// DEMANGLED: CBoosterGage::make_postar(CUser *,Inven_Item const&,char const*,int)
void CBoosterGage::make_postar(CUser* user,
                               const Inven_Item& item,
                               const char* agency_no,
                               int duration_days) {
    char text[256];
    char title[21];
    std::memset(text, 0, sizeof(text));

    const std::string title_text =
        g_ServerString_.GetServerString(0x436, 0);
    std::strncpy(title, title_text.c_str(), 0x14u);
    const std::string body_text =
        g_ServerString_.GetServerString(0x437, 0);
    std::strncpy(text, body_text.c_str(), 0xFFu);

    CItem* item_script = G_CDataManager()->find_item(item.getKey());
    if (!item_script) {
        return;
    }

    const CItemVTable* const category =
        *reinterpret_cast<const CItemVTable* const*>(item_script);
    if (category->is_avatar_item(item_script)) {
        int expire_time = 0;
        if (duration_days && duration_days != 365) {
            expire_time = duration_days * 86400 +
                          GlobalData::s_systemTime_.getCurSec();
        }
        WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(
            user,
            user->getCurCharacNo(),
            item.getKey(),
            expire_time,
            0,
            static_cast<char>(-1),
            agency_no,
            text,
            static_cast<int>(std::strlen(text)));
    } else if (category->is_creature_item(item_script)) {
        WongWork::CMailBoxHelper::ReqDBSendNewCreatureMail(
            user,
            user->getCurCharacNo(),
            item,
            text,
            static_cast<int>(std::strlen(text)));
    } else {
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
    }
}

// IDA_EA: 0x80DC9CA
// IDA_NAME: _ZN12CBoosterGage29log_booster_gage_bonus_rewardEP5CUserRKSt6vectorISt4pairI10Inven_ItembESaIS5_EE
// DEMANGLED: CBoosterGage::log_booster_gage_bonus_reward(CUser *,std::vector<std::pair<Inven_Item,bool>,std::allocator<std::pair<Inven_Item,bool>>> const&)
void CBoosterGage::log_booster_gage_bonus_reward(
    CUser* user,
    const BoosterRewardList& rewards) {
    std::string item_list;
    item_list.clear();
    for (BoosterRewardList::const_iterator it = rewards.begin();
         it != rewards.end(); ++it) {
        char item_text[255];
        std::memset(item_text, 0, sizeof(item_text));
        std::sprintf(item_text, "%d ", it->first.getKey());
        item_list += item_text;
    }

    cMyTrace trace(
        "void CBoosterGage::log_booster_gage_bonus_reward(CUser*, const MULTIBOX_REWARD_LIST&)",
        124,
        0);
    trace("BOOSTER_GAGE : REWARD ITEM (%s) x 2 : CHARAC_NO (%u)",
          item_list.c_str(),
          static_cast<unsigned int>(user->getCurCharacNo()));
}

// IDA_EA: 0x80DCBD6
// IDA_NAME: _ZN12CBoosterGage13AddAvatarItemERSt6vectorISt4pairIiiESaIS2_EEbP5CUserRK10Inven_ItemibccPKc20eAvatarItemAddReasonRi
// DEMANGLED: CBoosterGage::AddAvatarItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,CUser *,Inven_Item const&,int,bool,char,char,char const*,eAvatarItemAddReason,int &)
int CBoosterGage::AddAvatarItem(BoosterInsertedSlotList& inserted_slots,
                                bool booster_bonus,
                                CUser* user,
                                const Inven_Item& item,
                                int usable_period,
                                bool period_is_seconds,
                                char first_flag,
                                char avatar_select,
                                const char* agency_no,
                                eAvatarItemAddReason reason,
                                int& add_info) {
    int inserted_slot = 0;
    int repeat_count = 1;
    if (booster_bonus) {
        repeat_count = G_CDataManager()->GetCeraShopEventMultiplier();
        reason = static_cast<eAvatarItemAddReason>(22);
    }

    for (int index = 0; index < repeat_count; ++index) {
        CInventory* inventory = user->getCurCharacInvenW();
        inserted_slot = inventory->AddAvatarItem(
            item.getKey(),
            usable_period,
            period_is_seconds,
            0,
            static_cast<char>(-1),
            agency_no,
            reason,
            false,
            0);
        if (inserted_slot == -1) {
            make_postar(user, item, agency_no, usable_period);
            continue;
        }

        const int current_time = static_cast<int>(std::time(0));
        const Inven_Item stored_item =
            user->getCurCharacInvenR()->GetInvenSlot(2, inserted_slot);
        const WongWork::CAvatarItemMgr* const avatar_manager =
            user->getCurCharacInvenR()->GetAvatarItemMgrR();
        add_info = avatar_manager->GetRemainDate(
            stored_item.get_add_info(), current_time);
        inserted_slots.push_back(
            std::make_pair(inserted_slot, item.GetItemSpace()));
    }
    return inserted_slot;
}

// IDA_EA: 0x80DCD8C
// IDA_NAME: _ZN12CBoosterGage18InsertCreatureItemERSt6vectorISt4pairIiiESaIS2_EEbP5CUserRK10Inven_Item14eItemAddReasonbbiiPKc
// DEMANGLED: CBoosterGage::InsertCreatureItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,CUser *,Inven_Item const&,eItemAddReason,bool,bool,int,int,char const*)
int CBoosterGage::InsertCreatureItem(
    BoosterInsertedSlotList& inserted_slots,
    bool booster_bonus,
    CUser* user,
    const Inven_Item& item,
    eItemAddReason reason,
    bool log_history,
    bool creature_flag,
    int usable_period,
    int expiration_date,
    const char* agency_no) {
    int inserted_slot = 0;
    int repeat_count = 1;
    if (booster_bonus) {
        repeat_count = G_CDataManager()->GetCeraShopEventMultiplier();
        reason = static_cast<eItemAddReason>(58);
    }

    for (int index = 0; index < repeat_count; ++index) {
        CInventory* inventory = user->getCurCharacInvenW();
        inserted_slot = inventory->insertItemIntoInventory(
            item, reason, log_history, creature_flag);
        if (inserted_slot >= 0) {
            inventory->GetCreatureMgrW()->InsertCreatureItem(
                const_cast<Inven_Item*>(&item),
                inserted_slot,
                static_cast<int>(reason),
                usable_period,
                expiration_date);
            inserted_slots.push_back(
                std::make_pair(inserted_slot, item.GetItemSpace()));
        } else {
            inserted_slot = -1;
            make_postar(user, item, agency_no, 0);
        }
    }
    return inserted_slot;
}

// IDA_EA: 0x80DCF56
// IDA_NAME: _ZN12CBoosterGage23insertItemIntoInventoryERSt6vectorISt4pairIiiESaIS2_EEbP5CUserRK10Inven_Item14eItemAddReasonb
// DEMANGLED: CBoosterGage::insertItemIntoInventory(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,CUser *,Inven_Item const&,eItemAddReason,bool)
int CBoosterGage::insertItemIntoInventory(
    BoosterInsertedSlotList& inserted_slots,
    bool booster_bonus,
    CUser* user,
    const Inven_Item& item,
    eItemAddReason reason,
    bool log_history) {
    int inserted_slot = 0;
    int repeat_count = 1;
    if (booster_bonus) {
        repeat_count = G_CDataManager()->GetCeraShopEventMultiplier();
        reason = static_cast<eItemAddReason>(58);
    }

    for (int index = 0; index < repeat_count; ++index) {
        inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
            item, reason, log_history, true);
        if (inserted_slot >= 0) {
            inserted_slots.push_back(
                std::make_pair(inserted_slot, item.GetItemSpace()));
        } else {
            inserted_slot = -1;
            make_postar(user, item, 0, 0);
        }
    }
    return inserted_slot;
}
