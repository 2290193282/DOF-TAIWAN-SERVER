#include "old_game/economy/mail/mail_trade_runtime.h"
#include "common/config.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/player/core/cuser.h"

namespace private_store {

// IDA_EA: 0x85C9BB8
// IDA_NAME: _ZN13private_store13CPrivateStore14CheckValidItemE10INVEN_TYPEii
// DEMANGLED: private_store::CPrivateStore::CheckValidItem(INVEN_TYPE,int,int)
int CPrivateStore::CheckValidItem(INVEN_TYPE inventory_type,
                                  int inventory_slot,
                                  int item_count) {
    CInventory* const inventory = seller->getCurCharacInvenR();
    const int item_space = static_cast<int>(inventory_type);
    if (inventory->CheckValidSlot(item_space, inventory_slot) != 1) return 0;

    const Inven_Item item = inventory->GetInvenSlot(item_space, inventory_slot);
    if (!item.getKey()) return 0;

    if (item_space == 1) {
        if (!item.isEquipableItemType() && item.get_add_info() - item_count < 0) {
            return 0;
        }
    } else if (item_space == 2) {
        if (item.item_category_ != 8u) return 0;
        const int key = item.get_add_info();
        if (WongWork::CAvatarItemMgr::IsTempKey(key)) return 0;
        if (inventory->GetAvatarItemMgrR()->GetExpireDate(key)) return 0;
    }
    return 1;
}

// IDA_EA: 0x85C9CC2
// IDA_NAME: _ZN13private_store13CPrivateStore14CheckValidItemEii
// DEMANGLED: private_store::CPrivateStore::CheckValidItem(int,int)
int CPrivateStore::CheckValidItem(int cargo_slot, int item_count) {
    CCargo* const cargo = seller->getCurCharacCargoR();
    if (!cargo->IsOperatorable(cargo_slot)) return 0;

    const Inven_Item item = cargo->get_cargo_slot(cargo_slot);
    if (!item.getKey()) return 0;
    return item.isEquipableItemType() || item.get_add_info() - item_count >= 0;
}

// IDA_EA: 0x85C9D4E
// IDA_NAME: _ZN13private_store13CPrivateStore14CheckValidItemERNS_16PrivateStoreItemEi
// DEMANGLED: private_store::CPrivateStore::CheckValidItem(private_store::PrivateStoreItem &,int)
int CPrivateStore::CheckValidItem(PrivateStoreItem& store_item,
                                  int original_count) {
    CItem* const item = G_CDataManager()->find_item(store_item.item.getKey());
    if (!item) return 0;

    const int attach_type = item->GetAttachType();
    if (attach_type == 3) {
        if (store_item.item.GetItemAttr() != 1) return 0;
    } else if (attach_type != 0) {
        if (attach_type != 7 || !store_item.item.GetTradeLimitCount()) return 0;
    }

    if (IsCreatureItemType(store_item.item.item_category_)) return 0;
    if (item->vtable_->is_avatar_item(item)) return 0;
    if (item->is_stackable() && store_item.item.get_add_info() < original_count) {
        return 0;
    }

    switch (store_item.inventory_space) {
        case 0:
            return CheckValidItem(static_cast<INVEN_TYPE>(1),
                                  store_item.inventory_slot,
                                  store_item.item.get_add_info());
        case 1:
            return CheckValidItem(static_cast<INVEN_TYPE>(2),
                                  store_item.inventory_slot,
                                  store_item.item.get_add_info());
        case 2:
            return CheckValidItem(store_item.inventory_slot,
                                  store_item.item.get_add_info());
        case 3:
            return CheckValidItem(static_cast<INVEN_TYPE>(0),
                                  store_item.inventory_slot, 1);
        default:
            return 0;
    }
}

// IDA_EA: 0x85C9F22
// IDA_NAME: _ZN13private_store13CPrivateStore11GetItemInfoEP5CUserRNS_16PrivateStoreItemE
// DEMANGLED: private_store::CPrivateStore::GetItemInfo(CUser *,private_store::PrivateStoreItem &)
int CPrivateStore::GetItemInfo(CUser* user, PrivateStoreItem& store_item) {
    const int original_count = store_item.item.get_add_info();
    Inven_Item item;
    switch (store_item.inventory_space) {
        case 0:
            item = user->getCurCharacInvenR()->GetInvenSlot(
                1, store_item.inventory_slot);
            break;
        case 1:
            item = user->getCurCharacInvenR()->GetInvenSlot(
                2, store_item.inventory_slot);
            break;
        case 2:
            item = user->getCurCharacCargoR()->get_cargo_slot(
                store_item.inventory_slot);
            break;
        case 3:
            item = user->getCurCharacInvenR()->GetInvenSlot(
                0, store_item.inventory_slot);
            break;
        case 7:
            item = user->getCurCharacInvenR()->GetInvenSlot(
                3, store_item.inventory_slot);
            break;
        default:
            return 0;
    }
    store_item.item.setCopy(item);
    return store_item.item.getKey() && CheckValidItem(store_item, original_count);
}

// IDA_EA: 0x85C68B4
// IDA_NAME: _ZN13private_store13CPrivateStore10DeleteItemERNS_16PrivateStoreItemE
// DEMANGLED: private_store::CPrivateStore::DeleteItem(private_store::PrivateStoreItem &)
int CPrivateStore::DeleteItem(PrivateStoreItem& store_item) {
    cMyTrace trace("bool private_store::CPrivateStore::DeleteItem(private_store::PrivateStoreItem&)",
                   1239, 0);
    trace("PrivateStore del charac_name(%s)(%d), item_id(%d), add_info(%d), upgrade(%d), reseal_count(%d), sell_price(%d)",
          seller->getCurCharacName(), seller->getCurCharacNo(),
          store_item.item.getKey(), store_item.item.get_add_info(),
          store_item.item.GetUpgrade(), store_item.item.GetReSealCount(),
          store_item.price);

    switch (store_item.inventory_space) {
        case 0:
            if (seller->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(1), store_item.inventory_slot,
                    store_item.item.get_add_info(),
                    static_cast<eItemDelReason>(11), true) != 1) {
                error_handler.Do(17);
                return 0;
            }
            break;
        case 1:
            if (seller->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(2), store_item.inventory_slot, 1,
                    static_cast<eItemDelReason>(4), true) != 1) {
                error_handler.Do(17);
                return 0;
            }
            seller->getCurCharacInvenR()->SendAvatarEvent(
                54, store_item.item.get_add_info(), 3,
                static_cast<int>(store_item.inventory_slot) +
                    oldsrv::config::kAvatarInventorySlotBase,
                0);
            break;
        case 2:
            if (static_cast<CCargo*>(seller->getCurCharacCargoW())->delete_item(
                    store_item.inventory_slot, store_item.item.get_add_info(),
                    static_cast<eItemDelReason>(11)) != 1) {
                error_handler.Do(17);
                return 0;
            }
            break;
        case 3:
            if (seller->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(0), store_item.inventory_slot, 1,
                    static_cast<eItemDelReason>(11), true) != 1) {
                error_handler.Do(17);
                return 0;
            }
            seller->send_equip(store_item.inventory_slot);
            break;
    }
    return 1;
}

