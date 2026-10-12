#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace WongWork {

namespace {

int GetPickupItemCategory(const CItem& item) {
    int category = 0;
    if (item.is_stackable()) {
        if (item.vtable_->get_item_type(&item) == 9) {
            category = 7;
        }
    } else if (item.vtable_->is_creature_item(&item)) {
        category = 7;
    } else if (item.vtable_->is_avatar_item(&item)) {
        category = 1;
    }
    return category;
}

}  // namespace

// IDA_EA: 0x814FA76
// IDA_NAME: _ZN8WongWork10CBossStage16handlePickupItemEP5CUseribRc
// DEMANGLED: WongWork::CBossStage::handlePickupItem(CUser *,int,bool,char &)
int CBossStage::handlePickupItem(CUser* user,
                                 int unique_id,
                                 bool /*client_flag*/,
                                 char& item_rarity) {
    std::map<int, map_item>::const_iterator found =
        map_info_.items_.find(unique_id);
    if (found == map_info_.items_.end()) {
        cMyTrace trace(
            "virtual bool WongWork::CBossStage::handlePickupItem(CUser*, int, bool, char&)",
            1390,
            5);
        trace("stageData_.item not find itemUID : %d", unique_id);

        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const map_item item(found->second);
    if (item.item.getKey() == 0) {
        return GoldPickupItem(user, static_cast<unsigned int>(unique_id), item) == 1;
    }

    CItem* const item_script = G_CDataManager()->find_item(
        item.item.getKey());
    if (!item_script) {
        cMyTrace trace(
            "virtual bool WongWork::CBossStage::handlePickupItem(CUser*, int, bool, char&)",
            1407,
            5);
        trace("item not find - item_id : %d, itemUID : %d",
              item.item.getKey(),
              unique_id);
        return 0;
    }

    const int pickup_category = GetPickupItemCategory(*item_script);
    (void)pickup_category;
    const bool is_group_pickup =
        party_->get_member_count() > 1 &&
        getLiveMemberCount() > 1 &&
        item.routing_eligible_;
    if (is_group_pickup &&
        (item_script->get_rarity() == 4 || item_script->IsRoutingItem())) {
        map_item routing_item(item);
        return RoutingPickupItem(user,
                                 static_cast<unsigned int>(unique_id),
                                 routing_item) == 1;
    }

    item_rarity = static_cast<char>(item_script->get_rarity());
    const int rarity = item_script->get_rarity();
    if (is_group_pickup && (rarity == 1 || rarity == 2 || rarity == 3)) {
        if (RandomPickupItem(user, static_cast<unsigned int>(unique_id), item) == 1) {
            return 1;
        }
        cMyTrace trace(
            "virtual bool WongWork::CBossStage::handlePickupItem(CUser*, int, bool, char&)",
            1452,
            5);
        trace("RandomPickupItem false");
        return 0;
    }

    if (NormalPickupItem(user, static_cast<unsigned int>(unique_id), item) == 1) {
        return 1;
    }
    cMyTrace trace(
        "virtual bool WongWork::CBossStage::handlePickupItem(CUser*, int, bool, char&)",
        1460,
        5);
    trace("NormalPickupItem false");
    return 0;
}

// IDA_EA: 0x814E684
// IDA_NAME: _ZN8WongWork10CBossStage16RandomPickupItemEP5CUserjRK8map_item
// DEMANGLED: WongWork::CBossStage::RandomPickupItem(CUser *,unsigned int,map_item const&)
int CBossStage::RandomPickupItem(CUser* user,
                                 unsigned int unique_id,
                                 const map_item& item) {
    uint8_t dice[4] = {0, 0, 0, 0};
    CUser* winner = 0;
    int highest_dice = 0;

    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const member = party_->get_user(member_slot);
        if (member && party_->checkValidUser(member_slot) == 1) {
            const int can_insert =
                member->getCurCharacInvenR()->tryInsertItemIntoInventory(item.item);
            if (can_insert != -1 && isMemberAlive(member_slot)) {
                dice[member_slot] = static_cast<uint8_t>(
                    get_rand_int(100) + 1);
                if (dice[member_slot] > highest_dice) {
                    highest_dice = dice[member_slot];
                    winner = member;
                }
            }
        }
    }

    if (!winner) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    CItem* const item_script = G_CDataManager()->find_item(
        item.item.getKey());
    if (!item_script) {
        cMyTrace trace(
            "bool WongWork::CBossStage::RandomPickupItem(CUser*, unsigned int, const map_item&)",
            924,
            5);
        trace("item not find : %d", item.item.getKey());
        return 0;
    }

    const int pickup_category = GetPickupItemCategory(*item_script);
    int inserted = -1;
    if (item.item.isAvatarItemType()) {
        char ipg_no[24] = {0};
        CGenUniqueNo::genIPGNo(
            static_cast<CGenUniqueNo::ENUM_IPGNO_TYPE>(5),
            static_cast<unsigned int>(winner->getCurCharacNo()),
            ipg_no);
        inserted = winner->addAvatarItem(
            static_cast<unsigned long>(item.item.getKey()),
            0,
            static_cast<char>(item.item.avatar_select_),
            ipg_no,
            static_cast<eAvatarItemAddReason>(16));
    } else {
        inserted = winner->getCurCharacInvenW()->insertItemIntoInventory(
            item.item,
            static_cast<eItemAddReason>(4),
            true,
            true);
    }

    if (inserted == -1) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        winner->Send(packet);
        return 0;
    }

    map_info_.items_.erase(static_cast<int>(unique_id));
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_GET_ITEM);
    packet.put_short(static_cast<int>(unique_id));
    packet.put_short(user->get_unique_id());
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        packet.put_byte(dice[member_slot]);
    }
    for (int member_slot = 4; member_slot < 8; ++member_slot) {
        packet.put_byte(0);
    }
    packet.put_short(winner->get_unique_id());
    packet.put_short(inserted);
    packet.put_byte(pickup_category);
    packet.finalize(true);
    party_->send_to_party(packet);
    return 1;
}

