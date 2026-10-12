#include "old_game/player/core/cuser.h"

#include "old_data/PVF/arad/datamanager/arad_item.h"
#include "old_data/PVF/data/datamanager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/world_area_allocators.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

class CBuffGroup : public CBelong {
public:
    // IDA_EA: 0x8697C3E
    // IDA_NAME: _ZN10CBuffGroupC2EP5CUser
    // DEMANGLED: CBuffGroup::CBuffGroup(CUser *)
    explicit CBuffGroup(CUser* user);

    // IDA_EA: 0x8697C66
    // IDA_NAME: _ZN10CBuffGroup7IsRightEP5CUser
    // DEMANGLED: CBuffGroup::IsRight(CUser *)
    bool IsRight(CUser* user) {
        if (user->get_charac_level() > 69) {
            return false;
        }

        const int delta_x =
            static_cast<uint16_t>(user->get_posX()) -
            static_cast<uint16_t>(user_->get_posX());
        if (delta_x < -300 || delta_x > 300) {
            return false;
        }

        const int delta_y =
            static_cast<uint16_t>(user->get_posY()) -
            static_cast<uint16_t>(user_->get_posY());
        return delta_y >= -300 && delta_y <= 300;
    }

private:
    CUser* user_;
};

// IDA_EA: 0x8697C3E
// IDA_NAME: _ZN10CBuffGroupC2EP5CUser
// DEMANGLED: CBuffGroup::CBuffGroup(CUser *)
CBuffGroup::CBuffGroup(CUser* user)
    : CBelong(),
      user_(user) {}

namespace {

void SendStackableFailure(CUser* user,
                          int error,
                          int item_space,
                          int arg3,
                          int arg4) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_STACKABLE);
    packet.put_byte(0);
    packet.put_byte(error);
    packet.put_byte(item_space);
    packet.put_int(arg3);
    packet.put_int(arg4);
    packet.finalize(true);
    user->Send(packet);
}

void SendStackableAlchemistDenied(CUser* user,
                                  int item_space,
                                  int arg3,
                                  int arg4) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_STACKABLE);
    packet.put_byte(0);
    packet.put_byte(7);
    packet.put_byte(item_space);
    packet.put_int(arg3);
    packet.put_int(arg4);
    packet.finalize(true);
    user->Send(packet);
}

void SendStackableCooldownDenied(CUser* user,
                                 int item_space,
                                 int arg3,
                                 int arg4) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_STACKABLE);
    packet.put_byte(0);
    packet.put_byte(19);
    packet.put_byte(item_space);
    packet.put_int(arg3);
    packet.put_int(arg4);
    packet.finalize(true);
    user->Send(packet);
}

void ReduceFatigue(CUser* user, int amount) {
    int fatigue = user->getCurCharacFatigue() - amount;
    if (fatigue < 0) {
        fatigue = 0;
    }
    user->setCurCharacFatigue(fatigue);
    user->SendFatigue();
}

void BroadcastSecretShopEffect(CUser* user,
                               unsigned long item_index,
                               bool send_to_user) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 256);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(item_index));
    packet.put_int(1800);
    packet.finalize(true);
    if (send_to_user) {
        user->Send(packet);
    }
    G_GameWorld()->send_to_area(
        user->getCurCharacVill(), user->get_area(false), packet);
}

}  // namespace

// IDA_EA: 0x865DEFC
// IDA_NAME: _Z12SendLV70BuffP5CUserm
// DEMANGLED: SendLV70Buff(CUser *,unsigned long)
void SendLV70Buff(CUser* user, unsigned long item_index) {
    CBuffGroup group(user);
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 256);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(item_index));
    packet.put_int(3600);
    packet.finalize(true);
    G_GameWorld()->send_to_area(
        user->getCurCharacVill(), user->get_area(false), packet, &group);

    packet.clear();
    packet.put_header(0, 259);
    packet.put_short(user->get_unique_id());
    packet.put_int(static_cast<int>(item_index));
    packet.finalize(true);
    G_GameWorld()->send_to_area(
        user->getCurCharacVill(), user->get_area(false), packet);
}

