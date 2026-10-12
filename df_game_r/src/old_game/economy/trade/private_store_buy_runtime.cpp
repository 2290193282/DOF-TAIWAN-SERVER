#include "old_game/economy/mail/mail_trade_runtime.h"
#include "common/config.h"

#include <cstring>

#include "common/manager/history_log_abi.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/avatar_aura/db_avatar_change_owner.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/manager/os_api.h"

// IDA_EA: 0x80CB0E8
// IDA_NAME: _Z14G_CDataManagerv
// DEMANGLED: G_CDataManager(void)
CDataManager* G_CDataManager();

// IDA_EA: 0x80CA15C
// IDA_NAME: _Z12get_rand_inti
// DEMANGLED: get_rand_int(int)
unsigned int get_rand_int(int limit);

// IDA_EA: 0x84F525A
// IDA_NAME: _Z26IsTradeLimitAttachTypeItemRK10Inven_Item
// DEMANGLED: IsTradeLimitAttachTypeItem(Inven_Item const&)
int IsTradeLimitAttachTypeItem(const Inven_Item* item)
    __asm__("_Z26IsTradeLimitAttachTypeItemRK10Inven_Item");

// IDA_EA: 0x84F5290
// IDA_NAME: _Z23DecreaseTradeLimitCountR10Inven_Item
// DEMANGLED: DecreaseTradeLimitCount(Inven_Item &)
int DecreaseTradeLimitCount(Inven_Item* item)
    __asm__("_Z23DecreaseTradeLimitCountR10Inven_Item");

namespace private_store {

// IDA_EA: 0x85C7E74
// IDA_NAME: _ZN13private_store13CPrivateStore19BuyPrivateStoreItemEii
// DEMANGLED: private_store::CPrivateStore::BuyPrivateStoreItem(int,int)
int CPrivateStore::BuyPrivateStoreItem(int slot, int requested_count) {
    error_handler.Set(buyer, 1, ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM);
    if (!buyer) return 0;
    if (!manager) {
        error_handler.Do(58);
        return 0;
    }
    if (!IsExistBuyerList(buyer)) {
        error_handler.Do(21);
        return 0;
    }
    if (!buyer->getCurCharacR()) {
        error_handler.Do(1);
        return 0;
    }
    if (slot < 0 || slot >= 14) {
        error_handler.Do(57);
        return 0;
    }
    if (manager->IsBusyPrivateStore(buyer)) {
        error_handler.Do(60);
        return 0;
    }

    PrivateStoreItem& store_item = items[slot];
    CItem* const item =
        G_CDataManager()->find_item(store_item.item.getKey());
    if (!item) {
        error_handler.Do(17);
        return 0;
    }
    const int attach_type = item->GetAttachType();
    if (attach_type == 3) {
        if (store_item.item.seal_or_state_ != 1) {
            error_handler.Do(17);
            return 0;
        }
    } else if (attach_type != 0) {
        if (attach_type == 7) {
            if (!store_item.item.GetTradeLimitCount()) return 0;
        } else {
            error_handler.Do(17);
            return 0;
        }
    }
    if (IsCreatureItemType(store_item.item.item_category_)) {
        error_handler.Do(17);
        return 0;
    }

    int money = 0;
    unsigned int first_value = 0;
    unsigned int second_value = 0;
    if (GetItemPrice(item, store_item, requested_count, money, first_value,
                     second_value) != 1 ||
        IsPossibleDeal(money) != 1) {
        return 0;
    }
    if (!buyer->GetReliablePerson() &&
        !buyer->IsAvailableCurCharacTradeGoldDaily(
            static_cast<unsigned int>(money))) {
        error_handler.Do(70);
        return 0;
    }

    Inven_Item purchased_item;
    purchased_item.setCopy(store_item.item);
    if (IsTradeLimitAttachTypeItem(&purchased_item)) {
        DecreaseTradeLimitCount(&purchased_item);
        seller->history_log().ChangeItemLimitCount(
            -1, -1, purchased_item.getKey(), -1, 1);
    }

    int item_count = 0;
    const int inventory_slot = BuyItem(slot, purchased_item, money,
                                       static_cast<int>(first_value),
                                       static_cast<int>(second_value),
                                       item_count);
    if (inventory_slot < 0) {
        error_handler.Do(4);
        return 0;
    }

    const int commission = money * GetCommissionPercent() / 100;
    const int original_item_count = store_item.item.get_add_info();
    SellItem(slot, purchased_item, money, commission, item_count,
             original_item_count);
    SubtractItemCount(item, store_item, requested_count);
    SavePrivateStore();
    seller->UpdateData();
    buyer->UpdateData();

    CPacketBufPrivate packet;
    packet.MakeNotipacketPrivateStoreItem(slot, requested_count);
    SendUsers(&packet, 0, 0);
    packet.MakeNotipacketBuyPrivateStoreItem(
        seller, purchased_item.getKey(), slot,
        purchased_item.get_add_info(), buyer->getCurCharacName(), money,
        commission);
    seller->Send(packet);
    packet.MakeCmdpacketBuyPrivateStoreItem(
        buyer, purchased_item, static_cast<int>(second_value),
        static_cast<int>(first_value), inventory_slot,
        purchased_item.get_add_info());
    buyer->Send(packet);

    const int low_rarity_non_avatar =
        item->vtable_->is_avatar_item(item) != true && item->get_rarity() <= 1;
    buyer->IncrementTradeGold(static_cast<unsigned int>(money),
                               low_rarity_non_avatar);

    const unsigned short used_fatigue = static_cast<unsigned short>(
        buyer->getCurCharacUsedFatigue());
    const int random_ratio = static_cast<int>(get_rand_int(25));
    const int random_fatigue = static_cast<int>(get_rand_int(20));
    const int buyer_money = buyer->getCurCharacInvenR()->get_money();
    if (static_cast<double>(money) >
        static_cast<double>(money + buyer_money) *
            (static_cast<double>(random_ratio) * 0.01 + 0.7)) {
        const unsigned int seller_account =
            static_cast<unsigned int>(seller->get_acc_id());
        const unsigned int buyer_etc = static_cast<unsigned int>(
            buyer->SetETC(704, seller_account));
        buyer->getHackAnalyzer()->addServerHackCnt(
            buyer, static_cast<WongWork::ENUM_HACKTYPE>(0x2c0), 1,
            buyer_etc, 0);
        if (used_fatigue >= random_fatigue + 120) {
            const unsigned int fatigue_etc = static_cast<unsigned int>(
                buyer->SetETC(701, seller_account));
            buyer->getHackAnalyzer()->addServerHackCnt(
                buyer, static_cast<WongWork::ENUM_HACKTYPE>(0x2bd), 1,
                fatigue_etc, 0);
        }
        const unsigned int buyer_account =
            static_cast<unsigned int>(buyer->get_acc_id());
        const unsigned int seller_etc = static_cast<unsigned int>(
            seller->SetETC(705, buyer_account));
        seller->getHackAnalyzer()->addServerHackCnt(
            seller, static_cast<WongWork::ENUM_HACKTYPE>(0x2c1), 1,
            seller_etc, 0);
    }

    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(18), seller,
        static_cast<unsigned int>(commission));
    GetInstanceCirculationStatistic()->AddCirculationStatistic(
        static_cast<CIRCULATION_STATISTIC_FIELD>(21), seller,
        static_cast<unsigned int>(commission));
    return 1;
}

