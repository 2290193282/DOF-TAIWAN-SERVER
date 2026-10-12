#include "old_game/pvp/battlefield/war_room.h"

#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/inventory/db_insert_random_option_item_inform.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

void SendGetItemError(CUser* user, int error) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
    packet.put_byte(0);
    packet.put_byte(static_cast<unsigned char>(error));
    packet.finalize(true);
    user->Send(packet);
}

int InsertWarRoomItem(CUser* user, const Inven_Item& item) {
    if (item.isAvatarItemType()) {
        char ipg_no[24] = {0};
        WongWork::CGenUniqueNo::genIPGNo(
            static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(5),
            static_cast<unsigned int>(user->getCurCharacNo()),
            ipg_no);
        return user->addAvatarItem(static_cast<unsigned long>(item.getKey()),
                                   0,
                                   static_cast<char>(item.avatar_select_),
                                   ipg_no,
                                   static_cast<eAvatarItemAddReason>(17));
    }
    return user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(22), true, true);
}

void InformRandomOptionFirstPickup(CUser* user, const CItem& item) {
    if (!item.IsRandomOption() || user->getCheckPickUpRandomOptionItem() == 1) {
        return;
    }
    user->setCheckPickUpRandomOptionItem(true);
    DB_InsertRandomOptionItemInform::makeRequest(
        user->GetUID(), static_cast<unsigned int>(user->get_acc_id()), 1u);
    PacketGuard packet;
    packet.put_header(0, static_cast<ENUM_NOTIPACKET>(369));
    packet.put_short(user->get_unique_id());
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

// IDA_EA: 0x86BEA2E
// IDA_NAME: _ZN7WarRoom13HandleGetItemEP5CUseri
// DEMANGLED: WarRoom::HandleGetItem(CUser *,int)
int WarRoom::HandleGetItem(CUser* user, int unique_id) {
    if (!user) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(0);
        packet.put_byte(22);
        packet.finalize(true);
        SendToRoom(packet);
        return 0;
    }

    map_item dropped_item;
    if (war_field_.GetFieldItem(unique_id, dropped_item) != 1 ||
        GetWaiterCount() <= 0) {
        SendGetItemError(user, 21);
        return 0;
    }

    std::map<int, map_item>::iterator field_item;
    const int pickup_error = war_field_.CheckPickupItem(
        unique_id, user->get_unique_id(), field_item);
    if (pickup_error > 0) {
        SendGetItemError(user, pickup_error);
        return 0;
    }

    if (dropped_item.item.getKey() != 0) {
        CItem* const item = G_CDataManager()->find_item(dropped_item.item.getKey());
        if (!item) return 0;

        InformRandomOptionFirstPickup(user, *item);
        const int inserted = InsertWarRoomItem(user, dropped_item.item);
        if (inserted == -1) {
            SendGetItemError(user, 4);
            return 0;
        }
        if (war_field_.PickupItem(field_item) != 1) return 0;

        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_WARROOM_GET_ITEM);
        packet.put_short(static_cast<int16_t>(unique_id));
        packet.put_short(user->get_unique_id());
        packet.put_short(static_cast<int16_t>(inserted));
        packet.finalize(true);
        SendToRoom(packet);
        return 1;
    }

    const int requested_money = dropped_item.item.get_add_info();
    int gained_money = user->getCurCharacInvenW()->gain_money(
        requested_money, static_cast<eMoneyAddReason>(15), true, 0);
    user->saveTaxMoneyForUpperMember(requested_money);
    if (gained_money < requested_money) {
        if (gained_money < 0) gained_money = 0;
        user->SendMoneyFullReason(static_cast<ENUM_MONEY_FULL_REASON>(0),
                                  static_cast<unsigned long>(requested_money),
                                  static_cast<unsigned long>(gained_money));
    }
    if (war_field_.PickupItem(field_item) != 1) return 0;

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_WARROOM_GET_ITEM);
    packet.put_short(static_cast<int16_t>(unique_id));
    packet.put_short(user->get_unique_id());
    packet.put_int(gained_money);
    packet.finalize(true);
    SendToRoom(packet);
    return 1;
}