// IDA_EA: 0x85C7246
// IDA_NAME: _ZN13private_store13CPrivateStore9MoveItemsEv
// DEMANGLED: private_store::CPrivateStore::MoveItems(void)
int CPrivateStore::MoveItems() {
    for (int slot = 0; slot <= 13; ++slot) {
        PrivateStoreItem& store_item = items[slot];
        if (store_item.GetState() != 1) continue;

        const int original_count = store_item.item.get_add_info();
        if (!GetItemInfo(seller, store_item)) {
            ResetItems();
            error_handler.Do(17);
            return 0;
        }
        store_item.state = 2;
        if (store_item.item.item_category_ == 8u) {
            if (seller->getCurCharacInvenR()->GetAvatarItemMgrR()->GetExpireDate(
                    store_item.item.get_add_info())) {
                error_handler.Do(17);
                return 0;
            }
        } else {
            store_item.item.set_add_info(original_count);
        }
    }

    if (CheckTotalItemPrice() != 1) {
        ResetItems();
        error_handler.Do(17);
        return 0;
    }

    for (int slot = 0; slot <= 13; ++slot) {
        PrivateStoreItem& store_item = items[slot];
        if (store_item.GetState() != 2) continue;
        if (!DeleteItem(store_item)) {
            cMyTrace trace("bool private_store::CPrivateStore::MoveItems()", 1591,
                           5);
            trace("CPrivateStore [%s][%d]",
                  "bool private_store::CPrivateStore::MoveItems()", 1591);
            ResetItems();
            error_handler.Do(17);
            return 0;
        }
        store_item.state = 3;
    }
    return 1;
}

}  // namespace private_store
