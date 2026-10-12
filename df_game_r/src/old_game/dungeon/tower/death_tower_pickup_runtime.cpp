#include "old_game/pvp/battlefield/battle_field.h"

#include "common/config.h"
#include <cstring>

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/statistc_proxy_abi.h"
#include "common/manager/statistics_runtime.h"
#include "common/manager/server_utility.h"
#include "old_game/dungeon/tower/death_tower_packet_handler_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"
#include "infra/network/packet/PacketEnum.h"

namespace WongWork {

namespace {

int GetPickupCategory(const CItem& item) {
    if (item.is_stackable()) {
        return item.vtable_->get_item_type(&item) == 9 ? 7 : 0;
    }
    if (item.vtable_->is_creature_item(&item)) {
        return 7;
    }
    return item.vtable_->is_avatar_item(&item) ? 1 : 0;
}

int InsertPickedItem(CUser* user, const Inven_Item& item) {
    if (item.isAvatarItemType()) {
        char ipg_no[24] = {0};
        CGenUniqueNo::genIPGNo(
            static_cast<CGenUniqueNo::ENUM_IPGNO_TYPE>(5),
            static_cast<unsigned int>(user->getCurCharacNo()), ipg_no);
        return user->addAvatarItem(static_cast<unsigned long>(item.getKey()),
                                   0,
                                   static_cast<char>(item.avatar_select_),
                                   ipg_no,
                                   static_cast<eAvatarItemAddReason>(16));
    }
    return user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(4), true, true);
}

void MakeGoldDrop(int count, map_item& item) {
    item.routing_eligible_ = 0;
    item.item.item_category_ = 2;
    item.item.item_id_ = 0;
    item.item.set_add_info(count);
    item.item.avatar_select_ = 0;
    item.item.ResetItemAttr();
}

void CopyDropItem(const Inven_Item& source, int count, map_item& item) {
    item.routing_eligible_ = 0;
    item.item.item_category_ = source.item_category_;
    item.item.item_id_ = source.item_id_;
    item.item.extend_info_ = source.extend_info_;
    item.item.globally_unique_id_ = source.globally_unique_id_;
    item.item.amplify_option_ = source.amplify_option_;
    item.item.set_add_info(source.isEquipableItemType() ? source.get_add_info()
                                                         : count);
    item.item.SetItemAttr(static_cast<uint8_t>(source.GetItemAttr()));
    item.item.avatar_select_ = source.avatar_select_;
    item.item.seal_or_state_ = source.seal_or_state_;
}

}  // namespace

// IDA_EA: 0x8461534
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage8peekItemEiR8map_item
// DEMANGLED: WongWork::CDeathTower::CStage::peekItem(int,map_item &)
int CDeathTower::CStage::peekItem(int unique_id, map_item& item) {
    std::map<int, map_item>::const_iterator found = map_info_.items_.find(unique_id);
    if (found == map_info_.items_.end()) {
        return 0;
    }
    std::memcpy(&item, &found->second, sizeof(item));
    return 1;
}

// IDA_EA: 0x84615E4
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage10pickupItemEiR8map_item
// DEMANGLED: WongWork::CDeathTower::CStage::pickupItem(int,map_item &)
int CDeathTower::CStage::pickupItem(int unique_id, map_item& item) {
    std::map<int, map_item>::iterator found = map_info_.items_.find(unique_id);
    if (found == map_info_.items_.end()) {
        return 0;
    }
    std::memcpy(&item, &found->second, sizeof(item));
    map_info_.items_.erase(found);
    return 1;
}