namespace ARAD {

// IDA_EA: 0x8196ED7
// IDA_NAME: _ZN4ARAD9ARAD_ITEM27arad_secret_shop_event_itemEP5CUsermP14CStackableItem
// DEMANGLED: ARAD::ARAD_ITEM::arad_secret_shop_event_item(CUser *,unsigned long,CStackableItem *)
void ARAD_ITEM::arad_secret_shop_event_item(CUser* user,
                                            unsigned long item_index,
                                            CStackableItem* item) {
    const std::vector<CRabbitBox>& additional_items = item->additional_items();
    const unsigned long item_id = static_cast<unsigned long>(item->get_index());
    if (additional_items.empty() || item_id == 2670155ul) {
        return;
    }

    if (item_id == 2670186ul) {
        BroadcastSecretShopEffect(user, item_index, true);
        return;
    }

    for (std::vector<CRabbitBox>::const_iterator it = additional_items.begin();
         it != additional_items.end();
         ++it) {
        const CRabbitBox& additional_item = *it;
        CItem* const item_prototype = G_CDataManager()->find_item(
            additional_item.getItemIndex());
        if (!item_prototype) {
            continue;
        }

        Inven_Item inventory_item;
        inventory_item.item_id_ = additional_item.getItemIndex();
        inventory_item.set_add_info(additional_item.getItemCount());
        item_prototype->vtable_->make_item(item_prototype, inventory_item);

        const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
            inventory_item,
            static_cast<eItemAddReason>(13),
            true,
            false);
        if (slot == -1) {
            WongWork::CMailBoxHelper::ReqDBSendNewMail(
                user,
                inventory_item,
                0,
                static_cast<unsigned int>(user->getCurCharacNo()),
                true,
                0,
                0);
        } else if (user->SendUpdateItemList(
                       static_cast<CUser::eSendTarget>(1),
                       static_cast<ENUM_ITEMSPACE>(0),
                       slot) != 1) {
            return;
        }
    }

    BroadcastSecretShopEffect(user, item_index, false);
}

}  // namespace ARAD

// IDA_EA: 0x865E0AE
// IDA_NAME: _ZN5CUser13use_stackableEsiii
// DEMANGLED: CUser::use_stackable(short,int,int,int)
int CUser::use_stackable(short slot, int item_space, int arg3, int arg4) {
    int error = CheckInTrade() ? 19 : 0;
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(item_space));
    const Inven_Item inventory_item = getCurCharacInvenR()->GetInvenSlot(
        inventory_type, slot);
    const unsigned long item_index = inventory_item.getKey();
    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(item_index));
    if (!item || item->is_stackable() != 1) {
        error = 23;
    }

    if (!error &&
        expert_job::CAlchemist::CheckUsableStackableItem(this, item) != 1) {
        SendStackableAlchemistDenied(this, item_space, arg3, arg4);
        return 1;
    }

    if (!error && CheckCoolTimeItem(item_index)) {
        SendStackableCooldownDenied(this, item_space, arg3, arg4);
        return 1;
    }

    if (!error) {
        error = getCurCharacInvenW()->use_item(slot, 1, item_space);
    }
    if (!error) {
        Global::CBossTowerFunc::UseItem(this, inventory_item);
    }
    if (!error) {
        _processContinuousItem(
            static_cast<unsigned int>(item_index),
            static_cast<CStackableItem*>(item));
    }
    if (!error) {
        expert_job::CAlchemist::OnUseStackableItem(this, item_index, item);
    }

    if (!error) {
        switch (item_index) {
        case 7789ul:
            SendLV70Buff(this, 7796ul);
            break;
        case 7790ul:
            SendLV70Buff(this, 7797ul);
            break;
        case 7791ul:
            SendLV70Buff(this, 7798ul);
            break;
        case 7792ul:
            SendLV70Buff(this, 7799ul);
            break;
        case 7793ul:
            SendLV70Buff(this, 7800ul);
            break;
        case 7794ul:
            SendLV70Buff(this, 7801ul);
            break;
        case 7795ul:
            SendLV70Buff(this, 7802ul);
            break;
        case 7456ul:
            ReduceFatigue(this, 40);
            break;
        case 7518ul:
            ReduceFatigue(this, 10);
            break;
        case 7644ul:
            ReduceFatigue(this, 6);
            break;
        case 8079ul:
        case 2600026ul:
        case 2600413ul:
        case 690000094ul:
            ReduceFatigue(this, 30);
            break;
        case 8221ul:
            ReduceFatigue(this, 20);
            break;
        default:
            break;
        }
    }

    if (error) {
        SendStackableFailure(this, error, item_space, arg3, arg4);
        const Inven_Item logged_item = getCurCharacInvenR()->GetInvenSlot(1, slot);
        cMyTrace trace("bool CUser::use_stackable(short int, int, int, int)",
                       14517,
                       5);
        trace("CUser::use_stackable\tError:%d\tSlot_no:%d\tItem_no:%d",
              error,
              slot,
              logged_item.getKey());
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_STACKABLE);
    packet.put_byte(1);
    packet.put_short(slot);
    packet.put_byte(item_space);
    packet.put_int(arg3);
    packet.put_int(arg4);
    packet.finalize(true);
    Send(packet);
    ARAD::ARAD_ITEM::arad_secret_shop_event_item(
        this,
        item_index,
        static_cast<CStackableItem*>(item));
    return 1;
}
