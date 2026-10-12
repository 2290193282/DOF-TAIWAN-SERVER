#include "old_game/economy/mail/mail_trade_runtime.h"

#include <algorithm>

#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

namespace private_store {

// IDA_EA: 0x85C4F9C
// IDA_NAME: _ZN13private_store13CPrivateStore9CopyItemsEPNS_16PrivateStoreItemEi
int CPrivateStore::CopyItems(PrivateStoreItem* source, int count) {
    ResetItems();
    std::copy(source, source + count, items.begin());
    return 0;
}

// IDA_EA: 0x85C5B60
// IDA_NAME: _ZN13private_store13CPrivateStore20RecreatePrivateStoreEPNS_17SIG_PRIVATE_STOREE
int CPrivateStore::RecreatePrivateStore(SIG_PRIVATE_STORE* payload) {
    if (!manager || state || !seller) return 0;

    static CCharacterControllerPrivate character_controller;
    character_controller.GetStartPoint(seller, position);
    buyers.clear();
    ResetItems();
    CopyItems(payload->item, 14);

    if (GlobalData::s_auction_proxy->IsRunning()) {
        PCK_AUCTION_OPEN_PRIVATE_STORE_GA packet;
        packet.account_id = seller->get_acc_id();
        const int charac_count = seller->get_charac_count();
        for (int index = 0; index < charac_count; ++index)
            packet.charac_no[index] = seller->get_charac_no(index);
        GlobalData::s_auction_proxy->SendPacket(
            reinterpret_cast<nsl::PACKET_HEADER*>(&packet));
    }

    state = 2;
    return 1;
}

// IDA_EA: 0x85C751A
// IDA_NAME: _ZN13private_store13CPrivateStore10MoveToGateEv
int CPrivateStore::MoveToGate() {
    error_handler.Set(seller, 1, ENUM_CMDPACKET_REMOVE_PRIVATE_STORE);
    if (!seller) return 0;
    if (!manager) {
        error_handler.Do(58);
        return 0;
    }

    CPacketBufPrivate packet;
    if (position.zone.village || position.zone.area || position.x || position.y) {
        packet.MakeNotipacketRemovePrivateStore(GetSellerIndex());
        CBroadcastPrivate::Send(position.zone.village, position.zone.area, packet);

        village_object::Zone zone(position.zone.village, position.zone.area);
        if (GlobalData::s_villageObjectMgr->unregister_object(zone,
                                                               &world_position) != 1) {
            LogManager::logFormat(
                1,
                "PrivateStore.cpp",
                "virtual bool private_store::CPrivateStore::MoveToGate()",
                0x658,
                "unregister_object error");
        }
        g_CharacterController.MoveCharacterStartPoint(seller);
    }

    packet.MakeCmdpacketMoveToGate();
    seller->Send(packet);
    return 1;
}

// IDA_EA: 0x85C8DD6
// IDA_NAME: _ZN13private_store13CPrivateStore17EnterPrivateStoreEv
int CPrivateStore::EnterPrivateStore() {
    error_handler.Set(buyer, 1, ENUM_CMDPACKET_ENTER_PRIVATE_STORE);
    if (!buyer) return 0;

    if (!manager) {
        error_handler.Do(58);
        return 0;
    }

    if (state != 2) {
        manager->FreeStoreBuyer(buyer);
        error_handler.Do(59);
        return 0;
    }

    if (std::find(buyers.begin(), buyers.end(), buyer) != buyers.end()) {
        error_handler.Do(20);
        return 0;
    }

    buyers.push_back(buyer);
    CPacketBufPrivate packet;
    packet.MakeCmdpacketEnterPrivateStore(seller, GetSellerIndex(), items);
    buyer->Send(packet);
    return 1;
}

// IDA_EA: 0x85C8FD4
// IDA_NAME: _ZN13private_store13CPrivateStore16ExitPrivateStoreEv
int CPrivateStore::ExitPrivateStore() {
    error_handler.Set(buyer, 1, ENUM_CMDPACKET_EXIT_PRIVATE_STORE);
    if (!buyer) return 0;

    if (!manager) {
        error_handler.Do(58);
        return 0;
    }

    std::list<CUser*>::iterator it = std::find(buyers.begin(), buyers.end(), buyer);
    if (!buyers.empty() && it == buyers.end()) {
        error_handler.Do(21);
        return 0;
    }

    CPacketBufPrivate packet;
    packet.MakeCmdpacketExitPrivateStore();
    buyer->Send(packet);
    manager->FreeStoreBuyer(buyer);
    if (!buyers.empty()) buyers.erase(it);
    buyer = 0;
    return 1;
}

}  // namespace private_store
