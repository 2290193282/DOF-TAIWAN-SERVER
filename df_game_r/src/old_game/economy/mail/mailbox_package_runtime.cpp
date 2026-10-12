#include "old_game/economy/mail/mail_trade_runtime.h"
#include "common/config.h"

#include <cstring>

#include "old_data/PVF/data/datamanager/avatar_roulette_server.h"
#include "common/manager/global_data.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/security/policy/secu_gold_control_abi.h"
#include "infra/log/format/log_manager.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/anti_cheat/db_account_hack_check_my_account.h"
#include "old_data/db/avatar_aura/db_insert_cera_item_history.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/manager/system_time.h"

namespace {

const ENUM_CMDPACKET kMailCommand = ENUM_CMDPACKET_MAILBOX_EXTRACT_ITEM;
const int kPackageCapacityError = 4;
const int kPackageGoldLimitError = 10;
const int kPackageCeraPointLimitError = 11;
const int kPackageCargoLimitError = 13;
const int kPackageMissingItemError = 1;
const int kPackageCoinLimitError = 79;
const int kMailMoneyAddReason = 14;
const int kMailItemAddReason = 21;
const int kMailAvatarAddReason = 5;
const unsigned int kCeraPointMaximum = 1000000000u;
const unsigned int kCoinMaximum = 100000u;
const unsigned int kGoldItemFirst = 2675336u;
const unsigned int kGoldItemPivot = 2675345u;
const unsigned int kGoldItemLast = 2675353u;
const unsigned long kCeraPointItem = 2681762ul;
const unsigned long kCeraPointItemGift = 2681917ul;

bool IsUnusableSpecialItemInVillage(const CItem& item) {
    const int item_type = item.vtable_->get_item_type(&item);
    return item_type != 16 && item_type != 34;
}

bool IsAvatarItem(const CItem& item) {
    return item.vtable_->is_avatar_item(&item);
}

bool IsCreatureItem(const CItem& item) {
    return item.vtable_->is_creature_item(&item);
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x8553672
// IDA_NAME: _ZN8WongWork14CMailBoxHelper17_DoGetPackageCoinEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
// DEMANGLED: WongWork::CMailBoxHelper::_DoGetPackageCoin(CUser *,WongWork::CMailBox::CMail const*,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
bool CMailBoxHelper::_DoGetPackageCoin(CUser* user,
                                       const CMailBox::CMail* mail,
                                       IntPairVector& inserted_slots,
                                       bool is_received,
                                       ENUM_ERROR& error) {
    CInventory* const inventory = user->getCurCharacInvenW();
    const int coin = inventory->GetCoin();
    const int event_coin = inventory->GetEventCoin();
    const int pay_coin = inventory->GetPayCoin();
    if (pay_coin + coin + event_coin > static_cast<int>(kCoinMaximum)) {
        error = static_cast<ENUM_ERROR>(kPackageCoinLimitError);
        return false;
    }
    if (is_received) {
        return true;
    }

    const Inven_Item item = mail->GetPackageItem();
    switch (item.avatar_select_) {
    case 0:
        inventory->SetCoin(inventory->GetCoin() + item.add_info_);
        user->history_log().CoinAdd(inventory->GetEventCoin(), item.add_info_,
                                    static_cast<eCoinAddReason>(0));
        break;
    case 1:
        inventory->SetEventCoin(inventory->GetEventCoin() + item.add_info_);
        user->history_log().EventCoinAdd(
            inventory->GetEventCoin(), item.add_info_,
            static_cast<eCoinAddReason>(2));
        break;
    case 2:
        inventory->SetPayCoin(inventory->GetPayCoin() + item.add_info_);
        user->history_log().PayCoinAdd(inventory->GetEventCoin(), item.add_info_,
                                       static_cast<eCoinAddReason>(3));
        break;
    default:
        break;
    }
    inserted_slots.push_back(std::make_pair(0, 1));
    return true;
}

// IDA_EA: 0x8553912
// IDA_NAME: _ZN8WongWork14CMailBoxHelper15IsCeraPointItemEm
// DEMANGLED: WongWork::CMailBoxHelper::IsCeraPointItem(unsigned long)
bool CMailBoxHelper::IsCeraPointItem(unsigned long item_id) {
    return item_id == kCeraPointItem || item_id == kCeraPointItemGift;
}

// IDA_EA: 0x8553936
// IDA_NAME: _ZN8WongWork14CMailBoxHelper17_DoGetPackageItemEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
// DEMANGLED: WongWork::CMailBoxHelper::_DoGetPackageItem(CUser *,WongWork::CMailBox::CMail const*,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
bool CMailBoxHelper::_DoGetPackageItem(CUser* user,
                                       const CMailBox::CMail* mail,
                                       IntPairVector& inserted_slots,
                                       bool is_received,
                                       ENUM_ERROR& error) {
    const Inven_Item item = mail->GetPackageItem();
    const unsigned int item_id = static_cast<unsigned int>(item.item_id_);
    if (item_id > kGoldItemPivot && item_id <= kGoldItemLast) {
        return user->DoGetGoldItem(item_id, inserted_slots, is_received, error) !=
               0;
    }
    if (IsCeraPointItem(item_id)) {
        return user->DoGetCeraPointItem(
                   static_cast<unsigned int>(mail->GetPackageMoney()),
                   is_received, error, item_id) != 0;
    }

    int special_type = 0;
    int special_value = 0;
    bool is_unique = false;
    const bool is_special =
        GlobalData::s_pCeraShop->IsSpecialItem(item.item_id_, special_type,
                                                special_value, is_unique) == 1;
    CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
    const bool can_place_as_item =
        !is_special || !IsUnusableSpecialItemInVillage(*prototype);

    if (!is_received) {
        if (can_place_as_item) {
            const int inserted = user->getCurCharacInvenW()->insertItemIntoInventory(
                item, static_cast<eItemAddReason>(kMailItemAddReason), true,
                false);
            if (inserted < 0) {
                return true;
            }
            inserted_slots.push_back(std::make_pair(0, inserted));
            return true;
        }

        const int restrict_code =
            G_CDataManager()->get_item_list()->GetRestrictCode(item.item_id_);
        const long current_seconds = GlobalData::s_systemTime_.getCurSec();
        std::vector<Inven_Item> output_items;
        CCeraShop::stCeraShopItemParam_t item_param;
        int processing_result = 0;
        if (GlobalData::s_pCeraShop->ProcessSpecialItem(
                user, item.item_id_, restrict_code, processing_result,
                current_seconds, true, output_items, item_param, 0, 0, false,
                1) == 1) {
            if (processing_result) {
                return false;
            }
            if (restrict_code && !user->IsRestrictedGoods(restrict_code)) {
                user->SetRestrictedGoods(restrict_code);
                GlobalData::s_pCeraShop->SaveFeaturedIdx(
                    user, restrict_code, current_seconds, 0);
                user->SendRestrictedGoods(true, restrict_code);
            }
            if (item.item_id_ == 193 &&
                GlobalData::s_pCeraShop->ProcessSpecialItem(
                    user, 16, 50002, processing_result, current_seconds, true,
                    output_items, item_param, 0, 0, false, 1) != 1) {
                LogManager::logFormat(
                    1, "MailBox.cpp",
                    "static bool WongWork::CMailBoxHelper::_DoGetPackageItem(CUser*, const WongWork::CMailBox::CMail*, WongWork::CMailBoxHelper::IntPairVector&, bool, ENUM_ERROR&)",
                    0x34c, "AuctionVIP Error - Present to friend(add private-store)");
            }
        }
        return true;
    }

    if (can_place_as_item) {
        if (user->getCurCharacInvenR()->tryInsertItemIntoInventory(item) < 0) {
            error = static_cast<ENUM_ERROR>(kPackageCapacityError);
            return false;
        }
        return true;
    }
    if (special_type == 1) {
        if (user->getCurCharacCargoR()->GetCapacity() >= special_value) {
            error = static_cast<ENUM_ERROR>(kPackageCargoLimitError);
            return false;
        }
        return true;
    }
    if (special_type != 5) {
        return true;
    }

    const CStackableItem& package_item =
        static_cast<const CStackableItem&>(*prototype);
    IntPairVector package_items;
    const std::vector<STStackablePackageData>& package_data =
        package_item.package_data();
    for (std::vector<STStackablePackageData>::const_iterator it =
             package_data.begin();
         it != package_data.end(); ++it) {
        package_items.push_back(
            std::make_pair(static_cast<int>(it->item_index),
                           static_cast<int>(it->item_count)));
    }

    CInventory scratch_inventory;
    InventoryMemory scratch_memory;
    scratch_inventory.SetInventoryMemory(&scratch_memory);
    scratch_inventory.setCopy(*user->getCurCharacInvenRefR());
    if (scratch_inventory.tryInsertEventItems(package_items) !=
        static_cast<int>(package_items.size())) {
        error = static_cast<ENUM_ERROR>(kPackageCapacityError);
        return false;
    }
    return true;
}

// IDA_EA: 0x855419C
// IDA_NAME: _ZN8WongWork14CMailBoxHelper19_DoGetPackageAvatarEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
// DEMANGLED: WongWork::CMailBoxHelper::_DoGetPackageAvatar(CUser *,WongWork::CMailBox::CMail const*,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
bool CMailBoxHelper::_DoGetPackageAvatar(CUser* user,
                                         const CMailBox::CMail* mail,
                                         IntPairVector&,
                                         bool is_received,
                                         ENUM_ERROR& error) {
    const Inven_Item item = mail->GetPackageItem();
    if (!mail->isAvatarPackage()) {
        error = static_cast<ENUM_ERROR>(kPackageMissingItemError);
        return false;
    }
    if (is_received) {
        const CAvatarItemMgr* const avatar_items =
            user->getCurCharacInvenR()->GetAvatarItemMgrR();
        if (avatar_items->GetExpireDate(item.add_info_) == -1) {
            return false;
        }
        if (user->getCurCharacInvenW()->tryInsertAvatarIntoInventory(item) < 0) {
            error = static_cast<ENUM_ERROR>(kPackageCapacityError);
            return false;
        }
        return true;
    }

    const int inserted = user->getCurCharacInvenW()->insertAvatarIntoInventory(
        item, static_cast<eAvatarItemAddReason>(kMailAvatarAddReason), true);
    if (inserted < 0) {
        return false;
    }
    user->getCurCharacInvenW()->SendAvatarEvent(
        54,
        item.add_info_,
        0,
        inserted + oldsrv::config::kAvatarInventorySlotBase,
        0);

    const char* const system_sender =
        g_scriptStringManager_.findString(4, "game_server_msg_221", 0);
    DB_InsertCeraItemHistory::makeRequest(
        item.add_info_, static_cast<unsigned int>(mail->GetSenderCharacNo()),
        static_cast<unsigned int>(user->getCurCharacNo()),
        std::strcmp(mail->GetSenderName(), system_sender) == 0 ? 1 : 0);
    return true;
}

// IDA_EA: 0x855447C
// IDA_NAME: _ZN8WongWork14CMailBoxHelper21_DoGetPackageCreatureEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
// DEMANGLED: WongWork::CMailBoxHelper::_DoGetPackageCreature(CUser *,WongWork::CMailBox::CMail const*,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
bool CMailBoxHelper::_DoGetPackageCreature(CUser* user,
                                           const CMailBox::CMail* mail,
                                           IntPairVector& inserted_slots,
                                           bool is_received,
                                           ENUM_ERROR& error) {
    Inven_Item item = mail->GetPackageItem();
    if (!mail->isCreaturePackage()) {
        error = static_cast<ENUM_ERROR>(kPackageMissingItemError);
        return false;
    }
    if (is_received) {
        if (user->getCurCharacInvenRefR()->tryInsertItemIntoCreature(item) < 0) {
            error = static_cast<ENUM_ERROR>(kPackageCapacityError);
            return false;
        }
        return true;
    }

    const int inserted = user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(kMailItemAddReason), true, true);
    if (inserted < 0) {
        return false;
    }
    user->getCurCharacInvenW()->GetCreatureMgrW()->RecvMail(
        &item, inserted, kMailItemAddReason);
    if (item.item_category_ != 5) {
        inserted_slots.push_back(std::make_pair(7, inserted));
    }
    return true;
}

// IDA_EA: 0x8554650
// IDA_NAME: _ZN8WongWork14CMailBoxHelper13_DoGetPackageEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
// DEMANGLED: WongWork::CMailBoxHelper::_DoGetPackage(CUser *,WongWork::CMailBox::CMail const*,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
bool CMailBoxHelper::_DoGetPackage(CUser* user,
                                   const CMailBox::CMail* mail,
                                   IntPairVector& inserted_slots,
                                   bool is_received,
                                   ENUM_ERROR& error) {
    if (!user->GetMailBox()) {
        return false;
    }

    const int package_money = mail->GetPackageMoney();
    if (package_money) {
        const Inven_Item money_item = mail->GetPackageItem();
        if (is_received) {
            if (IsCeraPointItem(money_item.item_id_)) {
                if (static_cast<unsigned int>(package_money + user->GetCeraPoint()) >
                    kCeraPointMaximum) {
                    error = static_cast<ENUM_ERROR>(kPackageCeraPointLimitError);
                    return false;
                }
            } else {
                const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
                    user->get_charac_level(), user->get_acc_name());
                if (money_limit - user->getCurCharacMoney() < package_money) {
                    error = static_cast<ENUM_ERROR>(kPackageGoldLimitError);
                    return false;
                }
            }
        } else {
            if (!IsCeraPointItem(money_item.item_id_)) {
                static_cast<Secu_GoldControl*>(user->GetGoldControl())
                    ->SetMailCharName(mail->GetSenderName());
                user->getCurCharacInvenW()->gain_money(
                    package_money, static_cast<eMoneyAddReason>(kMailMoneyAddReason),
                    true, 0);
            }
            inserted_slots.push_back(std::make_pair(0, 0));
        }
    }