// IDA_EA: 0x814ECEC
// IDA_NAME: _ZN8WongWork10CBossStage16NormalPickupItemEP5CUserjRK8map_item
// DEMANGLED: WongWork::CBossStage::NormalPickupItem(CUser *,unsigned int,map_item const&)
int CBossStage::NormalPickupItem(CUser* user,
                                 unsigned int unique_id,
                                 const map_item& item) {
    if (!user) {
        return 0;
    }

    CItem* const item_script = G_CDataManager()->find_item(
        item.item.getKey());
    if (!item_script) {
        return 0;
    }

    const int pickup_category = GetPickupItemCategory(*item_script);
    int inserted = -1;
    if (item.item.isAvatarItemType()) {
        char ipg_no[24] = {0};
        CGenUniqueNo::genIPGNo(
            static_cast<CGenUniqueNo::ENUM_IPGNO_TYPE>(5),
            static_cast<unsigned int>(user->getCurCharacNo()),
            ipg_no);
        inserted = user->addAvatarItem(
            static_cast<unsigned long>(item.item.getKey()),
            0,
            static_cast<char>(item.item.avatar_select_),
            ipg_no,
            static_cast<eAvatarItemAddReason>(16));
    } else {
        inserted = user->getCurCharacInvenW()->insertItemIntoInventory(
            item.item,
            static_cast<eItemAddReason>(4),
            true,
            true);
    }

    if (inserted == -1) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    map_info_.items_.erase(static_cast<int>(unique_id));
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_GET_ITEM);
    packet.put_short(static_cast<int>(unique_id));
    packet.put_short(user->get_unique_id());
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        packet.put_byte(0);
    }
    for (int member_slot = 4; member_slot < 8; ++member_slot) {
        packet.put_byte(0);
    }
    packet.put_short(user->get_unique_id());
    packet.put_short(inserted);
    packet.put_byte(pickup_category);
    packet.finalize(true);
    party_->send_to_party(packet);
    return 1;
}

}  // namespace WongWork
