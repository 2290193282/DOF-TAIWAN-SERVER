#include "old_game/economy/mail/mail_trade_runtime.h"
#include "common/config.h"

#include <cstring>

#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

namespace private_store {

// IDA_EA: 0x85C5CEC
// IDA_NAME: _ZN13private_store13CPrivateStore28RecallPrivateStoreItemToMailEv
// DEMANGLED: private_store::CPrivateStore::RecallPrivateStoreItemToMail(void)
int CPrivateStore::RecallPrivateStoreItemToMail() {
    Inven_Item mail_items[14];
    int item_count = 0;
    for (int slot = 0; slot <= 13; ++slot) {
        PrivateStoreItem& store_item = items[slot];
        if (store_item.GetState() == 3 && store_item.item.getKey()) {
            mail_items[item_count++].setCopy(store_item.item);
        }
    }
    if (item_count <= 0) {
        ResetItems();
        return 1;
    }

    char title[21] = {};
    char text[256] = {};
    std::strcpy(title,
                g_scriptStringManager_.findString(4, "game_server_msg_183", 0));
    std::strcpy(text,
                g_scriptStringManager_.findString(4, "game_server_msg_186", 0));
    if (WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(
            title, mail_items, item_count, 0, seller->getCurCharacNo(), text,
            static_cast<int>(std::strlen(text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(seller->GetServerGroup()),
            false)) {
        cMyTrace trace(
            "bool private_store::CPrivateStore::RecallPrivateStoreItemToMail()",
            961, 5);
        trace("CPrivateStore::RecallPrivateStoreItemToMail Error - Mail Send Fail User(%d), Charac_no(%d), item_count(%d)",
              seller->get_acc_id(), seller->getCurCharacNo(), item_count);
        return 1;
    }

    ResetItems();
    return 1;
}

// IDA_EA: 0x85C63CA
// IDA_NAME: _ZN13private_store13CPrivateStore22RecallPrivateStoreItemEv
// DEMANGLED: private_store::CPrivateStore::RecallPrivateStoreItem(void)
int CPrivateStore::RecallPrivateStoreItem() {
    int slot = 0;
    for (; slot <= 13; ++slot) {
        PrivateStoreItem& store_item = items[slot];
        if (store_item.GetState() != 3) continue;

        int inserted = -1;
        if (store_item.item.item_category_ == 8u) {
            inserted = seller->getCurCharacInvenW()->insertAvatarIntoInventory(
                store_item.item, static_cast<eAvatarItemAddReason>(10), false);
            if (inserted < 0) break;
            seller->getCurCharacInvenR()->SendAvatarEvent(
                54,
                store_item.item.get_add_info(),
                0,
                inserted + oldsrv::config::kAvatarInventorySlotBase,
                0);
        } else {
            inserted = seller->getCurCharacInvenW()->insertItemIntoInventory(
                store_item.item, static_cast<eItemAddReason>(18), false, true);
        }
        if (inserted < 0) break;

        PrivateStoreItem restored_item;
        restored_item.inventory_slot = static_cast<uint16_t>(inserted);
        GetItemInfo(seller, restored_item);
        const int restored_count = store_item.item.isEquipableItemType()
                                       ? 1
                                       : restored_item.item.get_add_info();
        const int previous_count = store_item.item.isEquipableItemType()
                                       ? 1
                                       : store_item.item.get_add_info();
        seller->history_log().PrivateStoreItemAdd(
            static_cast<INVEN_TYPE>(1), store_item.item, restored_count,
            previous_count, static_cast<eItemAddReason>(18),
            seller->getCurCharacName(), seller->getCurCharacName());
        store_item.inventory_slot = static_cast<uint16_t>(inserted);
        store_item.state = 4;
    }

    if (slot == 14) {
        ResetItems();
        return 1;
    }

    for (slot = 0; slot <= 13; ++slot) {
        PrivateStoreItem& store_item = items[slot];
        if (store_item.GetState() == 4) {
            if (DeleteItem(store_item)) {
                cMyTrace trace(
                    "bool private_store::CPrivateStore::RecallPrivateStoreItem()",
                    1225, 5);
                trace("[%s][%d]",
                      "bool private_store::CPrivateStore::RecallPrivateStoreItem()",
                      1225);
            }
            store_item.state = 3;
        }
    }
    return 0;
}

// IDA_EA: 0x85C5FD0
// IDA_NAME: _ZN13private_store13CPrivateStore18RemovePrivateStoreEb
// DEMANGLED: private_store::CPrivateStore::RemovePrivateStore(bool)
int CPrivateStore::RemovePrivateStore(bool force) {
    error_handler.Set(seller, 1, ENUM_CMDPACKET_REMOVE_PRIVATE_STORE);
    if (!seller) return 0;
    if (!manager) {
        error_handler.Do(58);
        return 0;
    }
    if (!state) {
        error_handler.Do(59);
        return 0;
    }

    CPacketBufPrivate packet;
    if (state == 2) {
        if (isDollCreated()) {
            if (RecallPrivateStoreItemToMail()) SavePrivateStore();
        } else if (RecallPrivateStoreItem()) {
            SavePrivateStore();
            seller->UpdateData();
            seller->send_itemspace(0);
            seller->send_itemspace(1);
            seller->getCurCharacInvenR()->SendItemLockListInven();
            seller->getCurCharacInvenR()->SendItemLockListAvatar();
        } else {
            SavePrivateStore();
            seller->UpdateData();
            error_handler.Do(22);
            if (!force) return 0;
        }
        if (!buyers.empty()) {
            packet.MakeNotipacketForceRemovePrivateStore();
            SendUsers(&packet, 0, 0);
        }
    }

    seller->history_log().RemovePrivateStore(position.zone.village,
                                              position.zone.area,
                                              seller->getCurCharacName());
    if (GlobalData::s_auction_proxy->IsRunning()) {
        PCK_AUCTION_CLOSE_PRIVATE_STORE_GA auction_packet;
        auction_packet.account_id = seller->get_acc_id();
        GlobalData::s_auction_proxy->SendPacket(
            reinterpret_cast<nsl::PACKET_HEADER*>(&auction_packet));
    }

    packet.MakeCmdpacketRemovePrivateStore();
    seller->Send(packet);
    packet.MakeNotipacketRemovePrivateStore(GetSellerIndex());
    CBroadcastPrivate::Send(position.zone.village, position.zone.area, packet);

    village_object::Zone zone(position.zone.village, position.zone.area);
    if (GlobalData::s_villageObjectMgr->unregister_object(zone,
                                                           &world_position) != 1) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::RemovePrivateStore(bool)",
            0x473, "unregister_object error");
    }
    manager->FreeStoreSeller(seller);
    const int user_uid = seller->get_unique_id();
    cMyTrace trace("virtual bool private_store::CPrivateStore::RemovePrivateStore(bool)",
                   1144, 0);
    trace("RemovePrivateStore() uid:%d\n", user_uid);
    Clear();
    return 1;
}

}  // namespace private_store