// IDA_EA: 0x8464036
// IDA_NAME: _ZN8WongWork11CDeathTower16handlePickupItemEP5CUseribc
// DEMANGLED: WongWork::CDeathTower::handlePickupItem(CUser *,int,bool,char)
int CDeathTower::handlePickupItem(CUser* user,
                                  int unique_id,
                                  bool client_flag,
                                  char client_rarity) {
    map_item dropped_item;
    CPacketHandler packet;

    if (stage_.peekItem(unique_id, dropped_item) != 1) {
        packet.makeCommonError(ENUM_CMDPACKET_GET_ITEM,
                               static_cast<ENUM_ERROR>(
                                   client_rarity <= 3 ? 21 : 81));
        user->Send(packet);
        return 0;
    }

    const int member_count = party_->get_member_count();
    if (member_count <= 0) {
        packet.makeCommonError(ENUM_CMDPACKET_GET_ITEM,
                               static_cast<ENUM_ERROR>(21));
        user->Send(packet);
        return 0;
    }

    if (dropped_item.item.item_id_ == 0) {
        if (stage_.pickupItem(unique_id, dropped_item) != 1) {
            return 0;
        }

        int earned_money[4] = {0, 0, 0, 0};
        int routed_money[4] = {0, 0, 0, 0};
        _pickupItemMoney(user, member_count, dropped_item, earned_money,
                         routed_money);
        packet.makePickupItemHeader(user->get_unique_id(),
                                    static_cast<unsigned short>(unique_id));
        packet.appendPickupMoney(earned_money, routed_money, party_);
        party_->send_to_party(packet);
        return 1;
    }

    CItem* const item_data = G_CDataManager()->find_item(
        dropped_item.item.item_id_);
    if (!item_data) {
        return 0;
    }

    char picked_by[4] = {0, 0, 0, 0};
    CUser* const winner = _pickupItem(user,
                                      picked_by,
                                      member_count,
                                      dropped_item,
                                      client_flag,
                                      client_rarity,
                                      item_data);
    if (!winner) {
        packet.makeCommonError(ENUM_CMDPACKET_GET_ITEM,
                               static_cast<ENUM_ERROR>(22));
        user->Send(packet);
        return 0;
    }

    const int inserted = InsertPickedItem(winner, dropped_item.item);
    if (inserted == -1) {
        packet.makeCommonError(ENUM_CMDPACKET_GET_ITEM,
                               static_cast<ENUM_ERROR>(4));
        user->Send(packet);
        return 0;
    }

    if (item_data->vtable_->is_random_option(item_data)) {
        statistc_proxy::add(winner,
                             "log_random_option",
                             "pick_cnt",
                             1u,
                             "%d,'%s'",
                             winner->get_charac_10_level_section(),
                             GlobalData::s_systemTime_.getCurDate());
    }

    if (stage_.pickupItem(unique_id, dropped_item) != 1) {
        return 0;
    }

    packet.makePickupItemHeader(user->get_unique_id(),
                                static_cast<unsigned short>(unique_id));
    packet.appendPickupItem(picked_by,
                            winner->get_unique_id(),
                            inserted,
                            GetPickupCategory(*item_data));
    party_->send_to_party(packet);
    return 1;
}

// IDA_EA: 0x846466E
// IDA_NAME: _ZN8WongWork11CDeathTower14handleDropItemEP5CUseriicii
// DEMANGLED: WongWork::CDeathTower::handleDropItem(CUser *,int,int,char,int,int)
int CDeathTower::handleDropItem(CUser* user,
                                int x,
                                int y,
                                char item_space,
                                int slot,
                                int count) {
    if (count < 0) {
        return 0;
    }

    map_item item;
    CPacketHandler packet;
    if (!_makeDropItem(user, item_space, slot, count, item)) {
        packet.makeDropItemError(ENUM_CMDPACKET_DROP_ITEM,
                                 static_cast<ENUM_ERROR>(17), item_space);
        user->Send(packet);
        return 0;
    }

    const int unique_id = stage_.dropItem(item);
    if (unique_id == -1) {
        packet.makeDropItemError(ENUM_CMDPACKET_DROP_ITEM,
                                 static_cast<ENUM_ERROR>(22), item_space);
        user->Send(packet);
        return 0;
    }

    packet.makeDropItemNotify(user->get_unique_id(),
                              x,
                              y,
                              static_cast<unsigned short>(unique_id),
                              item);
    party_->send_to_party(packet);

    packet.clear();
    packet.makeDropItem(item_space, slot, count);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x846638C
// IDA_NAME: _ZN8WongWork11CDeathTower13_makeDropItemEP5CUserciiR8map_item
// DEMANGLED: WongWork::CDeathTower::_makeDropItem(CUser *,char,int,int,map_item &)
bool CDeathTower::_makeDropItem(CUser* user,
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
                return false;
            }
            source = user->getCurCharacInvenR()->GetInvenSlot(3, slot);
            if (source.item_category_ == 5) {
                return false;
            }
        }
    } else {
        if (item_space != 0) {
            return false;
        }
        if (slot != 0) {
            source = user->getCurCharacInvenR()->GetInvenSlot(1, slot);
        } else {
            MakeGoldDrop(count, item);
        }
    }

    int sell_price = 0;
    if (source.item_id_ != 0) {
        CItem* const item_data = G_CDataManager()->find_item(source.item_id_);
        if (!item_data) {
            return false;
        }

        const int attach_type = item_data->GetAttachType();
        if (attach_type == 1 || attach_type == 2 || attach_type == 5 ||
            (attach_type == 3 && source.seal_or_state_ != 1) ||
            source.upgrade_separate_.IsTradeRestriction()) {
            return false;
        }

        if (source.isEquipableItemType()) {
            const int item_type = static_cast<const CEquipItem*>(item_data)->GetItemType();
            if (item_type <= oldsrv::config::kAvatarEquipmentTypeLast ||
                item_type == oldsrv::config::kEquipmentTypeTitle) {
                return false;
            }
        }

        CopyDropItem(source, count, item);
        G_Store()->GetSellItemPrice(source,
                                    item_data,
                                    static_cast<short>(count),
                                    false,
                                    sell_price);
    }

    if (item_space == 2) {
        return user->getCurCharacCargoW()->delete_item(
                   slot, count, static_cast<eItemDelReason>(16)) == 1;
    }

    if (item_space > 2) {
        if (item_space == 3) {
            if (user->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(0),
                    slot,
                    1,
                    static_cast<eItemDelReason>(6),
                    true) != 1) {
                return false;
            }
        } else {
            if (item_space != 7 ||
                user->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(3),
                    slot,
                    1,
                    static_cast<eItemDelReason>(6),
                    true) != 1) {
                return false;
            }
        }
    } else {
        if (item_space != 0) {
            return false;
        }
        if (slot == 0) {
            if (user->getCurCharacInvenW()->use_money(
                    count, static_cast<eMoneySubReason>(6), true) != 1) {
                return false;
            }
            GetInstanceValueStatistic()->AddValueStatistic(
                static_cast<VALUE_STATISTIC_FIELD>(23),
                user,
                static_cast<unsigned int>(count));
            return true;
        }
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                count,
                static_cast<eItemDelReason>(6),
                true) != 1) {
            return false;
        }
    }

    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(22),
        user,
        static_cast<unsigned int>(sell_price));
    return true;
}

