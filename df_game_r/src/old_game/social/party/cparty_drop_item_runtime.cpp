#include "old_game/social/party/party.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
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

void CopyDropItemFields(const Inven_Item& source, int count, map_item& item) {
    item.routing_eligible_ = 0;
    item.item = source;
    item.item.set_add_info(source.isEquipableItemType() ? source.get_add_info()
                                                         : count);
}

}  // namespace

// IDA_EA: 0x85A63F4
// IDA_NAME: _ZN6CParty9drop_itemEP5CUseriicii
// DEMANGLED: CParty::drop_item(CUser *,int,int,char,int,int)
int CParty::drop_item(CUser* user,
                      int pos_x,
                      int pos_y,
                      char item_space,
                      int slot,
                      int count) {
    if (count < 0) {
        LogManager::logFormat(1,
                              "party.cpp",
                              "int CParty::drop_item(CUser*, int, int, char, int, int)",
                              0x1a0c,
                              "user - accid[%u] %s : count<0",
                              static_cast<unsigned int>(user->get_acc_id()),
                              user->getCurCharacName());
        return 17;
    }
    if (user->CheckInTrade()) {
        return 19;
    }

    map_item map_drop;
    Inven_Item source;
    if (item_space == 2) {
        source = user->getCurCharacCargoR()->get_cargo_slot(slot);
    } else if (item_space == 3) {
        source = user->getCurCharacInvenR()->GetInvenSlot(0, slot);
    } else if (item_space == 7) {
        source = user->getCurCharacInvenR()->GetInvenSlot(3, slot);
        if (source.item_category_ == 5) {
            return 23;
        }
    } else if (item_space == 0 && slot) {
        source = user->getCurCharacInvenR()->GetInvenSlot(1, slot);
    } else if (item_space == 0) {
        InitializeGoldDrop(count, map_drop);
    }

    int sell_price = 0;
    if (!source.isEmpty()) {
        CItem* const item_data = G_CDataManager()->find_item(source.getKey());
        if (!item_data) {
            LogManager::logFormat(1,
                                  "party.cpp",
                                  "int CParty::drop_item(CUser*, int, int, char, int, int)",
                                  0x1a48,
                                  "[CParty::drop_item] Null Item");
            return -1;
        }

        const int attach_type = item_data->GetAttachType();
        if (attach_type == 1 || attach_type == 2 || attach_type == 5 ||
            (attach_type == 3 && source.seal_or_state_ != 1) ||
            source.upgrade_separate_.IsTradeRestriction()) {
            return 23;
        }
        if (item_data->get_rarity() > 2) {
            return 13;
        }
        if (source.isEquipableItemType()) {
            const int item_type =
                static_cast<const CEquipItem*>(item_data)->GetItemType();
            if (item_type <= oldsrv::config::kAvatarEquipmentTypeLast ||
                item_type == oldsrv::config::kEquipmentTypeTitle) {
                return 23;
            }
        }

        CopyDropItemFields(source, count, map_drop);
        G_Store()->GetSellItemPrice(source,
                                    item_data,
                                    static_cast<short>(count),
                                    false,
                                    sell_price);
    }

    if (item_space == 2) {
        if (user->getCurCharacCargoW()->delete_item(
                slot, count, static_cast<eItemDelReason>(6)) != 1) {
            return 17;
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(22), user, sell_price);
    } else if (item_space == 3) {
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(0),
                slot,
                1,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return 17;
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(22), user, sell_price);
    } else if (item_space == 7) {
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(3),
                slot,
                1,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return 17;
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(22), user, sell_price);
    } else if (item_space == 0 && slot == 0) {
        if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                user, RestrictCategory::AuthMobile, 2)) {
            return 209;
        }
        if (user->getCurCharacInvenW()->use_money(
                count, static_cast<eMoneySubReason>(6), true) != 1) {
            return 17;
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(23), user, count);
    } else if (item_space == 0) {
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                count,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return 17;
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(22), user, sell_price);
    }

    const int unique_id = battle_field_.drop_item(map_drop);
    if (unique_id == -1) {
        return 22;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_DROP_ITEM);
    packet.put_short(user->get_unique_id());
    packet.put_short(static_cast<short>(pos_x));
    packet.put_short(static_cast<short>(pos_y));
    packet.put_short(static_cast<short>(unique_id));
    packet.put_int(map_drop.item.getKey());
    packet.put_byte(map_drop.item.GetItemAttr());
    packet.put_int(map_drop.item.get_add_info());
    packet.put_short(map_drop.item.avatar_select_);
    packet.put_int(map_drop.item.extend_info_);
    packet.put_byte(map_drop.item.amplify_option_.getAbilityType());
    packet.put_short(map_drop.item.amplify_option_.getAbilityValue());
    packet.put_packet(map_drop.item);
    packet.put_byte(map_drop.routing_eligible_ == 0);
    packet.finalize(true);
    send_to_party(packet);

    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_DROP_ITEM);
    packet.put_byte(1);
    packet.put_byte(item_space);
    packet.put_short(static_cast<short>(slot));
    packet.put_int(count);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