// IDA_EA: 0x85C924C
// IDA_NAME: _ZN13private_store13CPrivateStore7BuyItemEiR10Inven_ItemiiiRi
// DEMANGLED: private_store::CPrivateStore::BuyItem(int,Inven_Item &,int,int,int,int &)
int CPrivateStore::BuyItem(int /*slot*/,
                           Inven_Item& item,
                           int money,
                           int /*first_value*/,
                           int /*second_value*/,
                           int& item_count) {
    cMyTrace trace(
        "int private_store::CPrivateStore::BuyItem(int, Inven_Item&, int, int, int, int&)",
        2659, 0);
    trace("PrivateStore buy seller_charac_name(%s), buyer_charac_name(%s), item_id(%d), add_info(%d), upgrade(%d), reseal_count(%d), sell_price(%d)",
          seller->getCurCharacName(), buyer->getCurCharacName(),
          item.getKey(), item.get_add_info(), item.GetUpgrade(),
          item.GetReSealCount(), money);

    int inserted_slot = -1;
    if (item.isAvatarItemType()) {
        inserted_slot = buyer->getCurCharacInvenW()->tryInsertAvatarIntoInventory(item);
        if (inserted_slot >= 0) {
            WongWork::Avatar_Item* const avatar_item =
                seller->getCurCharacInvenW()->GetAvatarItemMgrW()
                    ->EraseAvatarItem(item.get_add_info());
            if (!avatar_item) return -1;

            inserted_slot = buyer->getCurCharacInvenW()
                                ->insertAvatarIntoSpecificSlot(
                                    item, inserted_slot,
                                    static_cast<eAvatarItemAddReason>(11), false);
            if (inserted_slot == -1) return -1;

            buyer->getCurCharacInvenW()->GetAvatarItemMgrW()->AddAvatarItem(
                item.get_add_info(), avatar_item);
            DB_AvatarChangeOwner::makeRequest(
                static_cast<unsigned int>(item.get_add_info()),
                static_cast<unsigned int>(seller->getCurCharacNo()),
                static_cast<unsigned int>(buyer->getCurCharacNo()),
                static_cast<unsigned char>(inserted_slot));
            buyer->getCurCharacInvenR()->SendAvatarEvent(
                54,
                item.get_add_info(),
                0,
                inserted_slot + oldsrv::config::kAvatarInventorySlotBase,
                0);
        }
    } else {
        inserted_slot = buyer->getCurCharacInvenW()->insertItemIntoInventory(
            item, static_cast<eItemAddReason>(17), false, false);
    }
    if (inserted_slot < 0) return inserted_slot;

    PrivateStoreItem purchased_store_item;
    purchased_store_item.inventory_slot = static_cast<uint16_t>(inserted_slot);
    GetItemInfo(buyer, purchased_store_item);
    const int source_count = item.isEquipableItemType() ? 1 : item.get_add_info();
    item_count = item.isEquipableItemType()
                     ? 1
                     : purchased_store_item.item.get_add_info();
    buyer->history_log().PrivateStoreItemAdd(
        static_cast<INVEN_TYPE>(1), item, item_count, source_count,
        static_cast<eItemAddReason>(17), seller->getCurCharacName(),
        buyer->getCurCharacName());

    char buyer_ip[16] = {};
    seller->getHades()->GetIP(buyer_ip);
    buyer->getAcccountHackInfo()->sendGold(
        0,
        static_cast<unsigned int>(seller->get_acc_id()),
        static_cast<unsigned int>(money),
        static_cast<unsigned int>(buyer->getCurCharacInvenR()->get_money()), 1,
        buyer_ip);
    buyer->getCurCharacInvenW()->use_money(
        money, static_cast<eMoneySubReason>(14), true);
    HistoryLog::WriteBuyItem(buyer->history_stream(),
                             item.getKey(), item.get_add_info());
    return inserted_slot;
}