// IDA_EA: 0x8466B02
// IDA_NAME: _ZN8WongWork11CDeathTower16_pickupItemMoneyEP5CUseriRK8map_itemPiS6_
// DEMANGLED: WongWork::CDeathTower::_pickupItemMoney(
//     CUser *,int,map_item const&,int *,int *)
bool CDeathTower::_pickupItemMoney(CUser* user,
                                   int member_count,
                                   const map_item& item,
                                   int* earned_money,
                                   int* routed_money) {
    const int total_money = item.item.get_add_info();
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party_->get_user(seat);
        if (!member || party_->checkValidUser(seat) != 1) {
            continue;
        }

        int requested = total_money / member_count;
        if (member == user) {
            requested += total_money % member_count;
        }
        int credited = requested;
        if (item.routing_eligible_) {
            routed_money[seat] = member->getCurCharacInvenR()
                                     ->GetCreatureMgrR()
                                     ->GetExtraGold(requested);
            credited *= 2;
        }

        earned_money[seat] = member->getCurCharacInvenW()->gain_money(
            credited, static_cast<eMoneyAddReason>(4), true, 0);
        if (item.routing_eligible_) {
            member->saveTaxMoneyForUpperMember(credited);
        }
        if (earned_money[seat] < credited) {
            if (earned_money[seat] < 0) {
                earned_money[seat] = 0;
            }
            member->SendMoneyFullReason(
                static_cast<ENUM_MONEY_FULL_REASON>(0),
                static_cast<unsigned long>(credited),
                static_cast<unsigned long>(earned_money[seat]));
        }
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(1),
            member,
            static_cast<unsigned int>(earned_money[seat]));
    }

    // The original loop leaves AL clear at its exit.  The caller only uses
    // its output arrays, but the return ABI remains the original false.
    return false;
}

// IDA_EA: 0x8466CFE
// IDA_NAME: _ZN8WongWork11CDeathTower11_pickupItemEP5CUserPciRK8map_itembcPK5CItem
// DEMANGLED: WongWork::CDeathTower::_pickupItem(
//     CUser *,char *,int,map_item const&,bool,char,CItem const*)
CUser* CDeathTower::_pickupItem(CUser* user,
                                char* picked_by,
                                int member_count,
                                const map_item& item,
                                bool client_flag,
                                char /*client_rarity*/,
                                const CItem* item_data) {
    if (member_count <= 1) {
        return user;
    }

    bool use_routing = item.routing_eligible_ == 1;
    if (item_data->get_rarity() > 3) {
        use_routing = client_flag;
    }
    if (item.routing_eligible_ && item_data->isEpicRoutingItem()) {
        use_routing = client_flag;
    }
    if (!use_routing || !item_data->check_item_routing()) {
        return user;
    }

    CUser* winner = 0;
    int routed_turn = play_data_.getLastRoutedTurn() + 1;
    if (routed_turn >= member_count) {
        routed_turn = 0;
    }

    int eligible_member = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party_->get_user(seat);
        if (!member || party_->checkValidUser(seat) != 1 ||
            play_data_.isMemberAlive(seat) != 1 ||
            member->getCurCharacInvenR()->tryInsertItemIntoInventory(
                item.item) == -1) {
            continue;
        }

        ++picked_by[seat];
        if (eligible_member == routed_turn) {
            picked_by[seat] += 40;
            play_data_.setLastRoutedTurn(eligible_member);
        }
        if (!item_data->is_stackable() &&
            item_data->vtable_->get_item_type(item_data) == 10 &&
            item_data->check_job_type(member->get_charac_job())) {
            picked_by[seat] += 30;
        }
        picked_by[seat] += get_rand_int(100 - picked_by[seat]);
        ++eligible_member;
    }

    int highest_pick = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party_->get_user(seat);
        if (!member || party_->checkValidUser(seat) != 1 ||
            play_data_.isMemberAlive(seat) != 1) {
            continue;
        }

        if (picked_by[seat] <= highest_pick) {
            if (picked_by[seat] == highest_pick && picked_by[seat] > 0) {
                --picked_by[seat];
            }
            continue;
        }
        highest_pick = picked_by[seat];
        winner = member;
    }
    return winner;
}

}  // namespace WongWork
