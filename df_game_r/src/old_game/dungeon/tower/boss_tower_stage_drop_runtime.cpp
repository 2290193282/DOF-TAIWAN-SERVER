#include "old_game/pvp/battlefield/battle_field.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/statistics_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace WongWork {

namespace {

void InitializeGoldDrop(int count, map_item& item) {
    item.routing_eligible_ = 0;
    item.item.item_category_ = 2;
    item.item.item_id_ = 0;
    item.item.avatar_select_ = 0;
    item.item.set_add_info(count);
    item.item.ResetItemAttr();
}

void CopyDropItemFields(const Inven_Item& source,
                        int count,
                        map_item& item) {
    item.routing_eligible_ = 0;
    item.item.item_category_ = source.item_category_;
    item.item.item_id_ = source.item_id_;
    item.item.extend_info_ = source.extend_info_;
    item.item.globally_unique_id_ =
        source.globally_unique_id_;
    item.item.amplify_option_ = source.amplify_option_;
    item.item.set_add_info(
        source.isEquipableItemType() ? source.get_add_info() : count);
    item.item.SetItemAttr(static_cast<uint8_t>(source.GetItemAttr()));
    item.item.avatar_select_ = source.avatar_select_;
    item.item.seal_or_state_ = source.seal_or_state_;
}

}  // namespace

// IDA_EA: 0x814DE1A
// IDA_NAME: _ZN8WongWork10CBossStage8dropItemEP5CUserciiR8map_item
// DEMANGLED: WongWork::CBossStage::dropItem(CUser *,char,int,int,map_item &)
int CBossStage::dropItem(CUser* user,
                         char item_space,
                         int slot,
                         int count,
                         map_item& item) {
    Inven_Item source;
    if (item_space == 2) {
        source = user->getCurCharacCargoR()->get_cargo_slot(slot);
    } else if (item_space > 2) {
        if (item_space == 3) {
            source = user->getCurCharacInvenR()->GetInvenSlot(0, slot);
        } else {
            if (item_space != 7) {
                return 0;
            }
            source = user->getCurCharacInvenR()->GetInvenSlot(3, slot);
            if (source.item_category_ == 5) {
                return 0;
            }
        }
    } else {
        if (item_space != 0) {
            return 0;
        }
        if (slot != 0) {
            source = user->getCurCharacInvenR()->GetInvenSlot(1, slot);
        } else {
            InitializeGoldDrop(count, item);
        }
    }

    int sell_price = 0;
    if (!source.isEmpty()) {
        CItem* const item_data =
            G_CDataManager()->find_item(source.getKey());
        if (!item_data) {
            return 0;
        }

        const int attach_type = item_data->GetAttachType();
        if (attach_type == 1 || attach_type == 2 || attach_type == 5) {
            return 0;
        }
        if (attach_type == 3 && source.seal_or_state_ != 1) {
            return 0;
        }

        if (source.isEquipableItemType()) {
            const int item_type =
                static_cast<const CEquipItem*>(item_data)->GetItemType();
            if (item_type <= oldsrv::config::kAvatarEquipmentTypeLast ||
                item_type == oldsrv::config::kEquipmentTypeTitle) {
                return 0;
            }
        }

        CopyDropItemFields(source, count, item);
        G_Store()->GetSellItemPrice(
            source, item_data, static_cast<short>(count), false, sell_price);
    } else {
        InitializeGoldDrop(count, item);
    }

    int deleted = 0;
    if (item_space == 2) {
        deleted = user->getCurCharacCargoW()->delete_item(
            slot, count, static_cast<eItemDelReason>(16));
    } else if (item_space > 2) {
        if (item_space == 3) {
            deleted = user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(0),
                slot,
                1,
                static_cast<eItemDelReason>(6),
                true);
        } else {
            if (item_space != 7) {
                return 0;
            }
            deleted = user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(3),
                slot,
                1,
                static_cast<eItemDelReason>(6),
                true);
        }
    } else {
        if (item_space != 0) {
            return 0;
        }
        if (slot != 0) {
            deleted = user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                count,
                static_cast<eItemDelReason>(6),
                true);
        } else {
            deleted = user->getCurCharacInvenW()->use_money(
                count, static_cast<eMoneySubReason>(6), true);
        }
    }

    if (deleted != 1) {
        return 0;
    }
    if (item_space != 0 || slot != 0) {
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(0x16),
            user,
            static_cast<unsigned int>(sell_price));
    } else {
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(0x17),
            user,
            static_cast<unsigned int>(count));
    }
    return 1;
}

// IDA_EA: 0x814FF4A
// IDA_NAME: _ZN8WongWork10CBossStage14handleDropItemEP5CUseriicii
// DEMANGLED: WongWork::CBossStage::handleDropItem(CUser *,int,int,char,int,int)
int CBossStage::handleDropItem(CUser* user,
                               int item_id,
                               int item_count,
                               char rarity,
                               int x,
                               int y) {
    if (item_count < 0) {
        return 0;
    }

    PacketGuard packet;
    packet.clear();
    map_item item;
    if (dropItem(user, rarity, item_id, item_count, item) != 1) {
        packet.put_header(1, ENUM_CMDPACKET_DROP_ITEM);
        packet.put_byte(0);
        packet.put_byte(17);
        packet.put_byte(rarity);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const int unique_id = map_info_.next_drop_item_unique_id_++;
    if (unique_id == -1) {
        packet.put_header(1, ENUM_CMDPACKET_DROP_ITEM);
        packet.put_byte(0);
        packet.put_byte(17);
        packet.put_byte(rarity);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    item.unique_id_ = static_cast<uint32_t>(unique_id);
    map_info_.Add_Item(item);

    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_DROP_ITEM);
    packet.put_short(user->get_unique_id());
    packet.put_short(x);
    packet.put_short(y);
    packet.put_short(unique_id);
    packet.put_int(item.item.getKey());
    packet.put_byte(item.item.GetItemAttr());
    packet.put_int(item.item.get_add_info());
    packet.put_short(item.item.avatar_select_);
    packet.put_int(item.item.extend_info_);
    packet.put_byte(item.item.amplify_option_.getAbilityType());
    packet.put_short(item.item.amplify_option_.getAbilityValue());
    packet.put_packet(item.item);
    packet.put_byte(item.routing_eligible_ == 0);
    packet.finalize(true);
    party_->send_to_party(packet);

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_DROP_ITEM);
    packet.put_byte(1);
    packet.put_byte(rarity);
    packet.put_short(item_id);
    packet.put_int(item_count);
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

}  // namespace WongWork