// IDA_EA: 0x85C97E2
// IDA_NAME: _ZN13private_store13CPrivateStore8SellItemEiR10Inven_Itemiiii
// DEMANGLED: private_store::CPrivateStore::SellItem(int,Inven_Item &,int,int,int,int)
int CPrivateStore::SellItem(int /*slot*/,
                            Inven_Item& item,
                            int money,
                            int commission,
                            int item_count,
                            int original_item_count) {
    const int seller_money = money - commission;
    const int previous_count = item.isEquipableItemType()
                                   ? 0
                                   : original_item_count - item_count;
    if (item.isEquipableItemType()) item_count = 1;
    seller->history_log().PrivateStoreItemDel(
        static_cast<INVEN_TYPE>(1), item, item_count, previous_count,
        static_cast<eItemDelReason>(22), seller->getCurCharacName(),
        buyer->getCurCharacName());

    if (isDollCreated()) {
        char title[21] = {};
        char text[256] = {};
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_183", 0),
                     0x14u);
        CItem* const data_item =
            G_CDataManager()->find_item(item.getKey());
        if (data_item->is_stackable() == 1) {
            OS_API::snprintf(
                text, 0xffu,
                g_scriptStringManager_.findString(4, "game_server_msg_185", 0),
                data_item->GetItemName(), item_count, money, commission,
                data_item->GetItemName(), seller_money);
        } else {
            OS_API::snprintf(
                text, 0xffu,
                g_scriptStringManager_.findString(4, "game_server_msg_184", 0),
                data_item->GetItemName(), money, commission,
                data_item->GetItemName(), seller_money);
        }
        Inven_Item mail_item;
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title, mail_item, static_cast<unsigned int>(seller_money),
            static_cast<unsigned int>(seller->getCurCharacNo()), text,
            static_cast<int>(std::strlen(text)), 0,
            static_cast<ENUM_SERVER_GROUP>(seller->GetServerGroup()), false,
            false);
    } else {
        seller->getCurCharacInvenW()->gain_money(
            seller_money, static_cast<eMoneyAddReason>(11), true, 0);
    }
    HistoryLog::WriteSellItem(seller->history_stream(),
                              item.getKey(), item.get_add_info());
    return 1;
}

}  // namespace private_store
