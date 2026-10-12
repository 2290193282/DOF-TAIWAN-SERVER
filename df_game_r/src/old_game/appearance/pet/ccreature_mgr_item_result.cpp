#include "old_game/appearance/pet/creature.h"

#include "common/manager/os_api.h"
#include "infra/log/format/trace.h"
#include "old_data/db/creature/creature_item_payload.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

#include <stddef.h>

namespace user_creature {

namespace {

Inven_Item_Compatible MakeCompatibleItem(const SIG_CREATURE_ITEM* signal) {
    Inven_Item_Compatible compatible;
    compatible.seal_or_state_ = signal->inven_item_copy.item_flag;
    compatible.item_category_ = signal->inven_item_copy.item_type;
    compatible.item_id_ = static_cast<int32_t>(signal->inven_item_copy.item_id);
    compatible.item_attr_ = signal->inven_item_copy.item_attr;
    compatible.add_info_ = signal->inven_item_copy.add_info;
    compatible.avatar_select_ = signal->inven_item_copy.endurance;
    compatible.extend_info_ = signal->inven_item_copy.expire_date;
    compatible.amplify_option_ = signal->inven_item_copy.amplify_option;
    compatible.globally_unique_id_ = signal->inven_item_copy.identifier;
    return compatible;
}

static_assert(offsetof(SIG_CREATURE_ITEM, detail) == 0x2Cu,
              "creature result detail must begin at IDA +0x2c");
static_assert(sizeof(SIG_CREATURE_ITEM_DETAIL_INFO) == 0x38u,
              "creature result detail must retain its 14-dword ABI");

}  // namespace

// IDA_EA: 0x833B38C
// IDA_NAME: _ZN13user_creature12CCreatureMgr24InsertCreatureItemResultEP17SIG_CREATURE_ITEM
// DEMANGLED: user_creature::CCreatureMgr::InsertCreatureItemResult(SIG_CREATURE_ITEM *)
int CCreatureMgr::InsertCreatureItemResult(SIG_CREATURE_ITEM* signal) {
    Inven_Item item;
    Inven_Item_Compatible compatible = MakeCompatibleItem(signal);
    compatible.makeInvenItem(item);

    if (LoadCreatureItem(&item, &signal->detail) != 1) {
        return 0;
    }
    if (signal->update_kind == 6u) {
        SendCreatureItemInfo(signal);
    }

    owner_->set_cera_item_order_no(signal->detail.cera_order_no);
    CInventory* const inventory = owner_->getCurCharacInvenR();
    inventory->WriteCreatureLog(
        static_cast<INVEN_TYPE>(3),
        static_cast<int>(signal->inven_item_copy.item_id),
        static_cast<int>(signal->detail.ui_id),
        1,
        static_cast<int>(signal->inven_item_copy.item_type),
        static_cast<eItemAddReason>(
            static_cast<int>(signal->detail.creature_flag)));
    owner_->set_cera_item_order_no(0);
    return 1;
}

// IDA_EA: 0x833B498
// IDA_NAME: _ZN13user_creature12CCreatureMgr20SendCreatureItemInfoEP17SIG_CREATURE_ITEM
// DEMANGLED: user_creature::CCreatureMgr::SendCreatureItemInfo(SIG_CREATURE_ITEM *)
int CCreatureMgr::SendCreatureItemInfo(SIG_CREATURE_ITEM* signal) {
    CInventory* const inventory = owner_->getCurCharacInvenW();
    Inven_Item* const inventory_item =
        inventory->GetInvenRef(3, signal->detail.slot);
    if (inventory_item && inventory_item->item_id_ != 0) {
        CPacketRespondent respondent(owner_, 0, 0);
        respondent.MakeNotipacketCreatureItemList(&creature_items_,
                                                  signal->detail.slot);
        respondent.Send();
        owner_->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                   static_cast<ENUM_ITEMSPACE>(7),
                                   signal->detail.slot);
    }
    return 1;
}

// IDA_EA: 0x833B592
// IDA_NAME: _ZN13user_creature12CCreatureMgr24UpdateCreatureItemResultEP17SIG_CREATURE_ITEM
// DEMANGLED: user_creature::CCreatureMgr::UpdateCreatureItemResult(SIG_CREATURE_ITEM *)
int CCreatureMgr::UpdateCreatureItemResult(SIG_CREATURE_ITEM* signal) {
    Inven_Item item;
    Inven_Item_Compatible compatible = MakeCompatibleItem(signal);
    compatible.makeInvenItem(item);
    if (LoadCreatureItem(&item, &signal->detail) != 1) {
        return 0;
    }
    if (signal->update_kind == 4u) {
        SendCreatureItemInfo(signal);
    }
    return 1;
}

// IDA_EA: 0x833B68C
// IDA_NAME: _ZN13user_creature12CCreatureMgr16LoadCreatureItemEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFO
// DEMANGLED: user_creature::CCreatureMgr::LoadCreatureItem(Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *)
int CCreatureMgr::LoadCreatureItem(Inven_Item* item,
                                   SIG_CREATURE_ITEM_DETAIL_INFO* detail) {
    CCreatureItem* const creature_item =
        GetInstanceCreatureItemFactory()->AcquireCreatureItem(
            static_cast<CREATURE_ITME_TYPE>(
                static_cast<int8_t>(detail->creature_type)));
    if (!creature_item) {
        return 0;
    }

    if (temporary_expire_time_ != -1) {
        detail->expiration_value = temporary_expire_time_ == 0
            ? temporary_expire_time_
            : temporary_expire_time_ + OS_API::GetDateTimeTick();
        setTempExpireTime(-1);
    }
    UnRegistTempPeriod(detail->ui_id);

    if (creature_item->LoadCreatureItem(this, item, detail, owner_) != 1) {
        cMyTrace trace(
            "bool user_creature::CCreatureMgr::LoadCreatureItem(Inven_Item*, SIG_CREATURE_ITEM_DETAIL_INFO*)",
            3382,
            5);
        trace("LoadCreatureItem() failed %d,%d,%d\n",
              owner_->get_unique_id(),
              item->item_id_,
              item->add_info_);
        GetInstanceCreatureItemFactory()->FreeCreatureItem(creature_item);
        return 0;
    }

    if (detail->state != 1u && detail->creature_type == 1u) {
        save_state_ = detail->no_charge;
    }
    return 1;
}

}  // namespace user_creature