int CParty::drop_item_generated(CUser* user,
                                unsigned int item_id,
                                int pos_x,
                                int pos_y) {
    if (!user) {
        return -1;
    }
    if (user->CheckInTrade()) {
        return 19;
    }

    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(item_id));
    if (!item) {
        return -1;
    }

    map_item map_drop;
    Inven_Item prototype;
    item->vtable_->make_item(item, prototype);
    if (item_id != 0u) {
        if (prototype.item_category_ == 1u) {
            const int equipment_type =
                static_cast<const CEquipItem*>(item)->GetItemType();
            if (equipment_type == oldsrv::config::kEquipmentTypeTitle ||
                equipment_type <=
                    oldsrv::config::kAvatarEquipmentTypeLast) {
                return 23;
            }
        }

        // 0x85A7469..0x85A754A copies this exact subset into a fresh
        // map_item.  In particular, the first six Chronicle bytes and the
        // ReservedCapacity tail keep map_item's constructor values rather
        // than inheriting prototype data.
        map_drop.routing_eligible_ = 0;
        map_drop.item.item_category_ = prototype.item_category_;
        map_drop.item.item_id_ = static_cast<int32_t>(item_id);
        map_drop.item.set_add_info(1);
        map_drop.item.SetItemAttr(prototype.GetItemAttr());
        map_drop.item.avatar_select_ = prototype.avatar_select_;
        map_drop.item.seal_or_state_ = prototype.seal_or_state_;
        map_drop.item.extend_info_ = prototype.extend_info_;
        map_drop.item.amplify_option_ = prototype.amplify_option_;
        map_drop.item.globally_unique_id_ = prototype.globally_unique_id_;
        map_drop.item.chronicle_random_option_.random_option_ =
            prototype.chronicle_random_option_.random_option_;
        map_drop.item.upgrade_separate_.SetUpgradeSeparate(
            static_cast<uint8_t>(
                prototype.upgrade_separate_.GetUpgradeSeparate()));
    }

    const int unique_id = battle_field_.drop_item(map_drop);
    if (unique_id == -1) {
        return 22;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_DROP_ITEM);
    packet.put_short(user->get_unique_id());
    packet.put_short(pos_x);
    packet.put_short(pos_y);
    packet.put_short(unique_id);
    packet.put_int(map_drop.item.item_id_);
    packet.put_byte(map_drop.item.GetItemAttr());
    packet.put_int(map_drop.item.get_add_info());
    packet.put_short(map_drop.item.avatar_select_);
    packet.put_int(map_drop.item.extend_info_);
    packet.put_byte(map_drop.item.amplify_option_.getAbilityType());
    packet.put_short(map_drop.item.amplify_option_.getAbilityValue());
    packet.put_packet(map_drop.item);
    packet.put_byte(map_drop.routing_eligible_ == 0);
    packet.finalize(true);
    send_to_party(packet);
    return 0;
}

// IDA_EA: 0x85A73A6
// IDA_NAME: _ZN6CParty12drop_item_gmEP5CUserj
// DEMANGLED: CParty::drop_item_gm(CUser *,unsigned int)
int CParty::drop_item_gm(CUser* user, unsigned int item_id) {
    return drop_item_generated(user, item_id, 300, 240);
}