    const Inven_Item item = mail->GetPackageItem();
    if (!item.item_id_) {
        return true;
    }
    if (item.item_id_ == 1) {
        return _DoGetPackageCoin(user, mail, inserted_slots, is_received, error);
    }
    if (GlobalData::s_pAvatarRoulette->isAvatarPottery(item.item_id_)) {
        return AvatarCoin::GetPackage(user, item.add_info_, is_received) != 0;
    }

    CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
    if (!prototype) {
        error = static_cast<ENUM_ERROR>(kPackageMissingItemError);
        return false;
    }
    if (IsAvatarItem(*prototype)) {
        return _DoGetPackageAvatar(user, mail, inserted_slots, is_received,
                                   error);
    }
    if (IsCreatureItem(*prototype)) {
        return _DoGetPackageCreature(user, mail, inserted_slots, is_received,
                                     error);
    }
    return _DoGetPackageItem(user, mail, inserted_slots, is_received, error);
}

// IDA_EA: 0x85549DE
// IDA_NAME: _ZN8WongWork14CMailBoxHelper10GetPackegeEP5CUserj
// DEMANGLED: WongWork::CMailBoxHelper::GetPackege(CUser *,unsigned int)
void CMailBoxHelper::GetPackege(CUser* user, unsigned int mail_idx) {
    CMailBox* const mailbox = user->GetMailBox();
    CMailBox::CMail* const mail = mailbox ? mailbox->GetMail(mail_idx) : 0;
    if (!mail) {
        user->SendCmdErrorPacket(kMailCommand, 21);
        return;
    }

    IntPairVector inserted_slots;
    ENUM_ERROR error = static_cast<ENUM_ERROR>(0);
    if (!_DoGetPackage(user, mail, inserted_slots, true, error)) {
        user->SendCmdErrorPacket(kMailCommand,
                                 static_cast<unsigned char>(error));
        return;
    }

    _DoGetPackage(user, mail, inserted_slots, false, error);
    PacketGuard packet;
    for (IntPairVector::const_iterator it = inserted_slots.begin();
         it != inserted_slots.end(); ++it) {
        packet.put_header(0, 14);
        packet.put_byte(it->first);
        packet.put_short(1);
        if (it->first == 1) {
            user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(2), it->second, packet);
        } else if (it->first == 7) {
            user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(3), it->second, packet);
        } else {
            user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(1), it->second, packet);
        }
        packet.finalize(true);
        user->Send(packet);
    }

    if (static_cast<unsigned int>(mail->GetPackageMoney()) % 100000u) {
        const char* const sender_name = mail->GetSenderName();
        DB_AccountHack_CheckMyAccount::makeRequest(
            user->GetUID(), static_cast<unsigned int>(user->get_acc_id()), 0,
            static_cast<unsigned int>(mail->GetPackageMoney()),
            const_cast<char*>(sender_name),
            static_cast<int>(std::strlen(sender_name)));
    }
    user->history_log().RecvMail(mail_idx, mail->GetSenderName(),
                                 user->get_acc_name());

    packet.clear();
    packet.put_header(1, kMailCommand);
    packet.put_byte(1);
    packet.put_int(mail_idx);
    packet.finalize(true);
    user->Send(packet);

    unsigned int postal_ids[2] = {1u, mail_idx};
    ReqDBRemoveMail(user, postal_ids, false);
    mailbox->RemoveMail(mail_idx);
}

}  // namespace WongWork
