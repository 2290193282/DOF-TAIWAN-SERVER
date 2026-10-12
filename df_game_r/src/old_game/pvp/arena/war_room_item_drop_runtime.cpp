#include "old_game/pvp/battlefield/war_room.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

void InitializeGoldDrop(int count, map_item& item) {
    item.routing_eligible_ = 0;
    item.item.item_category_ = 2;
    item.item.item_id_ = 0;
    item.item.avatar_select_ = 0;
    item.item.set_add_info(count);
    item.item.ResetItemAttr();
}

void CopyDropItem(const Inven_Item& source, int count, map_item& item) {
    item.routing_eligible_ = 0;
    item.item = source;
    item.item.set_add_info(source.isEquipableItemType() ? source.get_add_info()
                                                         : count);
}

}  // namespace

// IDA_EA: 0x86BF5FC
// IDA_NAME: _ZN7WarRoom14HandleItemDropEP5CUseriicii
// DEMANGLED: WarRoom::HandleItemDrop(CUser *,int,int,char,int,int)
int WarRoom::HandleItemDrop(CUser* user,
                            int pos_x,
                            int pos_y,
                            char item_space,
                            int slot,
                            int count) {
    if (GetState() != 3 && GetState() != 4) {
        return 19;
    }
    if (count < 0) {
        LogManager::logFormat(
            1,
            "WarRoom.cpp",
            "int WarRoom::HandleItemDrop(CUser*, int, int, char, int, int)",
            0xA1A,
            "user - accid[%s] %s : count<0",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            user->getCurCharacName());
        return 17;
    }
    if (user->CheckInTrade()) {
        return 19;
    }

    map_item dropped_item;
    Inven_Item source;
    switch (item_space) {
    case 2:
        source = user->getCurCharacCargoR()->get_cargo_slot(slot);
        break;
    case 3:
        source = user->getCurCharacInvenR()->GetInvenSlot(
            static_cast<INVEN_TYPE>(0), slot);
        break;
    case 0:
        if (slot != 0) {
            source = user->getCurCharacInvenR()->GetInvenSlot(
                static_cast<INVEN_TYPE>(1), slot);
        } else {
            InitializeGoldDrop(count, dropped_item);
        }
        break;
    default:
        break;
    }

    int sell_price = 0;
    if (!source.isEmpty()) {
        CItem* const item_data = G_CDataManager()->find_item(source.item_id_);
        if (!item_data) {
            LogManager::logFormat(
                1,
                "WarRoom.cpp",
                "int WarRoom::HandleItemDrop(CUser*, int, int, char, int, int)",
                0xA4C,
                "[WarRoom::HandleDropItem] Null Item %d",
                source.item_id_);
            return -1;
        }

        const int attach_type = item_data->GetAttachType();
        if (attach_type == 1 || attach_type == 2 || attach_type == 5 ||
            (attach_type == 3 && source.seal_or_state_ != 1)) {
            return 23;
        }
        if (source.isEquipableItemType()) {
            const int item_type =
                static_cast<const CEquipItem*>(item_data)->GetItemType();
            if (item_type <= oldsrv::config::kAvatarEquipmentTypeLast ||
                item_type == oldsrv::config::kEquipmentTypeTitle) {
                return 23;
            }
        }

        CopyDropItem(source, count, dropped_item);
        G_Store()->GetSellItemPrice(source,
                                    item_data,
                                    static_cast<short>(count),
                                    false,
                                    sell_price);
    }

    if (item_space == 2) {
        if (user->getCurCharacCargoW()->delete_item(
                slot, count, static_cast<eItemDelReason>(16)) != 1) {
            return 17;
        }
    } else if (item_space == 3) {
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(0),
                slot,
                1,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return 17;
        }
    } else if (item_space == 7) {
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(3),
                slot,
                1,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return 17;
        }
    } else if (item_space == 0 && slot == 0) {
        if (user->getCurCharacInvenW()->use_money(
                count, static_cast<eMoneySubReason>(6), true) != 1) {
            return 17;
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(23),
            user,
            static_cast<unsigned int>(count));
    } else if (item_space == 0) {
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                count,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return 17;
        }
    }

    if (!(item_space == 0 && slot == 0)) {
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(22),
            user,
            static_cast<unsigned int>(sell_price));
    }

    const int unique_id = war_field_.DropItem(dropped_item);
    if (unique_id == -1) {
        return 22;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_DROP_ITEM);
    packet.put_short(user->get_unique_id());
    packet.put_short(static_cast<short>(pos_x));
    packet.put_short(static_cast<short>(pos_y));
    packet.put_short(static_cast<short>(unique_id));
    packet.put_int(dropped_item.item.item_id_);
    packet.put_byte(dropped_item.item.GetItemAttr());
    packet.put_int(dropped_item.item.get_add_info());
    packet.put_short(dropped_item.item.avatar_select_);
    packet.put_int(dropped_item.item.extend_info_);
    packet.put_byte(dropped_item.item.amplify_option_.getAbilityType());
    packet.put_short(dropped_item.item.amplify_option_.getAbilityValue());
    packet.put_packet(dropped_item.item);
    packet.put_byte(dropped_item.routing_eligible_ == 0);
    packet.finalize(true);
    SendToRoom(packet);

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_DROP_ITEM);
    packet.put_byte(1);
    packet.put_byte(static_cast<unsigned char>(item_space));
    packet.put_short(static_cast<short>(slot));
    packet.put_int(count);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}
