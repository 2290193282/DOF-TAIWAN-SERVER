#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/special_item_routing_manager_abi.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace WongWork {

// IDA_EA: 0x814E57A
// IDA_NAME: _ZN8WongWork10CBossStage17RoutingPickupItemEP5CUserjR8map_item
// DEMANGLED: WongWork::CBossStage::RoutingPickupItem(CUser *,unsigned int,map_item &)
int CBossStage::RoutingPickupItem(CUser* user,
                                  unsigned int unique_id,
                                  map_item& item) {
    CSpecialItemRoutingManager* const manager =
        G_CGameManager()->GetSpecialItemRoutingManager();
    const int started = manager->StartRouting(user, item.item, unique_id);
    if (!started) {
        return 1;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
    packet.put_byte(0);
    packet.put_byte(started);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x814F0E6
// IDA_NAME: _ZN8WongWork10CBossStage14GoldPickupItemEP5CUserjRK8map_item
// DEMANGLED: WongWork::CBossStage::GoldPickupItem(CUser *,unsigned int,map_item const&)
int CBossStage::GoldPickupItem(CUser* user,
                               unsigned int unique_id,
                               const map_item& item) {
    int live_member_count = 0;
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        if (member_alive_[member_slot]) {
            ++live_member_count;
        }
    }

    PacketGuard packet;
    packet.clear();
    if (live_member_count == 0) {
        packet.put_header(1, ENUM_CMDPACKET_GET_ITEM);
        packet.put_byte(0);
        packet.put_byte(21);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const int money = item.item.get_add_info();
    const int individual_money = money / live_member_count;
    const int picker_money = individual_money + money % live_member_count;

    packet.put_header(0, ENUM_NOTIPACKET_GET_ITEM);
    packet.put_short(static_cast<int>(unique_id));
    packet.put_short(user->get_unique_id());
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const member = party_->get_user(member_slot);
        if (member &&
            static_cast<unsigned char>(party_->checkValidUser(member_slot))) {
            packet.put_byte(1);
            const int requested_money =
                member == user ? picker_money : individual_money;
            int gained_money = member->getCurCharacInvenW()->gain_money(
                requested_money, static_cast<eMoneyAddReason>(4), true, 0);
            if (gained_money < requested_money) {
                if (gained_money < 0) {
                    gained_money = 0;
                }
                member->SendMoneyFullReason(
                    static_cast<ENUM_MONEY_FULL_REASON>(0),
                    static_cast<unsigned long>(requested_money),
                    static_cast<unsigned long>(gained_money));
            }
            packet.put_int(gained_money);
            packet.put_byte(1);
            packet.put_int(0);
            packet.put_int(0);
        } else {
            packet.put_byte(0);
            packet.put_int(0);
        }
    }
    for (int member_slot = 4; member_slot < 8; ++member_slot) {
        packet.put_byte(0);
        packet.put_int(0);
    }

    map_info_.items_.erase(static_cast<int>(unique_id));
    packet.finalize(true);
    party_->send_to_party(packet);
    return 1;
}

}  // namespace WongWork
